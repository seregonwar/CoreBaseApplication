#pragma once

#include "MonitoringSystem.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <memory>
#include <chrono>
#include <algorithm>
#include <atomic>
#include <cstdint>

namespace Core {
namespace Monitoring {

/**
 * @brief Informazioni relative a un'allocazione di memoria.
 */
struct MemoryAllocationInfo {
    void* address;                       ///< Indirizzo di memoria
    size_t size;                         ///< Dimensione in byte
    std::string tag;                     ///< Tag opzionale per categorizzare l'allocazione
    std::string callstack;               ///< Stack delle chiamate (se disponibile)
    std::chrono::steady_clock::time_point timestamp;  ///< Timestamp dell'allocazione
    std::thread::id threadId;            ///< ID del thread che ha eseguito l'allocazione
    
    /**
     * @brief Costruttore.
     * @param addr Indirizzo di memoria
     * @param sz Dimensione in byte
     * @param tg Tag opzionale
     * @param cs Stack delle chiamate (se disponibile)
     */
    MemoryAllocationInfo(void* addr, size_t sz, const std::string& tg = "", const std::string& cs = "")
        : address(addr), size(sz), tag(tg), callstack(cs),
          timestamp(std::chrono::steady_clock::now()), 
          threadId(std::this_thread::get_id()) {}
};

/**
 * @brief Statistiche di utilizzo della memoria.
 */
struct MemoryStats {
    size_t totalAllocated;                    ///< Totale byte allocati
    size_t totalDeallocated;                  ///< Totale byte deallocati
    size_t currentUsage;                      ///< Utilizzo corrente in byte
    size_t peakUsage;                         ///< Utilizzo massimo raggiunto in byte
    size_t allocationCount;                   ///< Numero totale di allocazioni
    size_t deallocationCount;                 ///< Numero totale di deallocazioni
    size_t activeAllocations;                 ///< Numero di allocazioni attive
    
    std::unordered_map<std::string, size_t> usageByTag;  ///< Utilizzo per tag
    std::unordered_map<std::string, size_t> countByTag;  ///< Conteggio per tag
    
