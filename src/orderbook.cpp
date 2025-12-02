#include "orderbook.h"
#include <memory>
#include <optional>
#include <stdexcept>

// PRIVATE FUNCTION IMPLEMENTATIONS
trades_t Orderbook::matchOrder(orderPtr_t order) {
    Side side = order->getSide();
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

            Trade trade = (side == Side::Buy ? newTrade(order, opposite, toFill) : newTrade(opposite, order, toFill));
            trades.push_back(trade);

            opposite->fill(toFill);
            order->fill(toFill);
            levelData_[currPrice].volume -= toFill;
            if (opposite->isFullyFilled()) {
                levelData_[currPrice].orderCnt--;
                orders.pop_front();
            }

            if (levelData_.at(currPrice).orderCnt == 0)
                levelData_.erase(currPrice);
        }

        if (orders.empty())
            it = side == Side::Buy ? ask_.erase(it) : bid_.erase(it);
    }

    // For orders that are fine to rest on the book, fill orders that have a valid price and leave the rest on the book
    if (!order->isFullyFilled() &&
        (order->getType() == OrderType::GoodTillCancel || order->getType() == OrderType::GoodTillEOD)) {
        addAtOrderPrice(order);
        processAddedOrder(order);
    }
    return trades;
}

microsec_t Orderbook::getCurrTime() const {
    auto time = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<microsec_t>(time);
}

void Orderbook::processAddedOrder(orderPtr_t order) {
    orderId_t orderId = order->getOrderid();

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
    // respectively ask_ or bid_ are not empty - checked in doesCrossSpread function
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

// PUBLIC FUNCTION IMPLEMENTATIONS
trades_t Orderbook::addOrder(orderPtr_t order) {
    OrderType type = order->getType();

    if (type == OrderType::FillAndKill) {
        if (!doesCrossSpread(order->getPrice(), order->getSide()))
            return {};
    }
    else if (type == OrderType::FillOrKill) {
        if (!canBeFullyFilled(order->getPrice(), order->getInitialQuantity(), order->getSide()))
            return {};
    }
    return matchOrder(order);
}

void Orderbook::cancelOrder(orderId_t orderId) {
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

trades_t Orderbook::modifyOrder(orderPtr_t order, ModifyOrder modifications) {
    cancelOrder(order->getOrderid());

    std::shared_ptr<Order> newOrder =
        std::make_shared<Order>(order->getOrderid(), modifications.quantity, modifications.price, modifications.type,
                                modifications.side, getCurrTime());
    return addOrder(newOrder);
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

std::map<price_t, Orderbook::LevelData, std::less<price_t>> Orderbook::fullDepthAsk() const {
    std::map<price_t, LevelData, std::less<price_t>> levels;

    for (const auto& [price, _]: ask_) {
        const LevelData& data = levelData_.at(price);
        levels.emplace(price, data);
    }

    return levels;
}

std::map<price_t, Orderbook::LevelData, std::greater<price_t>> Orderbook::fullDepthBid() const {
    std::map<price_t, LevelData, std::greater<price_t>> levels;

    for (const auto& [price, _]: bid_) {
        const LevelData& data = levelData_.at(price);
        levels.emplace(price, data);
    }

    return levels;
}
