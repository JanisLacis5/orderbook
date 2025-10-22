#include <map>
#include <unordered_map>
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
    struct LevelData {
        uint32_t volume = 0;
        uint32_t orderCnt = 0;
    };

    std::map<price_t, orderPtrs_t, std::less<price_t>> ask_;
    std::map<price_t, orderPtrs_t, std::greater<price_t>> bid_;
    std::map<price_t, LevelData> levelData_;
    std::unordered_map<orderId_t, OrderInfo> orders_;

    trades_t passiveMatchOrders();  // TODO: implement
    trades_t aggressiveMatchOrder(orderPtr_t order);  // TODO: implement
    microsec_t getCurrTime() const;
    orderPtr_t toFillAndKill(orderPtr_t order);
    void processAddedOrder(orderPtr_t order);
    bool canBeFullyFilled(price_t price, quantity_t quantity, Side side) const;
    bool doesCrossSpread(price_t price, Side side) const;
    void addAtOrderPrice(orderPtr_t order);
};