    /**
     * @brief Costruttore.
     */
    MemoryStats()
        : totalAllocated(0), totalDeallocated(0), currentUsage(0), 
          peakUsage(0), allocationCount(0), deallocationCount(0), 
          activeAllocations(0) {}
};

/**
 * @brief Categoria di allocazione di memoria basata sulla dimensione.
 */
enum class MemorySizeCategory {
    TINY,          ///< < 128 byte
    SMALL,         ///< 128 - 1023 byte
    MEDIUM,        ///< 1 KB - 16 KB
    LARGE,         ///< 16 KB - 1 MB
    HUGE,          ///< > 1 MB
    CUSTOM         ///< Categoria personalizzata
};

/**
 * @brief Ottiene la categoria di dimensione di memoria per un'allocazione.
 * @param size Dimensione dell'allocazione in byte
 * @return Categoria di dimensione
 */
inline MemorySizeCategory getSizeCategory(size_t size) {
    if (size < 128) return MemorySizeCategory::TINY;
    if (size < 1024) return MemorySizeCategory::SMALL;
    if (size < 16 * 1024) return MemorySizeCategory::MEDIUM;
    if (size < 1024 * 1024) return MemorySizeCategory::LARGE;
    return MemorySizeCategory::HUGE;
}

/**
 * @brief Ottiene il nome di una categoria di dimensione di memoria.
 * @param category Categoria di dimensione
 * @return Nome della categoria
 */
inline std::string getSizeCategoryName(MemorySizeCategory category) {
    switch (category) {
        case MemorySizeCategory::TINY: return "Tiny (<128B)";
        case MemorySizeCategory::SMALL: return "Small (128B-1KB)";
        case MemorySizeCategory::MEDIUM: return "Medium (1KB-16KB)";
        case MemorySizeCategory::LARGE: return "Large (16KB-1MB)";
        case MemorySizeCategory::HUGE: return "Huge (>1MB)";
        case MemorySizeCategory::CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

/**
 * @brief Rappresenta un'alerta di memoria.
 */
struct MemoryAlert {
    enum class Type {
        LEAK,               ///< Potenziale perdita di memoria
        FRAGMENTATION,      ///< Frammentazione elevata
        HIGH_USAGE,         ///< Utilizzo elevato
        ALLOCATION_SPIKE    ///< Picco di allocazioni
    };
    
    Type type;                         ///< Tipo di alerta
    std::string description;           ///< Descrizione
    std::chrono::steady_clock::time_point timestamp; ///< Timestamp
    MonitoringSeverity severity;       ///< Severità
    
    /**
     * @brief Costruttore.
     * @param t Tipo di alerta
     * @param desc Descrizione
     * @param sev Severità
     */
    MemoryAlert(Type t, const std::string& desc, MonitoringSeverity sev = MonitoringSeverity::WARNING)
        : type(t), description(desc), timestamp(std::chrono::steady_clock::now()),
          severity(sev) {}
    
    /**
     * @brief Ottiene il nome del tipo di alerta.
     * @return Nome del tipo
     */
    std::string getTypeName() const {
        switch (type) {
            case Type::LEAK: return "Memory Leak";
            case Type::FRAGMENTATION: return "Memory Fragmentation";
            case Type::HIGH_USAGE: return "High Memory Usage";
            case Type::ALLOCATION_SPIKE: return "Allocation Spike";
            default: return "Unknown";
        }
    }
};

/**
 * @brief Tracciatore di allocazione di memoria.
 */
class MemoryTracker {
public:
    /**
     * @brief Ottiene l'istanza singleton del tracciatore di memoria.
     * @return Riferimento all'istanza singleton
     */
    static MemoryTracker& getInstance() {
        static MemoryTracker instance;
        return instance;
    }
    
    /**
     * @brief Inizializza il tracciatore di memoria.
     * @param captureCallstack Se true, acquisisce lo stack delle chiamate per ogni allocazione
     * @param trackSystemAllocations Se true, traccia anche le allocazioni di sistema
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize(bool captureCallstack = true, bool trackSystemAllocations = false) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_initialized) return true;
        
        m_captureCallstack = captureCallstack;
        m_trackSystemAllocations = trackSystemAllocations;
        
        // Registra l'inizializzazione nel sistema di monitoraggio
        MonitoringSystem::getInstance().logEvent(
            MonitoringEventType::MEMORY,
            MonitoringSeverity::INFO,
            "MemoryTrackerInitialized",
            "Memory tracking initialized with callstack capture " + 
            std::string(m_captureCallstack ? "enabled" : "disabled") +
            " and system allocations " +
            std::string(m_trackSystemAllocations ? "tracked" : "not tracked")
        );
        
        m_initialized = true;
        return true;
    }
    
    /**
     * @brief Termina il tracciatore di memoria.
     * @return true se la terminazione è avvenuta con successo, false altrimenti
     */
    bool shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) return true;
        
        // Crea un report finale
        createMemoryReport();
        
        // Registra la terminazione nel sistema di monitoraggio
        MonitoringSystem::getInstance().logEvent(
            MonitoringEventType::MEMORY,
            MonitoringSeverity::INFO,
            "MemoryTrackerShutdown",
            "Memory tracking shutting down - Final stats: " +
            std::to_string(m_stats.totalAllocated) + " bytes allocated, " +
            std::to_string(m_stats.totalDeallocated) + " bytes deallocated, " +
            std::to_string(m_stats.activeAllocations) + " active allocations"
        );
        
        // Resetta le strutture dati
        m_allocations.clear();
        m_stats = MemoryStats();
        
        m_initialized = false;
        return true;
    }
    
    /**
     * @brief Traccia un'allocazione di memoria.
     * @param address Indirizzo di memoria
     * @param size Dimensione in byte
     * @param tag Tag opzionale per categorizzare l'allocazione
     */
    void trackAllocation(void* address, size_t size, const std::string& tag = "") {
        if (!m_initialized) return;
        
        std::string callstack;
        if (m_captureCallstack) {
            callstack = captureCallstack();
        }
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Crea l'informazione di allocazione
        MemoryAllocationInfo info(address, size, tag, callstack);
        
        // Aggiorna le statistiche
        m_stats.totalAllocated += size;
        m_stats.currentUsage += size;
        m_stats.allocationCount++;
        m_stats.activeAllocations++;
        
        // Aggiorna l'utilizzo massimo
        m_stats.peakUsage = std::max(m_stats.peakUsage, m_stats.currentUsage);
        
        // Aggiorna le statistiche per tag
        if (!tag.empty()) {
            m_stats.usageByTag[tag] += size;
            m_stats.countByTag[tag]++;
        }
        
        // Registra l'allocazione
        m_allocations[address] = info;
        
        // Invia un evento di allocazione
        MonitoringEvent event(
            MonitoringEventType::MEMORY,
            MonitoringSeverity::DEBUG,
            "MemoryAllocation",
            "Memory allocated: " + std::to_string(size) + " bytes" +
            (tag.empty() ? "" : " [" + tag + "]")
        );
        
        event.addNumericData("size", size);
        event.addStringData("tag", tag);
        event.addNumericData("current_usage", m_stats.currentUsage);
        
        MonitoringSystem::getInstance().logEvent(event);
        
        // Verifica la necessità di generare alert
        checkMemoryAlerts();
    }
    
    /**
     * @brief Traccia una deallocazione di memoria.
     * @param address Indirizzo di memoria
     */
    void trackDeallocation(void* address) {
        if (!m_initialized || address == nullptr) return;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_allocations.find(address);
        if (it == m_allocations.end()) {
            // Deallocazione di memoria non tracciata
            MonitoringSystem::getInstance().logEvent(
                MonitoringEventType::MEMORY,
                MonitoringSeverity::WARNING,
                "MemoryDeallocationUnknown",
                "Trying to deallocate unknown memory address: " + std::to_string(reinterpret_cast<uintptr_t>(address))
            );
            return;
        }
        
        const MemoryAllocationInfo& info = it->second;
        
        // Aggiorna le statistiche
        m_stats.totalDeallocated += info.size;
        m_stats.currentUsage -= info.size;
        m_stats.deallocationCount++;
        m_stats.activeAllocations--;
        
        // Aggiorna le statistiche per tag
        if (!info.tag.empty()) {
            m_stats.usageByTag[info.tag] -= info.size;
            m_stats.countByTag[info.tag]--;
        }
        
        // Invia un evento di deallocazione
        MonitoringEvent event(
            MonitoringEventType::MEMORY,
            MonitoringSeverity::DEBUG,
            "MemoryDeallocation",
            "Memory deallocated: " + std::to_string(info.size) + " bytes" +
            (info.tag.empty() ? "" : " [" + info.tag + "]")
        );
        
        event.addNumericData("size", info.size);
        event.addStringData("tag", info.tag);
        event.addNumericData("current_usage", m_stats.currentUsage);
        
        MonitoringSystem::getInstance().logEvent(event);
        
        // Rimuovi l'allocazione dalla mappa
        m_allocations.erase(it);
    }
    
    /**
     * @brief Registra un tag per un'allocazione esistente.
     * @param address Indirizzo di memoria
     * @param tag Tag da assegnare
     * @return true se il tag è stato assegnato con successo, false altrimenti
     */
    bool tagAllocation(void* address, const std::string& tag) {
        if (!m_initialized || address == nullptr) return false;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_allocations.find(address);
        if (it == m_allocations.end()) {
            return false;
        }
        
        // Aggiorna le statistiche per tag
        if (!it->second.tag.empty()) {
            m_stats.usageByTag[it->second.tag] -= it->second.size;
            m_stats.countByTag[it->second.tag]--;
        }
        
        // Aggiorna il tag
        it->second.tag = tag;
        
        // Aggiorna le statistiche per il nuovo tag
        if (!tag.empty()) {
            m_stats.usageByTag[tag] += it->second.size;
            m_stats.countByTag[tag]++;
        }
        
        return true;
    }
    
    /**
     * @brief Ottiene le statistiche di memoria attuale.
     * @return Statistiche di memoria
     */
    MemoryStats getMemoryStats() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }
    
