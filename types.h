#include "usings.h"

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
