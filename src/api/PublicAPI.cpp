#include "PublicAPI.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>

std::array<std::byte, MAX_RESPONSE_LEN> PublicAPI::createResBuf(API_STATUS_CODE status, std::span<std::byte> data)
{
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
    auto codeBytes = std::bit_cast<std::array<std::byte, 4>>(codeNetEnd);

    std::array<std::byte, MAX_RESPONSE_LEN> buf;
    std::copy(codeBytes.begin(), codeBytes.end(), buf.begin());
    std::memcpy(buf.data() + 4, hdr.message.data(), HDR_MESSAGE_SIZE);
    std::copy(data.begin(), data.begin() + data.size(), buf.begin() + HDR_SIZE);

    return buf;
}

int PublicAPI::acceptNewListener()
{
    auto user = std::make_unique<User>(listenSocket_.fd(), [&](userId_t uid) {
        return std::find_if(users_.begin(), users_.end(), [uid](const auto& user) { return user.second->id == uid; }) ==
               users_.end();
    });
    users_[user->sckFd()] = std::move(user);

    if (!epollManager_.add(user->sckFd())) {
        users_.erase(user->sckFd());
        return -1;
    }

    return user->sckFd();
}

PublicAPI::PublicAPI()
{
    listenSocket_.bind(INADDR_ANY, 8000);
    listenSocket_.listen();
}

void PublicAPI::run()
{
    std::array<epoll_event, MAX_EVENTS> events;
    while (true) {
        int nfds = epollManager_.getEvents(events);

        for (int i = 0; i < nfds; ++i) {
            auto event = events[i];
            int incfd = event.data.fd;

            if (incfd == listenSocket_.fd()) {
                if (acceptNewListener() == -1)
                    continue;
            } else {
                auto user = users_.find(incfd);
                if (user == users_.end() || !user->second)
                    // TODO: send a response to user (500: internal error or something)
                    throw std::logic_error("no pointer to a user");

                if (!user->second->receive()) {
                    // TODO: handle the error
                    return;
                }
            }
        }
    }
}
