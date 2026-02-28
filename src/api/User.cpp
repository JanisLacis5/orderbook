#include "User.h"

bool User::receive() {
    while (received_ < MAX_BYTES_PER_HANDLE) {
        auto n = ::read(socket_.fd, inBuffer_.data() + received_, MAX_MESSAGE_LEN);

        if (n == 0)
            break;

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            logger_.logerrno("receive");
            return false;
        }

        received_ += n;
    }

    return true;
}
