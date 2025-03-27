#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <functional>
#include <optional>

namespace Core {
namespace AdvancedLogging {

/**
 * @brief Livelli di log supportati.
 */
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    OFF
};

/**
 * @brief Converte un LogLevel in stringa.
 * 
 * @param level Livello di log
 * @return Nome del livello di log
 */
inline std::string logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        case LogLevel::OFF: return "OFF";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Converte una stringa in LogLevel.
 * 
 * @param level Nome del livello di log
 * @return Livello di log
 */
inline LogLevel stringToLogLevel(const std::string& level) {
    if (level == "TRACE") return LogLevel::TRACE;
    if (level == "DEBUG") return LogLevel::DEBUG;
    if (level == "INFO") return LogLevel::INFO;
    if (level == "WARNING") return LogLevel::WARNING;
    if (level == "ERROR") return LogLevel::ERROR;
    if (level == "FATAL") return LogLevel::FATAL;
    if (level == "OFF") return LogLevel::OFF;
    return LogLevel::INFO; // Default
}

/**
 * @brief Struttura che rappresenta un messaggio di log.
 */
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string message;
    std::string category;
    std::string file;
    int line;
    std::string function;
    std::thread::id threadId;
    
    /**
     * @brief Formatta il messaggio di log.
     * 
     * @param includeMetadata Se true, include metadati come file e linea
     * @return Messaggio formattato
     */
    std::string formatMessage(bool includeMetadata = true) const {
        std::stringstream ss;
        
        // Timestamp
        auto time = std::chrono::system_clock::to_time_t(timestamp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            timestamp.time_since_epoch()) % 1000;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count() << " ";
        
        // Thread ID
        ss << "[" << std::hex << std::setw(4) << std::setfill('0') << threadId << "] ";
        
        // Log level
        ss << "[" << logLevelToString(level) << "] ";
        
        // Category
        if (!category.empty()) {
            ss << "[" << category << "] ";
        }
        
        // Message
        ss << message;
        
        // Metadata
        if (includeMetadata && !file.empty()) {
            ss << " (" << file;
            if (line > 0) {
                ss << ":" << line;
            }
            if (!function.empty()) {
                ss << ", " << function;
            }
            ss << ")";
        }
        
        return ss.str();
    }
};

/**
 * @brief Classe base per gli appender di log.
 * 
 * Gli appender si occupano di scrivere i messaggi di log su diversi destinazioni,
 * come file, console, ecc.
 */
class LogAppender {
public:
    /**
     * @brief Costruttore.
     * 
     * @param minLevel Livello minimo di log da processare
     */
    LogAppender(LogLevel minLevel = LogLevel::TRACE)
        : m_minLevel(minLevel) {}
    
    /**
     * @brief Distruttore virtuale.
     */
    virtual ~LogAppender() = default;
    
    /**
     * @brief Scrive un messaggio di log.
     * 
     * @param entry Messaggio di log
     */
    virtual void append(const LogEntry& entry) = 0;
    
    /**
     * @brief Imposta il livello minimo di log.
     * 
     * @param level Livello minimo di log
     */
    void setMinLevel(LogLevel level) {
        m_minLevel = level;
    }
    
    /**
     * @brief Ottiene il livello minimo di log.
     * 
     * @return Livello minimo di log
     */
    LogLevel getMinLevel() const {
        return m_minLevel;
    }
    
    /**
     * @brief Verifica se un livello di log deve essere processato.
     * 
     * @param level Livello di log da verificare
     * @return true se il livello deve essere processato, false altrimenti
     */
    bool shouldLog(LogLevel level) const {
        return level >= m_minLevel && m_minLevel != LogLevel::OFF;
    }

protected:
    LogLevel m_minLevel;
};

/**
 * @brief Appender per la console.
 */
class ConsoleAppender : public LogAppender {
public:
    /**
     * @brief Costruttore.
     * 
     * @param useColors Se true, usa colori diversi per i diversi livelli di log
     * @param minLevel Livello minimo di log da processare
     */
    ConsoleAppender(bool useColors = true, LogLevel minLevel = LogLevel::TRACE)
        : LogAppender(minLevel), m_useColors(useColors) {}
    
