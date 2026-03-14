#pragma once

#include "Logger.h"
#include "Messager.h"
#include "Socket.h"
#include "apiConstants.h"
#include "ring_buffer.h"
#include "usings.h"

class User
{
public:
    template <typename TIdValidationFunction>
    User(int serverSckFd, TIdValidationFunction validId)
        : socket_{serverSckFd}
        , outBuffer_(MAX_MESSAGE_LEN)
        , inBuffer_(MAX_MESSAGE_LEN)
    {
        id = generateId();

        // Extremely low probability of happening
        while (!validId(id))
            id = generateId();
    }

    ~User() {}

    User(const User& other) = delete;
    User& operator=(const User& other) = delete;
    User(User&& other) = delete;
    User& operator=(User&&) = delete;

    userId_t id;
    int sckFd() const { return socket_.fd(); }

    template <typename EpollSet, typename EpollUnset>
    bool send(EpollSet&& set, EpollUnset&& unset); // server -> client
    bool receive();                                // client -> server

private:
    // in - incoming (order management, data request etc), out - outgoing (for the client, error message, data, etc)
    ring_buffer<std::byte> inBuffer_;
    std::vector<FormattedMessage> inFormatted_;

    ring_buffer<std::byte> outBuffer_;
    std::vector<FormattedMessage> outFormatted_;

    Logger logger_{"User"};
    Socket socket_;

    userId_t generateId();
};

template <typename EpollSet, typename EpollUnset>
bool User::send(EpollSet&& set, EpollUnset&& unset)
{
    if (!set(socket_.fd(), socket_.epollEvents))
        return false;

    auto chunk = outBuffer_.readable_contiguous();
    auto n = ::send(socket_.fd(), chunk.data(), chunk.size(), 0);
    if (n <= 0) {
        logger_.logerrno("[sendRes]: send1");
        return false;
    }
    if (!outBuffer_.consume_front(n)) {
        logger_.error("failed to delete sent bytes");
        return false;
    }

    while (!outBuffer_.empty() && n > 0) {
        auto chunk = outBuffer_.readable_contiguous();
        n = ::send(socket_.fd(), chunk.data(), chunk.size(), 0);

        if (n == 0)
            break;
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            perror("[sendRes]: send2");
            return false;
        }

        if (!outBuffer_.consume_front(n)) {
            logger_.error("failed to delete sent bytes");
            return false;
        }
    }

    if (outBuffer_.empty())
        return unset(socket_.fd(), socket_.epollEvents);
    return true;
}
