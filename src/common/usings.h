#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

using price_t = std::int32_t;
using quantity_t = std::uint32_t;
using orderId_t = std::uint64_t;
using orderIds_t = std::vector<orderId_t>;
using microsec_t = std::chrono::microseconds;

using userId_t = std::uint64_t;
using apiCallId_t = std::uint32_t;

constexpr inline orderId_t INVALID_ORDER_ID = std::numeric_limits<std::uint64_t>::max();
