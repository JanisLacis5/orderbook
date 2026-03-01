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

    EpollManager(const EpollManager& other) = delete;
    EpollManager(EpollManager&& other) = delete;
    EpollManager& operator=(const EpollManager& other) = delete;
    EpollManager& operator=(EpollManager&& other) = delete;

    bool add(int fd);
    int getEvents(std::array<epoll_event, MAX_EVENTS>& out);
    bool setWriteable(int fd, uint32_t& events);
    bool unsetWriteable(int fd, uint32_t& events);

private:
    int epollfd_;
    Logger logger_{"EpollManager"};
};
