#include "Messager.h"

formattedMessages_t Messager::decode(rawMessage_t& raw, size_t size)
{
    size_t currPos = 0;

    while (currPos < size)
        if (size - currPos < 4)
            break;
}

rawMessage_t Messager::encode(FormattedMessage& mes) {}
