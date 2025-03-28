#pragma once

#include "RotatingLogger.h"
#include "../CoreClass/ErrorHandler.h"

#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <functional>
#include <sstream>

namespace Core {
namespace AdvancedLogging {

/**
 * @brief Classe che integra il logger avanzato con l'ErrorHandler del Core.
 */
class LoggingIntegration {
public:
    /**
     * @brief Ottiene l'istanza della classe (singleton).
     * 
     * @return Riferimento all'istanza
     */
    static LoggingIntegration& getInstance() {
        static LoggingIntegration instance;
        return instance;
    }
    
    /**
     * @brief Configura l'integrazione con l'ErrorHandler.
     * 
     * @param errorHandler Puntatore all'ErrorHandler
     */
    void configure(ErrorHandler* errorHandler) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!errorHandler) return;
        
        m_errorHandler = errorHandler;
        
        // Registriamo un callback nell'ErrorHandler che inoltri i messaggi al nostro Logger
        m_errorHandler->registerLogCallback(LogLevel::INFO, [this](const LogMessage& logMsg) {
            // Convertiamo il LogLevel dell'ErrorHandler nel nostro LogLevel
            AdvancedLogging::LogLevel advLevel = convertLogLevel(logMsg.level);
            
            // Invio il messaggio al logger avanzato
            Logger::getInstance().log(advLevel, logMsg.message, "Core");
        });
    }
    
    /**
     * @brief Configura gli appender da utilizzare.
     * 
     * @param useConsole Se true, usa un appender console
     * @param logFilePath Percorso del file di log, se vuoto non usa file logging
     * @param maxFileSize Dimensione massima del file di log in byte
     * @param maxFiles Numero massimo di file di log da mantenere
     * @param compressOldLogs Se true, comprime i file di log vecchi
     */
    void configureAppenders(bool useConsole = true, 
                          const std::string& logFilePath = "", 
                          size_t maxFileSize = 10 * 1024 * 1024, 
                          int maxFiles = 5,
                          bool compressOldLogs = true) {
        // Otteniamo l'istanza del logger
        auto& logger = Logger::getInstance();
        
        // Rimuoviamo tutti gli appender esistenti
        logger.clearAppenders();
        
        // Aggiungiamo l'appender console se richiesto
        if (useConsole) {
            logger.addAppender(LogAppenderFactory::createConsoleAppender());
        }
        
        // Aggiungiamo l'appender file se il percorso è fornito
        if (!logFilePath.empty()) {
            logger.addAppender(LogAppenderFactory::createRotatingFileAppender(
                logFilePath, maxFileSize, maxFiles, compressOldLogs));
        }
    }
    
    /**
     * @brief Imposta il livello minimo di log.
     * 
     * @param level Livello minimo di log
     */
    void setLogLevel(AdvancedLogging::LogLevel level) {
        Logger::getInstance().setLevel(level);
    }
    
    /**
     * @brief Ottiene il livello minimo di log.
     * 
     * @return Livello minimo di log
     */
    AdvancedLogging::LogLevel getLogLevel() const {
        return Logger::getInstance().getLevel();
    }
    
    /**
     * @brief Registra un messaggio di log specificando il livello.
     * 
     * @param level Livello del log
     * @param message Messaggio da registrare
     * @param category Categoria del log
     */
    void log(AdvancedLogging::LogLevel level, const std::string& message, const std::string& category = "") {
        Logger::getInstance().log(level, message, category);
        
        // Se abbiamo un ErrorHandler configurato, inoltra anche a quello
        if (m_errorHandler) {
            m_errorHandler->log(convertLogLevel(level), message);
        }
    }
    
    /**
     * @brief Registra un messaggio di debug.
     * 
     * @param message Messaggio da registrare
     * @param category Categoria del log
     */
    void debug(const std::string& message, const std::string& category = "") {
        log(AdvancedLogging::LogLevel::DEBUG, message, category);
    }
    
