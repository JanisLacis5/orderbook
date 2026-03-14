#include "User.h"
#include <random>

bool User::receive()
{
    size_t received = 0;
    while (received < MAX_BYTES_PER_HANDLE && inBuffer_.size() < MAX_MESSAGE_LEN) {
        auto chunk = inBuffer_.writable_contiguous();
        auto n = ::read(socket_.fd(), chunk.data(), std::min(chunk.size(), MAX_MESSAGE_LEN));
        if (!inBuffer_.commit_chunk_write(chunk, n)) {
            logger_.error("failed to commit read");
            return false;
        }

        if (n == 0)
            break;

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            logger_.logerrno("receive");
            return false;
        }
    }

    return true;
}

userId_t User::generateId()
{
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    // 42 bits for timestamp, 22 bits for random
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist(0, (1ull << 22) - 1);

    uint64_t timestamp = static_cast<uint64_t>(ms) & 0x3ffffffffff; // 42 bits
    uint64_t random = dist(gen);

    return (timestamp << 22) | random;
}
