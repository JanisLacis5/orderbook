#include "PublicAPI.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <array>
#include <bit>
#include <cerrno>

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
    int connFd = ::accept(serverSockFd_, (sockaddr*)&local, &addrlen);
    if (connFd == -1) {
        perror("accept");
        return -1;
    }

    // Make non-blocking
    ::fcntl(connFd, F_SETFL, ::fcntl(connFd, F_GETFL, 0) | O_NONBLOCK);

    uint32_t events = EPOLLIN | EPOLLET;
    epoll_event ev{.events = events, .data = {.fd = connFd}};
    if (::epoll_ctl(epollfd_, EPOLL_CTL_ADD, connFd, &ev) == -1) {
        perror("epoll_ctl: conn_sock");
        return -1;
    }

    // TODO: generate userId and add it to the conn->holderId
    // TODO: create a new spsc queue in messageQueues map for the new user
    conns_[connFd] = std::make_unique<Conn>();
    return 0;
}

void PublicAPI::handle_read_sck(int fd) {
    auto& conn = conns_.at(fd);
    auto& buf = conn->in;
    auto totalRead = 0u;
    auto received = 0u;

    while (totalRead < MAX_BYTES_PER_HANDLE) {
        auto n = ::read(fd, buf.data() + totalRead, 4);
        // TODO: error handling, break out of the loop here at some point
        received += n;
        totalRead += n;

        const uint32_t mesLen = std::bit_cast<uint32_t>(std::array<std::byte, 4>{buf[0], buf[1], buf[2], buf[3]});
        // TODO: meslen checks (> 0 and < maxlen)

        while (totalRead < MAX_BYTES_PER_HANDLE && mesLen < received) {
            n = ::read(fd, buf.data() + totalRead, MAX_MESSAGE_LEN);
            // TODO: error handling
            received += n;
            totalRead += n;
        }

        if (mesLen <= received) {
            messageQueue_t& queue = messageQueues_[conn->holderId];
            std::array<std::byte, MAX_MESSAGE_LEN> message;

            std::copy(buf.begin(), buf.begin() + mesLen, message.begin());
            queue.push(buf);
            received -= mesLen;
        }
    }
}

void PublicAPI::run() {
    serverSockFd_ = open_sck();
    bind_sck(serverSockFd_);
    if (::listen(serverSockFd_, SOMAXCONN) == -1)
        throw std::system_error(errno, std::system_category(), "listen");

    epollfd_ = ::epoll_create1(0);
    if (epollfd_ == -1)
        throw std::system_error(errno, std::system_category(), "epoll_create");
    epoll_add(serverSockFd_);

    std::array<epoll_event, MAX_EVENTS> events;
    while (true) {
        int nfds = ::epoll_wait(epollfd_, events.data(), MAX_EVENTS, 0);
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
                auto* conn = conns_[incfd].get();
                /*
                    There is a queue here because in the future we would
                    like to process messages on multiple threads and the
                    management is easier with a queue. There may be changes
                    in the future
                */
                incomings_.push(incfd);
            }
        }

        while (!incomings_.empty()) {
            auto topfd = incomings_.front();
            incomings_.pop();

            handle_read_sck(topfd);
        }
    }
}
