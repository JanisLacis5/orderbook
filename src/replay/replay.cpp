#include "replay.h"
#include "strfuncs.h"
#include "usings.h"
#include <format>
#include <fstream>

std::ostream& operator<<(std::ostream& os, const Operation& op) {
    std::string action = "";
    if (op.action == Actions::ADD) action = "add";
    if (op.action == Actions::CANCEL) action = "cancel";
    if (op.action == Actions::MODIFY) action = "modify";
    if (op.action == Actions::NULLACTION) action = "nullaction";

    std::string ret = "action: " + action;
    for (auto arg : op.args) {
        ret += "\n\targ: " + arg;
    }

    return os << ret;
}

replay::replay(std::filesystem::path inFp)
    : inputFp_{inFp}
{
    logger_.info(std::format("input file path: {}", inputFp_.string()));

    if (!std::filesystem::exists(inputFp_)) {
        auto mes = std::format("path {} does not exist", inputFp_.string());
        throw std::logic_error(mes);
    }
}
replay::replay(std::filesystem::path inFp, std::string outFp)
    : inputFp_{inFp}
    , outputFp_{outFp}
{
    logger_.info(std::format("input file path: {}", inputFp_.string()));
    logger_.info(std::format("output file path: {}", outputFp_.string()));

    if (!std::filesystem::exists(inputFp_)) {
        auto mes = std::format("path {} does not exist", inputFp_.string());
        throw std::logic_error(mes);
    }

    if (!std::filesystem::exists(outputFp_)) {
        auto mes = std::format("path {} does not exist", outputFp_.string());
        throw std::logic_error(mes);
    }
}
replay::~replay() {}

