#include "PublicAPI.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <array>
#include <bit>
#include <cerrno>
#include <cstring>
#include <random>

/*
    TODO:
        - create function that populates conn->in buffer, sets inSize and inReceived integers
        - create function that populates conn->out buffer
*/

namespace {
    std::array<std::byte, MAX_RESPONSE_LEN> createResBuf(API_STATUS_CODE status, std::span<std::byte> data) {
        APIResponse hdr;
        hdr.payloadSize = data.size();
        switch (status) {
            case API_STATUS_CODE::SUCCESS:
                hdr.message = "Message processed successfuly";
                hdr.code = 200;
                break;
            case API_STATUS_CODE::SYSTEM_ERROR:
                hdr.message = "Internal error";
                hdr.code = 500;
                break;
            case API_STATUS_CODE::BAD_MESSAGE_LEN:
                hdr.message = "Invalid message length";
                hdr.code = 400;
                break;
        }
        assert(hdr.message.size() <= HDR_MESSAGE_SIZE);
        hdr.message.resize(HDR_MESSAGE_SIZE, '\0');

        uint32_t codeNetEnd = htonl(hdr.code);
        auto codeBytes = std::bit_cast<std::array<std::byte, 4> >(codeNetEnd);

        std::array<std::byte, MAX_RESPONSE_LEN> buf;
        std::copy(codeBytes.begin(), codeBytes.end(), buf.begin());
        std::memcpy(buf.data() + 4, hdr.message.data(), HDR_MESSAGE_SIZE);
        std::copy(data.begin(), data.begin() + data.size(), buf.begin() + HDR_SIZE);

        return buf;
    }
}  // namespace

userId_t PublicAPI::generateUserId() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    // 42 bits for timestamp, 22 bits for random
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist(0, (1ull << 22) - 1);

    uint64_t timestamp = static_cast<uint64_t>(ms) & 0x3ffffffffff;  // 42 bits
    uint64_t random = dist(gen);

    userId_t id = (timestamp << 22) | random;

    if (uid2fd_.find(id) != uid2fd_.end())
        return generateUserId();

    return id;
}

API_STATUS_CODE PublicAPI::handleInBuf(int fd) {
    auto& conn = conns_.at(fd);
    auto& buf = conn->in;
    auto& received = conn->inReceived;

    while (received < MAX_BYTES_PER_HANDLE) {
        auto n = ::read(fd, buf.data() + received, MAX_MESSAGE_LEN);

        if (n == 0)
            break;

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            perror("[handle_read_sck]: read2");
            return API_STATUS_CODE::SYSTEM_ERROR;
        }

        received += n;
    }

    return API_STATUS_CODE::SUCCESS;
}

API_STATUS_CODE PublicAPI::handleOutBuf(int fd) {
    auto& conn = conns_[fd];
    auto& buf = conn->out;
    auto& sent = conn->outSent;
    auto bufSize = conn->outSize;

    auto toSend = [bufSize, sent] { return bufSize - sent; };
    auto bufPtr = [&buf, sent] { return buf.data() + sent; };

    if (!epollManager_.setWriteable(fd, conn->epollEvents))
        return API_STATUS_CODE::SYSTEM_ERROR;

    auto n = ::send(fd, bufPtr(), toSend(), 0);
    if (n <= 0) {
        perror("[sendRes]: send1");
        return API_STATUS_CODE::SYSTEM_ERROR;
    }
    sent += n;

    while (toSend() > 0 && n > 0) {
        n = ::send(fd, bufPtr(), toSend(), 0);

        if (n == 0)
            break;

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            perror("[sendRes]: send2");
            return API_STATUS_CODE::SYSTEM_ERROR;
        }

        sent += n;
    }

    if (!toSend()) {
        conn->resetOut();
        if (!epollManager_.unsetWriteable(fd, conn->epollEvents))
            return API_STATUS_CODE::SYSTEM_ERROR;
    }

    return API_STATUS_CODE::SUCCESS;
}

API_STATUS_CODE PublicAPI::acceptNewListener() {
    Socket acceptedSck{listenSocket_.fd};

    if (!epollManager_.add(acceptedSck.fd)) {
        return API_STATUS_CODE::SYSTEM_ERROR;
    }

    // create a user with this socket

    return API_STATUS_CODE::SUCCESS;
}

PublicAPI::PublicAPI() {
    listenSocket_.bind(INADDR_ANY, 8000);
    listenSocket_.listen();
}

// TODO: replace throws with responses to the sender
void PublicAPI::run() {
    std::array<epoll_event, MAX_EVENTS> events;
    while (true) {
        int nfds = epollManager_.getEvents(events);

        for (int i = 0; i < nfds; ++i) {
            auto event = events[i];
            int incfd = event.data.fd;

            if (incfd == listenSocket_.fd) {
                API_STATUS_CODE status = acceptNewListener();
                if (status != API_STATUS_CODE::SUCCESS) {
                    // TODO: prepare an error message to actually send
                    outgoings_.push(incfd);
                }
            }
            else
                incomings_.push(incfd);
        }

        while (!incomings_.empty()) {
            auto topfd = incomings_.front();
            incomings_.pop();

            handleInBuf(topfd);
        }

        while (!outgoings_.empty()) {
            auto topfd = outgoings_.front();
            outgoings_.pop();

            handleOutBuf(topfd);
        }
    }
}
