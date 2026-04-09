#include "replay.h"

std::vector<std::string> replay::strSplit(const std::string& toSplit) {
    std::vector<std::string> tokens;

    size_t l = 0ull;
    size_t r = toSplit.find(" ");
    while (r != std::string::npos) {
        auto str = toSplit.substr(l, r - l);
        if (!str.empty())
            tokens.push_back(str);

        l = r + 1;
        r = toSplit.find(" ", l);
    } 
    auto str = toSplit.substr(l, r - l);
    if (!str.empty())
        tokens.push_back(str);
    return tokens;
}

Operation replay::parseLine(const std::string& raw) {
    auto tokens = strSplit(raw);
    if (tokens.empty())
        return {};
    
    auto action = tokens[0];    
    if (action == "add") {}
    if (action == "cancel") {}
    if (action == "modify") {}
    else 
        return {};
}
