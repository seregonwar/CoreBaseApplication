#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <algorithm>

namespace Core {
namespace Extensions {

/**
 * @brief Enum che definisce le strategie di ottimizzazione disponibili.
 */
enum class OptimizationStrategy {
    MEMORY_POOL,              ///< Usa pool di memoria per ridurre le allocazioni
    MULTI_THREADING,          ///< Utilizza il multithreading quando possibile
    LAZY_LOADING,             ///< Carica le risorse solo quando necessario
    CACHING,                  ///< Memorizza nella cache i risultati delle operazioni costose
    BATCH_PROCESSING,         ///< Elabora i dati in batch per migliorare l'efficienza
    JIT_COMPILATION,          ///< Compila il codice just-in-time per velocizzare l'esecuzione
    CODE_INLINING,            ///< Inline delle funzioni critiche
    DATA_STRUCTURE_OPTIMIZATION, ///< Sceglie strutture dati ottimali per specifici pattern di accesso
    COMPRESSION,              ///< Comprime i dati per ridurre l'uso di memoria e I/O
    SIMD_INSTRUCTIONS         ///< Utilizza istruzioni SIMD per operazioni vettoriali
};

/**
 * @brief Enum che definisce i tipi di risorse che possono essere ottimizzate.
 */
enum class ResourceType {
    CPU,                      ///< Utilizzo della CPU
    MEMORY,                   ///< Utilizzo della memoria
    DISK_IO,                  ///< Operazioni di I/O su disco
    NETWORK_IO,               ///< Operazioni di I/O di rete
    GPU                       ///< Utilizzo della GPU
};

/**
 * @brief Struct che rappresenta un profilo di ottimizzazione.
 */
struct OptimizationProfile {
    std::string name;         ///< Nome del profilo
    std::unordered_map<OptimizationStrategy, bool> enabledStrategies; ///< Strategie abilitate
    std::unordered_map<ResourceType, float> resourcePriorities; ///< Priorità delle risorse (0.0-1.0)
    
    /**
     * @brief Costruttore con parametri minimi.
     */
    OptimizationProfile(const std::string& _name = "Default")
        : name(_name) {
        // Configurazione predefinita
        for (int i = 0; i < static_cast<int>(OptimizationStrategy::SIMD_INSTRUCTIONS) + 1; ++i) {
            enabledStrategies[static_cast<OptimizationStrategy>(i)] = false;
        }
        
        // Priorità predefinite
        resourcePriorities[ResourceType::CPU] = 0.5f;
        resourcePriorities[ResourceType::MEMORY] = 0.5f;
        resourcePriorities[ResourceType::DISK_IO] = 0.5f;
        resourcePriorities[ResourceType::NETWORK_IO] = 0.5f;
        resourcePriorities[ResourceType::GPU] = 0.5f;
    }
    
    /**
     * @brief Verifica se una strategia è abilitata.
     */
    bool isStrategyEnabled(OptimizationStrategy strategy) const {
        auto it = enabledStrategies.find(strategy);
        return it != enabledStrategies.end() && it->second;
    }
    
    /**
     * @brief Ottiene la priorità di una risorsa.
     */
    float getResourcePriority(ResourceType resource) const {
        auto it = resourcePriorities.find(resource);
        return (it != resourcePriorities.end()) ? it->second : 0.5f;
    }
};

/**
 * @brief Struct che rappresenta una metrica di prestazione.
 */
struct PerformanceMetric {
    std::string name;                     ///< Nome della metrica
    double value;                         ///< Valore della metrica
    std::string unit;                     ///< Unità di misura
    std::chrono::system_clock::time_point timestamp; ///< Timestamp della misurazione
    
    /**
     * @brief Costruttore.
     */
    PerformanceMetric(const std::string& _name = "", double _value = 0.0, const std::string& _unit = "")
        : name(_name), value(_value), unit(_unit), timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @brief Classe per la misurazione del tempo di esecuzione.
 */
class ScopedTimer {
public:
    /**
     * @brief Costruttore che avvia il timer.
     * @param name Nome dell'operazione misurata
     * @param callback Funzione da chiamare alla fine della misurazione
     */
    explicit ScopedTimer(const std::string& name, 
                        std::function<void(const std::string&, double)> callback = nullptr)
        : m_name(name), m_callback(callback), m_start(std::chrono::high_resolution_clock::now()) {}
    
