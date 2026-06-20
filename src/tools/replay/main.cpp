#include "replay.h"
#include "strfuncs.h"
#include <iostream>

int main(int argc, char** argv)
{
    // args for replay class, declare them here and process in the loop
    std::filesystem::path filename = "/home/janis/dev/orderbook/data/input.txt";
    bool waitBeforeOp = false;

    // Process user input
    for (int i = 1; i < argc; ++i) {
        auto split = strfuncs::split(argv[i], "=");
        if (split.size() == 1) {
            if (split[0] == "--wait-before-op")
                waitBeforeOp = true;
            else
                std::cout << "Unknown flag: " << std::quoted(split[0]) << std::endl;
        }
        else if (split.size() == 2) {
            if (split[0] == "--filename")
                filename = split[1];
            else if (split[0] == "--log-level") {
                auto level = strfuncs::lower(split[1]);
                if (level == "error") LoggerConfig::setLevel(LogLevel::ERROR);
                if (level == "warn") LoggerConfig::setLevel(LogLevel::WARN);
                if (level == "log") LoggerConfig::setLevel(LogLevel::LOG);
                if (level == "debug") LoggerConfig::setLevel(LogLevel::DEBUG);
                else 
                    std::cout << "Unknown log level: " << std::quoted(level) << ", available ones: 'error', 'warn', 'log', 'debug'" << std::endl;
            }
            else
                std::cout << "Unknown flag: " << std::quoted(split[0]) << std::endl;
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
