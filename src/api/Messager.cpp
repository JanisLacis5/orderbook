#include "Messager.h"

std::vector<FormattedMessage> Messager::decode(std::array<std::byte, MAX_MESSAGE_LEN>& raw, size_t size)
{
    size_t currPos = 0;

    while (currPos < size)
        if (size - currPos < 4)
            break;
}

std::array<std::byte, MAX_MESSAGE_LEN> serialize(FormattedMessage& mes) {}
