#include <vector>
#include "order.h"

struct Trade {
    orderPtr_t seller;
    orderPtr_t buyer;
};
using trades_t = std::vector<Trade>;

inline Trade newTrade(orderPtr_t buyer, orderPtr_t seller) {
    Trade trade;
    trade.buyer = buyer;
    trade.seller = seller;
    return trade;
}