    /**
     * @brief Distruttore che ferma il timer e chiama la callback.
     */
    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start).count() / 1000.0;
        
        if (m_callback) {
            m_callback(m_name, duration);
        }
    }

private:
    std::string m_name;
    std::function<void(const std::string&, double)> m_callback;
    std::chrono::high_resolution_clock::time_point m_start;
};

/**
 * @brief Implementazione di un memory pool.
 * 
 * @tparam T Tipo degli oggetti nel pool
 * @tparam BlockSize Numero di oggetti per blocco
 */
template<typename T, size_t BlockSize = 1024>
class MemoryPool {
public:
    /**
     * @brief Costruttore.
     */
    MemoryPool() : m_currentBlock(nullptr), m_currentSlot(nullptr), m_lastSlot(nullptr), m_freeSlots(nullptr) {}
    
    /**
     * @brief Distruttore.
     */
    ~MemoryPool() {
        for (auto block : m_blocks) {
            delete[] block;
        }
    }
    
    /**
     * @brief Alloca un oggetto.
     * @return Puntatore all'oggetto allocato
     */
    T* allocate() {
        if (m_freeSlots) {
            T* result = m_freeSlots;
            m_freeSlots = *reinterpret_cast<T**>(m_freeSlots);
            return result;
        } else {
            if (m_currentSlot >= m_lastSlot) {
                allocateBlock();
            }
            return m_currentSlot++;
        }
    }
    
    /**
     * @brief Dealloca un oggetto.
     * @param p Puntatore all'oggetto da deallocare
     */
    void deallocate(T* p) {
        if (p) {
            *reinterpret_cast<T**>(p) = m_freeSlots;
            m_freeSlots = p;
        }
    }
    
    /**
     * @brief Ottiene il numero di blocchi allocati.
     * @return Numero di blocchi
     */
    size_t getBlockCount() const {
        return m_blocks.size();
    }
    
    /**
     * @brief Ottiene la capacità totale del pool.
     * @return Capacità totale
     */
    size_t getCapacity() const {
        return m_blocks.size() * BlockSize;
    }

private:
    /**
     * @brief Alloca un nuovo blocco di memoria.
     */
    void allocateBlock() {
        T* newBlock = new T[BlockSize];
        m_blocks.push_back(newBlock);
        m_currentSlot = newBlock;
        m_lastSlot = newBlock + BlockSize;
    }
    
    T* m_currentBlock;
    T* m_currentSlot;
    T* m_lastSlot;
    T* m_freeSlots;
    std::vector<T*> m_blocks;
};

/**
 * @brief Cache LRU (Least Recently Used).
 * 
 * @tparam Key Tipo della chiave
 * @tparam Value Tipo del valore
 */
template<typename Key, typename Value>
class LRUCache {
public:
    /**
     * @brief Costruttore.
     * @param capacity Capacità massima della cache
     */
    explicit LRUCache(size_t capacity) : m_capacity(capacity) {}
    
    /**
     * @brief Ottiene un valore dalla cache.
     * @param key Chiave
     * @return Puntatore al valore, o nullptr se non presente
     */
    Value* get(const Key& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_map.find(key);
        if (it == m_map.end()) {
            return nullptr;
        }
        
        // Sposta l'elemento in testa (più recentemente usato)
        m_list.splice(m_list.begin(), m_list, it->second);
        return &(it->second->second);
    }
    
