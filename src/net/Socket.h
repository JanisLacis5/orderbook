#include "Logger.h"

#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>


class Socket {
public:
    Socket();
    ~Socket() {
        ::close(fd);
    };

    int fd;

    bool bind(in_addr_t ip, int port);
    bool listen(int backlog = SOMAXCONN);

private:
    Logger logger_{"Socket"};
};
