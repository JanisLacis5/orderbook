#pragma once

#include <string_view>

// TODO:
// - make this into DebugLogger
// - do not allow to use DebugLogger in release builds
// - create eventlogger that logs an event in a ring buffer and
//     make another thread that takes this buffer and pushes it
//     to a file

enum class LogLevel : int { ERROR = 0, WARN = 1, LOG = 2, DEBUG = 3 };
enum class LogColor { GREEN, RED, YELLOW, WHITE, PURPLE, NONE, RESET };

class Logger
{
public:
    Logger(std::string_view prefix)
        : prefix_{prefix}
    {
    }

    void success(std::string_view mes, LogColor color = LogColor::GREEN);
    void error(std::string_view mes, LogColor color = LogColor::RED);
    void logerrno(std::string_view mes, LogColor color = LogColor::RED); // function that is used instead of perror
    void warn(std::string_view mes, LogColor color = LogColor::YELLOW);
    void log(std::string_view mes, LogColor color = LogColor::NONE);
    void debug(std::string_view mes, LogColor color = LogColor::NONE);

private:
    std::string_view colorCodes(LogColor color);
    std::string_view target_; // TODO: default to stdout, but allow it to have a file path as well
    std::string_view prefix_;
};

class LoggerConfig 
{
public:
    static void setLevel(LogLevel level) { logLevel_ = level; };
    static LogLevel logLevel() { return logLevel_; };
private:
    inline static LogLevel logLevel_{ LogLevel::ERROR };
};
