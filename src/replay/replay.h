#include "Logger.h"
#include "file.h"
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
    }
    explicit replay(std::filesystem::path inFp, std::string outFp)
        : inputFp_{inFp}
        , outputFp_{outFp}
    {
    }
    ~replay();

    void run();

private:
    std::filesystem::path inputFp_;
    std::filesystem::path outputFp_{"output.txt"};
    Logger logger_{"replay"};
    Orderbook ob_{};

    static const std::unordered_map<std::string, Actions> actionMap_;

    Operation parseLine(const std::string& raw);
    void processOperation(Operation& op);

    // Functions per each action
    bool onAdd(std::vector<std::string>& params);
    bool onCancel(orderId_t orderId);
    bool onModify(orderId_t orderId, std::vector<std::string>& params);
};
