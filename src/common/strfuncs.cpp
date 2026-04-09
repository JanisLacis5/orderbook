#include "strfuncs.h"
#include <algorithm>

namespace strfuncs {
std::vector<std::string> split(const std::string& toSplit, const std::string sep, bool filterEmpty) {
    std::vector<std::string> tokens;

    size_t l = 0ull;
    size_t r = toSplit.find(sep);
    while (r != std::string::npos) {
        auto str = toSplit.substr(l, r - l);
        if (!filterEmpty || !str.empty())
            tokens.push_back(str);

        l = r + 1;
        r = toSplit.find(" ", l);
    } 
    auto str = toSplit.substr(l, r - l);
    if (!filterEmpty || !str.empty())
        tokens.push_back(str);
    return tokens;
}

std::string lower(const std::string& str) {
    std::string out;
    std::transform(str.begin(), str.end(), out.begin(), [](auto c) { return std::tolower(c); });
    return out;
}

std::string upper(const std::string& str) {
    std::string out;
    std::transform(str.begin(), str.end(), out.begin(), [](auto c) { return std::toupper(c); });
    return out;
}
}// namespace strfuncs

