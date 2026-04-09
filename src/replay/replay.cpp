#include "replay.h"
#include "strfuncs.h"

Operation replay::parseLine(const std::string& raw)
{
    auto tokens = strfuncs::split(raw, " ");
    if (tokens.empty())
        return {};

    auto action = tokens[0];
    if (action == "add") {
    }
    if (action == "cancel") {
    }
    if (action == "modify") {
    } else
        return {};
}
