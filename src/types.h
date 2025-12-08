#pragma once

#include "usings.h"

enum OrderType { Market, GoodTillCancel, GoodTillEOD, FillOrKill, FillAndKill };

enum Side { Buy, Sell };

struct ModifyOrder {
    std::optional<price_t> price{};
    std::optional<quantity_t> quantity{};
    std::optional<OrderType> type{};
    std::optional<Side> side{};
};
