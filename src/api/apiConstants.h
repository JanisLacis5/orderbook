#pragma once

#include <cstdint>

constexpr std::size_t MESSAGE_QUEUE_SIZE = 100'000;
constexpr std::size_t MAX_MESSAGE_LEN = 4096;
constexpr std::size_t HDR_MESSAGE_SIZE = 32;
constexpr std::size_t HDR_SIZE = HDR_MESSAGE_SIZE + 4;
constexpr std::size_t MAX_RESPONSE_LEN = 1'000'000;
constexpr int MAX_BYTES_PER_HANDLE = 100'000;