    /**
     * @brief Crea un report di memoria.
     * @param detailed Se true, include informazioni dettagliate sulle allocazioni
     */
    void createMemoryReport(bool detailed = false) {
        if (!m_initialized) return;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Crea un report di base
        MonitoringEvent event(
            MonitoringEventType::MEMORY,
            MonitoringSeverity::INFO,
            "MemoryReport",
            "Memory usage report"
        );
        
        event.addNumericData("total_allocated", m_stats.totalAllocated);
        event.addNumericData("total_deallocated", m_stats.totalDeallocated);
        event.addNumericData("current_usage", m_stats.currentUsage);
        event.addNumericData("peak_usage", m_stats.peakUsage);
        event.addNumericData("allocation_count", m_stats.allocationCount);
        event.addNumericData("deallocation_count", m_stats.deallocationCount);
        event.addNumericData("active_allocations", m_stats.activeAllocations);
        
        // Aggiungi informazioni per tag
        std::string tagDetails = "Tag usage: ";
        for (const auto& [tag, usage] : m_stats.usageByTag) {
            if (usage > 0) {
                tagDetails += tag + "=" + std::to_string(usage) + " bytes (" + 
                              std::to_string(m_stats.countByTag[tag]) + " allocations), ";
            }
        }
        event.addStringData("tag_details", tagDetails);
        
        // Aggiungi informazioni per categoria di dimensione
        std::unordered_map<MemorySizeCategory, size_t> sizeCategories;
        std::unordered_map<MemorySizeCategory, size_t> sizeCategoriesBytes;
        
        for (const auto& [address, allocation] : m_allocations) {
            MemorySizeCategory category = getSizeCategory(allocation.size);
            sizeCategories[category]++;
            sizeCategoriesBytes[category] += allocation.size;
        }
        
        std::string sizeDetails = "Size categories: ";
        for (int i = 0; i < static_cast<int>(MemorySizeCategory::CUSTOM); ++i) {
            MemorySizeCategory category = static_cast<MemorySizeCategory>(i);
            sizeDetails += getSizeCategoryName(category) + "=" + 
                          std::to_string(sizeCategories[category]) + " allocations (" + 
                          std::to_string(sizeCategoriesBytes[category]) + " bytes), ";
        }
        event.addStringData("size_details", sizeDetails);
        
        // Invia il report al sistema di monitoraggio
        MonitoringSystem::getInstance().logEvent(event);
        
        // Se richiesto un report dettagliato, aggiungi informazioni sulle allocazioni attive
        if (detailed && !m_allocations.empty()) {
            // Ordina le allocazioni per dimensione
            std::vector<MemoryAllocationInfo> sortedAllocations;
            for (const auto& [address, allocation] : m_allocations) {
                sortedAllocations.push_back(allocation);
            }
            
            std::sort(sortedAllocations.begin(), sortedAllocations.end(),
                     [](const MemoryAllocationInfo& a, const MemoryAllocationInfo& b) {
                         return a.size > b.size;
                     });
            
            // Limita a 100 allocazioni più grandi
            const size_t maxAllocation = std::min(sortedAllocations.size(), static_cast<size_t>(100));
            
            for (size_t i = 0; i < maxAllocation; ++i) {
                const auto& allocation = sortedAllocations[i];
                
                MonitoringEvent detailEvent(
                    MonitoringEventType::MEMORY,
                    MonitoringSeverity::DEBUG,
                    "MemoryReportDetail",
                    "Memory allocation #" + std::to_string(i + 1) + " of " + std::to_string(maxAllocation)
                );
                
                detailEvent.addStringData("address", std::to_string(reinterpret_cast<uintptr_t>(allocation.address)));
                detailEvent.addNumericData("size", allocation.size);
                detailEvent.addStringData("tag", allocation.tag);
                
                if (!allocation.callstack.empty()) {
                    detailEvent.addStringData("callstack", allocation.callstack);
                }
                
                MonitoringSystem::getInstance().logEvent(detailEvent);
            }
        }
    }

