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
        - save send buffer in the conn->out
        - if there is an EWOULDBLCK or EAGAIN error, do not remove EPOLLOUT flag and process in another run
        - make this conceptually similar to the reading
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
        assert(hdr.message.size() <= MAX_HDR_MESSAGE_SIZE);
        hdr.message.resize(HDR_MESSAGE_SIZE, '\0');

        uint32_t codeNetEnd = htonl(hdr.code);
        auto codeBytes = std::bit_cast<std::array<std::byte, 4>>(codeNetEnd);

        std::array<std::byte, MAX_RESPONSE_LEN> buf;
        std::copy(codeBytes.begin(), codeBytes.end(), buf.begin());
        std::memcpy(buf.data() + 4, hdr.message.data(), HDR_MESSAGE_SIZE);
        std::copy(data.begin(), data.begin() + data.size(), buf.begin() + HDR_SIZE);

        return buf;
    }

}  // namespace

int PublicAPI::openSck() {
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1) {
        perror("[open_sck]: socket");
        return -1;
    }

    int so_reuseaddr_val = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr_val, sizeof(so_reuseaddr_val)) == -1) {
        perror("[open_sck]: setsockopt");
        return -1;
    }

    return fd;
}

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

void PublicAPI::unsetEpollWriteable(int fd) {
    auto& conn = conns_[fd];
    if (!(conn->epollEvents & EPOLLOUT))
        return;

    conn->epollEvents &= ~EPOLLOUT;
    epoll_event ev{.events = conn->epollEvents, .data = {.fd = fd}};

    if (::epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev) == -1)
        perror("[sendRes]: epoll_ctl1");
}

void PublicAPI::setEpollWriteable(int fd) {
    auto& conn = conns_[fd];
    if (conn->epollEvents & EPOLLOUT)
        return;

    conn->epollEvents |= EPOLLOUT;
    epoll_event ev{.events = conn->epollEvents, .data = {.fd = fd}};

    if (::epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev) == -1)
        perror("[sendRes]: epoll_ctl1");
}

API_STATUS_CODE PublicAPI::bindSck(int fd, int port, in_addr_t ipaddr) {
    sockaddr_in addr{.sin_family = AF_INET, .sin_port = htons(port), .sin_addr = {.s_addr = ipaddr}};

    if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("[bind_sck]: bind");
        return API_STATUS_CODE::SYSTEM_ERROR;
    }

    return API_STATUS_CODE::SUCCESS;
}

// TODO: bad function
API_STATUS_CODE PublicAPI::epollAdd(int fd) {
    epoll_event ev{.events = EPOLLIN, .data = {.fd = fd}};
    if (::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("[epoll_add]: epoll_ctl");
        return API_STATUS_CODE::SYSTEM_ERROR;
    }

    return API_STATUS_CODE::SUCCESS;
}

void PublicAPI::connInit(int fd, uint32_t events) {
    auto conn = std::make_unique<Conn>();
    auto userId = generateUserId();

    conn->fd = fd;
    conn->holderId = userId;
    conn->epollEvents = events;
    uid2fd_[userId] = fd;
    messageQueues_[userId] = std::make_unique<MessageQueue_t>(MESSAGE_QUEUE_SIZE);
}

API_STATUS_CODE PublicAPI::acceptSck() {
    sockaddr_in local{};
    socklen_t addrlen = sizeof(local);

    int connFd = ::accept(serverSockFd_, (sockaddr*)&local, &addrlen);
    if (connFd == -1) {
        perror("[accept_sck]: accept");
        return API_STATUS_CODE::SYSTEM_ERROR;
    }

    // Make non-blocking
    ::fcntl(connFd, F_SETFL, ::fcntl(connFd, F_GETFL, 0) | O_NONBLOCK);

    uint32_t events = EPOLLIN;
    epoll_event ev{.events = events, .data = {.fd = connFd}};
    if (::epoll_ctl(epollfd_, EPOLL_CTL_ADD, connFd, &ev) == -1) {
        perror("[accept_sck]: epoll_ctl");
        return API_STATUS_CODE::SYSTEM_ERROR;
    }

    connInit(connFd, events);
    return API_STATUS_CODE::SUCCESS;
}

