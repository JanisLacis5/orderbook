#include <gtest/gtest.h>
#include <cassert>
#include "order.h"
#include "types.h"
#include "usings.h"

TEST(Order, Constructor) {
    auto now = microsec_t{67};
    Order* order = new Order(1, 100, 12345, OrderType::GoodTillCancel, Side::Sell, now);

    EXPECT_EQ(order->getOrderid(), 1);
    EXPECT_EQ(order->getInitialQuantity(), 100);
    EXPECT_EQ(order->getRemainingQuantity(), 100);
    EXPECT_EQ(order->getPrice(), 12345);
    EXPECT_EQ(order->getType(), OrderType::GoodTillCancel);
    EXPECT_EQ(order->getSide(), Side::Sell);
    EXPECT_EQ(order->getOpenTime(), now);
}
