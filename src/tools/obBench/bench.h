#pragma once

#include "commandParser.h"
#include <vector>

class Bench
{
public:
    Bench();
    ~Bench();

private:
    std::vector<Command> commands;
};