    /**
     * @brief Scrive un messaggio di log sulla console.
     * 
     * @param entry Messaggio di log
     */
    void append(const LogEntry& entry) override {
        if (!shouldLog(entry.level)) return;
        
        std::string message = entry.formatMessage();
        
        if (m_useColors) {
            message = addColor(message, entry.level);
        }
        
        if (entry.level >= LogLevel::ERROR) {
            std::cerr << message << std::endl;
        } else {
            std::cout << message << std::endl;
        }
    }

private:
    bool m_useColors;
    
    /**
     * @brief Aggiunge colore a un messaggio.
     * 
     * @param message Messaggio da colorare
     * @param level Livello di log
     * @return Messaggio colorato
     */
    std::string addColor(const std::string& message, LogLevel level) const {
        // Codici colore ANSI
        const std::string reset = "\033[0m";
        std::string color;
        
        switch (level) {
            case LogLevel::TRACE:
                color = "\033[90m"; // Grigio
                break;
            case LogLevel::DEBUG:
                color = "\033[36m"; // Ciano
                break;
            case LogLevel::INFO:
                color = "\033[32m"; // Verde
                break;
            case LogLevel::WARNING:
                color = "\033[33m"; // Giallo
                break;
            case LogLevel::ERROR:
                color = "\033[31m"; // Rosso
                break;
            case LogLevel::FATAL:
                color = "\033[35m"; // Magenta
                break;
            default:
                return message;
        }
        
        return color + message + reset;
    }
};

/**
 * @brief Logger con supporto per rotazione dei file.
 * 
 * Questa classe permette di loggare su file con rotazione automatica
 * quando il file raggiunge una certa dimensione, con opzione di
 * compressione dei file vecchi.
 */
class RotatingFileAppender : public LogAppender {
public:
    /**
     * @brief Costruttore.
     * 
     * @param baseFileName Nome base del file di log
     * @param maxFileSize Dimensione massima del file in byte
     * @param maxFiles Numero massimo di file di log da mantenere
     * @param compressOldLogs Se true, comprime i file di log vecchi
     * @param minLevel Livello minimo di log da processare
     */
    RotatingFileAppender(const std::string& baseFileName,
                        size_t maxFileSize = 10 * 1024 * 1024, // 10 MB
                        int maxFiles = 5,
                        bool compressOldLogs = true,
                        LogLevel minLevel = LogLevel::TRACE)
        : LogAppender(minLevel),
          m_baseFileName(baseFileName),
          m_maxFileSize(maxFileSize),
          m_maxFiles(maxFiles),
          m_compressOldLogs(compressOldLogs) {
        // Apriamo il file di log
        openLogFile();
    }
    
    /**
     * @brief Distruttore.
     */
    ~RotatingFileAppender() {
        // Chiudiamo il file di log
        if (m_file.is_open()) {
            m_file.close();
        }
    }
    
    /**
     * @brief Scrive un messaggio di log su file.
     * 
     * @param entry Messaggio di log
     */
    void append(const LogEntry& entry) override {
        if (!shouldLog(entry.level)) return;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Verifichiamo se dobbiamo ruotare i file
        checkRotation();
        
        // Scriviamo il messaggio
        if (m_file.is_open()) {
            m_file << entry.formatMessage() << std::endl;
            m_file.flush();
        }
    }
    
    /**
     * @brief Abilita o disabilita la compressione dei log vecchi.
     * 
     * @param enable Se true, abilita la compressione
     */
    void enableCompression(bool enable) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_compressOldLogs = enable;
    }
    
    /**
     * @brief Imposta la dimensione massima dei file di log.
     * 
     * @param maxFileSize Dimensione massima in byte
     */
    void setMaxFileSize(size_t maxFileSize) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxFileSize = maxFileSize;
    }
    
    /**
     * @brief Imposta il numero massimo di file di log.
     * 
     * @param maxFiles Numero massimo di file
     */
    void setMaxFiles(int maxFiles) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxFiles = maxFiles;
    }
    
    /**
     * @brief Forza la rotazione dei file di log.
     */
    void rotate() {
        std::lock_guard<std::mutex> lock(m_mutex);
        rotateLogFiles();
    }

