#include <gtest/gtest.h>
#include <numeric>
#include <stdexcept>
#include "order.h"
#include "orderbook.h"
#include "usings.h"

struct SideState {
    uint32_t orderCnt{0};
    uint32_t volume{0};
    uint32_t depth{0};
    std::optional<price_t> bestPrice{};

    bool operator==(const SideState& other) const {
        return orderCnt == other.orderCnt && volume == other.volume && depth == other.depth && bestPrice == other.bestPrice;
    }
};

struct BookState {
    SideState ask{};
    SideState bid{};

    bool operator==(const BookState& other) const {
        return ask == other.ask && bid == other.bid;
    }
};

struct LevelState {
    price_t price{0};
    uint32_t volume{0};
    uint32_t orderCnt{0};

    bool operator==(const LevelView& other) const {
        return price == other.price && volume == other.volume && orderCnt == other.orderCnt;
    }
};

struct TradeState {
    orderPtr_t seller;
    orderPtr_t buyer;
    quantity_t quantity;

    bool operator==(const Trade& other) const {
        return seller == other.seller && buyer == other.buyer && quantity == other.quantity;
    }
};

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
    void assertBookState(BookState& expecetedState) const {
        std::vector<LevelView> bidDepth = orderbook.fullDepthBid();
        std::vector<LevelView> askDepth = orderbook.fullDepthAsk();
        auto cntVolume = [](size_t cnt, const LevelView& level) { return cnt + level.volume; };
        auto cntOrders = [](size_t cnt, const LevelView& level) { return cnt + level.orderCnt; };

        BookState actualState {
            .ask = {
                .orderCnt = std::accumulate(askDepth.begin(), askDepth.end(), 0u, cntOrders),
                .volume = std::accumulate(askDepth.begin(), askDepth.end(), 0u, cntVolume),
                .depth = static_cast<uint32_t>(askDepth.size()),
                .bestPrice = orderbook.bestAsk(),
            },
            .bid = {
                .orderCnt = std::accumulate(bidDepth.begin(), bidDepth.end(), 0u, cntOrders),
                .volume = std::accumulate(bidDepth.begin(), bidDepth.end(), 0u, cntVolume),
                .depth = static_cast<uint32_t>(bidDepth.size()),
                .bestPrice = orderbook.bestBid(),
            },
        };

        EXPECT_EQ(expecetedState, actualState);
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

    BookState expectedState {
        .bid {
            .orderCnt = 1,
            .volume = quantity,
            .depth = 1,
            .bestPrice = price,
        },
    };
    assertBookState(expectedState);

    LevelState expectedLevelState = {.price = price, .volume = quantity, .orderCnt = 1};
    LevelView actualLevelState = orderbook.fullDepthBid().front();
    EXPECT_EQ(actualLevelState, expectedLevelState);
}

TEST_F(PassiveOrderbookTest, OneAskOnEmptyBook) {
    price_t price = defaultPrice;
    quantity_t quantity = defaultQuantity;
    orderPtr_t order = generateOrder(price, OrderType::GoodTillCancel, Side::Sell, quantity);
    orderbook.addOrder(order);

    BookState expectedState {
        .ask {
            .orderCnt = 1,
            .volume = quantity,
            .depth = 1,
            .bestPrice = price,
        },
    };
    assertBookState(expectedState);

    LevelState expectedLevelState {.price = price, .volume = quantity, .orderCnt = 1};
    EXPECT_EQ(expectedLevelState, orderbook.fullDepthAsk().front());
}

