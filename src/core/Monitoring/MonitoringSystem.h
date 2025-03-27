#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <optional>
#include <queue>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <deque>

namespace Core {
namespace Monitoring {

/**
 * @brief Tipo di evento di monitoraggio.
 */
enum class MonitoringEventType {
    SYSTEM,         ///< Eventi di sistema
    PERFORMANCE,    ///< Eventi di prestazioni
    MEMORY,         ///< Eventi di memoria
    NETWORK,        ///< Eventi di rete
    FUNCTION,       ///< Eventi di funzione
    ERROR,          ///< Eventi di errore
    DEBUG,          ///< Eventi di debug
    USER,           ///< Eventi definiti dall'utente
    CUSTOM          ///< Eventi personalizzati
};

/**
 * @brief Livello di severità dell'evento.
 */
enum class MonitoringSeverity {
    TRACE,          ///< Traccia dettagliata
    DEBUG,          ///< Debug
    INFO,           ///< Informazione
    WARNING,        ///< Avviso
    ERROR,          ///< Errore
    CRITICAL        ///< Critico
};

/**
 * @brief Converte un tipo di evento in stringa.
 * @param type Tipo di evento
 * @return Stringa rappresentante il tipo
 */
inline std::string toString(MonitoringEventType type) {
    switch (type) {
        case MonitoringEventType::SYSTEM: return "SYSTEM";
        case MonitoringEventType::PERFORMANCE: return "PERFORMANCE";
        case MonitoringEventType::MEMORY: return "MEMORY";
        case MonitoringEventType::NETWORK: return "NETWORK";
        case MonitoringEventType::FUNCTION: return "FUNCTION";
        case MonitoringEventType::ERROR: return "ERROR";
        case MonitoringEventType::DEBUG: return "DEBUG";
        case MonitoringEventType::USER: return "USER";
        case MonitoringEventType::CUSTOM: return "CUSTOM";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Converte un livello di severità in stringa.
 * @param severity Livello di severità
 * @return Stringa rappresentante il livello
 */
inline std::string toString(MonitoringSeverity severity) {
    switch (severity) {
        case MonitoringSeverity::TRACE: return "TRACE";
        case MonitoringSeverity::DEBUG: return "DEBUG";
        case MonitoringSeverity::INFO: return "INFO";
        case MonitoringSeverity::WARNING: return "WARNING";
        case MonitoringSeverity::ERROR: return "ERROR";
        case MonitoringSeverity::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Rappresenta un evento di monitoraggio.
 */
class MonitoringEvent {
public:
    MonitoringEventType type;                     ///< Tipo di evento
    MonitoringSeverity severity;                  ///< Severità dell'evento
    std::string name;                             ///< Nome dell'evento
    std::string description;                      ///< Descrizione dell'evento
    std::string file;                             ///< File sorgente
    int line;                                     ///< Linea nel file
    std::string function;                         ///< Funzione
    std::chrono::system_clock::time_point timestamp; ///< Timestamp
    std::thread::id threadId;                     ///< ID del thread
    
    /**
     * @brief Costruttore.
     * @param t Tipo di evento
     * @param sev Severità dell'evento
     * @param n Nome dell'evento
     * @param desc Descrizione dell'evento
     * @param f File sorgente
     * @param l Linea nel file
     * @param func Funzione
     */
    MonitoringEvent(MonitoringEventType t, MonitoringSeverity sev, 
                  const std::string& n, const std::string& desc,
                  const std::string& f = "", int l = 0, const std::string& func = "")
        : type(t), severity(sev), name(n), description(desc),
          file(f), line(l), function(func),
          timestamp(std::chrono::system_clock::now()),
          threadId(std::this_thread::get_id()) {}
    
    /**
     * @brief Aggiunge dati numerici all'evento.
     * @param key Chiave
     * @param value Valore
     */
    void addNumericData(const std::string& key, double value) {
        m_numericData[key] = value;
    }
    
    /**
     * @brief Aggiunge dati stringa all'evento.
     * @param key Chiave
     * @param value Valore
     */
    void addStringData(const std::string& key, const std::string& value) {
        m_stringData[key] = value;
    }
    
    /**
     * @brief Ottiene i dati numerici dell'evento.
     * @return Mappa di dati numerici
     */
    const std::unordered_map<std::string, double>& getNumericData() const {
        return m_numericData;
    }
    
    /**
     * @brief Ottiene i dati stringa dell'evento.
     * @return Mappa di dati stringa
     */
    const std::unordered_map<std::string, std::string>& getStringData() const {
        return m_stringData;
    }
    
    /**
     * @brief Rappresentazione dell'evento come stringa.
     * @return Stringa rappresentante l'evento
     */
    std::string toString() const {
        std::ostringstream oss;
        
        // Formatta il timestamp
        auto time_t_timestamp = std::chrono::system_clock::to_time_t(timestamp);
        std::tm tm_timestamp = *std::localtime(&time_t_timestamp);
        
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_timestamp);
        
        // Formatta l'evento
        oss << buffer << " [" << ::Core::Monitoring::toString(severity) << "] [" << ::Core::Monitoring::toString(type) << "] [" 
            << threadId << "] " << name << ": " << description;
        
        // Aggiungi informazioni sul file se disponibili
        if (!file.empty()) {
            oss << " (" << file;
            if (line > 0) {
                oss << ":" << line;
            }
            if (!function.empty()) {
                oss << ", " << function;
            }
            oss << ")";
        }
        
        // Aggiungi dati numerici
        if (!m_numericData.empty()) {
            oss << " | Numeric Data: ";
            for (const auto& [key, value] : m_numericData) {
                oss << key << "=" << value << "; ";
            }
        }
        
        // Aggiungi dati stringa
        if (!m_stringData.empty()) {
            oss << " | String Data: ";
            for (const auto& [key, value] : m_stringData) {
                oss << key << "=\"" << value << "\"; ";
            }
        }
        
        return oss.str();
    }
    
private:
    std::unordered_map<std::string, double> m_numericData;  ///< Dati numerici
    std::unordered_map<std::string, std::string> m_stringData;  ///< Dati stringa
};

/**
 * @brief Interfaccia per un canale di comunicazione del monitoraggio.
 */
class IMonitoringChannel {
public:
    /**
     * @brief Distruttore virtuale.
     */
    virtual ~IMonitoringChannel() = default;
    
    /**
     * @brief Inizializza il canale.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Chiude il canale.
     * @return true se la chiusura è avvenuta con successo, false altrimenti
     */
    virtual bool shutdown() = 0;
    
    /**
     * @brief Invia un evento al canale.
     * @param event Evento da inviare
     * @return true se l'invio è avvenuto con successo, false altrimenti
     */
    virtual bool sendEvent(const MonitoringEvent& event) = 0;
    
    /**
     * @brief Ottiene il nome del canale.
     * @return Nome del canale
     */
    virtual std::string getName() const = 0;
};

/**
 * @brief Canale di monitoraggio che scrive su file.
 */
class FileMonitoringChannel : public IMonitoringChannel {
public:
    /**
     * @brief Costruttore.
     * @param filePath Percorso del file
     * @param minSeverity Severità minima degli eventi da registrare
     */
    FileMonitoringChannel(const std::string& filePath, 
                        MonitoringSeverity minSeverity = MonitoringSeverity::INFO)
        : m_filePath(filePath), m_minSeverity(minSeverity), m_initialized(false) {}
    
    /**
     * @brief Distruttore.
     */
    ~FileMonitoringChannel() override {
        shutdown();
    }
    
    /**
     * @brief Inizializza il canale.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_initialized) return true;
        
        // Crea la directory se necessario
        std::filesystem::path path(m_filePath);
        std::filesystem::create_directories(path.parent_path());
        
        // Apri il file
        m_file.open(m_filePath, std::ios::out | std::ios::app);
        if (!m_file.is_open()) {
            return false;
        }
        
        m_initialized = true;
        
        // Scrivi un'intestazione
        m_file << "=== Monitoring Session Started at " 
               << std::chrono::system_clock::now().time_since_epoch().count() 
               << " ===" << std::endl;
        
        return true;
    }
    
    /**
     * @brief Chiude il canale.
     * @return true se la chiusura è avvenuta con successo, false altrimenti
     */
    bool shutdown() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) return true;
        
        // Scrivi un footer
        m_file << "=== Monitoring Session Ended at " 
               << std::chrono::system_clock::now().time_since_epoch().count() 
               << " ===" << std::endl;
        
        // Chiudi il file
        m_file.close();
        
        m_initialized = false;
        
        return true;
    }
    
    /**
     * @brief Invia un evento al canale.
     * @param event Evento da inviare
     * @return true se l'invio è avvenuto con successo, false altrimenti
     */
    bool sendEvent(const MonitoringEvent& event) override {
        // Filtra per severità
        if (event.severity < m_minSeverity) {
            return true;
        }
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) {
            return false;
        }
        
        // Scrivi l'evento sul file
        m_file << event.toString() << std::endl;
        
        // Flush per assicurarsi che il contenuto sia scritto
        m_file.flush();
        
        return true;
    }
    
    /**
     * @brief Ottiene il nome del canale.
     * @return Nome del canale
     */
    std::string getName() const override {
        return "FileMonitoringChannel:" + m_filePath;
    }

private:
    std::string m_filePath;           ///< Percorso del file
    MonitoringSeverity m_minSeverity; ///< Severità minima
    bool m_initialized;               ///< Se il canale è inizializzato
    std::ofstream m_file;             ///< File di output
    std::mutex m_mutex;               ///< Mutex per accesso thread-safe
};

/**
 * @brief Canale di monitoraggio che mantiene gli eventi in memoria.
 */
class InMemoryMonitoringChannel : public IMonitoringChannel {
public:
    /**
     * @brief Costruttore.
     * @param maxEvents Numero massimo di eventi da mantenere in memoria
     * @param minSeverity Severità minima degli eventi da registrare
     */
    InMemoryMonitoringChannel(size_t maxEvents = 1000, 
                            MonitoringSeverity minSeverity = MonitoringSeverity::DEBUG)
        : m_maxEvents(maxEvents), m_minSeverity(minSeverity), m_initialized(false) {}
    
