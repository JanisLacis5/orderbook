#pragma once

#include <netinet/in.h>
#include <map>
#include <queue>
#include <vector>
#include "SPSCQueue.h"

constexpr size_t MESSAGE_QUEUE_SIZE = 100'000;
constexpr int MAX_EVENTS = 100;

// TODO: add inc / out buffers, upgrade the Conn class as the code grows
struct Conn {
    std::vector<uint8_t> in{};
    std::vector<uint8_t> out{};
};

class PublicAPI {
public:
    PublicAPI() : message_queue_(MESSAGE_QUEUE_SIZE) {};
    ~PublicAPI() {
        ::close(serverSockFd_);
        ::close(epollfd_);
    }

    void run();

private:
    int serverSockFd_{-1};
    int epollfd_{-1};
    SPSCQueue<int> message_queue_;
    std::map<int, std::unique_ptr<Conn>> conns_;
    // TODO: make these thread safe and process messages on multiple threads 
    std::queue<int> to_read_;  
    std::queue<int> to_write_;

    int open_sck();
    void bind_sck(int fd, int port = 8000, in_addr_t ipaddr = INADDR_ANY);
    void epoll_add(int fd);
    int accept_sck();
    void handle_read_sck(int fd);
    void handle_write_sck(int fd);
};