private:
    std::string m_baseFileName;
    size_t m_maxFileSize;
    int m_maxFiles;
    bool m_compressOldLogs;
    
    std::ofstream m_file;
    std::mutex m_mutex;
    
    /**
     * @brief Apre il file di log.
     */
    void openLogFile() {
        m_file.close();
        m_file.open(m_baseFileName, std::ios::app);
        
        if (!m_file) {
            // Tentiamo di creare le directory necessarie
            auto path = std::filesystem::path(m_baseFileName);
            if (path.has_parent_path()) {
                std::filesystem::create_directories(path.parent_path());
                m_file.open(m_baseFileName, std::ios::app);
            }
        }
    }
    
    /**
     * @brief Verifica se dobbiamo ruotare i file di log.
     */
    void checkRotation() {
        if (!m_file.is_open()) return;
        
        // Otteniamo la dimensione del file
        m_file.flush();
        auto fileSize = std::filesystem::file_size(m_baseFileName);
        
        // Se il file è più grande della dimensione massima, ruotiamo
        if (fileSize >= m_maxFileSize) {
            rotateLogFiles();
        }
    }
    
    /**
     * @brief Ruota i file di log.
     */
    void rotateLogFiles() {
        // Chiudiamo il file corrente
        if (m_file.is_open()) {
            m_file.close();
        }
        
        // Eliminiamo il file più vecchio, se necessario
        if (m_maxFiles > 0) {
            std::string oldestFile = m_baseFileName + "." + std::to_string(m_maxFiles);
            std::filesystem::remove(oldestFile);
            
            // Se abbiamo la compressione abilitata, eliminiamo anche il file compresso
            if (m_compressOldLogs) {
                std::filesystem::remove(oldestFile + ".gz");
            }
        }
        
        // Rinominiamo i file esistenti
        for (int i = m_maxFiles - 1; i >= 1; --i) {
            std::string oldName = m_baseFileName + "." + std::to_string(i);
            std::string newName = m_baseFileName + "." + std::to_string(i + 1);
            
            if (m_compressOldLogs) {
                // Se il file è compresso, rinominiamo quello
                std::string oldCompressed = oldName + ".gz";
                std::string newCompressed = newName + ".gz";
                
                if (std::filesystem::exists(oldCompressed)) {
                    std::filesystem::rename(oldCompressed, newCompressed);
                } else if (std::filesystem::exists(oldName)) {
                    // Se il file non è compresso, lo rinominiamo e lo comprimiamo
                    std::filesystem::rename(oldName, newName);
                    compressLogFile(newName);
                }
            } else {
                // Senza compressione, rinominiamo semplicemente i file
                if (std::filesystem::exists(oldName)) {
                    std::filesystem::rename(oldName, newName);
                }
            }
        }
        
        // Rinominiamo il file corrente
        if (std::filesystem::exists(m_baseFileName)) {
            std::string firstBackup = m_baseFileName + ".1";
            std::filesystem::rename(m_baseFileName, firstBackup);
            
            // Se abbiamo un solo file di backup e la compressione è abilitata, lo comprimiamo
            if (m_maxFiles == 1 && m_compressOldLogs) {
                compressLogFile(firstBackup);
            }
        }
        
        // Riapriamo il file
        openLogFile();
    }
    
    /**
     * @brief Comprime un file di log.
     * 
     * @param filePath Percorso del file da comprimere
     * @return true se la compressione è avvenuta con successo, false altrimenti
     */
    bool compressLogFile(const std::string& filePath) {
        // Questa è una semplice implementazione che usa comandi esterni
        // In un'implementazione reale, si potrebbe usare una libreria come zlib
        
        std::string command;
#ifdef _WIN32
        command = "powershell -Command \"Compress-Archive -Path '" + filePath + "' -DestinationPath '" + filePath + ".zip'\"";
#else
        command = "gzip -f \"" + filePath + "\"";
#endif
        
        int result = std::system(command.c_str());
        return result == 0;
    }
};