    /**
     * @brief Registra un messaggio informativo.
     * 
     * @param message Messaggio da registrare
     * @param category Categoria del log
     */
    void info(const std::string& message, const std::string& category = "") {
        log(AdvancedLogging::LogLevel::INFO, message, category);
    }
    
    /**
     * @brief Registra un avviso.
     * 
     * @param message Messaggio da registrare
     * @param category Categoria del log
     */
    void warning(const std::string& message, const std::string& category = "") {
        log(AdvancedLogging::LogLevel::WARNING, message, category);
    }
    
    /**
     * @brief Registra un errore.
     * 
     * @param message Messaggio da registrare
     * @param category Categoria del log
     */
    void error(const std::string& message, const std::string& category = "") {
        log(AdvancedLogging::LogLevel::ERROR, message, category);
    }
    
    /**
     * @brief Registra un errore fatale.
     * 
     * @param message Messaggio da registrare
     * @param category Categoria del log
     */
    void fatal(const std::string& message, const std::string& category = "") {
        log(AdvancedLogging::LogLevel::FATAL, message, category);
    }
    
    /**
     * @brief Registra le metriche di sistema.
     * 
     * Questo metodo registra periodicamente informazioni sul sistema,
     * come l'uso della memoria, del disco, ecc.
     * 
     * @param enabled Se true, abilita la registrazione delle metriche
     * @param intervalSeconds Intervallo in secondi tra le registrazioni
     */
    void enableMetricsLogging(bool enabled, unsigned int intervalSeconds = 60) {
        std::lock_guard<std::mutex> lock(m_metricsMutex);
        
        // Se era già abilitato e vogliamo disabilitarlo
        if (m_metricsLoggingEnabled && !enabled) {
            m_metricsLoggingEnabled = false;
            if (m_metricsThread.joinable()) {
                m_metricsThread.join();
            }
            return;
        }
        
        // Se era già disabilitato e vogliamo abilitarlo
        if (!m_metricsLoggingEnabled && enabled) {
            m_metricsLoggingEnabled = true;
            m_metricsIntervalSeconds = intervalSeconds;
            
            // Avviamo un thread che registra le metriche periodicamente
            m_metricsThread = std::thread([this, intervalSeconds]() {
                while (m_metricsLoggingEnabled) {
                    logSystemMetrics();
                    std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
                }
            });
        }
        
        // Se era già abilitato e vogliamo cambiare l'intervallo
        if (m_metricsLoggingEnabled && enabled && m_metricsIntervalSeconds != intervalSeconds) {
            m_metricsIntervalSeconds = intervalSeconds;
        }
    }
    
    /**
     * @brief Verifica se la registrazione delle metriche è abilitata.
     * 
     * @return true se la registrazione delle metriche è abilitata, false altrimenti
     */
    bool isMetricsLoggingEnabled() const {
        std::lock_guard<std::mutex> lock(m_metricsMutex);
        return m_metricsLoggingEnabled;
    }
    
    /**
     * @brief Ottiene l'intervallo di registrazione delle metriche.
     * 
     * @return Intervallo in secondi
     */
    unsigned int getMetricsInterval() const {
        std::lock_guard<std::mutex> lock(m_metricsMutex);
        return m_metricsIntervalSeconds;
    }
    
private:
    // Costruttore privato per pattern Singleton
    LoggingIntegration() : m_errorHandler(nullptr), m_metricsLoggingEnabled(false), m_metricsIntervalSeconds(60) {}
    
    // Distruttore privato
    ~LoggingIntegration() {
        if (m_metricsLoggingEnabled) {
            m_metricsLoggingEnabled = false;
            if (m_metricsThread.joinable()) {
                m_metricsThread.join();
            }
        }
    }
    
    // Disabilita copia e assegnazione
    LoggingIntegration(const LoggingIntegration&) = delete;
    LoggingIntegration& operator=(const LoggingIntegration&) = delete;
    
