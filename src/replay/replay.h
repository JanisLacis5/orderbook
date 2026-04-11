#include "Logger.h"
#include "file.h"
#include "orderbook.h"
#include <unordered_map>

enum class Actions { ADD, CANCEL, MODIFY, NULLACTION };

struct Operation {
    Actions action{Actions::NULLACTION};
    std::vector<std::string> args;
};

class replay
{
public:
    explicit replay(std::string inFp)
        : inputFile_{inFp}
    {
    }
    explicit replay(std::string inFp, std::string outFp)
        : inputFile_{inFp}
        , outputFile_{outFp}
    {
    }
    ~replay();

    void run();

private:
    File inputFile_;
    File outputFile_{"output.txt"};
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
