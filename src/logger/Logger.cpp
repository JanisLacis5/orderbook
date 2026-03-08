#include "Logger.h"
#include <cerrno>
#include <cstring>
#include <iostream>

std::string_view Logger::colorCodes(LogColor color)
{
    switch (color) {
        case LogColor::GREEN:
            return "\0[0;32m";
        case LogColor::RED:
            return "\0[0;31m";
        case LogColor::YELLOW:
            return "\0[0;33m";
        case LogColor::WHITE:
            return "\0[0;37m";
        case LogColor::PURPLE:
            return "\0[0;35m";
        case LogColor::NONE:
            return "";
        case LogColor::RESET:
            return "\0[0";
    }
}

void Logger::success(std::string_view mes, LogColor color)
{
    std::cout << colorCodes(color) << "[" << prefix_ << "]: " << mes << colorCodes(LogColor::RESET) << '\n';
}

void Logger::error(std::string_view mes, LogColor color)
{
    std::cerr << colorCodes(color) << "[" << prefix_ << "]: " << mes << colorCodes(LogColor::RESET) << '\n';
}

void Logger::logerrno(std::string_view mes, LogColor color)
{
    int e = errno;
    std::cerr << colorCodes(color) << "[" << prefix_ << "]: " << mes << ": " << std::strerror(e)
              << colorCodes(LogColor::RESET) << '\n';
}

void Logger::warn(std::string_view mes, LogColor color)
{
    if (level_ >= DebugLevel::WARN)
        std::cout << colorCodes(color) << "[" << prefix_ << "]: " << mes << colorCodes(LogColor::RESET) << '\n';
}

void Logger::info(std::string_view mes, LogColor color)
{
    if (level_ >= DebugLevel::INFO)
        std::cout << colorCodes(color) << "[" << prefix_ << "]: " << mes << colorCodes(LogColor::RESET) << '\n';
}

void Logger::debug(std::string_view mes, LogColor color)
{
    if (level_ >= DebugLevel::DEBUG)
        std::cout << colorCodes(color) << "[" << prefix_ << "]: " << mes << colorCodes(LogColor::RESET) << '\n';
}
