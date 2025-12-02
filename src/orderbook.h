#pragma once

#include <functional>
#include <map>
#include <optional>
#include <unordered_map>
#include "trade.h"
#include "types.h"
#include "usings.h"

class Orderbook {
public:
    struct LevelData {
        uint32_t volume = 0;
        uint32_t orderCnt = 0;
    };

    trades_t addOrder(orderPtr_t order);
    void cancelOrder(orderId_t orderId);
    trades_t modifyOrder(orderPtr_t order, ModifyOrder modifications);
    std::optional<price_t> bestAsk() const;
    std::optional<price_t> bestBid() const;
    std::map<price_t, LevelData, std::less<price_t>> fullDepthAsk() const;
    std::map<price_t, LevelData, std::greater<price_t>> fullDepthBid() const;

private:
    struct OrderInfo {
        orderPtr_t order_;
        orderPtrs_t::iterator location_;
    };

    std::map<price_t, orderPtrs_t, std::less<price_t>> ask_;
    std::map<price_t, orderPtrs_t, std::greater<price_t>> bid_;
    std::map<price_t, LevelData> levelData_;
    std::unordered_map<orderId_t, OrderInfo> orders_;

    trades_t matchOrder(orderPtr_t order);
    microsec_t getCurrTime() const;
    void processAddedOrder(orderPtr_t order);
    bool canBeFullyFilled(price_t price, quantity_t quantity, Side side) const;
    bool doesCrossSpread(price_t price, Side side) const;
    void addAtOrderPrice(orderPtr_t order);
};
