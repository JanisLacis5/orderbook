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

class IMessager
{
public:
    virtual std::vector<FormattedMessage> decode(std::array<std::byte, MAX_MESSAGE_LEN>& raw, size_t size) = 0;
    virtual std::array<std::byte, MAX_MESSAGE_LEN> serialize(FormattedMessage& mes) = 0;
};

class Messager : public IMessager
{
public:
    std::vector<FormattedMessage> decode(std::array<std::byte, MAX_MESSAGE_LEN>& raw, size_t size) override;
    std::array<std::byte, MAX_MESSAGE_LEN> serialize(FormattedMessage& mes) override;
};
