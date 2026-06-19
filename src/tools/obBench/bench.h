#pragma once

#include <vector>
#include "commandParser.h"

class Bench {

public:
    Bench();
    ~Bench();

private:
    std::vector<Command> commands;
};
