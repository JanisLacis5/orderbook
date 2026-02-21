#include "Logger.h"
#include <cerrno>
#include <cstring>
#include <iostream>

void Logger::success(std::string_view mes, LogColor color) {
    std::cout << colorCodes_[color] << "[" << prefix_ << "]: " << mes << std::endl;
}

void Logger::error(std::string_view mes, LogColor color) {
    std::cerr << colorCodes_[color] << "[" << prefix_ << "]: " << mes << std::endl;
}

void Logger::logerrno(std::string_view mes, LogColor color) {
    int e = errno;
    std::cerr << colorCodes_[color] << "[" << prefix_ << "]: " << mes << ": " << std::strerror(e)
              << colorCodes_[LogColor::RESET] << '\n';
}

void Logger::warn(std::string_view mes, LogColor color) {
    if (level_ >= DebugLevel::WARN)
        std::cout << colorCodes_[color] << "[" << prefix_ << "]: " << mes << std::endl;
}

void Logger::info(std::string_view mes, LogColor color) {
    if (level_ >= DebugLevel::INFO)
        std::cout << colorCodes_[color] << "[" << prefix_ << "]: " << mes << std::endl;
}

void Logger::debug(std::string_view mes, LogColor color) {
    if (level_ >= DebugLevel::DEBUG)
        std::cout << colorCodes_[color] << "[" << prefix_ << "]: " << mes << std::endl;
}
