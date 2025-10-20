#include <cstdint>
#include <map>
#include <vector>
#include <chrono>
#include <list>
#include <unordered_map>

using price_t = std::int32_t;
using quantity_t = std::uint32_t;
using orderId_t = std::uint64_t;
using orderIds_t = std::vector<orderId_t>;
using microsec_t = std::chrono::microseconds;

enum OrderType {
    Market,
    GoodTillCancel,
    GoodTillEOD,
    FillOrKill,
    FillAndKill
};

enum Side {
    Buy,
    Sell
};

struct ModifyOrder {
    price_t price;
    quantity_t quantity;
    OrderType type;
};

class Order {
public:
    Order(orderId_t orderid, quantity_t quantity, price_t price, OrderType type, Side side, microsec_t opentime)
        : orderid_{orderid}, initialQuantity_{quantity}, remainingQuantity_{quantity}, price_{price}, type_{type}, side_{side}, opentime_{opentime}
    {}

    orderId_t getOrderid() const { return orderid_; }
    quantity_t getInitialQuantity() const { return initialQuantity_; }
    quantity_t getRemainingQuantity() const { return remainingQuantity_; }
    price_t getPrice() const { return price_; }
    OrderType getType() const { return type_; }
    Side getSide() const { return side_; }
    microsec_t getOpenTime() const { return opentime_; }

    void fill(quantity_t quantity) {
        remainingQuantity_ -= quantity;
    }
    quantity_t getFilled() const { return initialQuantity_ - remainingQuantity_; };
    bool isFullyFilled() const { return remainingQuantity_ == 0; }

private:
    orderId_t orderid_;
    quantity_t initialQuantity_;
    quantity_t remainingQuantity_;
    price_t price_;
    OrderType type_;
    Side side_;
    microsec_t opentime_;
};

using orderPtr_t = std::shared_ptr<Order>;
using orderPtrs_t = std::list<orderPtr_t>;

struct Trade {
    orderPtr_t askTrade;
    orderPtr_t bidTrade;
};
using trades_t = std::vector<Trade>;

class OrderBook {
public:
    trades_t addOrder(orderPtr_t order);
    void cancelOrder(orderId_t orderId);
    orderPtr_t modifyOrder(orderPtr_t order, ModifyOrder modifications);
private:
    struct OrderInfo {
        orderPtr_t order_;
        orderPtrs_t::iterator location_;
    };

    std::map<price_t, orderPtrs_t, std::less<price_t>> ask_;
    std::map<price_t, orderPtrs_t, std::greater<price_t>> bid_;

    trades_t passiveMatchOrders();  // TODO: implement
    trades_t agressiveMatchOrders();
    microsec_t getCurrTime() const {
        auto time = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<microsec_t>(time);
    }
    orderPtr_t toFillAndKill(orderPtr_t order) {
        ModifyOrder modifications;
        modifications.quantity = order->getRemainingQuantity();
        modifications.price = order->getPrice();
        modifications.type = OrderType::FillAndKill;
        return modifyOrder(order, modifications);
    }
    void processAddedOrder(orderPtr_t order);  // TODO: implement
    bool canBeFullyFilled(orderPtr_t order);  // TODO: implement
    bool doesCrossSpread(orderPtr_t order) {
        if (order->getSide() == Side::Sell) {
            const auto& [bestBid, _] = *bid_.begin();
            return order->getPrice() <= bestBid;
        }
        else if (order->getSide() == Side::Buy) {
            const auto& [bestAsk, _] = *ask_.begin();
            return order->getPrice() >= bestAsk;
        }
        else
            throw std::logic_error("Invalid side");
    }
    void addAtOrderPrice(orderPtr_t order) {
        if (order->getSide() == Side::Sell) {
            ask_[order->getPrice()].push_back(order);
        }
        else if (order->getSide() == Side::Buy) {
            bid_[order->getPrice()].push_back(order);
        }
        else
            throw std::logic_error("Invalid side");
    }
};

trades_t OrderBook::addOrder(orderPtr_t order) {
    OrderType type = order->getType();
    Side side = order->getSide();
    price_t price = order->getPrice();

    if (type == OrderType::Market) {
        order = toFillAndKill(order);

        if (side == Side::Sell) {
            auto& [worstBidPrice, _] = *bid_.rbegin();
            ask_[worstBidPrice].push_back(order);
        }
        else if (side == Side::Buy) {
            auto& [worstAskPrice, _] = *ask_.rbegin();
            bid_[worstAskPrice].push_back(order);
        }
        else
            throw std::logic_error("Invalid side");
    }
    else if (type == OrderType::FillAndKill) {
        if (!doesCrossSpread(order))
            return {};
        addAtOrderPrice(order);
    }
    else if (type == OrderType::FillOrKill) {
        if (!canBeFullyFilled(order) || !doesCrossSpread(order))
            return {};
        addAtOrderPrice(order);
    }
    else if (type == OrderType::GoodTillCancel || type == OrderType::GoodTillEOD)
        addAtOrderPrice(order);
    else
        return {};

    processAddedOrder(order);
    return matchOrders();
}

int main() {
    OrderBook orderbook;
    return 0;
}