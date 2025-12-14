#include <gtest/gtest.h>
#include <numeric>
#include <random>
#include <stdexcept>
#include "orderbook.h"
#include "types.h"
#include "usings.h"

struct SideState {
    uint32_t orderCnt{0};
    uint32_t volume{0};
    uint32_t depth{0};
    std::optional<price_t> bestPrice{};

    bool operator==(const SideState& other) const {
        return orderCnt == other.orderCnt && volume == other.volume && depth == other.depth &&
               bestPrice == other.bestPrice;
    }
};

struct BookState {
    SideState ask{};
    SideState bid{};

    bool operator==(const BookState& other) const { return ask == other.ask && bid == other.bid; }
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
    orderId_t seller;
    orderId_t buyer;
    quantity_t quantity;

    bool operator==(const Trade& other) const {
        return seller == other.seller && buyer == other.buyer && quantity == other.quantity;
    }
};

class OrderbookTest : public testing::Test {
protected:
    Orderbook orderbook;
    price_t defaultPrice{100};
    quantity_t defaultQuantity{10};
    std::random_device rr;

    int randomInt(int l, int r) {
        std::default_random_engine e1(rr());
        std::uniform_int_distribution<int> uniform_dist(l, r);
        return uniform_dist(e1);
    }

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

        BookState actualState{
            .ask =
                {
                    .orderCnt = std::accumulate(askDepth.begin(), askDepth.end(), 0u, cntOrders),
                    .volume = std::accumulate(askDepth.begin(), askDepth.end(), 0u, cntVolume),
                    .depth = static_cast<uint32_t>(askDepth.size()),
                    .bestPrice = orderbook.bestAsk(),
                },
            .bid =
                {
                    .orderCnt = std::accumulate(bidDepth.begin(), bidDepth.end(), 0u, cntOrders),
                    .volume = std::accumulate(bidDepth.begin(), bidDepth.end(), 0u, cntVolume),
                    .depth = static_cast<uint32_t>(bidDepth.size()),
                    .bestPrice = orderbook.bestBid(),
                },
        };

