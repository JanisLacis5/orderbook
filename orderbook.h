#include <map>
#include "usings.h"
#include "types.h"
#include "trade.h"

class OrderBook {
public:
    trades_t addOrder(orderPtr_t order);
    void cancelOrder(orderId_t orderId);  // TODO: implement
    orderPtr_t modifyOrder(orderPtr_t order, ModifyOrder modifications);  // TODO: implement
private:
    struct OrderInfo {
        orderPtr_t order_;
        orderPtrs_t::iterator location_;
    };

    std::map<price_t, orderPtrs_t, std::less<price_t>> ask_;
    std::map<price_t, orderPtrs_t, std::greater<price_t>> bid_;

    trades_t passiveMatchOrders();  // TODO: implement
    trades_t aggressiveMatchOrder(orderPtr_t order);  // TODO: implement
    microsec_t getCurrTime() const;
    orderPtr_t toFillAndKill(orderPtr_t order);
    void processAddedOrder(orderPtr_t order);  // TODO: implement
    bool canBeFullyFilled(orderPtr_t order) const;  // TODO: implement
    bool doesCrossSpread(orderPtr_t order) const;
    void addAtOrderPrice(orderPtr_t order);
};