TEST_F(PassiveOrderbookTest, FIFOOnTheSameLevel) {
    price_t price = defaultPrice;
    quantity_t quantity = defaultQuantity;
    orderPtr_t order1 = generateOrder(price, OrderType::GoodTillCancel, Side::Buy, quantity);
    orderPtr_t order2 = generateOrder(price, OrderType::GoodTillCancel, Side::Buy, quantity);
    
    orderbook.addOrder(order1);
    orderbook.addOrder(order2);

    orderPtr_t oppositeOrder = generateOrder(price, OrderType::Market, Side::Sell, quantity);
    trades_t trades = orderbook.addOrder(oppositeOrder);

    EXPECT_TRUE(order1->isFullyFilled());
    EXPECT_TRUE(oppositeOrder->isFullyFilled());
    EXPECT_FALSE(order2->isFullyFilled());

    ASSERT_EQ(trades.size(), 1);
    Trade& trade = trades[0];
    TradeState expectedTrade {.seller = oppositeOrder, .buyer = order1, .quantity = quantity};
    EXPECT_EQ(trade, expectedTrade);

    BookState expectedBookState {
        .bid {
            .orderCnt = 1,
            .volume = quantity,
            .depth = 1,
            .bestPrice = price,
        },
    };
    assertBookState(expectedBookState);

    LevelState expectedLevelState {.price = price, .volume = quantity, .orderCnt = 1};
    std::vector<LevelView> levels = orderbook.fullDepthBid();
    ASSERT_EQ(levels.size(), 1);
    EXPECT_EQ(expectedLevelState, levels.front());
}

TEST_F(PassiveOrderbookTest, BookStateWithMultipleOrders) {
    price_t price1 = defaultPrice;
    price_t price2 = defaultPrice + 1;
    price_t price3 = defaultPrice + 2;
    price_t price4 = defaultPrice + 3;
    quantity_t q1 = defaultQuantity;
    quantity_t q2 = defaultQuantity + 1;
    quantity_t q3 = defaultQuantity + 2;
    quantity_t q4 = defaultQuantity + 3;
    orderPtr_t order1 = generateOrder(price1, OrderType::GoodTillCancel, Side::Buy, q1);
    orderPtr_t order2 = generateOrder(price2, OrderType::GoodTillCancel, Side::Buy, q2);
    orderPtr_t order3 = generateOrder(price3, OrderType::GoodTillCancel, Side::Sell, q3);
    orderPtr_t order4 = generateOrder(price4, OrderType::GoodTillCancel, Side::Sell, q4);

    // Make sure that none of these orders are filled - they stay on the book
    ASSERT_TRUE(orderbook.addOrder(order1).empty()); 
    ASSERT_TRUE(orderbook.addOrder(order2).empty()); 
    ASSERT_TRUE(orderbook.addOrder(order3).empty());
    ASSERT_TRUE(orderbook.addOrder(order4).empty());

    BookState expectedBookState {
        .ask {.orderCnt = 2, .volume = q3 + q4, .depth = 2, .bestPrice = std::min(price3, price4)},
        .bid {.orderCnt = 2, .volume = q1 + q2, .depth = 2, .bestPrice = std::max(price1, price2)},
    };
    assertBookState(expectedBookState);
    
    LevelState expLevel1 {.price = price1, .volume = q1, .orderCnt = 1};
    LevelState expLevel2 {.price = price2, .volume = q2, .orderCnt = 1};
    LevelState expLevel3 {.price = price3, .volume = q3, .orderCnt = 1};
    LevelState expLevel4 {.price = price4, .volume = q4, .orderCnt = 1};
    std::vector<LevelView> bidDepth = orderbook.fullDepthBid();
    std::vector<LevelView> askDepth = orderbook.fullDepthAsk();

    ASSERT_EQ(bidDepth.size(), 2);
    ASSERT_EQ(askDepth.size(), 2);
    // Bids are expected to be in descending order (largest (bestBid) price as the first level)
    EXPECT_EQ(expLevel2, bidDepth[0]);
    EXPECT_EQ(expLevel1, bidDepth[1]);
    // Asks are ascending (lowest (bestAsk) price first level)
    EXPECT_EQ(expLevel3, askDepth[0]);
    EXPECT_EQ(expLevel4, askDepth[1]);
}

