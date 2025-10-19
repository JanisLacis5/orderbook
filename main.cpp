#include <cstdint>
#include <map>
#include <vector>
#include <chrono>

using price_t = std::int32_t;
using quantity_t = std::uint32_t;
using orderid_t = std::uint64_t;
using orderids_t = std::vector<orderid_t>;

enum OrderType {
    Market,
    GoodTillCancel,
    GoodTillEOD,
    FillOrKill,
    FillAndKill
};

struct OrderInfo {
    orderid_t orderid;
    quantity_t quantity;
    price_t price;
    OrderType type;
    time_t open_on;
};

class OrderBook {
public:
    void newOrder(quantity_t quantity, price_t price, OrderType type);
    void cancelOrder(orderid_t orderid);
    void modifyOrder(orderid_t orderid, price_t price, quantity_t quantity, OrderType type);
private:
    orderid_t lastOrderId_{1};
    std::map<price_t, orderids_t, std::less<price_t>> ask;
    std::map<price_t, orderids_t, std::greater<price_t>> bid;

    void incrementLastOrderId() { lastOrderId_++; };
};

int main() {
    OrderBook orderbook;
    return 0;
}