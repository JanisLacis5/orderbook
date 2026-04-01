#include "file.h"

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
};
