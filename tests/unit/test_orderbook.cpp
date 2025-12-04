#include <gtest/gtest.h>
#include "order.h"
#include "orderbook.h"
#include "usings.h"

class OrderbookTest : public testing::Test {
protected:
    Orderbook orderbook;

    void assertBookState() const {
        std::optional<price_t> bestBid = orderbook.bestBid();
        std::optional<price_t> bestAsk = orderbook.bestAsk();
        if (bestBid.has_value() && bestAsk.has_value())
            EXPECT_TRUE(bestBid.value() < bestAsk.value());
    }
    void populateBook(size_t bids=100, size_t asks=100) {}
    orderPtr_t generateOrder(price_t price, OrderType type, Side side, quantity_t quantity=1) {
        orderPtr_t order = std::make_shared<Order>(lastOrderId_, quantity, price, type, side, now_);
        lastOrderId_++;
        now_++;

        return order;
    }

    void TearDown() override { assertBookState(); }

private:
    orderId_t lastOrderId_{0};
    microsec_t now_{0};
};

class PassiveOrderbookTest : public OrderbookTest {};
class MarketOrderbookTest : public OrderbookTest {};

// PASSIVE ORDERS
TEST_F(PassiveOrderbookTest, InitialState) {
    EXPECT_FALSE(orderbook.bestAsk().has_value());
    EXPECT_FALSE(orderbook.bestBid().has_value());
    EXPECT_TRUE(orderbook.fullDepthAsk().empty());
    EXPECT_TRUE(orderbook.fullDepthBid().empty());
}
TEST_F(PassiveOrderbookTest, OneBidOnEmptyBook) {
    price_t price = 100;
    quantity_t quantity = 1;
    orderPtr_t order = generateOrder(price, OrderType::GoodTillCancel, Side::Buy, quantity);
    orderbook.addOrder(order);

    ASSERT_TRUE(orderbook.bestBid().has_value());
    EXPECT_FALSE(orderbook.bestAsk().has_value());
    EXPECT_EQ(orderbook.bestBid().value(), price);
    ASSERT_EQ(orderbook.fullDepthBid().size(), 1);
    EXPECT_TRUE(orderbook.fullDepthAsk().empty());
    LevelView level = orderbook.fullDepthBid().front();
    EXPECT_EQ(level.price, price);
    EXPECT_EQ(level.volume, quantity);
    EXPECT_EQ(level.orderCnt, 1);
}
TEST_F(PassiveOrderbookTest, OneAskOnEmptyBook) {
    price_t price = 100;
    quantity_t quantity = 1;
    orderPtr_t order = generateOrder(price, OrderType::GoodTillCancel, Side::Sell, quantity);
    orderbook.addOrder(order);

    ASSERT_TRUE(orderbook.bestAsk().has_value());
    EXPECT_FALSE(orderbook.bestBid().has_value());
    EXPECT_EQ(orderbook.bestAsk().value(), price);
    ASSERT_EQ(orderbook.fullDepthAsk().size(), 1);
    EXPECT_TRUE(orderbook.fullDepthBid().empty());
    LevelView level = orderbook.fullDepthAsk().front();
    EXPECT_EQ(level.price, price);
    EXPECT_EQ(level.volume, quantity);
    EXPECT_EQ(level.orderCnt, 1);
}
TEST_F(PassiveOrderbookTest, FIFOOnTheSameLevel) {}
TEST_F(PassiveOrderbookTest, BookStateWithMultipleOrders) {}
TEST_F(PassiveOrderbookTest, CancelExistentBidOrder) {}
TEST_F(PassiveOrderbookTest, CancelExistentAskOrder) {}
TEST_F(PassiveOrderbookTest, CancelNonExistentOrder) {}
TEST_F(PassiveOrderbookTest, CancelTheOnlyOrderAtLevel) {}
TEST_F(PassiveOrderbookTest, ModifyQuantityUp) {}
TEST_F(PassiveOrderbookTest, ModifyQuantityDown) {}
TEST_F(PassiveOrderbookTest, ModifyPriceMovesToDifferentLevel) {}
TEST_F(PassiveOrderbookTest, LimitOrderFullyFilledWithoutResting) {}
TEST_F(PassiveOrderbookTest, LimitOrderDoesNotTradeAtWorsePriceThanLimit) {}
TEST_F(PassiveOrderbookTest, LimitOrderRestsOnTheBookIfDoesntCrossSpread) {}
TEST_F(PassiveOrderbookTest, LimitOrderPartialFillRestStaysOnBook) {}
TEST_F(PassiveOrderbookTest, LimitOrderSweepsAllLiquidityFullFill) {}
TEST_F(PassiveOrderbookTest, LimitOrderSweepsAllLiquidityPartialFillRestStays) {}

// MARKET ORDERS
TEST_F(MarketOrderbookTest, NoLiquidity) {}
TEST_F(MarketOrderbookTest, FullFillSingleLevel) {}
TEST_F(MarketOrderbookTest, PartialFillSingleLevel) {}
TEST_F(MarketOrderbookTest, FullFillMulitpleLevels) {}
TEST_F(MarketOrderbookTest, PartialFillMultipleLevels) {}
TEST_F(MarketOrderbookTest, SweepAllBookFullFill) {}
TEST_F(MarketOrderbookTest, SweepAllBookPartialFill) {}
TEST_F(MarketOrderbookTest, FIFOFirstFilled) {}

