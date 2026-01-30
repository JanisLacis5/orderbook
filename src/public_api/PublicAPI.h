#pragma once

#include <netinet/in.h>
#include <map>
#include <queue>
#include <array>
#include "usings.h"
#include "SPSCQueue.h"

constexpr size_t MESSAGE_QUEUE_SIZE = 100'000;
constexpr size_t MAX_MESSAGE_LEN = 4096;
// max amount of bytes handled per handle_read_sck call
constexpr int MAX_BYTES_PER_HANDLE = 100'000;  // TODO: implement the usage
constexpr int MAX_EVENTS = 100;

struct Conn {
    userId_t holderId;
    
    // buffers that hold excess data from a partial read e.g. unfinished message
    // always starts with total_mes_len
    std::array<std::byte, MAX_MESSAGE_LEN> in;  // from conn
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

    std::map<userId_t, messageQueue_t> messageQueues_;

    std::map<int, std::unique_ptr<Conn>> conns_;
    // TODO: make these thread safe and process messages on multiple threads
    std::queue<int> incomings_;

    int open_sck();
    void bind_sck(int fd, int port = 8000, in_addr_t ipaddr = INADDR_ANY);
    void epoll_add(int fd);
    int accept_sck();
    void handle_read_sck(int fd);
    void handle_write_sck(int fd);
};
