#include "replay.h"
#include "strfuncs.h"
#include "usings.h"
#include <fstream>

void replay::run()
{
    std::ifstream instream(inputFp_);
    for (std::string out(100, '\0'); instream.getline(out.data(), out.size());) {
        auto op = parseLine(out);
        processOperation(op);
    }
}

Operation replay::parseLine(const std::string& raw)
{
    Operation ret;

    auto tokens = strfuncs::split(raw, " ");
    if (tokens.empty())
        return {};

    std::string parsedLine = "";
    for (auto t : tokens)
        parsedLine += t + " ";
    logger_.debug(std::format("parsed line: {}", parsedLine));

    auto action = strfuncs::lower(tokens[0]);
    logger_.debug(std::format("raw action: {}, lowered: {}", tokens[0], action));

    if (actionMap_.find(action) == actionMap_.end()) {
        logger_.error(std::format("action '{}' invalid", action));
        return {};
    }

    ret.action = actionMap_.at(action);
    ret.args = std::vector<std::string>(tokens.size() - 1);
    for (auto i = 0u; i < ret.args.size(); ++i) {
        ret.args[i] = strfuncs::lower(tokens[i+1]);
    }

    return ret;
}

void replay::processOperation(Operation& op)
{
    if (op.action == Actions::NULLACTION)
        return;
    else if (op.action == Actions::ADD) {
        auto ret = onAdd(op.args);
    } else if (op.action == Actions::CANCEL) {
        auto orderId = strfuncs::strToType<orderId_t>(op.args[1]);
        if (!orderId.has_value()) {
            logger_.error("invalid order id received");
            return;
        }
        auto ret = onCancel(orderId.value());
    } else if (op.action == Actions::MODIFY) {
        auto orderId = strfuncs::strToType<orderId_t>(op.args[1]);
        if (!orderId.has_value()) {
            logger_.error("invalid order id received");
            return;
        }

        std::vector<std::string> newParams(op.args.begin() + 1, op.args.end());
        auto ret = onModify(orderId.value(), newParams);
    }
}

bool replay::onAdd(std::vector<std::string>& params) {
    if (params.size() != 4) {
        logger_.error(std::format("received wrong number of params for action ADD (received {}, expected 4)", params.size()));
        return false;
    }
    
    auto type = parseOrderType(params[0]);
    auto quantity = parseQuantity(params[1]);
    auto side = parseSide(params[2]);
    auto price = parsePrice(params[3]);

    if (
        type == OrderType::Bad ||
        quantity == badValues::quantity ||
        side == Side::Bad ||
        price == badValues::price
    )
        return false;

    return true;
}

bool replay::onCancel(orderId_t orderId) {
    return true;
}

bool replay::onModify(orderId_t orderId, std::vector<std::string>& params) {
    return true;
}

OrderType replay::parseOrderType(std::string_view type) {
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

quantity_t replay::parseQuantity(std::string_view quantity) {
    auto tmp = strfuncs::strToType<quantity_t>(quantity);
    if (!tmp.has_value()) {
        logger_.error(std::format("Failed to parse 'quantity' field, input: {}", quantity));
        return badValues::quantity;
    }

    return tmp.value();
}

Side replay::parseSide(std::string_view side) {
    if (side == "buy")
        return Side::Buy;
    if (side == "sell")
        return Side::Sell;

    logger_.error(std::format("Invalid order side, expected: 'buy' or 'sell', received: {}", side));
    return Side::Bad;
}

price_t replay::parsePrice(std::string_view price) {
    auto tmp = strfuncs::strToType<price_t>(price);
    if (!tmp.has_value()) {
        logger_.error(std::format("Failed to parse 'price' field, input: {}", price));
        return badValues::price;
    }

    return tmp.value();
}

const std::unordered_map<std::string, Actions> replay::actionMap_{
    {"add", Actions::ADD}, {"cancel", Actions::CANCEL}, {"modify", Actions::MODIFY}};
