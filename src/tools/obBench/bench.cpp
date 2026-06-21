#include "bench.h"
#include "strfuncs.h"
#include "usings.h"
#include <chrono>

Bench::Bench(std::filesystem::path inFp, size_t iterations) : iterations_{iterations}
{
    if (!std::filesystem::exists(inFp)) {
        auto mes = std::format("path {} does not exist", inFp.string());
        throw std::logic_error(mes);
    }

    CommandParser parser_;
    commands_ = parser_.parseFile(inFp.string());
}

BenchResult Bench::run()
{
    const auto start = std::chrono::steady_clock::now(); 

    for (size_t i{}; i < iterations_; ++i) {
        for (auto op : commands_) {
            processCommand(op);

            // consume something so compiler cannot delete the work
            volatile auto sink = ob_.bestBid().value_or(price_t{});
            (void)sink;
        }
    }

    const auto end = std::chrono::steady_clock::now();
    const auto elapsed_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    const auto commandCnt = commandCount() * iterations_;

    return BenchResult{
        .elapsed_ns = elapsed_ns,
        .ns_per_command = static_cast<double>(elapsed_ns) /
                          static_cast<double>(commandCnt),
        .commands_per_second =
            static_cast<double>(commandCnt) /
            (static_cast<double>(elapsed_ns) / 1'000'000'000.0),
        .total_commands = commandCnt,
    };
}

void Bench::processCommand(Command& op)
{
    if (op.action == Actions::NULLACTION)
        return;

    else if (op.action == Actions::ADD) {
        auto [orderId, trades, orderInfo] = ob_.addOrder(op.quantity, op.price, op.type, op.side);

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
    }
}