API_STATUS_CODE PublicAPI::handleReadSck(int fd) {
    auto& conn = conns_.at(fd);
    auto& buf = conn->in;
    auto totalRead = 0u;
    auto received = 0u;

    while (totalRead < MAX_BYTES_PER_HANDLE) {
        auto n = ::read(fd, buf.data() + totalRead, 4);

        if (n == 0)
            return API_STATUS_CODE::SUCCESS;
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return API_STATUS_CODE::SUCCESS;

            perror("[handle_read_sck]: read1");
            return API_STATUS_CODE::SYSTEM_ERROR;
        }

        received += n;
        totalRead += n;

        const int32_t mesLen = std::bit_cast<int32_t>(std::array<std::byte, 4>{buf[0], buf[1], buf[2], buf[3]});
        if (mesLen <= 0 || mesLen > MAX_MESSAGE_LEN)
            return API_STATUS_CODE::BAD_MESSAGE_LEN;

        while (totalRead < MAX_BYTES_PER_HANDLE && mesLen < received) {
            n = ::read(fd, buf.data() + totalRead, MAX_MESSAGE_LEN);
            if (n == 0)
                break;
            if (n == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;

                perror("[handle_read_sck]: read2");
                return API_STATUS_CODE::SYSTEM_ERROR;
            }

            received += n;
            totalRead += n;
        }

        if (mesLen <= received) {
            auto& queue = messageQueues_[conn->holderId];
            std::array<std::byte, MAX_MESSAGE_LEN> message;

            std::copy(buf.begin(), buf.begin() + mesLen, message.begin());
            queue->push(buf);
            received -= mesLen;
        }
    }

    return API_STATUS_CODE::SUCCESS;
}

/*
    responses to write to the client must be prepared and set in the conn->out buffer
    TODO: implement function that prepares the message to send.
*/
void PublicAPI::sendRes(int fd) {
    auto& conn = conns_[fd];
    auto& buf = conn->out;
    auto& sent = conn->outSent;
    auto bufSize = conn->outSize;

    auto toSend = [bufSize, sent] { return bufSize - sent; };
    auto bufPtr = [&buf, sent] { return buf.data() + sent; };

    setEpollWriteable(fd);
    auto n = ::send(fd, bufPtr(), toSend(), 0);
    if (n <= 0) {
        perror("[sendRes]: send1");
        return;
    }
    sent += n;

    while (toSend() > 0 && n > 0) {
        n = ::send(fd, bufPtr(), toSend(), 0);
        if (n < 0) {
            perror("[sendRes]: send2");
            return;
        }

        sent += n;
    }

    if (toSend() == 0) {
        conn->resetOut();
        unsetEpollWriteable(fd);
    }
}

// TODO: replace throws with responses to the sender
void PublicAPI::run() {
    serverSockFd_ = openSck();
    bindSck(serverSockFd_);
    if (::listen(serverSockFd_, SOMAXCONN) == -1)
        throw std::system_error(errno, std::system_category(), "listen");

    epollfd_ = ::epoll_create1(0);
    if (epollfd_ == -1)
        throw std::system_error(errno, std::system_category(), "epoll_create");
    epollAdd(serverSockFd_);

    std::array<epoll_event, MAX_EVENTS> events;
    while (true) {
        int nfds = ::epoll_wait(epollfd_, events.data(), MAX_EVENTS, 0);
        if (nfds == -1) {
            if (errno == EINTR)
                continue;
            throw std::system_error(errno, std::system_category(), "epoll_wait");
        }

        for (int i = 0; i < nfds; ++i) {
            auto event = events[i];
            int incfd = event.data.fd;

            if (incfd == serverSockFd_) {
                API_STATUS_CODE status = acceptSck();
                if (status != API_STATUS_CODE::SUCCESS) {
                    // TODO: prepare the message to actually send
                    outgoings_.push(incfd);
                }
            }
            else {
                auto* conn = conns_[incfd].get();
                /*
                    There is a queue here because in the future we would
                    like to process messages on multiple threads and the
                    management is easier with a queue. There may be changes
                    in the future
                */
                incomings_.push(incfd);
            }
        }

        while (!incomings_.empty()) {
            auto topfd = incomings_.front();
            incomings_.pop();

            handleReadSck(topfd);
        }

        while (!outgoings_.empty()) {
            auto topfd = outgoings_.front();
            outgoings_.pop();

            sendRes(topfd);
        }
    }
}
