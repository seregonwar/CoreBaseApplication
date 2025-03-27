#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <sstream>
#include <mutex>

/**
 * @brief Enum che definisce i livelli di log supportati
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

/**
 * @brief Struct che rappresenta un messaggio di log
 */
struct LogMessage {
    LogLevel level;
    std::string message;
    std::string timestamp;
    std::string sourceFile;
    int lineNumber;
    std::string functionName;
    std::string stackTrace;
};

/**
 * @brief Classe per la gestione degli errori e del logging
 */
class ErrorHandler {
public:
    /**
     * @brief Costruttore dell'ErrorHandler
     * @param logFilePath Percorso del file di log
     * @param enableConsoleOutput Se true, i log vengono anche stampati sulla console
     */
    ErrorHandler(const std::string& logFilePath = "application.log", bool enableConsoleOutput = true);
    
    /**
     * @brief Distruttore dell'ErrorHandler
     */
    ~ErrorHandler();
    
    /**
     * @brief Inizializza l'ErrorHandler
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize();
    
    /**
     * @brief Registra un messaggio di log
     * @param level Livello del log
     * @param message Messaggio da registrare
     * @param sourceFile Nome del file sorgente (opzionale)
     * @param lineNumber Numero di riga nel file sorgente (opzionale)
     * @param functionName Nome della funzione (opzionale)
     */
    void log(LogLevel level, const std::string& message, 
             const std::string& sourceFile = "", int lineNumber = 0, 
             const std::string& functionName = "");
    
    /**
     * @brief Registra un errore
     * @param errorMessage Messaggio di errore
     * @param sourceFile Nome del file sorgente (opzionale)
     * @param lineNumber Numero di riga nel file sorgente (opzionale)
     * @param functionName Nome della funzione (opzionale)
     */
    static void logError(const std::string& errorMessage, 
                         const std::string& sourceFile = "", int lineNumber = 0, 
                         const std::string& functionName = "");
    
    /**
     * @brief Registra un avviso
     * @param warningMessage Messaggio di avviso
     * @param sourceFile Nome del file sorgente (opzionale)
     * @param lineNumber Numero di riga nel file sorgente (opzionale)
     * @param functionName Nome della funzione (opzionale)
     */
    static void logWarning(const std::string& warningMessage, 
                          const std::string& sourceFile = "", int lineNumber = 0, 
                          const std::string& functionName = "");
    
    /**
     * @brief Registra un messaggio informativo
     * @param infoMessage Messaggio informativo
     * @param sourceFile Nome del file sorgente (opzionale)
     * @param lineNumber Numero di riga nel file sorgente (opzionale)
     * @param functionName Nome della funzione (opzionale)
     */
    static void logInfo(const std::string& infoMessage, 
                       const std::string& sourceFile = "", int lineNumber = 0, 
                       const std::string& functionName = "");
    
    /**
     * @brief Registra un messaggio di debug
     * @param debugMessage Messaggio di debug
     * @param sourceFile Nome del file sorgente (opzionale)
     * @param lineNumber Numero di riga nel file sorgente (opzionale)
     * @param functionName Nome della funzione (opzionale)
     */
    static void logDebug(const std::string& debugMessage, 
                        const std::string& sourceFile = "", int lineNumber = 0, 
                        const std::string& functionName = "");
    
    /**
     * @brief Registra un errore fatale e termina l'applicazione
     * @param fatalMessage Messaggio di errore fatale
     * @param sourceFile Nome del file sorgente (opzionale)
     * @param lineNumber Numero di riga nel file sorgente (opzionale)
     * @param functionName Nome della funzione (opzionale)
     */
    static void logFatal(const std::string& fatalMessage, 
                        const std::string& sourceFile = "", int lineNumber = 0, 
                        const std::string& functionName = "");
    
    /**
     * @brief Imposta il livello minimo di log da registrare
     * @param level Livello minimo di log
     */
    void setLogLevel(LogLevel level);
    
    /**
     * @brief Ottiene il livello minimo di log corrente
     * @return Livello minimo di log
     */
    LogLevel getLogLevel() const;
    
    /**
     * @brief Abilita o disabilita l'output sulla console
     * @param enable Se true, abilita l'output sulla console
     */
    void setConsoleOutput(bool enable);
    
    /**
     * @brief Registra una callback da chiamare quando viene registrato un log di un certo livello
     * @param level Livello di log
     * @param callback Funzione da chiamare
     * @return ID della callback registrata
     */
    int registerLogCallback(LogLevel level, std::function<void(const LogMessage&)> callback);
    
    /**
     * @brief Rimuove una callback registrata
     * @param callbackId ID della callback da rimuovere
     * @return true se la callback è stata rimossa, false se non esisteva
     */
    bool unregisterLogCallback(int callbackId);
    
    /**
     * @brief Ottiene lo stack trace corrente
     * @param skipFrames Numero di frame da saltare nello stack trace
     * @return Stack trace come stringa
     */
    static std::string getStackTrace(int skipFrames = 0);
    
private:
    std::string m_logFilePath;
    bool m_consoleOutput;
    LogLevel m_logLevel;
    static ErrorHandler* s_instance;
    std::mutex m_mutex;
    int m_nextCallbackId;
    std::unordered_map<int, std::pair<LogLevel, std::function<void(const LogMessage&)>>> m_callbacks;
    
    /**
     * @brief Ottiene un timestamp corrente
     * @return Timestamp come stringa
     */
    std::string getCurrentTimestamp() const;
    
    /**
     * @brief Converte un livello di log in stringa
     * @param level Livello di log
     * @return Stringa rappresentante il livello di log
     */
    std::string logLevelToString(LogLevel level) const;
    
    /**
     * @brief Scrive un messaggio di log su file
     * @param logMessage Messaggio di log da scrivere
     */
    void writeToFile(const LogMessage& logMessage);
    
    /**
     * @brief Scrive un messaggio di log sulla console
     * @param logMessage Messaggio di log da scrivere
     */
    void writeToConsole(const LogMessage& logMessage);
};