    /**
     * @brief Distruttore.
     */
    ~InMemoryMonitoringChannel() override {
        shutdown();
    }
    
    /**
     * @brief Inizializza il canale.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_initialized) return true;
        
        // Pulisci gli eventi
        m_events.clear();
        
        m_initialized = true;
        
        return true;
    }
    
    /**
     * @brief Chiude il canale.
     * @return true se la chiusura è avvenuta con successo, false altrimenti
     */
    bool shutdown() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) return true;
        
        // Pulisci gli eventi
        m_events.clear();
        
        m_initialized = false;
        
        return true;
    }
    
    /**
     * @brief Invia un evento al canale.
     * @param event Evento da inviare
     * @return true se l'invio è avvenuto con successo, false altrimenti
     */
    bool sendEvent(const MonitoringEvent& event) override {
        // Filtra per severità
        if (event.severity < m_minSeverity) {
            return true;
        }
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) {
            return false;
        }
        
        // Aggiungi l'evento
        m_events.push_back(event);
        
        // Limita il numero di eventi
        if (m_events.size() > m_maxEvents) {
            m_events.erase(m_events.begin());
        }
        
        return true;
    }
    
    /**
     * @brief Ottiene il nome del canale.
     * @return Nome del canale
     */
    std::string getName() const override {
        return "InMemoryMonitoringChannel";
    }
    
    /**
     * @brief Ottiene tutti gli eventi.
     * @return Vettore di eventi
     */
    std::vector<MonitoringEvent> getEvents() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_events;
    }
    
    /**
     * @brief Ottiene gli eventi filtrati per tipo.
     * @param type Tipo di evento
     * @return Vettore di eventi
     */
    std::vector<MonitoringEvent> getEventsByType(MonitoringEventType type) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<MonitoringEvent> filteredEvents;
        for (const auto& event : m_events) {
            if (event.type == type) {
                filteredEvents.push_back(event);
            }
        }
        
        return filteredEvents;
    }
    
    /**
     * @brief Ottiene gli eventi filtrati per severità.
     * @param severity Severità minima
     * @return Vettore di eventi
     */
    std::vector<MonitoringEvent> getEventsBySeverity(MonitoringSeverity severity) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<MonitoringEvent> filteredEvents;
        for (const auto& event : m_events) {
            if (event.severity >= severity) {
                filteredEvents.push_back(event);
            }
        }
        
        return filteredEvents;
    }
    
    /**
     * @brief Ottiene gli eventi filtrati per nome.
     * @param name Nome dell'evento
     * @return Vettore di eventi
     */
    std::vector<MonitoringEvent> getEventsByName(const std::string& name) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<MonitoringEvent> filteredEvents;
        for (const auto& event : m_events) {
            if (event.name == name) {
                filteredEvents.push_back(event);
            }
        }
        
        return filteredEvents;
    }
    
    /**
     * @brief Cancella tutti gli eventi.
     */
    void clearEvents() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_events.clear();
    }

