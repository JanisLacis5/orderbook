#include <gtest/gtest.h>
#include "orderbook.h"

/*
TODO:
    PASSIVE ORDERS (DO NOT FILL)
    - test an empty book state
    - test one bid / ask without any other orders on the book
    - add multiple orders on the same level to test FIFO - time priority
    - add multiple orders at different price levels and test if the book looks like it should
    - cancel order at a non empty order (should work), test if every affected container behaved correctly
    - cancel the last / only order at a level, check if this level is empty
    - cancel non-existent order
    - modify quantity +
    - modify quantity -
    - modifty price, order should move levels on the book
    MARKET ORDERS
    - agressive match without liquidity on the other side - nothing happens
    - agressive match fully filled
    - agressive match partial fill but less than the count on the price leve
    - agressive match larger than the liquidity on the level - moves to the next level
    - sweeps multiple levels
    - sweeps all liquidity and is fully filled - one side of the book is empty
    - sweeps all liquidity and is partially filled - rest is canceled
    - FIFO - add 2 resting orders on the same level, check if the first added is the first filled on a sweep agressive order
    LIMIT ORDERS
    - limit orders never trade at worse price than the limit (iterate all filled orders and check if the fill is as good as the limit)
    - limit order that does not cross the spread just rests on the book
    - test if limit order is fully filled without resting on the book
    - test limit order partially filled - the rest stays on the book
    - limit order sweeps all liquidity and is exactly filled - leaves one side of the book empty
    - limit order sweeps all liquidity and is only partially filled - rests at the limit price
    EDGE CASES
    - duplicate order id
    - cancel / modify when fully filled

FIXTURE:
    - classic OrderBook class
    - function to add N (large) orders on both bid and ask (populate the book)
    - function to generate an Order instance with a unique orderid
OTHER:
    - function that checks the state of the book (called after each test)
    - function to check if book is healthy (best bid < best ask etc)
*/

class OrderBookTest : public testing::Test {
protected:
    Orderbook orderbook = Orderbook();
};

TEST_F(OrderBookTest, InitialState) {

}