    /**
     * @brief Inserisce un valore nella cache.
     * @param key Chiave
     * @param value Valore
     */
    void put(const Key& key, const Value& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_map.find(key);
        if (it != m_map.end()) {
            // Aggiorna il valore e sposta in testa
            it->second->second = value;
            m_list.splice(m_list.begin(), m_list, it->second);
            return;
        }
        
        // Rimuovi l'elemento meno recentemente usato se necessario
        if (m_map.size() >= m_capacity) {
            auto last = m_list.end();
            --last;
            m_map.erase(last->first);
            m_list.pop_back();
        }
        
        // Inserisci il nuovo elemento in testa
        m_list.emplace_front(key, value);
        m_map[key] = m_list.begin();
    }
    
    /**
     * @brief Rimuove un valore dalla cache.
     * @param key Chiave
     * @return true se il valore è stato rimosso, false se non era presente
     */
    bool remove(const Key& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_map.find(key);
        if (it == m_map.end()) {
            return false;
        }
        
        m_list.erase(it->second);
        m_map.erase(it);
        return true;
    }
    
    /**
     * @brief Svuota la cache.
     */
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_list.clear();
        m_map.clear();
    }
    
    /**
     * @brief Ottiene il numero di elementi nella cache.
     * @return Numero di elementi
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_map.size();
    }
    
    /**
     * @brief Ottiene la capacità della cache.
     * @return Capacità
     */
    size_t capacity() const {
        return m_capacity;
    }

private:
    size_t m_capacity;
    std::list<std::pair<Key, Value>> m_list;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> m_map;
    mutable std::mutex m_mutex;
};

/**
 * @brief Gestore delle ottimizzazioni delle prestazioni.
 */
class PerformanceOptimizer {
public:
    /**
     * @brief Ottiene l'istanza singleton del gestore.
     * @return Riferimento all'istanza singleton
     */
    static PerformanceOptimizer& getInstance() {
        static PerformanceOptimizer instance;
        return instance;
    }
    
    /**
     * @brief Inizializza il gestore.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_initialized) return true;
        
        // Crea profili di ottimizzazione predefiniti
        createDefaultProfiles();
        
        // Avvia il thread di monitoraggio delle prestazioni
        startMonitoringThread();
        
        m_initialized = true;
        return true;
    }
    
    /**
     * @brief Crea un nuovo profilo di ottimizzazione.
     * @param profile Profilo di ottimizzazione
     * @return true se la creazione è avvenuta con successo, false altrimenti
     */
    bool createProfile(const OptimizationProfile& profile) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Verifica se il profilo esiste già
        if (m_profiles.find(profile.name) != m_profiles.end()) {
            return false;
        }
        
