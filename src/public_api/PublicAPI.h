#pragma once

#include <netinet/in.h>
#include <array>
#include <map>
#include <queue>
#include <set>
#include "SPSCQueue.h"
#include "usings.h"

constexpr size_t MESSAGE_QUEUE_SIZE = 100'000;
constexpr size_t MAX_MESSAGE_LEN = 4096;
constexpr int MAX_BYTES_PER_HANDLE = 100'000;
constexpr int MAX_EVENTS = 100;

enum class API_STATUS_CODE { SUCCESS, BAD_MESSAGE_LEN, SYSTEM_ERROR };

struct Conn {
    userId_t holderId;

    // buffers that hold excess data from a partial read e.g. unfinished message
    // always starts with total_mes_len
    std::array<std::byte, MAX_MESSAGE_LEN> in;   // from conn
    std::array<std::byte, MAX_MESSAGE_LEN> out;  // to conn
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
    using messageQueue_t = SPSCQueue<std::array<std::byte, MAX_MESSAGE_LEN>>;

    int serverSockFd_{-1};
    int epollfd_{-1};
    std::set<userId_t> userIds_;  // necessary to detect for clashes

    std::map<userId_t, messageQueue_t> messageQueues_;

    std::map<int, std::unique_ptr<Conn>> conns_;
    // TODO: make these thread safe and process messages on multiple threads
    std::queue<int> incomings_;

    int openSck();
    uint64_t generateUserId();
    API_STATUS_CODE bindSck(int fd, int port = 8000, in_addr_t ipaddr = INADDR_ANY);
    API_STATUS_CODE epollAdd(int fd);
    API_STATUS_CODE acceptSck();
    API_STATUS_CODE handleReadSck(int fd);
    API_STATUS_CODE handleWriteSck(int fd);
};
