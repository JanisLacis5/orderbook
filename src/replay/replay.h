#pragma once

#include "Logger.h"
#include "orderbook.h"
#include <unordered_map>
#include <filesystem>

enum class Actions { ADD, CANCEL, MODIFY, NULLACTION };

struct Operation {
    Actions action{Actions::NULLACTION};
    std::vector<std::string> args;
};

class replay
{
public:
    explicit replay(std::filesystem::path inFp)
        : inputFp_{inFp}
    {
        logger_.info(std::format("input file path: {}", inputFp_.string()));

        if (!std::filesystem::exists(inputFp_)) {
            auto mes = std::format("path {} does not exist", inputFp_.string());
            throw std::logic_error(mes);
        }
    }
    explicit replay(std::filesystem::path inFp, std::string outFp)
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
    ~replay() {}

    void run();

private:
    std::filesystem::path inputFp_;
    std::filesystem::path outputFp_{"/tmp/orderbook/replay_output.txt"};
    Logger logger_{"replay", DebugLevel::DEBUG};
    Orderbook ob_{};

    static const std::unordered_map<std::string, Actions> actionMap_;

    Operation parseLine(const std::string& raw);
    void processOperation(Operation& op);

    // Functions per each action
    bool onAdd(std::vector<std::string>& params);
    bool onCancel(orderId_t orderId);
    bool onModify(orderId_t orderId, std::vector<std::string>& params);
};
