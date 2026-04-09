#include <string>
#include <vector>

namespace strfuncs
{
    std::vector<std::string> split(const std::string& toSplit, const std::string sep, bool filterEmpty = true);
    std::string upper(const std::string& str);
    std::string lower(const std::string& str);
} // namespace strfuncs
