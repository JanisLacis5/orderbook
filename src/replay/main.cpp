#include "replay.h"
#include <iostream>

int main(int argc, char** argv)
{
    auto filename = "/home/janis/dev/orderbook/build/release/src/replay/input.txt";
    if (argc >= 2)
        filename = argv[1];

    if (!std::filesystem::exists(filename))
        std::cout << "file '" << filename << "' does not exist" << std::endl;

    replay replay{filename};
    replay.run();
}
