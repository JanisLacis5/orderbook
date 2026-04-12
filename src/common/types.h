#pragma once

#include "usings.h"

enum class OrderType { Bad, Market, GoodTillCancel, GoodTillEOD, FillOrKill, FillAndKill };

enum class Side { Bad, Buy, Sell };

struct ModifyOrder {
    std::optional<price_t> price{};
    std::optional<quantity_t> quantity{};
    std::optional<OrderType> type{};
    std::optional<Side> side{};
};
