#include "replay.h"
#include "strfuncs.h"
#include <iostream>

int main(int argc, char** argv)
{
    // args for replay class, declare them here and process in the loop
    auto filename = "/home/janis/dev/orderbook/data/input.txt";
    bool waitBeforeOp = false;

    // Process user input
    for (int i = 1; i < argc; ++i) {
        auto split = strfuncs::split(argv[i], "=");
        if (split.size() == 1) {
            if (split[0] == "--wait-before-op")
                waitBeforeOp = true;
        }
        else if (split.size() == 2) {
            if (split[0] == "--filename")
                filename = argv[i];
        }
        else {
            std::cout << "Bad argument: " << std::quoted(argv[i]) << std::endl;
            return 1;
        }
    }

    if (!std::filesystem::exists(filename)) {
        std::cout << "file '" << filename << "' does not exist" << std::endl;
        return 1;
    }

    replay replay{filename};
    replay.run(waitBeforeOp);
}