private:
    size_t m_maxEvents;                ///< Numero massimo di eventi
    MonitoringSeverity m_minSeverity;  ///< Severità minima
    bool m_initialized;                ///< Se il canale è inizializzato
    std::vector<MonitoringEvent> m_events;  ///< Eventi
    mutable std::mutex m_mutex;        ///< Mutex per accesso thread-safe
};

/**
 * @brief Canale di monitoraggio che invia eventi via socket.
 */
class SocketMonitoringChannel : public IMonitoringChannel {
public:
    /**
     * @brief Costruttore.
     * @param host Host per la connessione
     * @param port Porta per la connessione
     * @param minSeverity Severità minima degli eventi da registrare
     */
    SocketMonitoringChannel(const std::string& host, int port,
                          MonitoringSeverity minSeverity = MonitoringSeverity::INFO)
        : m_host(host), m_port(port), m_minSeverity(minSeverity), 
          m_initialized(false), m_connected(false) {}
    
    /**
     * @brief Distruttore.
     */
    ~SocketMonitoringChannel() override {
        shutdown();
    }
    
    /**
     * @brief Inizializza il canale.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_initialized) return true;
        
        // In una implementazione reale, qui si connetterebbe il socket
        // al server di monitoraggio
        
        m_initialized = true;
        m_connected = true;
        
        return true;
    }
    
    /**
     * @brief Chiude il canale.
     * @return true se la chiusura è avvenuta con successo, false altrimenti
     */
    bool shutdown() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) return true;
        
        // In una implementazione reale, qui si disconnetterebbe il socket
        
        m_initialized = false;
        m_connected = false;
        
        return true;
    }
    
    /**
     * @brief Invia un evento al canale.
     * @param event Evento da inviare
     * @return true se l'invio è avvenuto con successo, false altrimenti
     */
    bool sendEvent(const MonitoringEvent& event) override {
        // Filtra per severità
        if (event.severity < m_minSeverity) {
            return true;
        }
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized || !m_connected) {
            return false;
        }
        
        // In una implementazione reale, qui si invierebbe l'evento via socket
        
        return true;
    }
    
    /**
     * @brief Ottiene il nome del canale.
     * @return Nome del canale
     */
    std::string getName() const override {
        return "SocketMonitoringChannel:" + m_host + ":" + std::to_string(m_port);
    }

