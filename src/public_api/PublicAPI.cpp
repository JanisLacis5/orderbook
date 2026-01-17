#include "PublicAPI.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <array>
#include <cerrno>
#include <iostream>
#include "SPSCQueue.h"

int PublicAPI::open_sck() {
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1)
        throw std::system_error(errno, std::system_category(), "socket");

    int so_reuseaddr_val = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr_val, sizeof(so_reuseaddr_val)) == -1)
        throw std::system_error(errno, std::system_category(), "setsockopt");

    return fd;
}

void PublicAPI::bind_sck(int fd, int port, in_addr_t ipaddr) {
    sockaddr_in addr{.sin_family = AF_INET, .sin_port = htons(port), .sin_addr = {.s_addr = ipaddr}};
    if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::system_error(errno, std::system_category(), "bind");
}

void PublicAPI::epoll_add(int fd) {
    epoll_event ev{.events = EPOLLIN, .data = {.fd = fd}};
    if (::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1)
        throw std::system_error(errno, std::system_category(), "epoll_ctl");
}

int PublicAPI::accept_sck() {
    sockaddr_in local{};
    socklen_t addrlen = sizeof(local);
    int connFd = accept(serverSockFd_, (struct sockaddr*)&local, &addrlen);
    if (connFd == -1) {
        perror("accept");
        return -1;
    }

    // Make non-blocking
    fcntl(connFd, F_SETFL, fcntl(connFd, F_GETFL, 0) | O_NONBLOCK);

    uint32_t events = EPOLLIN | EPOLLET;
    epoll_event ev{.events = events, .data = {.fd = connFd}};
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, connFd, &ev) == -1) {
        perror("epoll_ctl: conn_sock");
        return -1;
    }
}

void PublicAPI::handle_message(int fd) {
    std::cout << "new messsage" << std::endl;
    return;
}

void PublicAPI::run() {
    serverSockFd_ = open_sck();
    bind_sck(serverSockFd_);
    if (::listen(serverSockFd_, SOMAXCONN) == -1)
        throw std::system_error(errno, std::system_category(), "listen");

    epollfd_ = epoll_create1(0);
    if (epollfd_ == -1)
        throw std::system_error(errno, std::system_category(), "epoll_create");
    epoll_add(serverSockFd_);

    std::array<epoll_event, MAX_EVENTS> events;
    while (true) {
        int nfds = ::epoll_wait(epollfd_, events.data(), MAX_EVENTS, -1);
        if (nfds == -1) {
            if (errno == EINTR)
                continue;
            throw std::system_error(errno, std::system_category(), "epoll_wait");
        }

        for (int i = 0; i < nfds; ++i) {
            int incfd = events[i].data.fd;
            if (incfd == serverSockFd_) {
                accept_sck();
            }
            else {
                handle_message(incfd);
            }
        }
    }
};
