#include <gtest/gtest.h>
#include <numeric>
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

