#include <numeric>
#include "orderbook.h"


// PRIVATE FUNCTION IMPLEMENTATIONS
trades_t OrderBook::passiveMatchOrders() { // TODO: implement
    return {};
}

trades_t OrderBook::aggressiveMatchOrder(orderPtr_t order) {
    quantity_t quantity = order->getRemainingQuantity();
    trades_t trades;

    if (order->getSide() == Side::Buy) {
        price_t ceilPrice = order->getType() == OrderType::Market ? INT32_MAX : order->getPrice();

        auto askIt = ask_.begin();
        while (askIt != ask_.end() && order->getRemainingQuantity() > 0) {
            auto& [currPrice, orders] = *askIt;
            if (currPrice > ceilPrice)
                break;

            while (!orders.empty() && order->getRemainingQuantity() > 0) {
                orderPtr_t seller = orders.front();
                quantity_t toFill = std::min(seller->getRemainingQuantity(), order->getRemainingQuantity());

                order->fill(toFill);
                seller->fill(toFill);
                levelData_[currPrice].volume -= toFill;
                levelData_[currPrice].orderCnt--;

                Trade trade = newTrade(order, seller, toFill);
                trades.push_back(trade);
                if (seller->getRemainingQuantity() == 0)
                    orders.pop_front();
            }

            if (orders.empty())
                ask_.erase(askIt);
            if (levelData_[currPrice].orderCnt == 0)
                levelData_.erase(currPrice);
            askIt++;
        }
    }
    else if (order->getSide() == Side::Sell) {
        price_t floorPrice = order->getType() == OrderType::Market ? INT32_MIN : order->getPrice();

        auto bidIt = bid_.begin();
        while (bidIt != bid_.end() && order->getRemainingQuantity() > 0) {
            auto& [currPrice, orders] = *bidIt;
            if (currPrice < floorPrice)
                break;

            while (!orders.empty() && order->getRemainingQuantity() > 0) {
                orderPtr_t buyer = orders.front();
                quantity_t toFill = std::min(order->getRemainingQuantity(), buyer->getRemainingQuantity());

                order->fill(toFill);
                buyer->fill(toFill);
                levelData_[currPrice].volume -= toFill;
                levelData_[currPrice].orderCnt--;

                Trade trade = newTrade(buyer, order, toFill);
                trades.push_back(trade);
                if (buyer->getRemainingQuantity() == 0)
                    orders.pop_front();
            }

            if (orders.empty())
                bid_.erase(bidIt);
            if (levelData_[currPrice].orderCnt == 0)
                levelData_.erase(currPrice);
            bidIt++;
        }
    }

    return trades;
}

microsec_t OrderBook::getCurrTime() const {
    auto time = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<microsec_t>(time);
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

void OrderBook::cancelOrder(orderId_t orderId) {
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

trades_t OrderBook::modifyOrder(orderPtr_t order, ModifyOrder modifications) {
    cancelOrder(order->getOrderid());

    std::shared_ptr<Order> newOrder = std::make_shared<Order>(
        order->getOrderid(),
        modifications.quantity,
        modifications.price,
        modifications.type,
        modifications.side,
        getCurrTime()
    );
    return addOrder(newOrder);
}
