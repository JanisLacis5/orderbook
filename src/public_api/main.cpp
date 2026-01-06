#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <cerrno>
#include <vector>
#include "SPSCQueue.h"

int main() {
    constexpr size_t MESSAGE_QUEUE_SIZE = 100'000;
    constexpr int MAX_EVENTS = 100;
    SPSCQueue<int> message_queue(MESSAGE_QUEUE_SIZE);

    int serverSock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (serverSock == -1)
        throw std::system_error(errno, std::system_category(), "main socket open");

    sockaddr_in addr{
        .sin_family = AF_INET, .sin_port = htons(8000), .sin_addr = {.s_addr = INADDR_ANY}};
    if (bind(serverSock, (sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::system_error(errno, std::system_category(), "main socket bind");

    if (listen(serverSock, SOMAXCONN) == -1)
        throw std::system_error(errno, std::system_category(), "main socket listen");

    int epollfd = epoll_create1(0);
    if (epollfd == -1)
        throw std::system_error(errno, std::system_category(), "epoll open");

    epoll_event ev{.events = EPOLLIN, .data = {.fd = serverSock}};
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSock, &ev) == -1)
        throw std::system_error(errno, std::system_category(), "epoll ctl");

    std::vector<epoll_event> events;
    while (true) {
        int nfds = epoll_wait(epollfd, events.data(), MAX_EVENTS, -1);

        for (auto event : events) {
        }
    }
}