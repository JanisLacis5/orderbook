#pragma once

#include "commandParser.h"
#include "orderbook.h"
#include <filesystem>
#include <vector>

struct BenchResult {
    int64_t elapsed_ns;
    double ns_per_command;
    double commands_per_second;
    size_t total_commands;
};

class Bench
{
public:
    Bench(std::filesystem::path inFp, size_t iterations);

    const size_t commandCount() const { return commands_.size(); }
    BenchResult run();

private:
    Orderbook ob_{};

    std::vector<Command> commands_;
    size_t iterations_{};

    void processCommand(Command& op, Orderbook& book);
};
