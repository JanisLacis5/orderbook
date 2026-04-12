#pragma once

#include "types.h"
#include <list>
#include <stdexcept>

class Order
{
public:
    Order(orderId_t orderid, quantity_t quantity, price_t price, OrderType type, Side side, microsec_t opentime)
        : orderid_{orderid}
        , initialQuantity_{quantity}
        , remainingQuantity_{quantity}
        , price_{price}
        , type_{type}
        , side_{side}
        , opentime_{opentime}
    {
        if (quantity <= 0 || quantity == badValues::quantity)
            throw std::invalid_argument("bad quantity");
        if (type == OrderType::Bad)
            throw std::invalid_argument("order has to have a type");
        if (price == badValues::price)
            throw std::invalid_argument("bad price");
        if (side == Side::Bad)
            throw std::invalid_argument("bad side");
    }

    orderId_t getOrderId() const { return orderid_; }
    quantity_t getInitialQuantity() const { return initialQuantity_; }
    quantity_t getRemainingQuantity() const { return remainingQuantity_; }
    price_t getPrice() const { return price_; }
    OrderType getType() const { return type_; }
    Side getSide() const { return side_; }
    microsec_t getOpenTime() const { return opentime_; }

    quantity_t getFilled() const { return initialQuantity_ - remainingQuantity_; }
    bool isFullyFilled() const { return remainingQuantity_ == 0; }
    void fill(quantity_t quantity)
    {
        if (quantity > remainingQuantity_)
            throw std::invalid_argument("fill: invalid quantities");

        remainingQuantity_ -= quantity;
    }

private:
    orderId_t orderid_;
    quantity_t initialQuantity_;
    quantity_t remainingQuantity_;
    price_t price_;
    OrderType type_;
    Side side_;
    microsec_t opentime_;
};

using orderPtr_t = std::shared_ptr<Order>;
using orderPtrs_t = std::list<orderPtr_t>;
