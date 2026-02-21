#include "EpollManager.h"
#include <cerrno>
#include <system_error>

EpollManager::EpollManager() {
    epollfd_ = ::epoll_create1(0);
    if (epollfd_ == -1)
        throw std::system_error(errno, std::system_category(), "epoll_create");

    add(epollfd_);
}

int EpollManager::getEvents(std::array<epoll_event, MAX_EVENTS>& out) {
    int nfds = ::epoll_wait(epollfd_, out.data(), MAX_EVENTS, 0);
    if (nfds == -1) {
        if (errno == EINTR)
            return 0;
        throw std::system_error(errno, std::system_category(), "epoll_wait");
    }

    return nfds;
}

bool EpollManager::add(int fd) {
    epoll_event ev{.events = EPOLLIN, .data = {.fd = fd}};

    if (::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("[epoll_add]: epoll_ctl");
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
        perror("[sendRes]: epoll_ctl1");
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
        perror("[sendRes]: epoll_ctl1");
        return false;
    }
    return true;
}
