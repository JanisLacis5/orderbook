#pragma once

#include <vector>
#include "usings.h"

struct Trade {
    orderId_t seller;
    orderId_t buyer;
    quantity_t quantity;
};
using trades_t = std::vector<Trade>;

inline Trade newTrade(orderId_t buyer, orderId_t seller, quantity_t quantity) {
    Trade trade;
    trade.buyer = buyer;
    trade.seller = seller;
    trade.quantity = quantity;
    return trade;
}