    /**
     * @brief Controlla se ci sono perdite di memoria.
     * @param minSize Dimensione minima per considerare una perdita (in byte)
     * @param minAge Età minima per considerare una perdita (in secondi)
     * @return Numero di potenziali perdite rilevate
     */
    size_t checkMemoryLeaks(size_t minSize = 1024, unsigned int minAge = 60) {
        if (!m_initialized) return 0;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        size_t leakCount = 0;
        auto now = std::chrono::steady_clock::now();
        
        for (const auto& [address, allocation] : m_allocations) {
            // Controlla se l'allocazione è abbastanza grande e vecchia
            auto age = std::chrono::duration_cast<std::chrono::seconds>(now - allocation.timestamp).count();
            
            if (allocation.size >= minSize && age >= minAge) {
                // Potenziale perdita di memoria
                leakCount++;
                
                // Registra un avviso
                MonitoringEvent event(
                    MonitoringEventType::MEMORY,
                    MonitoringSeverity::WARNING,
                    "PotentialMemoryLeak",
                    "Potential memory leak detected: " + std::to_string(allocation.size) + 
                    " bytes allocated " + std::to_string(age) + " seconds ago" +
                    (allocation.tag.empty() ? "" : " [" + allocation.tag + "]")
                );
                
                event.addStringData("address", std::to_string(reinterpret_cast<uintptr_t>(allocation.address)));
                event.addNumericData("size", allocation.size);
                event.addNumericData("age", age);
                event.addStringData("tag", allocation.tag);
                
                if (!allocation.callstack.empty()) {
                    event.addStringData("callstack", allocation.callstack);
                }
                
                MonitoringSystem::getInstance().logEvent(event);
            }
        }
        
        return leakCount;
    }

