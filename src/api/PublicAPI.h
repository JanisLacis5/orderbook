#pragma once

#include <netinet/in.h>
#include <array>
#include <map>
#include <queue>
#include "EpollManager.h"
#include "Socket.h"
#include "User.h"

enum class API_STATUS_CODE { SUCCESS, BAD_MESSAGE_LEN, SYSTEM_ERROR };

class PublicAPI {
public:
    PublicAPI();
    ~PublicAPI() {}

    void run();

private:
    struct APIResponse {
        uint32_t code;
        std::string message;
        size_t payloadSize;

        std::array<std::byte, MAX_RESPONSE_LEN> payload;  // already formatted in the required format
    };

    Socket listenSocket_{};
    EpollManager epollManager_{};

    std::map<int, std::unique_ptr<User>> users_;  // user socket fd : User*
    std::queue<int> incomings_;
    std::queue<int> outgoings_;

    uint64_t generateUserId();
    API_STATUS_CODE acceptNewListener();
    std::array<std::byte, MAX_RESPONSE_LEN> createResBuf(API_STATUS_CODE status, std::span<std::byte> data);
};
