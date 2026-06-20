#include "replay.h"
#include "strfuncs.h"
#include "usings.h"
#include <format>
#include <iostream>

Replay::Replay(std::filesystem::path inFp)
    : inputFp_{inFp}
{
    logger_.debug(std::format("input file path: {}", inputFp_.string()));

    if (!std::filesystem::exists(inputFp_)) {
        auto mes = std::format("path {} does not exist", inputFp_.string());
        throw std::logic_error(mes);
    }
}

Replay::Replay(std::filesystem::path inFp, std::string outFp)
    : inputFp_{inFp}
    , outputFp_{outFp}
{
    logger_.debug(std::format("input file path: {}", inputFp_.string()));
    logger_.debug(std::format("output file path: {}", outputFp_.string()));

    if (!std::filesystem::exists(inputFp_)) {
        auto mes = std::format("path {} does not exist", inputFp_.string());
        throw std::logic_error(mes);
    }

    if (!std::filesystem::exists(outputFp_)) {
        auto mes = std::format("path {} does not exist", outputFp_.string());
        throw std::logic_error(mes);
    }
}

void Replay::run(bool waitBeforeOperation)
{
    for (auto op : parser_.parseFile(inputFp_)) {
        if (waitBeforeOperation) {
            // waits for any input from user
            if (verbose_)
                logger_.log("Press Enter to process the next operation");
            std::cin.get();
        }
        processCommand(op);
    }
}

void Replay::processCommand(Command& op)
{
    if (op.action == Actions::NULLACTION)
        return;

    else if (op.action == Actions::ADD) {
        auto [orderId, trades, orderInfo] = ob_.addOrder(op.quantity, op.price, op.type, op.side);
        logStats(orderId, trades, orderInfo);

    } else if (op.action == Actions::CANCEL) {
        ob_.cancelOrder(op.oid);

    } else if (op.action == Actions::MODIFY) {
        ModifyOrder mods;
        if (op.quantity != badValues::quantity)
            mods.quantity = op.quantity;
        if (op.price != badValues::price)
            mods.price = op.price;
        if (op.side != Side::Bad)
            mods.side = op.side;
        if (op.type != OrderType::Bad)
            mods.type = op.type;

        auto [newOrderId, trades, newOrderInfo] = ob_.modifyOrder(op.oid, mods);
        logStats(op.oid, trades, newOrderInfo);
    }
}

void Replay::logStats(orderId_t orderId, trades_t& trades, OrderInfo info)
{
    if (orderId == 0) {
        logger_.log("Order rejected");
        return;
    }

    auto typeStr = parser_.type2str_.at(info.type);
    auto sideStr = info.side == Side::Buy ? "BUY" : "SELL";
    auto price = info.price;
    auto quantity = info.quantity;

    logger_.log(std::format("New order with id {}:\n\tprice: {}\n\tquantity: {}\n\tside: {}\n\ttype: {}", orderId,
                            price, quantity, sideStr, typeStr));
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

    logger_.log(std::format("Order accepted | id={} type={} side={} price={} qty={} | executed_qty={} trade_count={}{}",
                            orderId, sideStr, typeStr, price, quantity, executedQty, trades.size(), tradeDetails));
}
