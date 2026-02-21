#pragma once

#include <compare>
#include <string_view>
#include <unordered_map>
#include <utility>

enum class DebugLevel : int { ERROR = 0, WARN = 1, INFO = 2, DEBUG = 3 };
enum class LogColor { GREEN, RED, YELLOW, WHITE, PURPLE, NONE, RESET };

class Logger {
public:
    Logger(std::string_view prefix, DebugLevel level = DebugLevel::ERROR)
        : prefix_{std::move(prefix)}, level_{level} {};
    ~Logger() {};

    void success(std::string_view mes, LogColor color = LogColor::GREEN);
    void error(std::string_view mes, LogColor color = LogColor::RED);
    void logerrno(std::string_view mes, LogColor color = LogColor::RED); // function that is used instead of perror
    void warn(std::string_view mes, LogColor color = LogColor::YELLOW);
    void info(std::string_view mes, LogColor color = LogColor::NONE);
    void debug(std::string_view mes, LogColor color = LogColor::NONE);

private:
    std::unordered_map<LogColor, std::string_view> colorCodes_{
        {LogColor::GREEN, "\0[0;32m"}, {LogColor::RED, "\0[0;31m"},    {LogColor::YELLOW, "\0[0;33m"},
        {LogColor::WHITE, "\0[0;37m"}, {LogColor::PURPLE, "\0[0;35m"}, {LogColor::NONE, ""},
        {LogColor::RESET, "\0[0m"}};

    std::string_view target_;  // TODO: default to stdout, but allow it to have a file path as well
    std::string_view prefix_;
    DebugLevel level_;
};

constexpr std::strong_ordering operator<=>(DebugLevel lhs, DebugLevel rhs) {
    return std::to_underlying(lhs) <=> std::to_underlying(rhs);
}