void replay::run()
{
    std::ifstream instream(inputFp_);
    std::string out;

    while (std::getline(instream, out)) {
        if (out[0] == '#')
            continue;

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
    if (str2action_.find(action) == str2action_.end()) {
        logger_.error(std::format("action '{}' invalid", action));
        return {};
    }

    ret.action = str2action_.at(action);
    ret.args = std::vector<std::string>(tokens.size() - 1);
    for (size_t i = 0; i < ret.args.size(); ++i)
        ret.args[i] = strfuncs::lower(tokens[i + 1]);

    return ret;
}

void replay::processOperation(Operation& op)
{
    if (op.action == Actions::NULLACTION)
        return;
    else if (op.action == Actions::ADD) {
        auto ret = onAdd(op.args);
    } else if (op.action == Actions::CANCEL) {
        auto orderId = parseOrderId(op.args[0]);
        if (orderId == badValues::orderId)
            return;

        ob_.cancelOrder(orderId);
    } else if (op.action == Actions::MODIFY) {
        auto orderId = parseOrderId(op.args[1]);
        if (orderId == badValues::orderId)
            return;

        std::vector<std::string> newParams(op.args.begin() + 1, op.args.end());
        auto ret = onModify(orderId, newParams);
    }
}

bool replay::onAdd(std::vector<std::string>& params)
{
    if (params.size() != 4) {
        logger_.error(
            std::format("received wrong number of params for action ADD (received {}, expected 4)", params.size()));
        return false;
    }

    auto type = parseOrderType(params[0]);
    auto quantity = parseQuantity(params[1]);
    auto side = parseSide(params[2]);
    auto price = parsePrice(params[3]);

    if (type == OrderType::Bad || quantity == badValues::quantity || side == Side::Bad || price == badValues::price)
        return false;

    auto [orderId, trades, orderInfo] = ob_.addOrder(quantity, price, type, side);
    logStats(orderId, trades, orderInfo);
    return true;
}

bool replay::onModify(orderId_t orderId, std::vector<std::string>& params)
{
    if (params.size() < 1 || params.size() > 4) {
        logger_.error(std::format("received wrong number of params for action MODIFY (received {}, expected 1-4)",
                                  params.size()));
        return false;
    }

    ModifyOrder mods;
    for (auto kvPair : params) {
        auto kv = strfuncs::split(kvPair, "=");
        if (kv.size() != 2) {
            logger_.error(std::format("Invalid moidification pair: '{}'", kvPair));
            return false;
        }

        auto key = strfuncs::lower(kv[0]);
        auto value = kv[1];

        if (key == "ordertype") {
            auto newType = parseOrderType(value);
            if (newType == OrderType::Bad)
                return false;

            mods.type = newType;
        } else if (key == "quantity") {
            auto newQuantity = parseQuantity(value);
            if (newQuantity == badValues::quantity)
                return false;

            mods.quantity = newQuantity;
        } else if (key == "side") {
            auto newSide = parseSide(value);
            if (newSide == Side::Bad)
                return false;

            mods.side = newSide;
        } else if (key == "price") {
            auto newPrice = parsePrice(value);
            if (newPrice == badValues::price)
                return false;

            mods.price = newPrice;
        } else {
            logger_.error(std::format(
                "Modification argument '{}' is not valid, supported ones are: 'orderType', 'quantity', 'side;, 'price'",
                key));
            return false;
        }
    }

    auto [newOrderId, trades, newOrderInfo] = ob_.modifyOrder(orderId, mods);
    logStats(orderId, trades, newOrderInfo);
    return true;
}

orderId_t replay::parseOrderId(std::string_view id)
{
    auto tmp = strfuncs::strToType<orderId_t>(id);
    if (!tmp.has_value()) {
        logger_.error(std::format("Failed to parse 'order_id' field, input: {}", id));
        return badValues::orderId;
    }

    return tmp.value();
}

OrderType replay::parseOrderType(std::string_view type)
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

quantity_t replay::parseQuantity(std::string_view quantity)
{
    auto tmp = strfuncs::strToType<quantity_t>(quantity);
    if (!tmp.has_value()) {
        logger_.error(std::format("Failed to parse 'quantity' field, input: {}", quantity));
        return badValues::quantity;
    }

    return tmp.value();
}

Side replay::parseSide(std::string_view side)
{
    if (side == "buy")
        return Side::Buy;
    if (side == "sell")
        return Side::Sell;

    logger_.error(std::format("Invalid order side, expected: 'buy' or 'sell', received: {}", side));
    return Side::Bad;
}

price_t replay::parsePrice(std::string_view price)
{
    auto tmp = strfuncs::strToType<price_t>(price);
    if (!tmp.has_value()) {
        logger_.error(std::format("Failed to parse 'price' field, input: {}", price));
        return badValues::price;
    }

    return tmp.value();
}

void replay::logStats(orderId_t orderId, trades_t& trades, OrderInfo info)
{
    auto typeStr = type2str_.at(info.type);
    auto sideStr = info.side == Side::Buy ? "BUY" : "SELL";
    auto price = info.price;
    auto quantity = info.quantity;

    logger_.info(std::format("New order with id {}:\nprice: {}\nquantity: {}\nside: {}\ntype: {}", orderId, price,
                             quantity, sideStr, typeStr));
    if (orderId == 0) {
        logger_.info(
            std::format("Order rejected | type={} side={} price={} qty={} | reason=order conditions not satisfied",
                        typeStr, sideStr, price, quantity));
        return;
    }

    quantity_t executedQty = 0;
    for (const auto& trade : trades)
        executedQty += trade.quantity;

    std::string tradeDetails = "";
    if (!trades.empty()) {
        for (const auto& trade : trades) {
            tradeDetails += std::format("\n  trade | buyer={} seller={} qty={} price={}", trade.buyer, trade.seller,
                                        trade.quantity, trade.price);
        }
    }

    logger_.info(
        std::format("Order accepted | id={} type={} side={} price={} qty={} | executed_qty={} trade_count={}{}",
                    orderId, sideStr, typeStr, price, quantity, executedQty, trades.size(), tradeDetails));
}

const std::unordered_map<std::string, Actions> replay::str2action_{
    {"add", Actions::ADD}, {"cancel", Actions::CANCEL}, {"modify", Actions::MODIFY}};
const std::unordered_map<OrderType, std::string> replay::type2str_{{OrderType::Market, "MARKET"},
                                                                   {OrderType::GoodTillCancel, "GTC"},
                                                                   {OrderType::GoodTillEOD, "GTE"},
                                                                   {OrderType::FillOrKill, "FOK"},
                                                                   {OrderType::FillAndKill, "FAK"}};