        m_profiles[profile.name] = profile;
        return true;
    }
    
    /**
     * @brief Aggiorna un profilo di ottimizzazione esistente.
     * @param profile Profilo di ottimizzazione
     * @return true se l'aggiornamento è avvenuto con successo, false altrimenti
     */
    bool updateProfile(const OptimizationProfile& profile) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Verifica se il profilo esiste
        if (m_profiles.find(profile.name) == m_profiles.end()) {
            return false;
        }
        
        m_profiles[profile.name] = profile;
        return true;
    }
    
    /**
     * @brief Rimuove un profilo di ottimizzazione.
     * @param profileName Nome del profilo
     * @return true se la rimozione è avvenuta con successo, false altrimenti
     */
    bool removeProfile(const std::string& profileName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Verifica se il profilo esiste
        auto it = m_profiles.find(profileName);
        if (it == m_profiles.end()) {
            return false;
        }
        
        // Non permettere la rimozione del profilo attivo
        if (m_activeProfileName == profileName) {
            return false;
        }
        
        m_profiles.erase(it);
        return true;
    }
    
    /**
     * @brief Ottiene un profilo di ottimizzazione.
     * @param profileName Nome del profilo
     * @return Profilo di ottimizzazione, o nullptr se non esiste
     */
    std::shared_ptr<OptimizationProfile> getProfile(const std::string& profileName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_profiles.find(profileName);
        if (it == m_profiles.end()) {
            return nullptr;
        }
        
        return std::make_shared<OptimizationProfile>(it->second);
    }
    
    /**
     * @brief Ottiene i nomi di tutti i profili disponibili.
     * @return Vettore di nomi dei profili
     */
    std::vector<std::string> getProfileNames() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<std::string> names;
        for (const auto& pair : m_profiles) {
            names.push_back(pair.first);
        }
        
        return names;
    }
    
    /**
     * @brief Attiva un profilo di ottimizzazione.
     * @param profileName Nome del profilo
     * @return true se l'attivazione è avvenuta con successo, false altrimenti
     */
    bool activateProfile(const std::string& profileName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Verifica se il profilo esiste
        auto it = m_profiles.find(profileName);
        if (it == m_profiles.end()) {
            return false;
        }
        
        // Applica le ottimizzazioni del profilo
        m_activeProfileName = profileName;
        applyOptimizations(it->second);
        
        return true;
    }
    
    /**
     * @brief Ottiene il nome del profilo attivo.
     * @return Nome del profilo attivo
     */
    std::string getActiveProfileName() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_activeProfileName;
    }
    
    /**
     * @brief Registra una metrica di prestazione.
     * @param metric Metrica di prestazione
     */
    void recordMetric(const PerformanceMetric& metric) {
        std::lock_guard<std::mutex> lock(m_metricsMutex);
        
        m_metrics.push_back(metric);
        
        // Limita la dimensione della cronologia delle metriche
        if (m_metrics.size() > m_maxMetricsHistory) {
            m_metrics.erase(m_metrics.begin());
        }
    }
    
    /**
     * @brief Registra una durata di operazione.
     * @param operationName Nome dell'operazione
     * @param durationMs Durata in millisecondi
     */
    void recordDuration(const std::string& operationName, double durationMs) {
        PerformanceMetric metric(operationName, durationMs, "ms");
        recordMetric(metric);
    }
    
    /**
     * @brief Crea un timer a scopo limitato per misurare la durata di un blocco di codice.
     * @param operationName Nome dell'operazione
     * @return Timer a scopo limitato
     */
    std::unique_ptr<ScopedTimer> createTimer(const std::string& operationName) {
        return std::make_unique<ScopedTimer>(operationName, 
            [this](const std::string& name, double duration) {
                recordDuration(name, duration);
            });
    }
    
    /**
     * @brief Ottiene le metriche di prestazione per un'operazione specifica.
     * @param operationName Nome dell'operazione
     * @return Vettore di metriche
     */
    std::vector<PerformanceMetric> getMetricsForOperation(const std::string& operationName) const {
        std::lock_guard<std::mutex> lock(m_metricsMutex);
        
        std::vector<PerformanceMetric> result;
        for (const auto& metric : m_metrics) {
            if (metric.name == operationName) {
                result.push_back(metric);
            }
        }
        
        return result;
    }
    
    /**
     * @brief Ottiene le metriche di prestazione per un intervallo di tempo.
     * @param start Inizio dell'intervallo
     * @param end Fine dell'intervallo
     * @return Vettore di metriche
     */
    std::vector<PerformanceMetric> getMetricsInTimeRange(
            std::chrono::system_clock::time_point start,
            std::chrono::system_clock::time_point end) const {
        std::lock_guard<std::mutex> lock(m_metricsMutex);
        
        std::vector<PerformanceMetric> result;
        for (const auto& metric : m_metrics) {
            if (metric.timestamp >= start && metric.timestamp <= end) {
                result.push_back(metric);
            }
        }
        
        return result;
    }
    
    /**
     * @brief Ottiene le metriche di prestazione più recenti.
     * @param count Numero di metriche da ottenere
     * @return Vettore di metriche
     */
    std::vector<PerformanceMetric> getRecentMetrics(size_t count) const {
        std::lock_guard<std::mutex> lock(m_metricsMutex);
        
        size_t resultSize = std::min(count, m_metrics.size());
        return std::vector<PerformanceMetric>(
            m_metrics.end() - resultSize,
            m_metrics.end()
        );
    }
    
    /**
     * @brief Ottiene una cache LRU.
     * @tparam Key Tipo della chiave
     * @tparam Value Tipo del valore
     * @param cacheName Nome della cache
     * @param capacity Capacità della cache (se non esiste)
     * @return Puntatore alla cache
     */
    template<typename Key, typename Value>
    std::shared_ptr<LRUCache<Key, Value>> getCache(
            const std::string& cacheName, size_t capacity = 1000) {
        std::lock_guard<std::mutex> lock(m_cachesMutex);
        
        // Verifica se la cache esiste già
        auto it = m_caches.find(cacheName);
        if (it != m_caches.end()) {
            return std::static_pointer_cast<LRUCache<Key, Value>>(it->second);
        }
        
        // Crea una nuova cache
        auto cache = std::make_shared<LRUCache<Key, Value>>(capacity);
        m_caches[cacheName] = cache;
        
        return cache;
    }
    
    /**
     * @brief Ottiene un memory pool.
     * @tparam T Tipo degli oggetti nel pool
     * @tparam BlockSize Numero di oggetti per blocco
     * @param poolName Nome del pool
     * @return Puntatore al pool
     */
    template<typename T, size_t BlockSize = 1024>
    std::shared_ptr<MemoryPool<T, BlockSize>> getMemoryPool(const std::string& poolName) {
        std::lock_guard<std::mutex> lock(m_poolsMutex);
        
        // Verifica se il pool esiste già
        auto it = m_pools.find(poolName);
        if (it != m_pools.end()) {
            return std::static_pointer_cast<MemoryPool<T, BlockSize>>(it->second);
        }
        
        // Crea un nuovo pool
        auto pool = std::make_shared<MemoryPool<T, BlockSize>>();
        m_pools[poolName] = pool;
        
        return pool;
    }
    
    /**
     * @brief Imposta il numero massimo di thread per le operazioni parallele.
     * @param maxThreads Numero massimo di thread
     */
    void setMaxThreads(unsigned int maxThreads) {
        m_maxThreads = maxThreads;
    }
    
    /**
     * @brief Ottiene il numero massimo di thread per le operazioni parallele.
     * @return Numero massimo di thread
     */
    unsigned int getMaxThreads() const {
        return m_maxThreads;
    }
    
    /**
     * @brief Imposta l'intervallo di monitoraggio delle prestazioni.
     * @param interval Intervallo in secondi
     */
    void setMonitoringInterval(std::chrono::seconds interval) {
        m_monitoringInterval = interval;
    }
    
    /**
     * @brief Ottiene l'intervallo di monitoraggio delle prestazioni.
     * @return Intervallo in secondi
     */
    std::chrono::seconds getMonitoringInterval() const {
        return m_monitoringInterval;
    }
    
    /**
     * @brief Imposta la dimensione massima della cronologia delle metriche.
     * @param maxHistory Dimensione massima
     */
    void setMaxMetricsHistory(size_t maxHistory) {
        std::lock_guard<std::mutex> lock(m_metricsMutex);
        
        m_maxMetricsHistory = maxHistory;
        
        // Riduci la cronologia se necessario
        if (m_metrics.size() > m_maxMetricsHistory) {
            m_metrics.resize(m_maxMetricsHistory);
        }
    }
    
    /**
     * @brief Ottiene la dimensione massima della cronologia delle metriche.
     * @return Dimensione massima
     */
    size_t getMaxMetricsHistory() const {
        return m_maxMetricsHistory;
    }
    
    /**
     * @brief Registra una callback da chiamare quando vengono raccolte nuove metriche.
     * @param callback Funzione da chiamare
     * @return ID della callback, necessario per rimuoverla
     */
    int registerMetricsCallback(std::function<void(const std::vector<PerformanceMetric>&)> callback) {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        
        int id = m_nextCallbackId++;
        m_metricsCallbacks[id] = callback;
        return id;
    }
    
    /**
     * @brief Rimuove una callback registrata.
     * @param callbackId ID della callback da rimuovere
     * @return true se la callback è stata rimossa, false se non esisteva
     */
    bool unregisterMetricsCallback(int callbackId) {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        
        auto it = m_metricsCallbacks.find(callbackId);
        if (it != m_metricsCallbacks.end()) {
            m_metricsCallbacks.erase(it);
            return true;
        }
        
        return false;
    }

