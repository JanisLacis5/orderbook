#include "replay.h"
#include "strfuncs.h"
#include <iostream>

int main(int argc, char** argv)
{
    // args for replay class, declare them here and process in the loop
    std::filesystem::path filename = "/home/janis/dev/orderbook/data/input.txt";
    bool waitBeforeOp = false;
    LoggerConfig::setLevel(LogLevel::LOG);

    // Process user input
    for (int i = 1; i < argc; ++i) {
        auto split = strfuncs::split(argv[i], "=");
        if (split.size() == 1) {
            if (split[0] == "--help") {
                std::cout << "Usage: " << std::endl;
                std::cout << "\t./replay [FLAGS]" << std::endl;
                std::cout << "Available flags: " << std::endl;
                std::cout << "\t--help: displays this message" << std::endl;
                std::cout << "\t--wait-before-op: waits for enter before processesing the next input operation"
                          << std::endl;
                std::cout << "\t--filename (string): path to input file, default: " << filename << std::endl;
                std::cout << "\t--log-level (default: LOG, available: ERROR, WARN, LOG, DEBUG in increasing log "
                             "amount): higher log level - more verbose logs\nNote: choosing anything lower than LOG "
                             "will result in not having enough logs to see what is going on"
                          << std::endl;
            } else if (split[0] == "--wait-before-op")
                waitBeforeOp = true;
            else
                std::cout << "Unknown flag: " << std::quoted(split[0]) << std::endl;

        } else if (split.size() == 2) {
            if (split[0] == "--filename")
                filename = split[1];
            else if (split[0] == "--log-level") {
                auto level = strfuncs::lower(split[1]);
                if (level == "error") {
                    std::cout << "WARNING: you will not see enough logs with this option, make sure you know what you "
                                 "are doing or use the default option"
                              << std::endl;
                    LoggerConfig::setLevel(LogLevel::ERROR);
                } else if (level == "warn") {
                    std::cout << "WARNING: you will not see enough logs with this option, make sure you know what you "
                                 "are doing or use the default option"
                              << std::endl;
                    LoggerConfig::setLevel(LogLevel::WARN);
                } else if (level == "log")
                    LoggerConfig::setLevel(LogLevel::LOG);
                else if (level == "debug")
                    LoggerConfig::setLevel(LogLevel::DEBUG);
                else
                    std::cout << "Unknown log level: " << std::quoted(level)
                              << ", available ones: 'error', 'warn', 'log', 'debug'" << std::endl;
            } else
                std::cout << "Unknown flag: " << std::quoted(split[0]) << std::endl;

        } else {
            std::cout << "Bad argument: " << std::quoted(argv[i]) << std::endl;
            return 1;
        }
    }

    if (!std::filesystem::exists(filename)) {
        std::cout << "file '" << filename << "' does not exist" << std::endl;
        return 1;
    }

    Replay replay{filename};
    replay.run(waitBeforeOp);
}
