#include <cstdint>
#include <map>
#include <vector>
#include <chrono>
#include <list>
#include <unordered_map>

using price_t = std::int32_t;
using quantity_t = std::uint32_t;
using orderid_t = std::uint64_t;
using orderids_t = std::vector<orderid_t>;
using ordertime_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

enum OrderType {
    Market,
    GoodTillCancel,
    GoodTillEOD,
    FillOrKill,
    FillAndKill
};

class Order {
    Order(orderid_t orderid, quantity_t quantity, price_t price, OrderType type)
        : orderid_{orderid}, quantity_{quantity}, price_{price}, type_{type}
    {}
private:
    orderid_t orderid_;
    quantity_t quantity_;
    price_t price_;
    OrderType type_;
    time_t open_on;
};

using orderptr_t = std::shared_ptr<Order>;
using orderptrs_t = std::list<orderptr_t>;

class OrderBook {
public:
    void newOrder(quantity_t quantity, price_t price, OrderType type);
    void cancelOrder(orderid_t orderid);
    void modifyOrder(orderid_t orderid, price_t price, quantity_t quantity, OrderType type);
private:
    struct OrderInfo {
        orderptr_t order_;
        orderptrs_t::iterator location_;
    };

    orderid_t lastOrderId_{1};
    std::map<price_t, orderids_t, std::less<price_t>> ask_;
    std::map<price_t, orderids_t, std::greater<price_t>> bid_;
    std::unordered_map<orderid_t, OrderInfo> orders_;
    orderptrs_t orderPtrs_;

    void incrementLastOrderId() { lastOrderId_++; };
};

int main() {
    OrderBook orderbook;
    return 0;
}