#include <string>
#include <type_traits>
#include <vector>
#include <charconv>
#include <expected>

namespace strfuncs
{
    std::vector<std::string> split(const std::string& toSplit, const std::string sep, bool filterEmpty = true);
    std::string upper(const std::string& str);
    std::string lower(const std::string& str);
    std::pair<bool, std::string> checkAfterConv(std::from_chars_result& res);
    
    template <typename resultType>
    std::expected<resultType, std::errc> strToType(std::string_view src) {
        static_assert(std::is_default_constructible_v<resultType>);
        static_assert(std::is_integral_v<resultType>);

        resultType res{};
        auto [ptr, ec] = std::from_chars(src.data(), src.data() + src.size(), res);

        if (ptr != src.data() + src.size())
            return std::unexpected(std::errc::invalid_argument);
        if (ec == std::errc::invalid_argument || ec == std::errc::result_out_of_range)
            return std::unexpected(ec);
        return res;
    }

} // namespace strfuncs
