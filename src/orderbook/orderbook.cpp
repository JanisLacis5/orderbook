#include "orderbook.h"
#include <optional>
#include <stdexcept>
#include "order.h"
#include "usings.h"

// PRIVATE FUNCTION IMPLEMENTATIONS
trades_t Orderbook::matchOrder(orderPtr_t order) {
    Side side = order->getSide();
    orderId_t orderId = order->getOrderId();
    trades_t trades;
    std::optional<price_t> threshold;

    if (order->getType() != OrderType::Market)
        threshold = order->getPrice();

    auto it = side == Side::Buy ? ask_.begin() : bid_.begin();
    auto itEnd = side == Side::Buy ? ask_.end() : bid_.end();

    while (it != itEnd && !order->isFullyFilled()) {
        auto& [currPrice, orders] = *it;
        if (threshold.has_value() && ((side == Side::Buy && currPrice > threshold.value()) ||
                                      (side == Side::Sell && currPrice < threshold.value())))
            break;

        while (!orders.empty() && !order->isFullyFilled()) {
            orderPtr_t opposite = orders.front();
            quantity_t toFill = std::min(order->getRemainingQuantity(), opposite->getRemainingQuantity());

            Trade trade = (side == Side::Buy ? newTrade(orderId, opposite->getOrderId(), toFill, currPrice)
                                             : newTrade(opposite->getOrderId(), orderId, toFill, currPrice));

            trades.push_back(trade);
            opposite->fill(toFill);
            order->fill(toFill);
            levelData_[currPrice].volume -= toFill;

            if (opposite->isFullyFilled()) {
                levelData_[currPrice].orderCnt--;
                orders.pop_front();
                orders_.erase(opposite->getOrderId());
            }

            if (levelData_.at(currPrice).orderCnt == 0)
                levelData_.erase(currPrice);
        }

        if (orders.empty())
            it = side == Side::Buy ? ask_.erase(it) : bid_.erase(it);
    }

    // For orders that are fine to rest on the book, fill orders that have a
    // valid price and leave the rest on the book
    if (!order->isFullyFilled() &&
        (order->getType() == OrderType::GoodTillCancel || order->getType() == OrderType::GoodTillEOD)) {
        addAtOrderPrice(order);
        processAddedOrder(order);
    }
    else if (order->isFullyFilled())
        orders_.erase(orderId);

    return trades;
}

microsec_t Orderbook::getCurrTime() const {
    auto time = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<microsec_t>(time);
}

void Orderbook::processAddedOrder(orderPtr_t order) {
    orderId_t orderId = order->getOrderId();

    orders_[orderId].order_ = order;
    if (order->getSide() == Side::Sell)
        orders_[orderId].location_ = std::prev(ask_[order->getPrice()].end());
    else if (order->getSide() == Side::Buy)
        orders_[orderId].location_ = std::prev(bid_[order->getPrice()].end());

    levelData_[order->getPrice()].volume += order->getInitialQuantity();
    levelData_[order->getPrice()].orderCnt++;
}

bool Orderbook::canBeFullyFilled(price_t price, quantity_t quantity, Side side) const {
    if (quantity <= 0)
        throw std::logic_error("Invalid quantity");
    if (!doesCrossSpread(price, side))
        return false;

    price_t treshold;
    // respectively ask_ or bid_ are not empty - checked in doesCrossSpread
    // function
    if (side == Side::Sell) {
        const auto& [bidPrice, _] = *bid_.begin();
        treshold = bidPrice;
    }
    else if (side == Side::Buy) {
        const auto& [askPrice, _] = *ask_.begin();
        treshold = askPrice;
    }

    for (const auto& [levelPrice, levelData] : levelData_) {
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

bool Orderbook::doesCrossSpread(price_t price, Side side) const {
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

void Orderbook::addAtOrderPrice(orderPtr_t order) {
    if (order->getSide() == Side::Sell)
        ask_[order->getPrice()].push_back(order);
    else if (order->getSide() == Side::Buy)
        bid_[order->getPrice()].push_back(order);
    else
        throw std::logic_error("Invalid side");
}

std::vector<LevelView> Orderbook::fullDepth(Side side) const {
    bool isAsk = side == Side::Sell;
    std::vector<LevelView> levels;
    levels.reserve((isAsk ? ask_.size() : bid_.size()));

    auto beg = (isAsk ? ask_.begin() : bid_.begin());
    auto end = (isAsk ? ask_.end() : bid_.end());

    for (auto it = beg; it != end; it++) {
        const auto& [price, _] = *it;
        const LevelData& data = levelData_.at(price);

        LevelView level;
        level.price = price;
        level.volume = data.volume;
        level.orderCnt = data.orderCnt;

        levels.push_back(level);
    }

    return levels;
}

orderPtr_t Orderbook::newOrder(quantity_t quantity, price_t price, OrderType type, Side side) {
    lastOrderId_++;
    return std::make_shared<Order>(lastOrderId_, quantity, price, type, side, getCurrTime());
}

// PUBLIC FUNCTION IMPLEMENTATIONS
std::pair<orderId_t, trades_t> Orderbook::addOrder(quantity_t quantity, price_t price, OrderType type, Side side) {
    orderPtr_t order = newOrder(quantity, price, type, side);

    if (type == OrderType::FillAndKill) {
        if (!doesCrossSpread(order->getPrice(), order->getSide()))
            return {};
    }
    else if (type == OrderType::FillOrKill) {
        if (!canBeFullyFilled(order->getPrice(), order->getInitialQuantity(), order->getSide()))
            return {};
    }
    return {order->getOrderId(), matchOrder(order)};
}

#include <iostream>

void Orderbook::cancelOrder(orderId_t orderId) {
    for (auto& orderInfo : orders_) {
        std::cout << orderInfo.second.order_->getOrderId() << std::endl;
    }
    OrderInfo orderInfo = orders_.at(orderId);
    orders_.erase(orderId);

    orderPtr_t order = orderInfo.order_;
    price_t price = order->getPrice();

    if (order->getSide() == Side::Sell) {
        ask_[price].erase(orderInfo.location_);
        if (ask_[price].empty())
            ask_.erase(price);
    }
    else if (order->getSide() == Side::Buy) {
        bid_[price].erase(orderInfo.location_);
        if (bid_[price].empty())
            bid_.erase(price);
    }

    levelData_[price].volume -= order->getRemainingQuantity();
    levelData_[price].orderCnt--;
    if (!levelData_[price].orderCnt)
        levelData_.erase(price);
}

std::pair<orderId_t, trades_t> Orderbook::modifyOrder(orderId_t orderId, ModifyOrder modifications) {
    orderPtr_t oldOrder = orders_.at(orderId).order_;

    quantity_t quantity =
        modifications.quantity.has_value() ? modifications.quantity.value() : oldOrder->getRemainingQuantity();
    price_t price = modifications.price.has_value() ? modifications.price.value() : oldOrder->getPrice();
    OrderType type = modifications.type.has_value() ? modifications.type.value() : oldOrder->getType();
    Side side = modifications.side.has_value() ? modifications.side.value() : oldOrder->getSide();

    cancelOrder(orderId);
    return addOrder(quantity, price, type, side);
}

std::optional<price_t> Orderbook::bestAsk() const {
    if (ask_.empty())
        return {};

    auto& [price, orders] = *ask_.begin();
    return price;
}

std::optional<price_t> Orderbook::bestBid() const {
    if (bid_.empty())
        return {};

    auto& [price, orders] = *bid_.begin();
    return price;
}
