#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>
#include <functional>

/**
 * @brief Enum che definisce i tipi di risorse monitorabili
 */
enum class ResourceType {
    CPU,
    MEMORY,
    DISK,
    NETWORK,
    GPU
};

/**
 * @brief Struct che contiene informazioni sulle risorse di sistema
 */
struct SystemResources {
    int cpuUsagePercent;
    uint64_t availableMemoryBytes;
    uint64_t totalMemoryBytes;
    uint64_t availableDiskBytes;
    uint64_t totalDiskBytes;
    int networkUsagePercent;
    int gpuUsagePercent;
    uint64_t availableGpuMemoryBytes;
    uint64_t totalGpuMemoryBytes;
};

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
     * @brief Inizializza il ResourceManager
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize();
    
    /**
     * @brief Ottiene la quantità di memoria disponibile
     * @return Memoria disponibile in bytes
     */
    uint64_t getAvailableMemory() const;
    
    /**
     * @brief Ottiene la memoria totale
     * @return Memoria totale in bytes
     */
    uint64_t getTotalMemory() const;
    
    /**
     * @brief Ottiene l'utilizzo della CPU
     * @return Percentuale di utilizzo della CPU (0-100)
     */
    int getCpuUsage() const;
    
    /**
     * @brief Ottiene lo spazio disponibile su disco
     * @param path Percorso del disco (default = directory corrente)
     * @return Spazio disponibile in bytes
     */
    uint64_t getDiskSpace(const std::string& path = ".") const;
    
    /**
     * @brief Ottiene lo spazio totale su disco
     * @param path Percorso del disco (default = directory corrente)
     * @return Spazio totale in bytes
     */
    uint64_t getTotalDiskSpace(const std::string& path = ".") const;
    
    /**
     * @brief Registra una callback da eseguire quando l'utilizzo di una risorsa supera una soglia
     * @param type Tipo di risorsa da monitorare
     * @param thresholdPercent Soglia percentuale (0-100)
     * @param callback Funzione da eseguire quando la soglia viene superata
     * @return ID della callback registrata
     */
    int registerThresholdCallback(ResourceType type, int thresholdPercent, 
                                 std::function<void(int currentUsage)> callback);
    
    /**
     * @brief Rimuove una callback registrata
     * @param callbackId ID della callback da rimuovere
     * @return true se la callback è stata rimossa, false se non esisteva
     */
    bool unregisterThresholdCallback(int callbackId);
    
    /**
     * @brief Ottiene un snapshot completo delle risorse di sistema
     * @return Struct contenente informazioni sulle risorse
     */
    SystemResources getSystemResources() const;
    
private:
    std::mutex m_mutex;
    std::unordered_map<int, std::pair<ResourceType, std::function<void(int)>>> m_callbacks;
    int m_nextCallbackId;
    
    /**
     * @brief Aggiorna le informazioni sulle risorse
     */
    void updateResourceInfo();
    
    /**
     * @brief Controlla se le soglie sono state superate e chiama le callback
     */
    void checkThresholds();
    
    // Informazioni sulle risorse
    SystemResources m_resources;
};
