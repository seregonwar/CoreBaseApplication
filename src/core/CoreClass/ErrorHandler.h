#pragma once

#include <string>
#include <functional>
#include <mutex>
#include <map>
#include "LogLevel.h"

namespace CoreNS {

/**
 * @brief Struttura per memorizzare le informazioni di un errore
 */
struct ErrorInfo {
    std::string message;
    std::string file;
    int line;
    std::string function;
};

/**
 * @brief Struttura per memorizzare un messaggio di log
 */
struct LogMessage {
    LogLevel level;
    std::string message;
};

using ErrorCallback = std::function<void(const ErrorInfo&)>;
using LogCallback = std::function<void(const LogMessage&)>;

/**
 * @brief Classe per la gestione degli errori e del logging
 */
class ErrorHandler {
public:
    ErrorHandler();
    ~ErrorHandler();
    
    void initialize();
    void shutdown();
    
    void handleError(const std::string& message, const std::string& file, int line, const std::string& function);
    
    void registerErrorCallback(ErrorCallback callback);
    void unregisterErrorCallback(int callbackId);
    void registerLogCallback(LogCallback callback);
    void unregisterLogCallback(int callbackId);
    
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;
    
    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERR, message); }
    void critical(const std::string& message) { log(LogLevel::CRITICAL, message); }
    
    void log(LogLevel level, const std::string& message);

private:
    std::map<int, ErrorCallback> m_errorCallbacks;
    std::map<int, LogCallback> m_logCallbacks;
    int m_nextCallbackId{0};
    std::mutex m_mutex;
    LogLevel m_logLevel{LogLevel::INFO};
};

} // namespace CoreNS