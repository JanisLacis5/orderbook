#include <gtest/gtest.h>
#include <cassert>
#include <stdexcept>
#include "order.h"
#include "types.h"
#include "usings.h"

constexpr microsec_t NOW = microsec_t{67};

class OrderTest : public testing::Test {
protected:
    orderId_t orderid = 1;
    quantity_t quantity = 100;
    price_t price = 12345;
    OrderType type = OrderType::GoodTillCancel;
    Side side = Side::Sell;
    Order order = Order(orderid, quantity, price, type, side, NOW);
};

TEST_F(OrderTest, ConstructorInvalidQuantity) {
    EXPECT_NO_THROW(Order(orderid, quantity, price, type, side, NOW));
    EXPECT_THROW(Order(orderid, -quantity, price, type, side, NOW), std::invalid_argument);
    EXPECT_THROW(Order(orderid, 0, price, type, side, NOW), std::invalid_argument);
}

TEST_F(OrderTest, Constructor) {
    EXPECT_EQ(order.getOrderid(), 1);
    EXPECT_EQ(order.getInitialQuantity(), 100);
    EXPECT_EQ(order.getRemainingQuantity(), 100);
    EXPECT_EQ(order.getPrice(), 12345);
    EXPECT_EQ(order.getType(), OrderType::GoodTillCancel);
    EXPECT_EQ(order.getSide(), Side::Sell);
    EXPECT_EQ(order.getOpenTime(), NOW);
}

TEST_F(OrderTest, InvalidFill) {
    EXPECT_NO_THROW(order.fill(quantity));
    EXPECT_THROW(order.fill(-quantity), std::invalid_argument);
    EXPECT_THROW(order.fill(0), std::invalid_argument);
}

TEST_F(OrderTest, FillFull) {
    order.fill(quantity);

    EXPECT_EQ(order.getRemainingQuantity(), 0);
    EXPECT_EQ(order.getFilled(), quantity);
    EXPECT_TRUE(order.isFullyFilled());
}

TEST_F(OrderTest, PartialFill) {
    order.fill(quantity / 2);

    EXPECT_EQ(order.getRemainingQuantity(), quantity - quantity / 2);
    EXPECT_EQ(order.getFilled(), quantity / 2);
    EXPECT_FALSE(order.isFullyFilled());
}

