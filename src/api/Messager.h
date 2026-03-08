#pragma once

#include "apiConstants.h"
#include <array>

struct FormattedMessage {
};

class IMessager {
public: 
    virtual ~IMessager(); 
    virtual FormattedMessage& decode(std::array<std::byte, MAX_MESSAGE_LEN>& raw) = 0;
    virtual std::array<std::byte, MAX_MESSAGE_LEN> serialize(FormattedMessage& mes) = 0;
};

class Messager : public IMessager {};
