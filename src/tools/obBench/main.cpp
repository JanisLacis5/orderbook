#include "bench.h"
#include "strfuncs.h"
#include <iostream>

int main(int argc, char** argv)
{
    const std::filesystem::path DATA_PATH = "/home/janis/dev/orderbook/data";

    // args for replay class, declare them here and process in the loop
    std::string filename = "input.txt";
    size_t iterations = 10000;
    LoggerConfig::setLevel(LogLevel::LOG);

    // Process user input
    for (int i = 1; i < argc; ++i) {
        auto split = strfuncs::split(argv[i], "=");
        if (split.size() == 1) {
            if (split[0] == "--help") {
                std::cout << "Usage: " << std::endl;
                std::cout << "\t./orderbook_benchmark [FLAGS]" << std::endl;
                std::cout << "Available flags: " << std::endl;
                std::cout << "\t--help: displays this message" << std::endl;
                std::cout << "\t--filename (string): path to input file, default: " << filename << std::endl;
                std::cout << "\t--iterations (int): how many times operations from the provided (or defualt) file are "
                             "exectued"
                          << std::endl;
            } else
                std::cout << "Unknown flag: " << std::quoted(split[0]) << std::endl;

        } else if (split.size() == 2) {
            if (split[0] == "--filename")
                filename = split[1];
            else if (split[0] == "--iterations")
                iterations = strfuncs::strToType<size_t>(split[1]).value();
            else
                std::cout << "Unknown flag: " << std::quoted(split[0]) << std::endl;

        } else {
            std::cout << "Bad argument: " << std::quoted(argv[i]) << std::endl;
            return 1;
        }
    }

    std::cout << DATA_PATH / filename << std::endl;
    if (!std::filesystem::exists(DATA_PATH / filename)) {
        std::cout << "file '" << filename << "' does not exist" << std::endl;
        return 1;
    }

    Bench bench{DATA_PATH / filename, iterations};

    const auto command_count = bench.commandCount();
    const auto total_commands = command_count * iterations;

    std::cout << "Benchmark workload\n";
    std::cout << "------------------\n";
    std::cout << "file: " << filename << '\n';
    std::cout << "commands/iteration: " << command_count << '\n';
    std::cout << "iterations: " << iterations << '\n';
    std::cout << "total commands: " << total_commands << '\n';

    const auto result = bench.run();

    std::cout << "\nBenchmark result\n";
    std::cout << "----------------\n";
    std::cout << "elapsed ns: " << result.elapsed_ns << '\n';
    std::cout << "ns/command: " << std::fixed << std::setprecision(2) << result.ns_per_command << '\n';
    std::cout << "commands/sec: " << std::fixed << std::setprecision(2) << result.commands_per_second << '\n';
}