    /**
     * @brief Definisce una soglia di memoria.
     * @param thresholdBytes Soglia in byte
     * @param callback Funzione da chiamare quando la soglia viene superata
     * @return ID della soglia
     */
    int setMemoryThreshold(size_t thresholdBytes, std::function<void(size_t)> callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        int id = m_nextThresholdId++;
        m_memoryThresholds[id] = { thresholdBytes, callback };
        
        return id;
    }
    
    /**
     * @brief Rimuove una soglia di memoria.
     * @param thresholdId ID della soglia
     * @return true se la soglia è stata rimossa, false altrimenti
     */
    bool removeMemoryThreshold(int thresholdId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_memoryThresholds.find(thresholdId);
        if (it == m_memoryThresholds.end()) {
            return false;
        }
        
        m_memoryThresholds.erase(it);
        
        return true;
    }

private:
    // Costruttore privato per pattern Singleton
    MemoryTracker()
        : m_initialized(false), 
          m_captureCallstack(true),
          m_trackSystemAllocations(false),
          m_nextThresholdId(0) {}
    
    // Distruttore privato
    ~MemoryTracker() {
        if (m_initialized) {
            shutdown();
        }
    }
    
    // Disabilita copia e assegnazione
    MemoryTracker(const MemoryTracker&) = delete;
    MemoryTracker& operator=(const MemoryTracker&) = delete;
    
    /**
     * @brief Cattura lo stack delle chiamate.
     * @return Stringa rappresentante lo stack delle chiamate
     */
    std::string captureCallstack() {
        // Questa è una implementazione fittizia
        // In una implementazione reale si userebbero API specifiche per piattaforma
        // per catturare lo stack delle chiamate
        
        #ifdef _WIN32
            // Windows implementation using CaptureStackBackTrace
        #elif defined(__linux__) || defined(__APPLE__)
            // Posix implementation using backtrace()
        #endif
        
        return "Callstack not implemented in this sample";
    }
    