private:
    std::string m_host;                ///< Host per la connessione
    int m_port;                        ///< Porta per la connessione
    MonitoringSeverity m_minSeverity;  ///< Severità minima
    bool m_initialized;                ///< Se il canale è inizializzato
    bool m_connected;                  ///< Se il canale è connesso
    std::mutex m_mutex;                ///< Mutex per accesso thread-safe
};

/**
 * @brief Sistema di monitoraggio principale.
 */
class MonitoringSystem {
public:
    /**
     * @brief Ottiene l'istanza singleton del sistema di monitoraggio.
     * @return Riferimento all'istanza singleton
     */
    static MonitoringSystem& getInstance() {
        static MonitoringSystem instance;
        return instance;
    }
    
    /**
     * @brief Inizializza il sistema di monitoraggio.
     * @param logFilePath Percorso del file di log
     * @param minSeverity Severità minima degli eventi da registrare
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize(const std::string& logFilePath = "", 
                   MonitoringSeverity minSeverity = MonitoringSeverity::INFO) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_initialized) return true;
        
        // Imposta la severità minima
        m_minSeverity = minSeverity;
        
        // Crea un canale in memoria
        auto inMemoryChannel = std::make_shared<InMemoryMonitoringChannel>(1000, minSeverity);
        registerChannel(inMemoryChannel);
        
        // Se è specificato un percorso per il log, crea un canale file
        if (!logFilePath.empty()) {
            auto fileChannel = std::make_shared<FileMonitoringChannel>(logFilePath, minSeverity);
            registerChannel(fileChannel);
        }
        
        m_initialized = true;
        
        // Registra l'inizializzazione
        logEvent(
            MonitoringEventType::SYSTEM,
            MonitoringSeverity::INFO,
            "MonitoringSystemInitialized",
            "Monitoring system initialized with min severity: " + toString(minSeverity)
        );
        
        return true;
    }
    
    /**
     * @brief Termina il sistema di monitoraggio.
     * @return true se la terminazione è avvenuta con successo, false altrimenti
     */
    bool shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) return true;
        
        // Registra la terminazione
        logEvent(
            MonitoringEventType::SYSTEM,
            MonitoringSeverity::INFO,
            "MonitoringSystemShutdown",
            "Monitoring system shutting down"
        );
        
        // Chiudi tutti i canali
        for (auto& channel : m_channels) {
            channel->shutdown();
        }
        
        // Pulisci i canali
        m_channels.clear();
        
        m_initialized = false;
        
        return true;
    }
    
    /**
     * @brief Registra un canale di monitoraggio.
     * @param channel Canale da registrare
     * @return true se la registrazione è avvenuta con successo, false altrimenti
     */
    bool registerChannel(std::shared_ptr<IMonitoringChannel> channel) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!channel) {
            return false;
        }
        
        // Inizializza il canale
        if (!channel->initialize()) {
            return false;
        }
        
        // Aggiungi il canale
        m_channels.push_back(channel);
        
        // Se il sistema è già inizializzato, registra l'evento
        if (m_initialized) {
            logEvent(
                MonitoringEventType::SYSTEM,
                MonitoringSeverity::INFO,
                "MonitoringChannelRegistered",
                "Monitoring channel registered: " + channel->getName()
            );
        }
        
        return true;
    }
    
    /**
     * @brief Annulla la registrazione di un canale di monitoraggio.
     * @param channelName Nome del canale da annullare
     * @return true se l'annullamento è avvenuto con successo, false altrimenti
     */
    bool unregisterChannel(const std::string& channelName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Cerca il canale
        auto it = std::find_if(m_channels.begin(), m_channels.end(),
                              [&channelName](const std::shared_ptr<IMonitoringChannel>& channel) {
                                  return channel->getName() == channelName;
                              });
        
        if (it == m_channels.end()) {
            return false;
        }
        
        // Chiudi il canale
        (*it)->shutdown();
        
        // Rimuovi il canale
        m_channels.erase(it);
        
        // Registra l'evento
        if (m_initialized) {
            logEvent(
                MonitoringEventType::SYSTEM,
                MonitoringSeverity::INFO,
                "MonitoringChannelUnregistered",
                "Monitoring channel unregistered: " + channelName
            );
        }
        
        return true;
    }
    
    /**
     * @brief Registra un evento.
     * @param event Evento da registrare
     */
    void logEvent(const MonitoringEvent& event) {
        // Filtra per severità
        if (event.severity < m_minSeverity) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) {
            return;
        }
        
        // Invia l'evento a tutti i canali
        for (auto& channel : m_channels) {
            channel->sendEvent(event);
        }
    }
    
    /**
     * @brief Registra un evento.
     * @param type Tipo di evento
     * @param severity Severità dell'evento
     * @param name Nome dell'evento
     * @param description Descrizione dell'evento
     * @param file File sorgente
     * @param line Linea nel file
     * @param function Funzione
     */
    void logEvent(MonitoringEventType type, MonitoringSeverity severity,
                  const std::string& name, const std::string& description,
                  const std::string& file = "", int line = 0, const std::string& function = "") {
        // Filtra per severità
        if (severity < m_minSeverity) {
            return;
        }
        
        MonitoringEvent event(type, severity, name, description, file, line, function);
        logEvent(event);
    }
    
    /**
     * @brief Imposta la severità minima.
     * @param severity Severità minima
     */
    void setMinSeverity(MonitoringSeverity severity) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_minSeverity = severity;
    }
    
    /**
     * @brief Ottiene la severità minima.
     * @return Severità minima
     */
    MonitoringSeverity getMinSeverity() const {
        return m_minSeverity;
    }
    
    /**
     * @brief Ottiene il canale di memoria.
     * @return Puntatore al canale di memoria, o nullptr se non trovato
     */
    std::shared_ptr<InMemoryMonitoringChannel> getInMemoryChannel() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Cerca il canale di memoria
        for (auto& channel : m_channels) {
            auto memoryChannel = std::dynamic_pointer_cast<InMemoryMonitoringChannel>(channel);
            if (memoryChannel) {
                return memoryChannel;
            }
        }
        
        return nullptr;
    }
    
    /**
     * @brief Registra un callback per gli eventi.
     * @param callback Funzione da chiamare quando un evento viene registrato
     * @return ID del callback
     */
    int registerEventCallback(std::function<void(const MonitoringEvent&)> callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        int id = m_nextCallbackId++;
        m_eventCallbacks[id] = callback;
        
        return id;
    }
    
    /**
     * @brief Annulla la registrazione di un callback.
     * @param callbackId ID del callback
     * @return true se l'annullamento è avvenuto con successo, false altrimenti
     */
    bool unregisterEventCallback(int callbackId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_eventCallbacks.find(callbackId);
        if (it == m_eventCallbacks.end()) {
            return false;
        }
        
        m_eventCallbacks.erase(it);
        
        return true;
    }

