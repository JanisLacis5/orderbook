#pragma once

#include "Logger.h"
#include "commandParser.h"
#include "orderbook.h"
#include "types.h"
#include "usings.h"
#include <filesystem>
#include <unordered_map>

class Replay
{
public:
    explicit Replay(std::filesystem::path inFp);
    explicit Replay(std::filesystem::path inFp, std::string outFp);

    void run(bool waitBeforeOperation);

private:
    std::filesystem::path inputFp_;
    std::filesystem::path outputFp_{"/tmp/orderbook/replay_output.txt"};
    Logger logger_{"replay"};
    CommandParser parser_{};
    Orderbook ob_{};

    // Flags
    bool verbose_{false};

    static const std::unordered_map<std::string, Actions> str2action_;
    static const std::unordered_map<OrderType, std::string> type2str_;

    void processCommand(Command& op);
    void logStats(orderId_t orderId, trades_t& trades, OrderInfo info);
};
