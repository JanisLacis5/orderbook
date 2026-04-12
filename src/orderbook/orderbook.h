#pragma once

#include "order.h"
#include "trade.h"
#include "types.h"
#include "usings.h"
#include <functional>
#include <map>
#include <optional>
#include <unordered_map>

struct LevelView {
    price_t price;
    uint32_t volume;
    uint32_t orderCnt;
};
using levels_t = std::vector<LevelView>;

struct OrderInfo {
    price_t price;
    quantity_t quantity;
    Side side;
    OrderType type;
};

class Orderbook
{
public:
    std::tuple<orderId_t, trades_t, OrderInfo> addOrder(quantity_t quantity, price_t price, OrderType type, Side side);
    void cancelOrder(orderId_t orderId);
    std::tuple<orderId_t, trades_t, OrderInfo> modifyOrder(orderId_t orderId, ModifyOrder modifications);
    std::optional<price_t> bestAsk() const;
    std::optional<price_t> bestBid() const;
    levels_t fullDepthAsk() const { return fullDepth(Side::Sell); }
    levels_t fullDepthBid() const { return fullDepth(Side::Buy); }

private:
    struct InternalOrderInfo {
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
    std::unordered_map<orderId_t, InternalOrderInfo> orders_;
    orderId_t lastOrderId_{
        1}; // TODO: change defualt to 0 when custom orderId_t is implemented. now id == 0 means that order was rejected

    orderPtr_t newOrder(quantity_t quantity, price_t price, OrderType type, Side side);
    trades_t matchOrder(orderPtr_t order);
    microsec_t getCurrTime() const;
    void processAddedOrder(orderPtr_t order);
    bool canBeFullyFilled(price_t price, quantity_t quantity, Side side) const;
    bool doesCrossSpread(price_t price, Side side) const;
    void addAtOrderPrice(orderPtr_t order);
    levels_t fullDepth(Side side) const;
};
