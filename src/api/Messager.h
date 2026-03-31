#pragma once

#include "apiConstants.h"
#include <array>
#include <cstdint>
#include <vector>

struct FormattedMessage {
    uint32_t totalLen;
    uint32_t callID;
    std::vector<std::pair<uint8_t, uint8_t>> params;
};

using formattedMessages_t = std::vector<FormattedMessage>;
using rawMessage_t = std::array<std::byte, MAX_MESSAGE_LEN>;

class IMessager
{
public:
    virtual formattedMessages_t decode(rawMessage_t& raw, size_t size) = 0;
    virtual rawMessage_t encode(FormattedMessage& mes) = 0;
};

class Messager : public IMessager
{
public:
    formattedMessages_t decode(rawMessage_t& raw, size_t size) override;
    rawMessage_t encode(FormattedMessage& mes) override;
};
