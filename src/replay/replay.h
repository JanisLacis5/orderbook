#pragma once

#include "Logger.h"
#include "orderbook.h"
#include "usings.h"
#include <filesystem>
#include <unordered_map>

enum class Actions { ADD, CANCEL, MODIFY, NULLACTION };

struct Operation {
    Actions action{Actions::NULLACTION};
    std::vector<std::string> args;
};

class replay
{
public:
    explicit replay(std::filesystem::path inFp);
    explicit replay(std::filesystem::path inFp, std::string outFp);
    ~replay();

    void run();

private:
    std::filesystem::path inputFp_;
    std::filesystem::path outputFp_{"/tmp/orderbook/replay_output.txt"};
    Logger logger_{"replay", DebugLevel::DEBUG};
    Orderbook ob_{};

    static const std::unordered_map<std::string, Actions> actionMap_;

    Operation parseLine(const std::string& raw);
    void processOperation(Operation& op);

    // Functions per each action except cancel (just ob.cancel)
    bool onAdd(std::vector<std::string>& params);
    bool onModify(orderId_t orderId, std::vector<std::string>& params);

    // Parsing functions for parameters
    orderId_t parseOrderId(std::string_view id);
    OrderType parseOrderType(std::string_view type);
    quantity_t parseQuantity(std::string_view quantity);
    Side parseSide(std::string_view side);
    price_t parsePrice(std::string_view price);
};
