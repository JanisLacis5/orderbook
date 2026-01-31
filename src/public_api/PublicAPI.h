#pragma once

#include <netinet/in.h>
#include <sys/epoll.h>
#include <array>
#include <map>
#include <queue>
#include "SPSCQueue.h"
#include "usings.h"

constexpr size_t MESSAGE_QUEUE_SIZE = 100'000;
constexpr size_t MAX_MESSAGE_LEN = 4096;
constexpr size_t HDR_MESSAGE_SIZE = 32;
constexpr size_t HDR_SIZE = HDR_MESSAGE_SIZE + 4;
constexpr size_t MAX_RESPONSE_LEN = 1'000'000;
constexpr int MAX_BYTES_PER_HANDLE = 100'000;
constexpr int MAX_EVENTS = 100;

enum class API_STATUS_CODE { SUCCESS, BAD_MESSAGE_LEN, SYSTEM_ERROR };

struct Conn {
    int fd;
    userId_t holderId;

    size_t outSize;
    size_t outSent;
    size_t inSize;
    size_t inSent;

    // buffers that hold excess data from a partial read e.g. unfinished message
    // always starts with total_mes_len
    std::array<std::byte, MAX_MESSAGE_LEN> in;   // from conn
    std::array<std::byte, MAX_MESSAGE_LEN> out;  // to conn

    void resetOut() {
        outSize = 0;
        outSent = 0;
    }
};

struct APIResponse {
    uint32_t code;
    std::string message;
    size_t payloadSize;

    std::array<std::byte, MAX_RESPONSE_LEN> payload;  // already formatted in the required format
};

class PublicAPI {
public:
    PublicAPI() {};
    ~PublicAPI() {
        ::close(serverSockFd_);
        ::close(epollfd_);
    }

    void run();

private:
    using MessageQueue_t = SPSCQueue<std::array<std::byte, MAX_MESSAGE_LEN>>;

    int serverSockFd_{-1};
    int epollfd_{-1};

    std::map<userId_t, int> uid2fd_;
    std::map<int, std::unique_ptr<Conn>> conns_;
    // TODO: make this thread safe and process messages on multiple threads
    std::queue<int> incomings_;
    std::queue<int> outgoings_;

    std::map<userId_t, std::unique_ptr<MessageQueue_t>> messageQueues_;

    int openSck();
    void connInit(int fd);
    void sendRes(int fd, epoll_event& event);
    uint64_t generateUserId();
    void setEpollWriteable(int fd, epoll_event& event);
    void unsetEpollWriteable(int fd, epoll_event& event);

    API_STATUS_CODE bindSck(int fd, int port = 8000, in_addr_t ipaddr = INADDR_ANY);
    API_STATUS_CODE epollAdd(int fd);
    API_STATUS_CODE acceptSck();
    API_STATUS_CODE handleReadSck(int fd);
    API_STATUS_CODE handleWriteSck(int fd);
};
