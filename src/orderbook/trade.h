#pragma once

#include <vector>
#include "usings.h"

struct Trade {
    orderId_t seller;
    orderId_t buyer;
    quantity_t quantity;
    price_t price;
};

using trades_t = std::vector<Trade>;

inline Trade newTrade(orderId_t buyer, orderId_t seller, quantity_t quantity, price_t price) {
    return {.seller = seller, .buyer = buyer, .quantity = quantity, .price = price};
}
