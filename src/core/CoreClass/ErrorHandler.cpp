#include "ErrorHandler.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <ctime>
#include <format>

namespace CoreNS {

namespace {
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
}

ErrorHandler::ErrorHandler() = default;
ErrorHandler::~ErrorHandler() = default;

void ErrorHandler::initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_errorCallbacks.clear();
    m_logCallbacks.clear();
    m_logLevel = LogLevel::INFO;
    m_nextCallbackId = 0;
}

void ErrorHandler::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_errorCallbacks.clear();
    m_logCallbacks.clear();
}

void ErrorHandler::handleError(const std::string& message, const std::string& file, int line, const std::string& function) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    ErrorInfo info{
        message,
        file,
        line,
        function
    };
    
    for (const auto& [id, callback] : m_errorCallbacks) {
        callback(info);
    }
}

void ErrorHandler::registerErrorCallback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_errorCallbacks[m_nextCallbackId++] = callback;
}

void ErrorHandler::unregisterErrorCallback(int callbackId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_errorCallbacks.erase(callbackId);
}

void ErrorHandler::registerLogCallback(LogCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logCallbacks[m_nextCallbackId++] = callback;
}

void ErrorHandler::unregisterLogCallback(int callbackId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logCallbacks.erase(callbackId);
}

void ErrorHandler::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logLevel = level;
}

LogLevel ErrorHandler::getLogLevel() const {
    return m_logLevel;
}

void ErrorHandler::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (level < m_logLevel) {
        return;
    }
    
    LogMessage msg{
        level,
        message
    };
    
    for (const auto& [id, callback] : m_logCallbacks) {
        callback(msg);
    }
}

} // namespace CoreNS 