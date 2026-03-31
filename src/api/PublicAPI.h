#pragma once

#include "EpollManager.h"
#include "Messager.h"
#include "Socket.h"
#include "User.h"
#include <map>
#include <netinet/in.h>

enum class API_STATUS_CODE { SUCCESS, BAD_MESSAGE_LEN, SYSTEM_ERROR };

class PublicAPI
{
public:
    PublicAPI();
    ~PublicAPI() {}

    PublicAPI(const PublicAPI& other) = delete;
    PublicAPI(PublicAPI&& other) = delete;
    PublicAPI& operator=(const PublicAPI& other) = delete;
    PublicAPI& operator=(PublicAPI&& other) = delete;

    void run();

private:
    struct APIResponse {
        uint32_t code;
        std::string message;
        size_t payloadSize;

        rawMessage_t payload; // already formatted in the required format
    };

    Socket listenSocket_{};
    EpollManager epollManager_{};

    std::map<int, std::unique_ptr<User>> users_; // user socket fd : User*

    int acceptNewListener();
    rawMessage_t createResBuf(API_STATUS_CODE status, std::span<std::byte> data);
};
