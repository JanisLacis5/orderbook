#pragma once

#include <compare>
#include <string_view>
#include <utility>

// TODO: 
// - make this into DebugLogger
// - do not allow to use DebugLogger in release builds
// - create eventlogger that logs an event in a ring buffer and 
//     make another thread that takes this buffer and pushes it 
//     to a file

enum class DebugLevel : int { ERROR = 0, WARN = 1, INFO = 2, DEBUG = 3 };
enum class LogColor { GREEN, RED, YELLOW, WHITE, PURPLE, NONE, RESET };

class Logger
{
public:
    Logger(std::string_view prefix, DebugLevel level = DebugLevel::ERROR)
        : prefix_{prefix}
        , level_{level}
    {
    }

    void success(std::string_view mes, LogColor color = LogColor::GREEN);
    void error(std::string_view mes, LogColor color = LogColor::RED);
    void logerrno(std::string_view mes, LogColor color = LogColor::RED); // function that is used instead of perror
    void warn(std::string_view mes, LogColor color = LogColor::YELLOW);
    void info(std::string_view mes, LogColor color = LogColor::NONE);
    void debug(std::string_view mes, LogColor color = LogColor::NONE);

private:
    std::string_view colorCodes(LogColor color);
    std::string_view target_; // TODO: default to stdout, but allow it to have a file path as well
    std::string_view prefix_;
    DebugLevel level_;
};

constexpr std::strong_ordering operator<=>(DebugLevel lhs, DebugLevel rhs)
{
    return std::to_underlying(lhs) <=> std::to_underlying(rhs);
}