    /**
     * @brief Converte un LogLevel di AdvancedLogging in un LogLevel standard.
     * @param level LogLevel di AdvancedLogging
     * @return LogLevel standard
     */
    LogLevel convertLogLevel(AdvancedLogging::LogLevel level) const {
        switch (level) {
            case AdvancedLogging::LogLevel::TRACE:
                return LogLevel::DEBUG;
            case AdvancedLogging::LogLevel::DEBUG:
                return LogLevel::DEBUG;
            case AdvancedLogging::LogLevel::INFO:
                return LogLevel::INFO;
            case AdvancedLogging::LogLevel::WARNING:
                return LogLevel::WARNING;
            case AdvancedLogging::LogLevel::ERROR:
                return LogLevel::ERROR;
            case AdvancedLogging::LogLevel::FATAL:
                return LogLevel::FATAL;
            case AdvancedLogging::LogLevel::OFF:
                return LogLevel::INFO;
            default:
                return LogLevel::INFO;
        }
    }
    
    /**
     * @brief Converte un LogLevel standard in un LogLevel di AdvancedLogging.
     * @param level LogLevel standard
     * @return LogLevel di AdvancedLogging
     */
    AdvancedLogging::LogLevel convertLogLevel(LogLevel level) const {
        switch (level) {
            case LogLevel::DEBUG:
                return AdvancedLogging::LogLevel::DEBUG;
            case LogLevel::INFO:
                return AdvancedLogging::LogLevel::INFO;
            case LogLevel::WARNING:
                return AdvancedLogging::LogLevel::WARNING;
            case LogLevel::ERROR:
                return AdvancedLogging::LogLevel::ERROR;
            case LogLevel::FATAL:
                return AdvancedLogging::LogLevel::FATAL;
            default:
                return AdvancedLogging::LogLevel::INFO;
        }
    }
    
    /**
     * @brief Registra le metriche di sistema.
     */
    void logSystemMetrics() {
        // Registriamo informazioni di base sul sistema
        std::stringstream ss;
        ss << "Sistema: Registrazione metriche attiva con intervallo di " << m_metricsIntervalSeconds << " secondi";
        Logger::getInstance().info(ss.str(), "SystemMetrics");
        
        // In una versione più avanzata, qui si potrebbero registrare metriche reali
        // come utilizzo CPU, memoria, ecc.
        
        // Registriamo anche informazioni sul logger stesso
        ss.str("");
        ss << "Logger: Current log level is " << logLevelToString(Logger::getInstance().getLevel());
        Logger::getInstance().info(ss.str(), "SystemMetrics");
    }
    
    ErrorHandler* m_errorHandler;
    std::mutex m_mutex;
    
    bool m_metricsLoggingEnabled;
    unsigned int m_metricsIntervalSeconds;
    std::thread m_metricsThread;
    mutable std::mutex m_metricsMutex;
};

/**
 * @brief Macro per l'accesso rapido all'integrazione del logging.
 */
#define LOG_INTEGRATION Core::AdvancedLogging::LoggingIntegration::getInstance()

/**
 * @brief Macro per loggare messaggi tramite l'integrazione.
 */
#define LOG_INTEGRATED_TRACE(message, category) \
    LOG_INTEGRATION.log(Core::AdvancedLogging::LogLevel::TRACE, message, category)

#define LOG_INTEGRATED_DEBUG(message, category) \
    LOG_INTEGRATION.log(Core::AdvancedLogging::LogLevel::DEBUG, message, category)

#define LOG_INTEGRATED_INFO(message, category) \
    LOG_INTEGRATION.log(Core::AdvancedLogging::LogLevel::INFO, message, category)

#define LOG_INTEGRATED_WARNING(message, category) \
    LOG_INTEGRATION.log(Core::AdvancedLogging::LogLevel::WARNING, message, category)

#define LOG_INTEGRATED_ERROR(message, category) \
    LOG_INTEGRATION.log(Core::AdvancedLogging::LogLevel::ERROR, message, category)

#define LOG_INTEGRATED_FATAL(message, category) \
    LOG_INTEGRATION.log(Core::AdvancedLogging::LogLevel::FATAL, message, category)

} // namespace AdvancedLogging
} // namespace Core 