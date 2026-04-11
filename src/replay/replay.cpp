#include "replay.h"
#include "strfuncs.h"
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
    ret.args = std::vector<std::string>{tokens.begin() + 1, tokens.end()};

    return ret;
}

void replay::processOperation(Operation& op)
{
    // this lambda expects the id to be at args[0]
    auto getOrderIdFromArgs = [&](std::span<std::string> args) {
        orderId_t orderId{};
        auto idstr = args[1];
        auto [ptr, ec] = std::from_chars(idstr.data(), idstr.data() + idstr.size(), orderId);

        if (ec == std::errc()) {
            logger_.error("bad input in processOperation");
            return INVALID_ORDER_ID;
        }
        if (ec == std::errc::invalid_argument) {
            logger_.error("not a number in processOperation");
            return INVALID_ORDER_ID;
        }
        if (ec == std::errc::result_out_of_range) {
            logger_.error("overflow in processOperation");
            return INVALID_ORDER_ID;
        }

        return orderId;
    };

    if (op.action == Actions::NULLACTION)
        return;
    else if (op.action == Actions::ADD) {
        auto ret = onAdd(op.args);
    } else if (op.action == Actions::CANCEL) {
        std::span<std::string> args{op.args};
        orderId_t orderId = getOrderIdFromArgs(args);
        if (orderId == INVALID_ORDER_ID) {
            logger_.error("invalid order id received");
            return;
        }

        auto ret = onCancel(orderId);
    } else if (op.action == Actions::MODIFY) {
        std::span<std::string> args{op.args};
        orderId_t orderId = getOrderIdFromArgs(args);
        if (orderId == INVALID_ORDER_ID) {
            logger_.error("invalid order id received");
            return;
        }

        std::vector<std::string> newParams(op.args.begin() + 1, op.args.end());
        auto ret = onModify(orderId, newParams);
    }
}

bool replay::onAdd(std::vector<std::string>& params) {}

bool replay::onCancel(orderId_t orderId) {}

bool replay::onModify(orderId_t orderId, std::vector<std::string>& params) {}

const std::unordered_map<std::string, Actions> replay::actionMap_{
    {"add", Actions::ADD}, {"cancel", Actions::CANCEL}, {"modify", Actions::MODIFY}};
