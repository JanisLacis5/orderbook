#include <vector>
#include "order.h"

struct Trade {
    orderPtr_t askTrade;
    orderPtr_t bidTrade;
};
using trades_t = std::vector<Trade>;
