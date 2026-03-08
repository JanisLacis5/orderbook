#pragma once

#include "Logger.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class Socket
{
public:
    Socket();
    Socket(int fd);
    ~Socket()
    {
        if (fd_ != -1) {
            ::close(fd_);
            fd_ = -1;
        }
    }

    Socket(const Socket& other) = delete;
    Socket& operator=(const Socket& other) = delete;

    Socket(Socket&& other)
    {
        fd_ = std::exchange(other.fd_, -1);
        epollEvents = std::exchange(other.epollEvents, 0u);
        logger_ = std::move(other.logger_);
    }
    Socket& operator=(Socket&& other)
    {
        std::swap(*this, other);
        return *this;
    }

    uint32_t epollEvents{0};

    int fd() const { return fd_; }
    bool bind(in_addr_t ip, int port);
    bool listen(int backlog = SOMAXCONN);

private:
    int fd_{-1};
    Logger logger_{"Socket"};
};
