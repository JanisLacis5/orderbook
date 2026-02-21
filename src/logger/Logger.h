#pragma once

#include <compare>
#include <string>
#include <string_view>
#include <utility>

enum class DebugLevel : int { ERROR = 0, WARN = 1, INFO = 2, DEBUG = 3 };

class Logger {
public:
    Logger(std::string prefix, DebugLevel level = DebugLevel::ERROR) : prefix_{std::move(prefix)}, level_{level} {};
    ~Logger() {};

    void error(std::string_view mes);
    void warn(std::string_view mes);
    void info(std::string_view mes);
    void debug(std::string_view mes);

private:
    std::string target_;  // TODO: default to stdout, but allow it to have a file path as well
    std::string prefix_;
    DebugLevel level_;
};

constexpr std::strong_ordering operator<=>(DebugLevel lhs, DebugLevel rhs) {
    return std::to_underlying(lhs) <=> std::to_underlying(rhs);
}