/**
 * @brief Factory per creare appender.
 */
class LogAppenderFactory {
public:
    /**
     * @brief Crea un console appender.
     * 
     * @param useColors Se true, usa colori diversi per i diversi livelli di log
     * @param minLevel Livello minimo di log da processare
     * @return Puntatore condiviso all'appender
     */
    static std::shared_ptr<ConsoleAppender> createConsoleAppender(bool useColors = true, LogLevel minLevel = LogLevel::TRACE) {
        return std::make_shared<ConsoleAppender>(useColors, minLevel);
    }
    
    /**
     * @brief Crea un file appender con rotazione.
     * 
     * @param baseFileName Nome base del file di log
     * @param maxFileSize Dimensione massima del file in byte
     * @param maxFiles Numero massimo di file di log da mantenere
     * @param compressOldLogs Se true, comprime i file di log vecchi
     * @param minLevel Livello minimo di log da processare
     * @return Puntatore condiviso all'appender
     */
    static std::shared_ptr<RotatingFileAppender> createRotatingFileAppender(
        const std::string& baseFileName,
        size_t maxFileSize = 10 * 1024 * 1024, // 10 MB
        int maxFiles = 5,
        bool compressOldLogs = true,
        LogLevel minLevel = LogLevel::TRACE) {
        return std::make_shared<RotatingFileAppender>(baseFileName, maxFileSize, maxFiles, compressOldLogs, minLevel);
    }
};

/**
 * @brief Logger principale che distribuisce i messaggi agli appender.
 */
class Logger {
public:
    /**
     * @brief Ottiene l'istanza singleton del logger.
     * 
     * @return Riferimento all'istanza singleton
     */
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    /**
     * @brief Aggiunge un appender.
     * 
     * @param appender Puntatore condiviso all'appender
     */
    void addAppender(std::shared_ptr<LogAppender> appender) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_appenders.push_back(appender);
    }
    
    /**
     * @brief Rimuove un appender.
     * 
     * @param appender Puntatore condiviso all'appender
     * @return true se l'appender è stato rimosso, false se non esisteva
     */
    bool removeAppender(std::shared_ptr<LogAppender> appender) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = std::find(m_appenders.begin(), m_appenders.end(), appender);
        if (it != m_appenders.end()) {
            m_appenders.erase(it);
            return true;
        }
        
        return false;
    }
    
    /**
     * @brief Rimuove tutti gli appender.
     */
    void clearAppenders() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_appenders.clear();
    }
    
    /**
     * @brief Imposta il livello minimo di log.
     * 
     * @param level Livello minimo di log
     */
    void setLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_level = level;
    }
    
    /**
     * @brief Ottiene il livello minimo di log.
     * 
     * @return Livello minimo di log
     */
    LogLevel getLevel() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_level;
    }
    
    /**
     * @brief Verifica se un livello di log deve essere processato.
     * 
     * @param level Livello di log da verificare
     * @return true se il livello deve essere processato, false altrimenti
     */
    bool shouldLog(LogLevel level) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return level >= m_level && m_level != LogLevel::OFF;
    }
    
    /**
     * @brief Registra un messaggio di log.
     * 
     * @param level Livello del log
     * @param message Messaggio da registrare
     * @param category Categoria del log
     * @param file Nome del file sorgente
     * @param line Numero di riga nel file sorgente
     * @param function Nome della funzione
     */
    void log(LogLevel level, const std::string& message, const std::string& category = "",
           const std::string& file = "", int line = 0, const std::string& function = "") {
        
        if (!shouldLog(level)) return;
        
        LogEntry entry;
        entry.timestamp = std::chrono::system_clock::now();
        entry.level = level;
        entry.message = message;
        entry.category = category;
        entry.file = file;
        entry.line = line;
        entry.function = function;
        entry.threadId = std::this_thread::get_id();
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto& appender : m_appenders) {
            if (appender->shouldLog(level)) {
                appender->append(entry);
            }
        }
    }
    
    /**
     * @brief Registra un messaggio di debug.
     * 
     * @param message Messaggio da registrare
     * @param category Categoria del log
     * @param file Nome del file sorgente
     * @param line Numero di riga nel file sorgente
     * @param function Nome della funzione
     */
    void debug(const std::string& message, const std::string& category = "",
             const std::string& file = "", int line = 0, const std::string& function = "") {
        log(LogLevel::DEBUG, message, category, file, line, function);
    }
    
    /**
     * @brief Registra un messaggio informativo.
     * 
     * @param message Messaggio da registrare
     * @param category Categoria del log
     * @param file Nome del file sorgente
     * @param line Numero di riga nel file sorgente
     * @param function Nome della funzione
     */
    void info(const std::string& message, const std::string& category = "",
            const std::string& file = "", int line = 0, const std::string& function = "") {
        log(LogLevel::INFO, message, category, file, line, function);
    }
    
    /**
     * @brief Registra un avviso.
     * 
     * @param message Messaggio da registrare
     * @param category Categoria del log
     * @param file Nome del file sorgente
     * @param line Numero di riga nel file sorgente
     * @param function Nome della funzione
     */
    void warning(const std::string& message, const std::string& category = "",
               const std::string& file = "", int line = 0, const std::string& function = "") {
        log(LogLevel::WARNING, message, category, file, line, function);
    }
    
    /**
     * @brief Registra un errore.
     * 
     * @param message Messaggio da registrare
     * @param category Categoria del log
     * @param file Nome del file sorgente
     * @param line Numero di riga nel file sorgente
     * @param function Nome della funzione
     */
    void error(const std::string& message, const std::string& category = "",
             const std::string& file = "", int line = 0, const std::string& function = "") {
        log(LogLevel::ERROR, message, category, file, line, function);
    }
    
    /**
     * @brief Registra un errore fatale.
     * 
     * @param message Messaggio da registrare
     * @param category Categoria del log
     * @param file Nome del file sorgente
     * @param line Numero di riga nel file sorgente
     * @param function Nome della funzione
     */
    void fatal(const std::string& message, const std::string& category = "",
             const std::string& file = "", int line = 0, const std::string& function = "") {
        log(LogLevel::FATAL, message, category, file, line, function);
    }

