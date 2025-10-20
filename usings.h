#include <cstdint>
#include <vector>
#include <chrono>

using price_t = std::int32_t;
using quantity_t = std::uint32_t;
using orderId_t = std::uint64_t;
using orderIds_t = std::vector<orderId_t>;
using microsec_t = std::chrono::microseconds;
