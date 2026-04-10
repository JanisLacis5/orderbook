#include "replay.h"
#include "strfuncs.h"

Operation replay::parseLine(const std::string& raw)
{
    Operation ret;

    auto tokens = strfuncs::split(raw, " ");
    if (tokens.empty())
        return {};

    auto action = strfuncs::lower(tokens[0]);
    if (actionMap_.find(action) == actionMap_.end()) {
        logger_.error("action invalid");
        return {};
    }
    ret.action = actionMap_.at(action);
    ret.args = std::vector<std::string>{tokens.begin() + 1, tokens.end()};

    return ret;
}

const std::unordered_map<std::string, Actions> replay::actionMap_{
    {"add", Actions::ADD}, {"cancel", Actions::CANCEL}, {"modify", Actions::MODIFY}};
