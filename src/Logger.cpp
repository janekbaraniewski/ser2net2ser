#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>

std::mutex Logger::mtx;

Logger::Logger(Level level) : logLevel(level) {}

Logger::~Logger() {
    mtx.lock();
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);

    std::cout << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << " " << levelToString(logLevel) << " " << stream.str() << std::endl;
    mtx.unlock();
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::Info:    return "[info]";
        case Level::Warning: return "[warning]";
        case Level::Error:   return "[error]";
    }
    return "[info]";
}
