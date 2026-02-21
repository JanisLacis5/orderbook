#include <sys/epoll.h>


class EpollLoop {
public:
    EpollLoop();
    ~EpollLoop();

private:
    int epollfd_;

    bool add(int fd);
    bool setWriteable(int fd, uint32_t initialEvents);
    bool unsetWriteable(int fd, uint32_t initialEvents);
};
