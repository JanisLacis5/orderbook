#include <numeric>
#include "orderbook.h"


// PRIVATE FUNCTION IMPLEMENTATIONS
trades_t OrderBook::passiveMatchOrders() { // TODO: implement
    return {};
}

trades_t OrderBook::aggressiveMatchOrder(orderPtr_t order) {  // TODO: implement
    return {};
}

microsec_t OrderBook::getCurrTime() const {
    auto time = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<microsec_t>(time);
}

orderPtr_t OrderBook::toFillAndKill(orderPtr_t order) {
    ModifyOrder modifications;
    modifications.quantity = order->getRemainingQuantity();
    modifications.price = order->getPrice();
    modifications.type = OrderType::FillAndKill;
    return modifyOrder(order, modifications);
}

void OrderBook::processAddedOrder(orderPtr_t order) {
    orderId_t orderId = order->getOrderid();

    orders_[orderId].order_ = order;
    if (order->getSide() == Side::Sell) {
        orders_[orderId].location_ = std::prev(ask_[order->getPrice()].end());
    }
    else if (order->getSide() == Side::Buy) {
        orders_[orderId].location_ = std::prev(bid_[order->getPrice()].end());
    }

    levelData_[order->getPrice()].volume += order->getInitialQuantity();
    levelData_[order->getPrice()].orderCnt++;
}

bool OrderBook::canBeFullyFilled(price_t price, quantity_t quantity, Side side) const {
    if (quantity <= 0)
        throw std::logic_error("Invalid quantity");
    if (!doesCrossSpread(price, side))
        return false;

    price_t treshold;
    // respectively ask_ or bid_ are not empty - checked in doesCrossSpread function
    if (side == Side::Sell) {
        const auto& [bidPrice, _] = *bid_.begin();
        treshold = bidPrice;
    }
    else if (side == Side::Buy) {
        const auto& [askPrice, _] = *ask_.begin();
        treshold = askPrice;
    }

    for (const auto& [levelPrice, levelData]: levelData_) {
        if ((side == Side::Buy && levelPrice < treshold) || (side == Side::Sell && levelPrice > treshold))
            continue;
        if ((side == Side::Sell && levelPrice < price) || (side == Side::Buy && levelPrice > price))
            continue;

        if (quantity <= levelData.volume)
            return true;
        quantity -= levelData.volume;
    }
    return false;
}

bool OrderBook::doesCrossSpread(price_t price, Side side) const {
    if (side == Side::Sell) {
        if (bid_.empty())
            return false;

        const auto& [bestBid, _] = *bid_.begin();
        return price <= bestBid;
    }
    else if (side == Side::Buy) {
        if (ask_.empty())
            return false;

        const auto& [bestAsk, _] = *ask_.begin();
        return price >= bestAsk;
    }
    else
        throw std::logic_error("Invalid side");
}

void OrderBook::addAtOrderPrice(orderPtr_t order) {
    if (order->getSide() == Side::Sell) {
        ask_[order->getPrice()].push_back(order);
    }
    else if (order->getSide() == Side::Buy) {
        bid_[order->getPrice()].push_back(order);
    }
    else
        throw std::logic_error("Invalid side");
}

// PUBLIC FUNCTION IMPLEMENTATIONS
trades_t OrderBook::addOrder(orderPtr_t order) {
    OrderType type = order->getType();

    if (type == OrderType::FillAndKill) {
        if (!doesCrossSpread(order->getPrice(), order->getSide()))
            return {};
    }
    else if (type == OrderType::FillOrKill) {
        if (!canBeFullyFilled(order->getPrice(), order->getInitialQuantity(), order->getSide()))
            return {};
    }
    else if (type == OrderType::GoodTillCancel || type == OrderType::GoodTillEOD) {
        addAtOrderPrice(order);
        processAddedOrder(order);
        return passiveMatchOrders();
    }
    return aggressiveMatchOrder(order);
}

void OrderBook::cancelOrder(orderId_t orderId) {  // TODO: implement

}

orderPtr_t OrderBook::modifyOrder(orderPtr_t order, ModifyOrder modifications) {  // TODO: implement

}
