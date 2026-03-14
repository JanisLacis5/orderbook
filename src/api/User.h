#pragma once

#include "Logger.h"
#include "Messager.h"
#include "Socket.h"
#include "apiConstants.h"
#include "usings.h"

class User
{
public:
    template <typename TIdValidationFunction>
    User(int serverSckFd, TIdValidationFunction validId)
        : socket_{serverSckFd}
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
    bool send(EpollSet&& set, EpollUnset&& unset);
    bool receive();

private:
    using RawMessage = std::array<std::byte, MAX_MESSAGE_LEN>;

    // in - incoming (order management, data request etc), out - outgoing (for the client, error message, data, etc)
    int sent_{0};
    int inSize_{0};
    int outSize_{0};
    RawMessage inBuffer_;
    std::vector<FormattedMessage> inFormatted_;
    RawMessage outBuffer_;
    std::vector<FormattedMessage> outFormatted_;

    Logger logger_{"User"};
    Socket socket_;

    userId_t generateId();
};

template <typename EpollSet, typename EpollUnset>
bool User::send(EpollSet&& set, EpollUnset&& unset)
{
    auto toSend = [&] { return outSize_ - sent_; };
    auto bufPtr = [&] { return outBuffer_.data() + sent_; };

    if (!set(socket_.fd(), socket_.epollEvents))
        return false;

    auto n = ::send(socket_.fd(), bufPtr(), toSend(), 0);
    if (n <= 0) {
        logger_.logerrno("[sendRes]: send1");
        return false;
    }
    sent_ += n;

    while (toSend() > 0 && n > 0) {
        n = ::send(socket_.fd(), bufPtr(), toSend(), 0);

        if (n == 0)
            break;

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            perror("[sendRes]: send2");
            return false;
        }

        sent_ += n;
    }

    if (!toSend()) {
        sent_ = 0;
        outSize_ = 0;
        if (!unset(socket_.fd(), socket_.epollEvents))
            return false;
    }

    return true;
}
