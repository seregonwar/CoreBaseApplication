#pragma once

#include <string>
#include <vector>
#include <mutex>
#include "LogLevel.h"

namespace CoreNS {

class LogManager {
public:
    LogManager() = default;
    ~LogManager() = default;

    void log(LogLevel level, const std::string& message, const std::string& file = "", int line = 0);
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;
    std::vector<std::string> getLogBuffer() const;
    void clearLogBuffer();

private:
    std::vector<std::string> m_logBuffer;
    LogLevel m_currentLevel = LogLevel::INFO;
    mutable std::mutex m_mutex;
};

} // namespace CoreNS 