private:
    // Costruttore privato per pattern Singleton
    Logger() : m_level(LogLevel::INFO) {
        // Aggiungiamo un appender console di default
        addAppender(LogAppenderFactory::createConsoleAppender());
    }
    
    // Distruttore privato
    ~Logger() = default;
    
    // Disabilita copia e assegnazione
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    LogLevel m_level;
    std::vector<std::shared_ptr<LogAppender>> m_appenders;
    mutable std::mutex m_mutex;
};

/**
 * @brief Macro per loggare messaggi.
 */
#define LOG_TRACE(message, category) \
    Core::AdvancedLogging::Logger::getInstance().log( \
        Core::AdvancedLogging::LogLevel::TRACE, message, category, __FILE__, __LINE__, __FUNCTION__)

#define LOG_DEBUG(message, category) \
    Core::AdvancedLogging::Logger::getInstance().log( \
        Core::AdvancedLogging::LogLevel::DEBUG, message, category, __FILE__, __LINE__, __FUNCTION__)

#define LOG_INFO(message, category) \
    Core::AdvancedLogging::Logger::getInstance().log( \
        Core::AdvancedLogging::LogLevel::INFO, message, category, __FILE__, __LINE__, __FUNCTION__)

#define LOG_WARNING(message, category) \
    Core::AdvancedLogging::Logger::getInstance().log( \
        Core::AdvancedLogging::LogLevel::WARNING, message, category, __FILE__, __LINE__, __FUNCTION__)

#define LOG_ERROR(message, category) \
    Core::AdvancedLogging::Logger::getInstance().log( \
        Core::AdvancedLogging::LogLevel::ERROR, message, category, __FILE__, __LINE__, __FUNCTION__)

#define LOG_FATAL(message, category) \
    Core::AdvancedLogging::Logger::getInstance().log( \
        Core::AdvancedLogging::LogLevel::FATAL, message, category, __FILE__, __LINE__, __FUNCTION__)

} // namespace AdvancedLogging
} // namespace Core 