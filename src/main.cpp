#include "PublicAPI.h"
#include "orderbook.h"

int main() {
    PublicAPI api;
    api.run();
    return 0;
}

/*
TODO:
    - write some cleanup functions, for example cleanup function when order is
fully filled - it has to be removed from orders_ etc.
*/
