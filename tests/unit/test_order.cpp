#include <gtest/gtest.h>
#include <cassert>
#include "order.h"
#include "types.h"
#include "usings.h"

constexpr microsec_t NOW = microsec_t{67};

class OrderTest : public testing::Test {
protected:
    OrderTest() {};
    
    orderId_t orderid = 1;
    quantity_t quantity = 100;
    price_t price = 12345;
    OrderType type = OrderType::GoodTillCancel;
    Side side = Side::Sell;
    Order order = Order(orderid, quantity, price, type, side, NOW);
};

TEST_F(OrderTest, Constructor) {
    EXPECT_EQ(order.getOrderid(), 1);
    EXPECT_EQ(order.getInitialQuantity(), 100);
    EXPECT_EQ(order.getRemainingQuantity(), 100);
    EXPECT_EQ(order.getPrice(), 12345);
    EXPECT_EQ(order.getType(), OrderType::GoodTillCancel);
    EXPECT_EQ(order.getSide(), Side::Sell);
    EXPECT_EQ(order.getOpenTime(), NOW);
}

TEST_F(OrderTest, FillFull) {
    order.fill(quantity);

    ASSERT_EQ(order.getInitialQuantity(), quantity);
    EXPECT_EQ(order.getRemainingQuantity(), 0);
}
