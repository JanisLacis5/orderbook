#include <gtest/gtest.h>
#include <numeric>
#include "order.h"
#include "orderbook.h"
#include "usings.h"

class OrderbookTest : public testing::Test {
protected:
    Orderbook orderbook;
    price_t defaultPrice{100};
    quantity_t defaultQuantity{1};

    void assertBookHealthy() const {
        std::optional<price_t> bestBid = orderbook.bestBid();
        std::optional<price_t> bestAsk = orderbook.bestAsk();
        if (bestBid.has_value() && bestAsk.has_value())
            EXPECT_TRUE(bestBid.value() < bestAsk.value());
    }
    void assertBookState(size_t bidOrderCnt, 
                         size_t askOrderCnt, 
                         size_t bidVolume,
                         size_t askVolume,
                         size_t bidDepthSize, 
                         size_t askDepthSize, 
                         std::optional<price_t> bestBid, 
                         std::optional<price_t> bestAsk) const {
        std::optional<price_t> obBestBid = orderbook.bestBid();
        std::optional<price_t> obBestAsk = orderbook.bestAsk();
        std::vector<LevelView> bidDepth = orderbook.fullDepthBid();
        std::vector<LevelView> askDepth = orderbook.fullDepthAsk();
        auto cntVolume = [](size_t cnt, const LevelView& level) { return cnt + level.volume; };
        auto cntOrders = [](size_t cnt, const LevelView& level) { return cnt + level.orderCnt; };
        size_t bidCnt = std::accumulate(bidDepth.begin(), bidDepth.end(), 0, cntOrders); 
        size_t askCnt = std::accumulate(askDepth.begin(), askDepth.end(), 0, cntOrders);
        size_t obBidVolume = std::accumulate(bidDepth.begin(), bidDepth.end(), 0, cntVolume); 
        size_t obAskVolume = std::accumulate(askDepth.begin(), askDepth.end(), 0, cntVolume); 

        ASSERT_EQ(obBestBid.has_value(), bestBid.has_value());
        ASSERT_EQ(obBestAsk.has_value(), bestAsk.has_value());
        if (obBestBid.has_value())
            EXPECT_EQ(obBestBid.value(), bestBid.value());
        if (obBestAsk.has_value())
            EXPECT_EQ(obBestAsk.value(), bestAsk.value());
        EXPECT_EQ(bidDepth.size(), bidDepthSize);
        EXPECT_EQ(askDepth.size(), askDepthSize);
        EXPECT_EQ(bidCnt, bidOrderCnt);
        EXPECT_EQ(askCnt, askOrderCnt);
        EXPECT_EQ(obBidVolume, bidVolume);
        EXPECT_EQ(obAskVolume, askVolume);
    }
    void populateBook(size_t bids, size_t asks) {}
    orderPtr_t generateOrder(price_t price, OrderType type, Side side, quantity_t quantity) {
        orderPtr_t order = std::make_shared<Order>(lastOrderId_, quantity, price, type, side, now_);
        lastOrderId_++;
        now_++;

        return order;
    }

    void TearDown() override { assertBookHealthy(); }

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
    price_t price = defaultPrice;
    quantity_t quantity = defaultQuantity;
    orderPtr_t order = generateOrder(price, OrderType::GoodTillCancel, Side::Buy, quantity);
    orderbook.addOrder(order);

    assertBookState(1, 0, 1, 0, 1, 0, 100, std::optional<price_t>{});
    LevelView level = orderbook.fullDepthBid().front();
    EXPECT_EQ(level.price, price);
    EXPECT_EQ(level.volume, quantity);
    EXPECT_EQ(level.orderCnt, 1);
}
TEST_F(PassiveOrderbookTest, OneAskOnEmptyBook) {
    price_t price = defaultPrice;
    quantity_t quantity = defaultQuantity;
    orderPtr_t order = generateOrder(price, OrderType::GoodTillCancel, Side::Sell, quantity);
    orderbook.addOrder(order);

    assertBookState(0, 1, 0, 1, 0, 1, std::optional<price_t>{}, 100);
    LevelView level = orderbook.fullDepthAsk().front();
    EXPECT_EQ(level.price, price);
    EXPECT_EQ(level.volume, quantity);
    EXPECT_EQ(level.orderCnt, 1);
}
TEST_F(PassiveOrderbookTest, FIFOOnTheSameLevel) {
    price_t price = defaultPrice;
    quantity_t quantity = defaultQuantity;
    orderPtr_t order1 = generateOrder(price, OrderType::GoodTillCancel, Side::Buy, quantity);
    orderPtr_t order2 = generateOrder(price, OrderType::GoodTillCancel, Side::Buy, quantity);
}
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

