#pragma once

#include <netinet/in.h>
#include "SPSCQueue.h"

constexpr size_t MESSAGE_QUEUE_SIZE = 100'000;
constexpr int MAX_EVENTS = 100;

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

    int open_sck();
    void bind_sck(int fd, int port = 8000, in_addr_t ipaddr = INADDR_ANY);
    void epoll_add(int fd);
    int accept_sck();
    void handle_message(int fd);
};
