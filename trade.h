#include <vector>
#include "order.h"

struct Trade {
    orderPtr_t seller;
    orderPtr_t buyer;
    quantity_t quantity;
};
using trades_t = std::vector<Trade>;

inline Trade newTrade(orderPtr_t buyer, orderPtr_t seller, quantity_t quantity) {
    Trade trade;
    trade.buyer = buyer;
    trade.seller = seller;
    trade.quantity = quantity;
    return trade;
}
