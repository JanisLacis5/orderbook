#include "commandParser.h"
#include "strfuncs.h"
#include <fstream>

std::vector<Command> CommandParser::parseFile(const std::string fp)
{
    std::vector<Command> ret;

    std::ifstream file{fp};
    for (std::string out; std::getline(file, out);) {
        if (out.size() == 0 || out[0] == '#')
            continue;

        auto op = parseLine(out);
        if (op.action != Actions::NULLACTION)
            ret.push_back(op);
    }

    return ret;
}

Command CommandParser::parseLine(const std::string& raw)
{
    auto tokens = strfuncs::split(raw, " ");
    if (tokens.empty())
        return Command{.action = Actions::NULLACTION};

    std::string parsedLine = "";
    for (auto t : tokens)
        parsedLine += t + " ";
    logger_.debug(std::format("parsed line: {}", parsedLine));

    auto action = strfuncs::lower(tokens[0]);
    if (str2action_.find(action) == str2action_.end()) {
        logger_.error(std::format("action '{}' invalid", action));
        return Command{.action = Actions::NULLACTION};
    }

    std::vector<std::string> args(tokens.size() - 1);
    for (size_t i = 0; i < args.size(); ++i)
        args[i] = strfuncs::lower(tokens[i + 1]);

    return processArgs(str2action_.at(action), args);
}

Command CommandParser::processArgs(Actions action, const std::vector<std::string>& args)
{
    if (action == Actions::NULLACTION)
        return Command{.action = Actions::NULLACTION};

    else if (action == Actions::ADD) {
        if (args.size() != 4) {
            logger_.error(
                std::format("received wrong number of params for action ADD (received {}, expected 4)", args.size()));
            return Command{.action = Actions::NULLACTION};
        }

        Command command;
        command.action = action;
        command.type = parseOrderType(args[0]);
        command.quantity = parseQuantity(args[1]);
        command.side = parseSide(args[2]);
        command.price = parsePrice(args[3]);

        if (command.type == OrderType::Bad || command.quantity == badValues::quantity || command.side == Side::Bad ||
            command.price == badValues::price)
            return Command{.action = Actions::NULLACTION};
        return command;

    } else if (action == Actions::CANCEL) {
        auto orderId = parseOrderId(args[0]);
        if (orderId == badValues::orderId)
            return Command{.action = Actions::NULLACTION};
        return Command{.action = action, .oid = orderId};

    } else if (action == Actions::MODIFY) {
        auto orderId = parseOrderId(args[0]);
        if (orderId == badValues::orderId)
            return Command{.action = Actions::NULLACTION};

        std::vector<std::string> newParams(args.begin() + 1, args.end());
        Command command{.action = action};
        for (auto kvPair : newParams) {
            auto kv = strfuncs::split(kvPair, "=");
            if (kv.size() != 2) {
                logger_.error(std::format("Invalid moidification pair: '{}'", kvPair));
                return Command{.action = Actions::NULLACTION};
            }

            auto key = strfuncs::lower(kv[0]);
            auto value = kv[1];
            if (key == "ordertype") {
                auto newType = parseOrderType(value);
                if (newType == OrderType::Bad)
                    return Command{.action = Actions::NULLACTION};

                command.type = newType;

            } else if (key == "quantity") {
                auto newQuantity = parseQuantity(value);
                if (newQuantity == badValues::quantity)
                    return Command{.action = Actions::NULLACTION};

                command.quantity = newQuantity;

            } else if (key == "side") {
                auto newSide = parseSide(value);
                if (newSide == Side::Bad)
                    return Command{.action = Actions::NULLACTION};

                command.side = newSide;

            } else if (key == "price") {
                auto newPrice = parsePrice(value);
                if (newPrice == badValues::price)
                    return Command{.action = Actions::NULLACTION};

                command.price = newPrice;

            } else {
                logger_.error(std::format("Modification argument '{}' is not valid, supported ones are: 'orderType', "
                                          "'quantity', 'side;, 'price'",
                                          key));
                return Command{.action = Actions::NULLACTION};
            }
        }

        return command;
    }
    return Command{.action = Actions::NULLACTION};
}

orderId_t CommandParser::parseOrderId(std::string_view id)
{
    auto tmp = strfuncs::strToType<orderId_t>(id);
    if (!tmp.has_value()) {
        logger_.error(std::format("Failed to parse 'order_id' field, input: {}", id));
        return badValues::orderId;
    }

    return tmp.value();
}

OrderType CommandParser::parseOrderType(std::string_view type)
{
    if (type == "market")
        return OrderType::Market;
    else if (type == "gtc")
        return OrderType::GoodTillCancel;
    else if (type == "gte")
        return OrderType::GoodTillEOD;
    else if (type == "fok")
        return OrderType::FillOrKill;
    else if (type == "fak")
        return OrderType::FillAndKill;

    logger_.error("Invalid order type");
    return OrderType::Bad;
}

quantity_t CommandParser::parseQuantity(std::string_view quantity)
{
    auto tmp = strfuncs::strToType<quantity_t>(quantity);
    if (!tmp.has_value()) {
        logger_.error(std::format("Failed to parse 'quantity' field, input: {}", quantity));
        return badValues::quantity;
    }

    return tmp.value();
}

Side CommandParser::parseSide(std::string_view side)
{
    if (side == "buy")
        return Side::Buy;
    if (side == "sell")
        return Side::Sell;

    logger_.error(std::format("Invalid order side, expected: 'buy' or 'sell', received: {}", side));
    return Side::Bad;
}

price_t CommandParser::parsePrice(std::string_view price)
{
    auto tmp = strfuncs::strToType<price_t>(price);
    if (!tmp.has_value()) {
        logger_.error(std::format("Failed to parse 'price' field, input: {}", price));
        return badValues::price;
    }

    return tmp.value();
}

const std::unordered_map<std::string, Actions> CommandParser::str2action_{
    {"add", Actions::ADD}, {"cancel", Actions::CANCEL}, {"modify", Actions::MODIFY}};
const std::unordered_map<OrderType, std::string> CommandParser::type2str_{{OrderType::Market, "MARKET"},
                                                                          {OrderType::GoodTillCancel, "GTC"},
                                                                          {OrderType::GoodTillEOD, "GTE"},
                                                                          {OrderType::FillOrKill, "FOK"},
                                                                          {OrderType::FillAndKill, "FAK"}};
