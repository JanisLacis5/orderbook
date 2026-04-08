#include "file.h"
#include "types.h"
#include "usings.h"
#include <variant>

enum class Actions { ADD, CANCEL, MODIFY };
enum class ArgType { OrderID, OrderType, Quantity, Side, Price };
using ArgValue = std::variant<orderId_t, OrderType, quantity_t, Side, price_t>;
using Arg = std::pair<ArgType, ArgValue>;

struct Operation {
    Actions action;
    std::vector<Arg> args;
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

    Operation parseLine(const std::string& raw);
    std::vector<std::string> strSplit(const std::string& toSplit);
};
