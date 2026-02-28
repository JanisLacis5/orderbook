#include "Socket.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <cerrno>
#include <system_error>

// Opening a new socket
Socket::Socket() {
    fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd == -1) {
        logger_.logerrno("[open_sck]: socket");
        throw std::system_error(errno, std::system_category(), "Socket::Socket -> socket");
    }

    int so_reuseaddr_val = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr_val, sizeof(so_reuseaddr_val)) == -1) {
        logger_.logerrno("[open_sck]: setsockopt");
        throw std::system_error(errno, std::system_category(), "Socket::Socket -> setsockopt");
    }
}

// Accepting a socket / connection
Socket::Socket(int listeningFd) {
    sockaddr_in local{};
    socklen_t addrlen = sizeof(local);

    fd = ::accept(listeningFd, (sockaddr*)&local, &addrlen);
    if (fd == -1) {
        logger_.logerrno("[accept_sck]: accept");
        throw std::system_error(errno, std::system_category(), "Socket::Socket(fd) -> accept");
    }

    ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

bool Socket::bind(in_addr_t ip, int port) {
    sockaddr_in addr{.sin_family = AF_INET, .sin_port = htons(port), .sin_addr = {.s_addr = ip}};

    if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        logger_.logerrno("bind");
        return false;
    }

    return true;
}

// TODO: maybe we should unlisten the previous listenings this socket had
bool Socket::listen(int backlog) {
    if (::listen(fd, backlog) == -1) {
        logger_.logerrno("listen");
        return false;
    }

    return true;
};