private:
    // Costruttore privato per pattern Singleton
    MonitoringSystem() : m_initialized(false), m_minSeverity(MonitoringSeverity::INFO), m_nextCallbackId(0) {}
    
    // Distruttore privato
    ~MonitoringSystem() {
        if (m_initialized) {
            shutdown();
        }
    }
    
    // Disabilita copia e assegnazione
    MonitoringSystem(const MonitoringSystem&) = delete;
    MonitoringSystem& operator=(const MonitoringSystem&) = delete;
    
    bool m_initialized;                ///< Se il sistema è inizializzato
    MonitoringSeverity m_minSeverity;  ///< Severità minima
    std::vector<std::shared_ptr<IMonitoringChannel>> m_channels;  ///< Canali di monitoraggio
    std::unordered_map<int, std::function<void(const MonitoringEvent&)>> m_eventCallbacks;  ///< Callback per gli eventi
    int m_nextCallbackId;              ///< Prossimo ID callback
    std::mutex m_mutex;                ///< Mutex per accesso thread-safe
};

/**
 * @brief Macro per registrare un evento con informazioni sul file e sulla linea.
 * @param type Tipo di evento
 * @param severity Severità dell'evento
 * @param name Nome dell'evento
 * @param description Descrizione dell'evento
 */
#define LOG_EVENT(type, severity, name, description) \
    Core::Monitoring::MonitoringSystem::getInstance().logEvent( \
        type, severity, name, description, __FILE__, __LINE__, __FUNCTION__ \
    )

