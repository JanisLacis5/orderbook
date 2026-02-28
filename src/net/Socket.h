#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "Logger.h"

class Socket {
public:
    Socket();
    Socket(int listeningFd);
    ~Socket() { ::close(fd); };

    int fd;

    bool bind(in_addr_t ip, int port);
    bool listen(int backlog = SOMAXCONN);

private:
    Logger logger_{"Socket"};
};