    /**
     * @brief Verifica la necessità di generare alert di memoria.
     */
    void checkMemoryAlerts() {
        // Verifica le soglie di memoria
        for (const auto& [id, threshold] : m_memoryThresholds) {
            if (m_stats.currentUsage > threshold.first) {
                threshold.second(m_stats.currentUsage);
            }
        }
        
        // Verifica l'utilizzo elevato
        const double HIGH_USAGE_THRESHOLD = 0.8;  // 80%
        
        // Questo valore sarebbe normalmente ottenuto dal sistema operativo
        const size_t SYSTEM_MEMORY_TOTAL = 16ULL * 1024 * 1024 * 1024;  // 16 GB
        
        // Controlla se l'uso della memoria è oltre la soglia
        if (m_stats.currentUsage > HIGH_USAGE_THRESHOLD * SYSTEM_MEMORY_TOTAL) {
            MemoryAlert alert(
                MemoryAlert::Type::HIGH_USAGE,
                "High memory usage detected: " + std::to_string(m_stats.currentUsage) + 
                " bytes (" + std::to_string(m_stats.currentUsage * 100.0 / SYSTEM_MEMORY_TOTAL) + "%)",
                MonitoringSeverity::WARNING
            );
            
            // Registra l'alerta
            MonitoringSystem::getInstance().logEvent(
                MonitoringEventType::MEMORY,
                alert.severity,
                "MemoryAlert_" + alert.getTypeName(),
                alert.description
            );
        }
        
        // Rileva picchi di allocazione
        // Implementazione semplificata, in pratica si monitorerebbe la velocità di crescita
        static size_t lastKnownMemory = 0;
        const double ALLOCATION_SPIKE_THRESHOLD = 0.1;  // 10%
        
        if (lastKnownMemory > 0) {
            double growthRate = static_cast<double>(m_stats.currentUsage - lastKnownMemory) / lastKnownMemory;
            
            if (growthRate > ALLOCATION_SPIKE_THRESHOLD) {
                MemoryAlert alert(
                    MemoryAlert::Type::ALLOCATION_SPIKE,
                    "Memory allocation spike detected: " + std::to_string(m_stats.currentUsage - lastKnownMemory) + 
                    " bytes (" + std::to_string(growthRate * 100.0) + "% increase)",
                    MonitoringSeverity::INFO
                );
                
                // Registra l'alerta
                MonitoringSystem::getInstance().logEvent(
                    MonitoringEventType::MEMORY,
                    alert.severity,
                    "MemoryAlert_" + alert.getTypeName(),
                    alert.description
                );
            }
        }
        
        lastKnownMemory = m_stats.currentUsage;
    }

    bool m_initialized;
    bool m_captureCallstack;
    bool m_trackSystemAllocations;
    
    std::unordered_map<void*, MemoryAllocationInfo> m_allocations;
    MemoryStats m_stats;
    
    std::unordered_map<int, std::pair<size_t, std::function<void(size_t)>>> m_memoryThresholds;
    int m_nextThresholdId;
    
    std::mutex m_mutex;
};

/**
 * @brief Macro per tracciare un'allocazione di memoria.
 * @param ptr Puntatore alla memoria allocata
 * @param size Dimensione in byte
 * @param tag Tag opzionale
 */
#define TRACK_ALLOCATION(ptr, size, tag) \
    Core::Monitoring::MemoryTracker::getInstance().trackAllocation(ptr, size, tag)

/**
 * @brief Macro per tracciare una deallocazione di memoria.
 * @param ptr Puntatore alla memoria da deallocare
 */
#define TRACK_DEALLOCATION(ptr) \
    Core::Monitoring::MemoryTracker::getInstance().trackDeallocation(ptr)

/**
 * @brief Wrapper per operatore new con tracking.
 * @param size Dimensione in byte
 * @param tag Tag opzionale
 * @return Puntatore alla memoria allocata
 */
inline void* TrackedAlloc(size_t size, const std::string& tag = "") {
    void* ptr = ::operator new(size);
    TRACK_ALLOCATION(ptr, size, tag);
    return ptr;
}

/**
 * @brief Wrapper per operatore delete con tracking.
 * @param ptr Puntatore alla memoria da deallocare
 */
inline void TrackedFree(void* ptr) {
    TRACK_DEALLOCATION(ptr);
    ::operator delete(ptr);
}

/**
 * @brief Alloca un array di oggetti con tracking.
 * @tparam T Tipo di oggetto
 * @param count Numero di oggetti
 * @param tag Tag opzionale
 * @return Puntatore all'array allocato
 */
template<typename T>
T* TrackedAllocArray(size_t count, const std::string& tag = "") {
    T* ptr = new T[count];
    TRACK_ALLOCATION(ptr, sizeof(T) * count, tag.empty() ? typeid(T).name() : tag);
    return ptr;
}

/**
 * @brief Dealloca un array di oggetti con tracking.
 * @tparam T Tipo di oggetto
 * @param ptr Puntatore all'array da deallocare
 */
template<typename T>
void TrackedFreeArray(T* ptr) {
    TRACK_DEALLOCATION(ptr);
    delete[] ptr;
}

/**
 * @brief Wrapper smart pointer per allocazioni tracciate.
 * @tparam T Tipo di oggetto
 */
template<typename T>
class TrackedPtr {
public:
    /**
     * @brief Costruttore.
     * @param tag Tag opzionale
     */
    explicit TrackedPtr(const std::string& tag = "")
        : m_ptr(static_cast<T*>(TrackedAlloc(sizeof(T), tag.empty() ? typeid(T).name() : tag))) {
        new(m_ptr) T();
    }
    
