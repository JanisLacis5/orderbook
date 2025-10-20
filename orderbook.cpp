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

void OrderBook::processAddedOrder(orderPtr_t order) {  // TODO: implement
    return;
}

bool OrderBook::canBeFullyFilled(orderPtr_t order) const {  // TODO: implement
    return false;
}

bool OrderBook::doesCrossSpread(orderPtr_t order) const {
    if (order->getSide() == Side::Sell) {
        const auto& [bestBid, _] = *bid_.begin();
        return order->getPrice() <= bestBid;
    }
    else if (order->getSide() == Side::Buy) {
        const auto& [bestAsk, _] = *ask_.begin();
        return order->getPrice() >= bestAsk;
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
        if (!doesCrossSpread(order))
            return {};
    }
    else if (type == OrderType::FillOrKill) {
        if (!canBeFullyFilled(order) || !doesCrossSpread(order))
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
    return;
}

orderPtr_t OrderBook::modifyOrder(orderPtr_t order, ModifyOrder modifications) {  // TODO: implement

}