private:
    // Costruttore privato per pattern Singleton
    PerformanceOptimizer() 
        : m_initialized(false), 
          m_activeProfileName("Default"),
          m_maxThreads(std::thread::hardware_concurrency()),
          m_monitoringInterval(std::chrono::seconds(60)),
          m_maxMetricsHistory(10000),
          m_nextCallbackId(0),
          m_isMonitoringActive(false) {}
    
    // Distruttore privato
    ~PerformanceOptimizer() {
        // Ferma il thread di monitoraggio
        stopMonitoringThread();
    }
    
    // Disabilita copia e assegnazione
    PerformanceOptimizer(const PerformanceOptimizer&) = delete;
    PerformanceOptimizer& operator=(const PerformanceOptimizer&) = delete;
    
    /**
     * @brief Crea profili di ottimizzazione predefiniti.
     */
    void createDefaultProfiles() {
        // Profilo predefinito (bilanciato)
        OptimizationProfile defaultProfile("Default");
        defaultProfile.enabledStrategies[OptimizationStrategy::MEMORY_POOL] = true;
        defaultProfile.enabledStrategies[OptimizationStrategy::MULTI_THREADING] = true;
        defaultProfile.enabledStrategies[OptimizationStrategy::CACHING] = true;
        m_profiles["Default"] = defaultProfile;
        
        // Profilo ad alte prestazioni (massimizza velocità)
        OptimizationProfile highPerformanceProfile("HighPerformance");
        for (int i = 0; i < static_cast<int>(OptimizationStrategy::SIMD_INSTRUCTIONS) + 1; ++i) {
            highPerformanceProfile.enabledStrategies[static_cast<OptimizationStrategy>(i)] = true;
        }
        highPerformanceProfile.resourcePriorities[ResourceType::CPU] = 1.0f;
        highPerformanceProfile.resourcePriorities[ResourceType::MEMORY] = 0.8f;
        m_profiles["HighPerformance"] = highPerformanceProfile;
        
        // Profilo a basso consumo di memoria
        OptimizationProfile lowMemoryProfile("LowMemory");
        lowMemoryProfile.enabledStrategies[OptimizationStrategy::MEMORY_POOL] = true;
        lowMemoryProfile.enabledStrategies[OptimizationStrategy::LAZY_LOADING] = true;
        lowMemoryProfile.enabledStrategies[OptimizationStrategy::COMPRESSION] = true;
        lowMemoryProfile.resourcePriorities[ResourceType::MEMORY] = 1.0f;
        lowMemoryProfile.resourcePriorities[ResourceType::CPU] = 0.6f;
        m_profiles["LowMemory"] = lowMemoryProfile;
        
        // Profilo per dispositivi mobili
        OptimizationProfile mobileProfile("Mobile");
        mobileProfile.enabledStrategies[OptimizationStrategy::MEMORY_POOL] = true;
        mobileProfile.enabledStrategies[OptimizationStrategy::LAZY_LOADING] = true;
        mobileProfile.enabledStrategies[OptimizationStrategy::CACHING] = true;
        mobileProfile.enabledStrategies[OptimizationStrategy::COMPRESSION] = true;
        mobileProfile.resourcePriorities[ResourceType::MEMORY] = 0.9f;
        mobileProfile.resourcePriorities[ResourceType::CPU] = 0.7f;
        mobileProfile.resourcePriorities[ResourceType::DISK_IO] = 0.5f;
        mobileProfile.resourcePriorities[ResourceType::NETWORK_IO] = 0.3f;
        m_profiles["Mobile"] = mobileProfile;
    }
    
    /**
     * @brief Applica le ottimizzazioni di un profilo.
     * @param profile Profilo di ottimizzazione
     */
    void applyOptimizations(const OptimizationProfile& profile) {
        // Esempio di applicazione delle ottimizzazioni
        if (profile.isStrategyEnabled(OptimizationStrategy::MULTI_THREADING)) {
            // Configura il multithreading
            m_maxThreads = std::thread::hardware_concurrency();
        } else {
            // Limita l'uso di thread
            m_maxThreads = 1;
        }
        
        // Altre ottimizzazioni...
    }
    
    /**
     * @brief Avvia il thread di monitoraggio delle prestazioni.
     */
    void startMonitoringThread() {
        if (m_isMonitoringActive) return;
        
        m_isMonitoringActive = true;
        m_monitoringThread = std::thread([this]() {
            while (m_isMonitoringActive) {
                // Raccogli metriche di prestazione del sistema
                collectSystemMetrics();
                
                // Notifica i sottoscrittori
                notifyMetricsSubscribers();
                
                // Attendi l'intervallo di monitoraggio
                std::this_thread::sleep_for(m_monitoringInterval);
            }
        });
    }
    
    /**
     * @brief Ferma il thread di monitoraggio delle prestazioni.
     */
    void stopMonitoringThread() {
        if (!m_isMonitoringActive) return;
        
        m_isMonitoringActive = false;
        if (m_monitoringThread.joinable()) {
            m_monitoringThread.join();
        }
    }
    
    /**
     * @brief Raccoglie metriche di prestazione del sistema.
     */
    void collectSystemMetrics() {
        // Raccoglie metriche sulla CPU
        double cpuUsage = getCPUUsage();
        recordMetric(PerformanceMetric("CPU Usage", cpuUsage, "%"));
        
        // Raccoglie metriche sulla memoria
        auto memoryInfo = getMemoryInfo();
        recordMetric(PerformanceMetric("Memory Used", memoryInfo.first, "MB"));
        recordMetric(PerformanceMetric("Memory Total", memoryInfo.second, "MB"));
        
        // Altre metriche di sistema...
    }
    
    /**
     * @brief Notifica i sottoscrittori delle metriche.
     */
    void notifyMetricsSubscribers() {
        std::unordered_map<int, std::function<void(const std::vector<PerformanceMetric>&)>> callbacksCopy;
        
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            callbacksCopy = m_metricsCallbacks;
        }
        
        std::vector<PerformanceMetric> recentMetrics = getRecentMetrics(100);
        
        for (const auto& [id, callback] : callbacksCopy) {
            callback(recentMetrics);
        }
    }
    
    /**
     * @brief Ottiene l'utilizzo della CPU.
     * @return Percentuale di utilizzo della CPU
     */
    double getCPUUsage() {
        // Implementazione semplificata (in un'implementazione reale, userebbe API specifiche per piattaforma)
        return 50.0; // Esempio di dato
    }
    
    /**
     * @brief Ottiene informazioni sull'utilizzo della memoria.
     * @return Coppia (memoria usata in MB, memoria totale in MB)
     */
    std::pair<double, double> getMemoryInfo() {
        // Implementazione semplificata (in un'implementazione reale, userebbe API specifiche per piattaforma)
        return std::make_pair(1024.0, 8192.0); // Esempio di dati
    }
    
    bool m_initialized;
    std::string m_activeProfileName;
    unsigned int m_maxThreads;
    std::chrono::seconds m_monitoringInterval;
    size_t m_maxMetricsHistory;
    int m_nextCallbackId;
    
    std::unordered_map<std::string, OptimizationProfile> m_profiles;
    mutable std::mutex m_mutex;
    
    std::vector<PerformanceMetric> m_metrics;
    mutable std::mutex m_metricsMutex;
    
    std::unordered_map<int, std::function<void(const std::vector<PerformanceMetric>&)>> m_metricsCallbacks;
    std::mutex m_callbackMutex;
    
    std::unordered_map<std::string, std::shared_ptr<void>> m_caches;
    std::mutex m_cachesMutex;
    
    std::unordered_map<std::string, std::shared_ptr<void>> m_pools;
    std::mutex m_poolsMutex;
    
    bool m_isMonitoringActive;
    std::thread m_monitoringThread;
};

// Macro per creare un timer a scopo limitato
#define PERFORMANCE_TIMER(name) \
    auto timer = Core::Extensions::PerformanceOptimizer::getInstance().createTimer(name)

} // namespace Extensions
} // namespace Core 