    /**
     * @brief Costruttore con parametri per l'oggetto.
     * @tparam Args Tipi dei parametri
     * @param tag Tag opzionale
     * @param args Parametri per il costruttore dell'oggetto
     */
    template<typename... Args>
    explicit TrackedPtr(const std::string& tag, Args&&... args)
        : m_ptr(static_cast<T*>(TrackedAlloc(sizeof(T), tag.empty() ? typeid(T).name() : tag))) {
        new(m_ptr) T(std::forward<Args>(args)...);
    }
    
    /**
     * @brief Distruttore.
     */
    ~TrackedPtr() {
        if (m_ptr) {
            m_ptr->~T();
            TrackedFree(m_ptr);
        }
    }
    
    // Disabilita copia
    TrackedPtr(const TrackedPtr&) = delete;
    TrackedPtr& operator=(const TrackedPtr&) = delete;
    
    // Abilita movimento
    TrackedPtr(TrackedPtr&& other) noexcept : m_ptr(other.m_ptr) {
        other.m_ptr = nullptr;
    }
    
    TrackedPtr& operator=(TrackedPtr&& other) noexcept {
        if (this != &other) {
            reset();
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }
        return *this;
    }
    
    /**
     * @brief Resetta il puntatore.
     */
    void reset() {
        if (m_ptr) {
            m_ptr->~T();
            TrackedFree(m_ptr);
            m_ptr = nullptr;
        }
    }
    
    /**
     * @brief Operatore di dereferenziazione.
     * @return Riferimento all'oggetto
     */
    T& operator*() const { return *m_ptr; }
    
    /**
     * @brief Operatore di accesso ai membri.
     * @return Puntatore all'oggetto
     */
    T* operator->() const { return m_ptr; }
    
    /**
     * @brief Ottiene il puntatore grezzo.
     * @return Puntatore all'oggetto
     */
    T* get() const { return m_ptr; }
    
    /**
     * @brief Converte in un valore booleano.
     * @return true se il puntatore è valido, false altrimenti
     */
    explicit operator bool() const { return m_ptr != nullptr; }

private:
    T* m_ptr;
};

/**
 * @brief Crea un TrackedPtr.
 * @tparam T Tipo di oggetto
 * @tparam Args Tipi dei parametri
 * @param tag Tag opzionale
 * @param args Parametri per il costruttore dell'oggetto
 * @return TrackedPtr all'oggetto creato
 */
template<typename T, typename... Args>
TrackedPtr<T> makeTracked(const std::string& tag = "", Args&&... args) {
    return TrackedPtr<T>(tag, std::forward<Args>(args)...);
}

/**
 * @brief Simulatore di perdite di memoria per test.
 */
class MemoryLeakSimulator {
public:
    /**
     * @brief Ottiene l'istanza singleton del simulatore.
     * @return Riferimento all'istanza singleton
     */
    static MemoryLeakSimulator& getInstance() {
        static MemoryLeakSimulator instance;
        return instance;
    }
    
    /**
     * @brief Simulia una perdita di memoria.
     * @param size Dimensione in byte
     * @param tag Tag opzionale
     */
    void simulateLeak(size_t size, const std::string& tag = "LeakTest") {
        void* ptr = ::operator new(size);
        TRACK_ALLOCATION(ptr, size, tag);
        
        // Non deallochiamo intenzionalmente per simulare la perdita
        m_leaks.push_back(ptr);
    }
    
    /**
     * @brief Ripulisce tutte le perdite simulate.
     */
    void cleanupLeaks() {
        for (void* ptr : m_leaks) {
            TRACK_DEALLOCATION(ptr);
            ::operator delete(ptr);
        }
        
        m_leaks.clear();
    }
    
private:
    MemoryLeakSimulator() = default;
    ~MemoryLeakSimulator() {
        cleanupLeaks();
    }
    
    MemoryLeakSimulator(const MemoryLeakSimulator&) = delete;
    MemoryLeakSimulator& operator=(const MemoryLeakSimulator&) = delete;
    
    std::vector<void*> m_leaks;
};

} // namespace Monitoring
} // namespace Core 