/**
 * @brief Macro per registrare un evento di informazione.
 * @param name Nome dell'evento
 * @param description Descrizione dell'evento
 */
#define LOG_INFO(name, description) \
    LOG_EVENT(Core::Monitoring::MonitoringEventType::SYSTEM, Core::Monitoring::MonitoringSeverity::INFO, name, description)

/**
 * @brief Macro per registrare un evento di warning.
 * @param name Nome dell'evento
 * @param description Descrizione dell'evento
 */
#define LOG_WARNING(name, description) \
    LOG_EVENT(Core::Monitoring::MonitoringEventType::SYSTEM, Core::Monitoring::MonitoringSeverity::WARNING, name, description)

/**
 * @brief Macro per registrare un evento di errore.
 * @param name Nome dell'evento
 * @param description Descrizione dell'evento
 */
#define LOG_ERROR(name, description) \
    LOG_EVENT(Core::Monitoring::MonitoringEventType::ERROR, Core::Monitoring::MonitoringSeverity::ERROR, name, description)

/**
 * @brief Macro per registrare un evento di debug.
 * @param name Nome dell'evento
 * @param description Descrizione dell'evento
 */
#define LOG_DEBUG(name, description) \
    LOG_EVENT(Core::Monitoring::MonitoringEventType::DEBUG, Core::Monitoring::MonitoringSeverity::DEBUG, name, description)

} // namespace Monitoring
} // namespace Core