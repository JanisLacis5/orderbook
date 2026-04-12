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
    
    // raw strings
    auto typeParam = params[0];
    auto quantityParam = params[1];
    auto sideParam = params[2];
    auto priceParam = params[3];

    // stirngs formatted
    OrderType type;
    if (typeParam == "market")
        type = OrderType::Market;
    else if (typeParam == "gtc") 
        type = OrderType::GoodTillCancel;
    else if (typeParam == "gte")
        type = OrderType::GoodTillEOD;
    else if (typeParam == "fok")
        type = OrderType::FillOrKill;
    else if (typeParam == "fak")
        type = OrderType::FillAndKill;
    else {
        logger_.error("Invalid order type");
        return false;
    }
    auto quantity = strfuncs::strToType<quantity_t>(quantityParam);
    if (!quantity.has_value()) {
        logger_.error(std::format("Failed to parse 'quantity' field, input: {}", quantityParam));
        return false;
    }
    auto side = sideParam == "sell" ? Side::Sell : Side::Buy;
    auto price = strfuncs::strToType<price_t>(priceParam);
    if (!price.has_value()) {
        logger_.error(std::format("Failed to parse 'price' field, input: {}", priceParam));
        return false;
    }
}

bool replay::onCancel(orderId_t orderId) {}

bool replay::onModify(orderId_t orderId, std::vector<std::string>& params) {}

const std::unordered_map<std::string, Actions> replay::actionMap_{
    {"add", Actions::ADD}, {"cancel", Actions::CANCEL}, {"modify", Actions::MODIFY}};
