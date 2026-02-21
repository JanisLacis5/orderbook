#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <array>
#include "Logger.h"

constexpr int MAX_EVENTS = 100;

class EpollManager {
public:
    EpollManager();
    ~EpollManager() {
        if (epollfd_ > 0)
            ::close(epollfd_);
    };

    bool add(int fd);
    int getEvents(std::array<epoll_event, MAX_EVENTS>& out);
    bool setWriteable(int fd, uint32_t initialEvents);
    bool unsetWriteable(int fd, uint32_t initialEvents);

private:
    int epollfd_;
    Logger logger_{"EpollManager"};
};
