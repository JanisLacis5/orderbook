#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "types.h"
#include "usings.h"
#include "Logger.h"

enum class Actions { ADD, CANCEL, MODIFY, NULLACTION };

struct Command {
    Actions action{Actions::NULLACTION};
    
    orderId_t oid = badValues::orderId;
    OrderType type = OrderType::Bad;
    quantity_t quantity = badValues::quantity;
    price_t price = badValues::price;
    Side side = Side::Bad;
};

class CommandParser {
public:
    static const std::unordered_map<std::string, Actions> str2action_;
    static const std::unordered_map<OrderType, std::string> type2str_;

    std::vector<Command> parseFile(const std::string fp);

private:
    Logger logger_{"parser: "};

    std::pair<Command, bool> processArgs(Actions action, const std::vector<std::string>& args);
    Command parseLine(const std::string& raw);
    orderId_t parseOrderId(const std::string_view id);
    OrderType parseOrderType(const std::string_view type);
    quantity_t parseQuantity(const std::string_view quantity);
    Side parseSide(const std::string_view side);
    price_t parsePrice(const std::string_view price);
};
