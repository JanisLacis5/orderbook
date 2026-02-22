#include "Socket.h"

#include <sys/socket.h>
#include <cerrno>
#include <system_error>


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

bool Socket::bind(in_addr_t ip, int port) {}

bool Socket::listen(int backlog) {};
