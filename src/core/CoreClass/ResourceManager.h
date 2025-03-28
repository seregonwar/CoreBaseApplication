#pragma once

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <memory>
#include "SystemResources.h"

namespace CoreNS {

/**
 * @brief Enum che definisce i tipi di risorse monitorabili
 */
enum class ResourceType {
    CPU,
    MEMORY,
    DISK,
    NETWORK,
    GPU,
    UNKNOWN
};

using ResourceCallback = std::function<void(const SystemResources&)>;

class ErrorHandler;

/**
 * @brief Classe per la gestione e il monitoraggio delle risorse di sistema
 */
class ResourceManager {
public:
    /**
     * @brief Costruttore del ResourceManager
     */
    ResourceManager();
    
    /**
     * @brief Distruttore del ResourceManager
     */
    ~ResourceManager();
    
    /**
     * @brief Inizializza il monitoraggio delle risorse
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize();
    
    /**
     * @brief Arresta il monitoraggio delle risorse
     */
    void shutdown();
    
    /**
     * @brief Ottiene la quantità di memoria disponibile
     * @return Memoria disponibile in bytes
     */
    uint64_t getAvailableMemory() const;
    
    /**
     * @brief Ottiene l'utilizzo della CPU
     * @return Percentuale di utilizzo della CPU (0-100)
     */
    double getCpuUsage() const;
    
    /**
     * @brief Ottiene l'utilizzo della memoria
     * @return Percentuale di utilizzo della memoria (0-100)
     */
    double getMemoryUsage() const;
    
    /**
     * @brief Ottiene l'utilizzo del disco
     * @return Percentuale di utilizzo del disco (0-100)
     */
    double getDiskUsage() const;
    
    /**
     * @brief Ottiene l'utilizzo della rete
     * @return Percentuale di utilizzo della rete (0-100)
     */
    double getNetworkUsage() const;
    
    /**
     * @brief Ottiene l'utilizzo della GPU
     * @return Percentuale di utilizzo della GPU (0-100)
     */
    double getGpuUsage() const;
    
    /**
     * @brief Ottiene un snapshot completo delle risorse di sistema
     * @return Struct contenente informazioni sulle risorse
     */
    SystemResources getSystemResources() const;
    
    /**
     * @brief Registra una callback da eseguire quando l'utilizzo di una risorsa supera una soglia
     * @param callback Funzione da eseguire quando la soglia viene superata
     */
    void registerCallback(const ResourceCallback& callback);
    
    /**
     * @brief Rimuove una callback registrata
     * @param callback Funzione da rimuovere
     */
    void unregisterCallback(const ResourceCallback& callback);
    
private:
    // Struttura per le callback dei threshold
    struct ThresholdCallback {
        int id;
        ResourceType type;
        int threshold;
        std::function<void(int)> callback;
    };

    // Thread di monitoraggio
    std::thread m_monitoringThread;
    std::atomic<bool> m_stopMonitoring{false};

    // Stato delle risorse
    SystemResources m_resources;
    std::vector<ThresholdCallback> m_thresholdCallbacks;
    int m_lastCallbackId{0};

    // Metodi privati per il monitoraggio
    void updateResources();
    void checkThresholds();

    std::unordered_map<int, std::pair<std::string, ResourceCallback>> m_callbacks;
    mutable std::mutex m_mutex;
    int m_nextCallbackId;

    std::shared_ptr<ErrorHandler> m_errorHandler;
    SystemResources m_currentResources;
};

} // namespace CoreNS
