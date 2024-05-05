#ifndef LOGGER_H
#define LOGGER_H

#include <sstream>
#include <mutex>
#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>

class Logger {
public:
    enum class Level {
        Info,
        Warning,
        Error
    };

private:
    std::ostringstream stream;
    Level logLevel;
    static std::mutex mtx;

public:
    Logger(Level level = Level::Info);
    ~Logger();

    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator<<(std::ostream& (*pf)(std::ostream&));
    static std::string levelToString(Level level);

    template <typename T>
    Logger& operator<<(const T& msg);
};

#include "Logger.inl"  // Include template implementation

#endif // LOGGER_H
