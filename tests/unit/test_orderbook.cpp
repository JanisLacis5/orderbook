#include <gtest/gtest.h>
#include "order.h"
#include "orderbook.h"
#include "usings.h"

class OrderbookTest : public testing::Test {
protected:
    Orderbook orderbook;

    void assertBookState() const {}
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
TEST_F(PassiveOrderbookTest, InitialState) {}
TEST_F(PassiveOrderbookTest, OneBidOnEmptyBook) {}
TEST_F(PassiveOrderbookTest, OneAskOnEmptyBook) {}
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