TEST_F(PassiveOrderbookTest, CancelExistentBidOrder) {
    price_t price = defaultPrice;
    quantity_t q = defaultQuantity;
    orderPtr_t bidOrderToCancel = generateOrder(price, OrderType::GoodTillCancel, Side::Buy, q);
    ASSERT_TRUE(orderbook.addOrder(bidOrderToCancel).empty());

    BookState expectedBookState {
        .bid {.orderCnt = 1, .volume = q, .depth = 1, .bestPrice = price},
    };
    assertBookState(expectedBookState);

    LevelState expLevel {.price = price, .volume = q, .orderCnt = 1};
    std::vector<LevelView> bidDepth = orderbook.fullDepthBid();
    
    ASSERT_EQ(bidDepth.size(), 1);
    EXPECT_TRUE(orderbook.fullDepthAsk().empty());
    EXPECT_EQ(expLevel, bidDepth[0]);
    ASSERT_TRUE(orderbook.bestBid().has_value());
    EXPECT_FALSE(orderbook.bestAsk().has_value());
    EXPECT_EQ(orderbook.bestBid().value(), price);

    orderbook.cancelOrder(bidOrderToCancel->getOrderId());
    expectedBookState = BookState{};
    assertBookState(expectedBookState);

    EXPECT_TRUE(orderbook.fullDepthAsk().empty());
    EXPECT_TRUE(orderbook.fullDepthBid().empty());
    EXPECT_FALSE(orderbook.bestAsk().has_value());
    EXPECT_FALSE(orderbook.bestBid().has_value());
    EXPECT_FALSE(bidOrderToCancel->isFullyFilled());
    EXPECT_EQ(bidOrderToCancel->getRemainingQuantity(), bidOrderToCancel->getInitialQuantity());
}

TEST_F(PassiveOrderbookTest, CancelExistentAskOrder) {
    price_t price = defaultPrice;
    quantity_t q = defaultQuantity;
    orderPtr_t askOrderToCancel = generateOrder(price, OrderType::GoodTillCancel, Side::Sell, q);
    ASSERT_TRUE(orderbook.addOrder(askOrderToCancel).empty());

    BookState expectedBookState {
        .ask {.orderCnt = 1, .volume = q, .depth = 1, .bestPrice = price},
    };
    assertBookState(expectedBookState);

    LevelState expLevel {.price = price, .volume = q, .orderCnt = 1};
    std::vector<LevelView> askDepth = orderbook.fullDepthAsk();
    
    ASSERT_EQ(askDepth.size(), 1);
    EXPECT_TRUE(orderbook.fullDepthBid().empty());
    EXPECT_EQ(expLevel, askDepth[0]);
    ASSERT_TRUE(orderbook.bestAsk().has_value());
    EXPECT_FALSE(orderbook.bestBid().has_value());
    EXPECT_EQ(orderbook.bestAsk().value(), price);

    orderbook.cancelOrder(askOrderToCancel->getOrderId());
    expectedBookState = BookState{};
    assertBookState(expectedBookState);

    EXPECT_TRUE(orderbook.fullDepthAsk().empty());
    EXPECT_TRUE(orderbook.fullDepthBid().empty());
    EXPECT_FALSE(orderbook.bestAsk().has_value());
    EXPECT_FALSE(orderbook.bestBid().has_value());
    EXPECT_FALSE(askOrderToCancel->isFullyFilled());
    EXPECT_EQ(askOrderToCancel->getRemainingQuantity(), askOrderToCancel->getInitialQuantity());
}

TEST_F(PassiveOrderbookTest, CancelNonExistentOrder) {
    ASSERT_TRUE(orderbook.fullDepthAsk().empty());
    ASSERT_TRUE(orderbook.fullDepthBid().empty());
    EXPECT_THROW(orderbook.cancelOrder(100), std::out_of_range);
}

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

