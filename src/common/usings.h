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

namespace badValues
{
    constexpr inline orderId_t orderId = std::numeric_limits<std::uint64_t>::max();
    constexpr inline price_t price = std::numeric_limits<std::uint32_t>::max();
    constexpr inline quantity_t quantity = std::numeric_limits<std::uint32_t>::max();
} // namespace badValues
