#include "EpollManager.h"
#include <cerrno>
#include <system_error>

EpollManager::EpollManager() {
    epollfd_ = ::epoll_create1(0);
    if (epollfd_ == -1) {
        logger_.logerrno("failed to create epoll pool");
        throw std::system_error(errno, std::system_category(), "epoll_create");
    }

    add(epollfd_);
}

int EpollManager::getEvents(std::array<epoll_event, MAX_EVENTS>& out) {
    int nfds = ::epoll_wait(epollfd_, out.data(), MAX_EVENTS, 0);
    if (nfds == -1) {
        if (errno == EINTR) {
            logger_.info("got EINTR, skipping...");
            return 0;
        }

        logger_.logerrno("failed to get epoll events");
        return -1;
    }

    return nfds;
}

bool EpollManager::add(int fd) {
    epoll_event ev{.events = EPOLLIN, .data = {.fd = fd}};

    if (::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        logger_.logerrno("failed to add to epoll pool");
        return false;
    }

    return true;
}

bool EpollManager::setWriteable(int fd, uint32_t initialEvents) {
    if (initialEvents & EPOLLOUT)
        return true;

    uint32_t events = initialEvents | EPOLLOUT;
    epoll_event ev{.events = events, .data = {.fd = fd}};

    if (::epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
        logger_.logerrno("failed to make socket writable");
        return false;
    }

    return true;
}

bool EpollManager::unsetWriteable(int fd, uint32_t initialEvents) {
    if (!(initialEvents & EPOLLOUT))
        return false;

    uint32_t events = initialEvents & (~EPOLLOUT);
    epoll_event ev{.events = events, .data = {.fd = fd}};

    if (::epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
        logger_.logerrno("failed to socket unwritable");
        return false;
    }

    return true;
}