        EXPECT_EQ(expecetedState, actualState);
    }

    orderId_t addRestingOrder(quantity_t quantity, price_t price, OrderType type, Side side) {
        auto [orderId, trades] = orderbook.addOrder(quantity, price, type, side);
        EXPECT_TRUE(trades.empty());
        return orderId;
    }

    void populateBook(size_t bids, size_t asks, price_t centerPrice) {}
    void TearDown() override { assertBookHealthy(); }
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
    auto orderId = addRestingOrder(quantity, price, OrderType::GoodTillCancel, Side::Buy);

    BookState expectedState{
        .bid{
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
    auto orderId = addRestingOrder(quantity, price, OrderType::GoodTillCancel, Side::Sell);

    BookState expectedState{
        .ask{
            .orderCnt = 1,
            .volume = quantity,
            .depth = 1,
            .bestPrice = price,
        },
    };
    assertBookState(expectedState);

    LevelState expectedLevelState{.price = price, .volume = quantity, .orderCnt = 1};
    EXPECT_EQ(expectedLevelState, orderbook.fullDepthAsk().front());
}

TEST_F(PassiveOrderbookTest, FIFOOnTheSameLevel) {
    price_t price = defaultPrice;
    quantity_t quantity = defaultQuantity;

    auto orderId1 = addRestingOrder(quantity, price, OrderType::GoodTillCancel, Side::Buy);
    auto orderId2 = addRestingOrder(quantity, price, OrderType::GoodTillCancel, Side::Buy);
    auto [oppositeOrderId, oppositeOrderTrades] =
        orderbook.addOrder(quantity, price, OrderType::Market, Side::Sell);

    // Order1 and the opposite one should not exist anymore so they will throw
    // on cancelation
    EXPECT_THROW(orderbook.cancelOrder(orderId1), std::out_of_range);
    EXPECT_THROW(orderbook.cancelOrder(oppositeOrderId), std::out_of_range);

    ASSERT_EQ(oppositeOrderTrades.size(), 1);
    Trade& trade = oppositeOrderTrades[0];
    TradeState expectedTrade{.seller = oppositeOrderId, .buyer = orderId1, .quantity = quantity};
    EXPECT_EQ(trade, expectedTrade);

    BookState expectedBookState{
        .bid{
            .orderCnt = 1,
            .volume = quantity,
            .depth = 1,
            .bestPrice = price,
        },
    };
    assertBookState(expectedBookState);

    LevelState expectedLevelState{.price = price, .volume = quantity, .orderCnt = 1};
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
    auto orderId1 = addRestingOrder(q1, price1, OrderType::GoodTillCancel, Side::Buy);
    auto orderId2 = addRestingOrder(q2, price2, OrderType::GoodTillCancel, Side::Buy);
    auto orderId3 = addRestingOrder(q3, price3, OrderType::GoodTillCancel, Side::Sell);
    auto orderId4 = addRestingOrder(q4, price4, OrderType::GoodTillCancel, Side::Sell);

    BookState expectedBookState{
        .ask{.orderCnt = 2, .volume = q3 + q4, .depth = 2, .bestPrice = std::min(price3, price4)},
        .bid{.orderCnt = 2, .volume = q1 + q2, .depth = 2, .bestPrice = std::max(price1, price2)},
    };
    assertBookState(expectedBookState);

    LevelState expLevel1{.price = price1, .volume = q1, .orderCnt = 1};
    LevelState expLevel2{.price = price2, .volume = q2, .orderCnt = 1};
    LevelState expLevel3{.price = price3, .volume = q3, .orderCnt = 1};
    LevelState expLevel4{.price = price4, .volume = q4, .orderCnt = 1};
    std::vector<LevelView> bidDepth = orderbook.fullDepthBid();
    std::vector<LevelView> askDepth = orderbook.fullDepthAsk();

    ASSERT_EQ(bidDepth.size(), 2);
    ASSERT_EQ(askDepth.size(), 2);
    // Bids are expected to be in descending order (largest (bestBid) price as
    // the first level)
    EXPECT_EQ(expLevel2, bidDepth[0]);
    EXPECT_EQ(expLevel1, bidDepth[1]);
    // Asks are ascending (lowest (bestAsk) price first level)
    EXPECT_EQ(expLevel3, askDepth[0]);
    EXPECT_EQ(expLevel4, askDepth[1]);
}

TEST_F(PassiveOrderbookTest, CancelExistentBidOrder) {
    price_t price = defaultPrice;
    quantity_t q = defaultQuantity;
    auto orderId = addRestingOrder(q, price, OrderType::GoodTillCancel, Side::Buy);

    BookState expectedBookState{
        .bid{.orderCnt = 1, .volume = q, .depth = 1, .bestPrice = price},
    };
    assertBookState(expectedBookState);

    LevelState expLevel{.price = price, .volume = q, .orderCnt = 1};
    std::vector<LevelView> bidDepth = orderbook.fullDepthBid();

    ASSERT_EQ(bidDepth.size(), 1);
    EXPECT_TRUE(orderbook.fullDepthAsk().empty());
    EXPECT_EQ(expLevel, bidDepth[0]);
    ASSERT_TRUE(orderbook.bestBid().has_value());
    EXPECT_FALSE(orderbook.bestAsk().has_value());
    EXPECT_EQ(orderbook.bestBid().value(), price);

    orderbook.cancelOrder(orderId);
    expectedBookState = BookState{};
    assertBookState(expectedBookState);

    EXPECT_TRUE(orderbook.fullDepthAsk().empty());
    EXPECT_TRUE(orderbook.fullDepthBid().empty());
    EXPECT_FALSE(orderbook.bestAsk().has_value());
    EXPECT_FALSE(orderbook.bestBid().has_value());
}

TEST_F(PassiveOrderbookTest, CancelExistentAskOrder) {
    price_t price = defaultPrice;
    quantity_t q = defaultQuantity;
    auto orderId = addRestingOrder(q, price, OrderType::GoodTillCancel, Side::Sell);

    BookState expectedBookState{
        .ask{.orderCnt = 1, .volume = q, .depth = 1, .bestPrice = price},
    };
    assertBookState(expectedBookState);

    LevelState expLevel{.price = price, .volume = q, .orderCnt = 1};
    std::vector<LevelView> askDepth = orderbook.fullDepthAsk();

    ASSERT_EQ(askDepth.size(), 1);
    EXPECT_TRUE(orderbook.fullDepthBid().empty());
    EXPECT_EQ(expLevel, askDepth[0]);
    ASSERT_TRUE(orderbook.bestAsk().has_value());
    EXPECT_FALSE(orderbook.bestBid().has_value());
    EXPECT_EQ(orderbook.bestAsk().value(), price);

    orderbook.cancelOrder(orderId);
    expectedBookState = BookState{};
    assertBookState(expectedBookState);

    EXPECT_TRUE(orderbook.fullDepthAsk().empty());
    EXPECT_TRUE(orderbook.fullDepthBid().empty());
    EXPECT_FALSE(orderbook.bestAsk().has_value());
    EXPECT_FALSE(orderbook.bestBid().has_value());
}

TEST_F(PassiveOrderbookTest, CancelNonExistentOrder) {
    ASSERT_TRUE(orderbook.fullDepthAsk().empty());
    ASSERT_TRUE(orderbook.fullDepthBid().empty());
    EXPECT_THROW(orderbook.cancelOrder(100), std::out_of_range);
}

TEST_F(PassiveOrderbookTest, CancelOrderWhichIsNotTheLastAtLevel) {
    price_t price = defaultPrice;
    quantity_t q1 = defaultQuantity;
    quantity_t q2 = defaultQuantity + 8;

    auto orderId1 = addRestingOrder(q1, price, OrderType::GoodTillCancel, Side::Buy);
    auto orderId2 = addRestingOrder(q2, price, OrderType::GoodTillCancel, Side::Buy);

    BookState expectedBookState{
        .bid{.orderCnt = 2, .volume = q1 + q2, .depth = 1, .bestPrice = price},
    };
    assertBookState(expectedBookState);

    LevelState expLevel{.price = price, .volume = q1 + q2, .orderCnt = 2};
    std::vector<LevelView> bidDepth = orderbook.fullDepthBid();
    std::vector<LevelView> askDepth = orderbook.fullDepthAsk();
    ASSERT_EQ(bidDepth.size(), 1);
    EXPECT_TRUE(askDepth.empty());
    EXPECT_EQ(bidDepth[0], expLevel);

    orderbook.cancelOrder(orderId1);
    expectedBookState = BookState{
        .bid{.orderCnt = 1, .volume = q2, .depth = 1, .bestPrice = price},
    };
    assertBookState(expectedBookState);

    expLevel = LevelState{.price = price, .volume = q2, .orderCnt = 1};
    bidDepth = orderbook.fullDepthBid();
    askDepth = orderbook.fullDepthAsk();
    ASSERT_EQ(bidDepth.size(), 1);
    EXPECT_TRUE(askDepth.empty());
    EXPECT_EQ(bidDepth[0], expLevel);
}

TEST_F(PassiveOrderbookTest, ModifyQuantityUp) {
    price_t price = defaultPrice;
    quantity_t q = defaultQuantity + 10;

    auto orderId = addRestingOrder(q, price, OrderType::GoodTillCancel, Side::Buy);

    BookState expectedBookState{
        .bid{.orderCnt = 1, .volume = q, .depth = 1, .bestPrice = price},
    };
    assertBookState(expectedBookState);

    LevelState expLevel{.price = price, .volume = q, .orderCnt = 1};
    std::vector<LevelView> bidDepth = orderbook.fullDepthBid();
    std::vector<LevelView> askDepth = orderbook.fullDepthAsk();
    ASSERT_EQ(bidDepth.size(), 1);
    EXPECT_TRUE(askDepth.empty());
    EXPECT_EQ(bidDepth[0], expLevel);

    auto [newOrderId, newTrades] = orderbook.modifyOrder(orderId, ModifyOrder{.quantity = q * 2});
    ASSERT_TRUE(newTrades.empty());
    expectedBookState = BookState{
        .bid{.orderCnt = 1, .volume = q * 2, .depth = 1, .bestPrice = price},
    };
    assertBookState(expectedBookState);

    expLevel = LevelState{.price = price, .volume = q * 2, .orderCnt = 1};
    bidDepth = orderbook.fullDepthBid();
    askDepth = orderbook.fullDepthAsk();
    ASSERT_EQ(bidDepth.size(), 1);
    EXPECT_TRUE(askDepth.empty());
    EXPECT_EQ(bidDepth[0], expLevel);
}

TEST_F(PassiveOrderbookTest, ModifyQuantityDown) {
    price_t price = defaultPrice;
    quantity_t q = defaultQuantity + 10;

    auto orderId = addRestingOrder(q, price, OrderType::GoodTillCancel, Side::Buy);

    BookState expectedBookState{
        .bid{.orderCnt = 1, .volume = q, .depth = 1, .bestPrice = price},
    };
    assertBookState(expectedBookState);

    LevelState expLevel{.price = price, .volume = q, .orderCnt = 1};
    std::vector<LevelView> bidDepth = orderbook.fullDepthBid();
    std::vector<LevelView> askDepth = orderbook.fullDepthAsk();
    ASSERT_EQ(bidDepth.size(), 1);
    EXPECT_TRUE(askDepth.empty());
    EXPECT_EQ(bidDepth[0], expLevel);

    auto [newOrderId, newTrades] = orderbook.modifyOrder(orderId, ModifyOrder{.quantity = q / 2});
    ASSERT_TRUE(newTrades.empty());
    expectedBookState = BookState{
        .bid{.orderCnt = 1, .volume = q / 2, .depth = 1, .bestPrice = price},
    };
    assertBookState(expectedBookState);

    expLevel = LevelState{.price = price, .volume = q / 2, .orderCnt = 1};
    bidDepth = orderbook.fullDepthBid();
    askDepth = orderbook.fullDepthAsk();
    ASSERT_EQ(bidDepth.size(), 1);
    EXPECT_TRUE(askDepth.empty());
    EXPECT_EQ(bidDepth[0], expLevel);
}

TEST_F(PassiveOrderbookTest, ModifyPriceMovesToDifferentLevel) {
    price_t before = defaultPrice;
    price_t after = defaultPrice * 2;
    quantity_t q = defaultQuantity;

    auto oldOrderId = addRestingOrder(q, before, OrderType::GoodTillCancel, Side::Buy);

    BookState expectedBookState{.bid{.orderCnt = 1, .volume = q, .depth = 1, .bestPrice = before}};
    assertBookState(expectedBookState);

    LevelState expLevel{.price = before, .volume = q, .orderCnt = 1};
    std::vector<LevelView> bidDepth = orderbook.fullDepthBid();
    std::vector<LevelView> askDepth = orderbook.fullDepthAsk();
    ASSERT_EQ(bidDepth.size(), 1);
    EXPECT_TRUE(askDepth.empty());
    EXPECT_EQ(bidDepth[0], expLevel);

    auto [newOrderId, newTrades] = orderbook.modifyOrder(oldOrderId, ModifyOrder{.price = after});
    ASSERT_TRUE(newTrades.empty());

    expectedBookState = BookState{.bid{.orderCnt = 1, .volume = q, .depth = 1, .bestPrice = after}};
    assertBookState(expectedBookState);

    expLevel = LevelState{.price = after, .volume = q, .orderCnt = 1};
    bidDepth = orderbook.fullDepthBid();
    askDepth = orderbook.fullDepthAsk();
    ASSERT_EQ(bidDepth.size(), 1);
    EXPECT_TRUE(askDepth.empty());
    EXPECT_EQ(bidDepth[0], expLevel);
}

TEST_F(PassiveOrderbookTest, ModifyPriceStaysAtSameLevel) {
    // Before and after are equal on purpose
    price_t before = defaultPrice;
    price_t after = defaultPrice;
    quantity_t q = defaultQuantity;

    auto oldOrderId = addRestingOrder(q, before, OrderType::GoodTillCancel, Side::Buy);

    BookState expectedBookState{.bid{.orderCnt = 1, .volume = q, .depth = 1, .bestPrice = before}};
    assertBookState(expectedBookState);

    LevelState expLevel{.price = before, .volume = q, .orderCnt = 1};
    std::vector<LevelView> bidDepth = orderbook.fullDepthBid();
    std::vector<LevelView> askDepth = orderbook.fullDepthAsk();
    ASSERT_EQ(bidDepth.size(), 1);
    EXPECT_TRUE(askDepth.empty());
    EXPECT_EQ(bidDepth[0], expLevel);

    auto [newOrderId, newTrades] = orderbook.modifyOrder(oldOrderId, ModifyOrder{.price = after});
    ASSERT_TRUE(newTrades.empty());

    // Same state expected for BookState and LevelState
    assertBookState(expectedBookState);
    bidDepth = orderbook.fullDepthBid();
    askDepth = orderbook.fullDepthAsk();

    ASSERT_EQ(bidDepth.size(), 1);
    EXPECT_TRUE(askDepth.empty());
    EXPECT_EQ(bidDepth[0], expLevel);
}

TEST_F(PassiveOrderbookTest, LimitOrderFullyFilledWithoutResting) {
    price_t price = defaultPrice;
    quantity_t q = defaultQuantity;

    // Add some orders near the default price (TODO: move to populateBook())
    // These orders will be filled by the limit order where buy_price > sell_price
    // order with orderId1 will be filled first because it is at better price
    auto orderId1 = addRestingOrder(q / 2, price - 2, OrderType::GoodTillCancel, Side::Sell);
    auto orderId2 = addRestingOrder(q - q / 2, price - 1, OrderType::GoodTillCancel, Side::Sell);

    auto [orderId, trades] = orderbook.addOrder(q, price, OrderType::GoodTillCancel, Side::Buy);
    ASSERT_EQ(trades.size(), 2);

    TradeState trade1{.seller = orderId1, .buyer = orderId, .quantity = q / 2};
    TradeState trade2{.seller = orderId2, .buyer = orderId, .quantity = q - q / 2};
    EXPECT_EQ(trade1, trades[0]);
    EXPECT_EQ(trade2, trades[1]);
    EXPECT_TRUE(orderbook.fullDepthAsk().empty());
    EXPECT_TRUE(orderbook.fullDepthBid().empty());
    EXPECT_FALSE(orderbook.bestAsk().has_value());
    EXPECT_FALSE(orderbook.bestAsk().has_value());
}

TEST_F(PassiveOrderbookTest, LimitOrderDoesNotTradeAtWorsePriceThanLimit) {
    quantity_t q = defaultQuantity;

    // Price is in interval [1, 2defaultPrice]
    auto price = [this]() { return defaultPrice + randomInt(-defaultPrice + 1, defaultPrice); };
    // Test this function for 100 times
    for (auto i{0}; i < 100; i++) {
        int generatedPrice = price();
        ASSERT_TRUE(generatedPrice <= 2 * defaultPrice);
        ASSERT_TRUE(generatedPrice > 0);
    }

    auto orderId1 = addRestingOrder(q, price(), OrderType::GoodTillCancel, Side::Sell);
    auto orderId2 = addRestingOrder(q, price(), OrderType::GoodTillCancel, Side::Sell);
    auto orderId3 = addRestingOrder(q, price(), OrderType::GoodTillCancel, Side::Sell);
    auto orderId4 = addRestingOrder(q, price(), OrderType::GoodTillCancel, Side::Sell);
    auto orderId5 = addRestingOrder(q, price(), OrderType::GoodTillCancel, Side::Sell);

    auto [orderId, trades] =
        orderbook.addOrder(q, defaultPrice, OrderType::GoodTillCancel, Side::Buy);

    price_t prevPrice = 0;
    for (const auto& trade : trades) {
        EXPECT_TRUE(trade.price >= 1 && trade.price <= defaultPrice);
        EXPECT_TRUE(prevPrice < trade.price);  // Best prices trade first
        prevPrice = trade.price;
    }
}

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
