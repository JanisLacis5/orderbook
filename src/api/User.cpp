#include "User.h"
#include <random>

bool User::receive() {
    while (received_ < MAX_BYTES_PER_HANDLE) {
        auto n = ::read(socket_.fd(), inBuffer_.data() + received_, MAX_MESSAGE_LEN);

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

userId_t User::generateId() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    // 42 bits for timestamp, 22 bits for random
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist(0, (1ull << 22) - 1);

    uint64_t timestamp = static_cast<uint64_t>(ms) & 0x3ffffffffff;  // 42 bits
    uint64_t random = dist(gen);

    return (timestamp << 22) | random;
};

