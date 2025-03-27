#pragma once

#include "MonitoringSystem.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <algorithm>

namespace Core {
namespace Monitoring {

/**
 * @brief Struct che rappresenta l'utilizzo della CPU.
 */
struct CPUUsage {
    double totalUsage;                  ///< Utilizzo totale della CPU (0-100%)
    std::vector<double> coreUsage;      ///< Utilizzo per core (0-100%)
    double temperature;                 ///< Temperatura in °C (se disponibile)
    double frequency;                   ///< Frequenza in MHz (se disponibile)
    uint32_t processCount;              ///< Numero di processi
    uint32_t threadCount;               ///< Numero di thread
    std::string arch;                   ///< Architettura della CPU
    std::string model;                  ///< Modello della CPU
    
    /**
     * @brief Costruttore.
     */
    CPUUsage()
        : totalUsage(0), temperature(0), frequency(0), processCount(0), threadCount(0) {}
};

/**
 * @brief Struct che rappresenta l'utilizzo della memoria.
 */
struct MemoryUsage {
    size_t totalPhysical;               ///< Memoria fisica totale in byte
    size_t usedPhysical;                ///< Memoria fisica utilizzata in byte
    size_t availablePhysical;           ///< Memoria fisica disponibile in byte
    size_t totalVirtual;                ///< Memoria virtuale totale in byte
    size_t usedVirtual;                 ///< Memoria virtuale utilizzata in byte
    size_t availableVirtual;            ///< Memoria virtuale disponibile in byte
    size_t totalSwap;                   ///< Memoria swap totale in byte
    size_t usedSwap;                    ///< Memoria swap utilizzata in byte
    
    /**
     * @brief Costruttore.
     */
    MemoryUsage()
        : totalPhysical(0), usedPhysical(0), availablePhysical(0),
          totalVirtual(0), usedVirtual(0), availableVirtual(0),
          totalSwap(0), usedSwap(0) {}
    
    /**
     * @brief Calcola la percentuale di memoria fisica utilizzata.
     * @return Percentuale di memoria fisica utilizzata (0-100%)
     */
    double getPhysicalUsagePercentage() const {
        return (totalPhysical > 0) ? (static_cast<double>(usedPhysical) / totalPhysical * 100.0) : 0.0;
    }
    
    /**
     * @brief Calcola la percentuale di memoria virtuale utilizzata.
     * @return Percentuale di memoria virtuale utilizzata (0-100%)
     */
    double getVirtualUsagePercentage() const {
        return (totalVirtual > 0) ? (static_cast<double>(usedVirtual) / totalVirtual * 100.0) : 0.0;
    }
    
    /**
     * @brief Calcola la percentuale di memoria swap utilizzata.
     * @return Percentuale di memoria swap utilizzata (0-100%)
     */
    double getSwapUsagePercentage() const {
        return (totalSwap > 0) ? (static_cast<double>(usedSwap) / totalSwap * 100.0) : 0.0;
    }
};

/**
 * @brief Struct che rappresenta l'utilizzo del disco.
 */
struct DiskUsage {
    struct Disk {
        std::string name;               ///< Nome del disco
        std::string mountPoint;         ///< Punto di montaggio
        std::string fileSystem;         ///< Filesystem
        size_t totalSpace;              ///< Spazio totale in byte
        size_t usedSpace;               ///< Spazio utilizzato in byte
        size_t availableSpace;          ///< Spazio disponibile in byte
        double readRate;                ///< Velocità di lettura in byte/s
        double writeRate;               ///< Velocità di scrittura in byte/s
        double busyTime;                ///< Tempo di utilizzo in %
        
        /**
         * @brief Calcola la percentuale di spazio utilizzato.
         * @return Percentuale di spazio utilizzato (0-100%)
         */
        double getUsagePercentage() const {
            return (totalSpace > 0) ? (static_cast<double>(usedSpace) / totalSpace * 100.0) : 0.0;
        }
    };
    
    std::vector<Disk> disks;            ///< Informazioni sui dischi
    
    /**
     * @brief Ottiene un disco per nome.
     * @param name Nome del disco
     * @return Puntatore al disco, o nullptr se non trovato
     */
    const Disk* getDiskByName(const std::string& name) const {
        auto it = std::find_if(disks.begin(), disks.end(),
                              [&name](const Disk& disk) { return disk.name == name; });
        return (it != disks.end()) ? &(*it) : nullptr;
    }
    
    /**
     * @brief Ottiene un disco per punto di montaggio.
     * @param mountPoint Punto di montaggio
     * @return Puntatore al disco, o nullptr se non trovato
     */
    const Disk* getDiskByMountPoint(const std::string& mountPoint) const {
        auto it = std::find_if(disks.begin(), disks.end(),
                              [&mountPoint](const Disk& disk) { return disk.mountPoint == mountPoint; });
        return (it != disks.end()) ? &(*it) : nullptr;
    }
};

/**
 * @brief Struct che rappresenta l'utilizzo della rete.
 */
struct NetworkUsage {
    struct Interface {
        std::string name;               ///< Nome dell'interfaccia
        std::string ipAddress;          ///< Indirizzo IP
        std::string macAddress;         ///< Indirizzo MAC
        size_t bytesReceived;           ///< Byte ricevuti
        size_t bytesSent;               ///< Byte inviati
        double receiveRate;             ///< Velocità di ricezione in byte/s
        double sendRate;                ///< Velocità di invio in byte/s
        bool isUp;                      ///< Se l'interfaccia è attiva
        
        /**
         * @brief Calcola la velocità totale.
         * @return Velocità totale in byte/s
         */
        double getTotalRate() const {
            return receiveRate + sendRate;
        }
    };
    
    std::vector<Interface> interfaces;  ///< Informazioni sulle interfacce
    
    /**
     * @brief Ottiene un'interfaccia per nome.
     * @param name Nome dell'interfaccia
     * @return Puntatore all'interfaccia, o nullptr se non trovata
     */
    const Interface* getInterfaceByName(const std::string& name) const {
        auto it = std::find_if(interfaces.begin(), interfaces.end(),
                              [&name](const Interface& interface) { return interface.name == name; });
        return (it != interfaces.end()) ? &(*it) : nullptr;
    }
};

/**
 * @brief Struct che rappresenta l'utilizzo della GPU.
 */
struct GPUUsage {
    struct GPU {
        std::string name;               ///< Nome della GPU
        std::string vendor;             ///< Produttore
        std::string driver;             ///< Versione del driver
        double usage;                   ///< Utilizzo in %
        size_t memoryTotal;             ///< Memoria totale in byte
        size_t memoryUsed;              ///< Memoria utilizzata in byte
        double temperature;             ///< Temperatura in °C
        double powerUsage;              ///< Consumo di energia in W
        double coreFrequency;           ///< Frequenza del core in MHz
        double memoryFrequency;         ///< Frequenza della memoria in MHz
        
        /**
         * @brief Calcola la percentuale di memoria utilizzata.
         * @return Percentuale di memoria utilizzata (0-100%)
         */
        double getMemoryUsagePercentage() const {
            return (memoryTotal > 0) ? (static_cast<double>(memoryUsed) / memoryTotal * 100.0) : 0.0;
        }
    };
    
    std::vector<GPU> gpus;              ///< Informazioni sulle GPU
    
    /**
     * @brief Ottiene una GPU per nome.
     * @param name Nome della GPU
     * @return Puntatore alla GPU, o nullptr se non trovata
     */
    const GPU* getGPUByName(const std::string& name) const {
        auto it = std::find_if(gpus.begin(), gpus.end(),
                              [&name](const GPU& gpu) { return gpu.name == name; });
        return (it != gpus.end()) ? &(*it) : nullptr;
    }
};

/**
 * @brief Struct che rappresenta le informazioni sulle prestazioni del sistema.
 */
struct SystemPerformanceInfo {
    CPUUsage cpu;                       ///< Utilizzo della CPU
    MemoryUsage memory;                 ///< Utilizzo della memoria
    DiskUsage disk;                     ///< Utilizzo del disco
    NetworkUsage network;               ///< Utilizzo della rete
    GPUUsage gpu;                       ///< Utilizzo della GPU
    std::chrono::system_clock::time_point timestamp; ///< Timestamp
    
    /**
     * @brief Costruttore.
     */
    SystemPerformanceInfo()
        : timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @brief Enum che definisce gli intervalli di monitoraggio.
 */
enum class MonitoringInterval {
    REAL_TIME,         ///< Tempo reale (100ms)
    FAST,              ///< Veloce (500ms)
    NORMAL,            ///< Normale (1s)
    SLOW,              ///< Lento (5s)
    VERY_SLOW          ///< Molto lento (30s)
};

/**
 * @brief Converti un intervallo di monitoraggio in millisecondi.
 * @param interval Intervallo di monitoraggio
 * @return Intervallo in millisecondi
 */
inline std::chrono::milliseconds getIntervalDuration(MonitoringInterval interval) {
    switch (interval) {
        case MonitoringInterval::REAL_TIME:
            return std::chrono::milliseconds(100);
        case MonitoringInterval::FAST:
            return std::chrono::milliseconds(500);
        case MonitoringInterval::NORMAL:
            return std::chrono::milliseconds(1000);
        case MonitoringInterval::SLOW:
            return std::chrono::milliseconds(5000);
        case MonitoringInterval::VERY_SLOW:
            return std::chrono::milliseconds(30000);
        default:
            return std::chrono::milliseconds(1000);
    }
}

/**
 * @brief Classe per il monitoraggio delle prestazioni del sistema.
 */
class PerformanceMonitor {
public:
    /**
     * @brief Ottiene l'istanza singleton del monitor delle prestazioni.
     * @return Riferimento all'istanza singleton
     */
    static PerformanceMonitor& getInstance() {
        static PerformanceMonitor instance;
        return instance;
    }
    
    /**
     * @brief Inizializza il monitor delle prestazioni.
     * @param interval Intervallo di monitoraggio
     * @param enableCPU Se true, abilita il monitoraggio della CPU
     * @param enableMemory Se true, abilita il monitoraggio della memoria
     * @param enableDisk Se true, abilita il monitoraggio del disco
     * @param enableNetwork Se true, abilita il monitoraggio della rete
     * @param enableGPU Se true, abilita il monitoraggio della GPU
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize(
        MonitoringInterval interval = MonitoringInterval::NORMAL,
        bool enableCPU = true,
        bool enableMemory = true,
        bool enableDisk = true,
        bool enableNetwork = true,
        bool enableGPU = true
    ) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_initialized) return true;
        
        m_interval = interval;
        m_enableCPU = enableCPU;
        m_enableMemory = enableMemory;
        m_enableDisk = enableDisk;
        m_enableNetwork = enableNetwork;
        m_enableGPU = enableGPU;
        
        // Registra questo monitor nel sistema di monitoraggio principale
        MonitoringSystem::getInstance().logEvent(
            MonitoringEventType::SYSTEM,
            MonitoringSeverity::INFO,
            "PerformanceMonitorInitialized",
            "Performance monitoring initialized with interval: " + std::to_string(getIntervalDuration(m_interval).count()) + "ms"
        );
        
        // Avvia il thread di monitoraggio
        startMonitoringThread();
        
        m_initialized = true;
        return true;
    }
    
    /**
     * @brief Termina il monitor delle prestazioni.
     * @return true se la terminazione è avvenuta con successo, false altrimenti
     */
    bool shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) return true;
        
        // Ferma il thread di monitoraggio
        stopMonitoringThread();
        
        // Registra l'evento di terminazione
        MonitoringSystem::getInstance().logEvent(
            MonitoringEventType::SYSTEM,
            MonitoringSeverity::INFO,
            "PerformanceMonitorShutdown",
            "Performance monitoring shutting down"
        );
        
        m_initialized = false;
        return true;
    }
    
    /**
     * @brief Imposta l'intervallo di monitoraggio.
     * @param interval Intervallo di monitoraggio
     */
    void setInterval(MonitoringInterval interval) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_interval == interval) return;
        
        m_interval = interval;
        
        // Registra il cambio di intervallo
        MonitoringSystem::getInstance().logEvent(
            MonitoringEventType::SYSTEM,
            MonitoringSeverity::INFO,
            "PerformanceMonitorIntervalChanged",
            "Performance monitoring interval changed to: " + std::to_string(getIntervalDuration(m_interval).count()) + "ms"
        );
    }
    
    /**
     * @brief Ottiene l'intervallo di monitoraggio.
     * @return Intervallo di monitoraggio
     */
    MonitoringInterval getInterval() const {
        return m_interval;
    }
    
    /**
     * @brief Abilita o disabilita il monitoraggio della CPU.
     * @param enable Se true, abilita il monitoraggio
     */
    void enableCPUMonitoring(bool enable) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_enableCPU = enable;
    }
    
    /**
     * @brief Abilita o disabilita il monitoraggio della memoria.
     * @param enable Se true, abilita il monitoraggio
     */
    void enableMemoryMonitoring(bool enable) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_enableMemory = enable;
    }
    
    /**
     * @brief Abilita o disabilita il monitoraggio del disco.
     * @param enable Se true, abilita il monitoraggio
     */
    void enableDiskMonitoring(bool enable) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_enableDisk = enable;
    }
    
    /**
     * @brief Abilita o disabilita il monitoraggio della rete.
     * @param enable Se true, abilita il monitoraggio
     */
    void enableNetworkMonitoring(bool enable) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_enableNetwork = enable;
    }
    
    /**
     * @brief Abilita o disabilita il monitoraggio della GPU.
     * @param enable Se true, abilita il monitoraggio
     */
    void enableGPUMonitoring(bool enable) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_enableGPU = enable;
    }
    
    /**
     * @brief Ottiene le informazioni sulle prestazioni attuali.
     * @return Informazioni sulle prestazioni
     */
    SystemPerformanceInfo getCurrentPerformance() {
        std::lock_guard<std::mutex> lock(m_performanceMutex);
        return m_currentPerformance;
    }
    
    /**
     * @brief Registra una callback da chiamare quando vengono aggiornate le informazioni sulle prestazioni.
     * @param callback Funzione da chiamare
     * @return ID della callback, necessario per rimuoverla
     */
    int registerPerformanceCallback(std::function<void(const SystemPerformanceInfo&)> callback) {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        
        int id = m_nextCallbackId++;
        m_performanceCallbacks[id] = callback;
        
        return id;
    }
    
    /**
     * @brief Rimuove una callback registrata.
     * @param callbackId ID della callback da rimuovere
     * @return true se la callback è stata rimossa, false se non esisteva
     */
    bool unregisterPerformanceCallback(int callbackId) {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        
        auto it = m_performanceCallbacks.find(callbackId);
        if (it == m_performanceCallbacks.end()) {
            return false;
        }
        
        m_performanceCallbacks.erase(it);
        
        return true;
    }
    
    /**
     * @brief Imposta una soglia per la CPU.
     * @param thresholdPercentage Percentuale di utilizzo da monitorare
     * @param callback Funzione da chiamare quando la soglia viene superata
     * @return ID della soglia, necessario per rimuoverla
     */
    int setCPUThreshold(double thresholdPercentage, std::function<void(double)> callback) {
        std::lock_guard<std::mutex> lock(m_thresholdMutex);
        
        int id = m_nextThresholdId++;
        m_cpuThresholds[id] = { thresholdPercentage, callback };
        
        return id;
    }
    
    /**
     * @brief Imposta una soglia per la memoria.
     * @param thresholdPercentage Percentuale di utilizzo da monitorare
     * @param callback Funzione da chiamare quando la soglia viene superata
     * @return ID della soglia, necessario per rimuoverla
     */
    int setMemoryThreshold(double thresholdPercentage, std::function<void(double)> callback) {
        std::lock_guard<std::mutex> lock(m_thresholdMutex);
        
        int id = m_nextThresholdId++;
        m_memoryThresholds[id] = { thresholdPercentage, callback };
        
        return id;
    }
    
    /**
     * @brief Rimuove una soglia registrata.
     * @param thresholdId ID della soglia da rimuovere
     * @return true se la soglia è stata rimossa, false se non esisteva
     */
    bool removeThreshold(int thresholdId) {
        std::lock_guard<std::mutex> lock(m_thresholdMutex);
        
        // Cerca nelle varie mappe di soglie
        if (m_cpuThresholds.erase(thresholdId) > 0) return true;
        if (m_memoryThresholds.erase(thresholdId) > 0) return true;
        
        return false;
    }

private:
    // Costruttore privato per pattern Singleton
    PerformanceMonitor()
        : m_initialized(false), 
          m_interval(MonitoringInterval::NORMAL),
          m_enableCPU(true),
          m_enableMemory(true),
          m_enableDisk(true),
          m_enableNetwork(true),
          m_enableGPU(true),
          m_nextCallbackId(0),
          m_nextThresholdId(0),
          m_isMonitoringActive(false) {}
    
    // Distruttore privato
    ~PerformanceMonitor() {
        shutdown();
    }
    
    // Disabilita copia e assegnazione
    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;
    
    /**
     * @brief Avvia il thread di monitoraggio.
     */
    void startMonitoringThread() {
        if (m_isMonitoringActive) return;
        
        m_isMonitoringActive = true;
        m_monitoringThread = std::thread([this]() {
            while (m_isMonitoringActive) {
                // Raccogli informazioni sulle prestazioni
                collectPerformanceInfo();
                
                // Attendi l'intervallo di monitoraggio
                std::this_thread::sleep_for(getIntervalDuration(m_interval));
            }
        });
    }
    
    /**
     * @brief Ferma il thread di monitoraggio.
     */
    void stopMonitoringThread() {
        if (!m_isMonitoringActive) return;
        
        m_isMonitoringActive = false;
        if (m_monitoringThread.joinable()) {
            m_monitoringThread.join();
        }
    }
    
    /**
     * @brief Raccoglie informazioni sulle prestazioni.
     */
    void collectPerformanceInfo() {
        // Crea una nuova struttura per le prestazioni
        SystemPerformanceInfo info;
        
        // Raccogli le informazioni in base alle impostazioni
        if (m_enableCPU) {
            collectCPUInfo(info.cpu);
        }
        
        if (m_enableMemory) {
            collectMemoryInfo(info.memory);
        }
        
        if (m_enableDisk) {
            collectDiskInfo(info.disk);
        }
        
        if (m_enableNetwork) {
            collectNetworkInfo(info.network);
        }
        
        if (m_enableGPU) {
            collectGPUInfo(info.gpu);
        }
        
        // Aggiorna il timestamp
        info.timestamp = std::chrono::system_clock::now();
        
        // Aggiorna le informazioni correnti
        {
            std::lock_guard<std::mutex> lock(m_performanceMutex);
            m_currentPerformance = info;
        }
        
        // Invia un evento di aggiornamento delle prestazioni
        MonitoringEvent event(
            MonitoringEventType::PERFORMANCE,
            MonitoringSeverity::INFO,
            "PerformanceUpdate",
            "Performance information updated"
        );
        
        // Aggiungi i dati di prestazione all'evento
        event.addNumericData("cpu_usage", info.cpu.totalUsage);
        event.addNumericData("memory_usage", info.memory.getPhysicalUsagePercentage());
        
        // Invia l'evento al sistema di monitoraggio
        MonitoringSystem::getInstance().logEvent(
            event.type,
            event.severity,
            event.name,
            event.description
        );
        
        // Verifica le soglie
        checkThresholds(info);
        
        // Notifica i sottoscrittori
        notifyPerformanceSubscribers(info);
    }
    
    /**
     * @brief Raccoglie informazioni sulla CPU.
     * @param cpu Struttura da popolare
     */
    void collectCPUInfo(CPUUsage& cpu) {
        // Implementazione per raccogliere informazioni sulla CPU
        // Questa è una versione semplificata/mock, in un'implementazione reale
        // utilizzerei API specifiche per piattaforma per ottenere dati reali
        
        #ifdef _WIN32
            // Implementazione Windows
            // Potrebbe utilizzare API come GetSystemTimes, GetSystemInfo, ecc.
        #elif defined(__linux__)
            // Implementazione Linux
            // Potrebbe leggere da /proc/stat, /proc/cpuinfo, ecc.
        #elif defined(__APPLE__)
            // Implementazione macOS
            // Potrebbe utilizzare IOKit, sysctl, ecc.
        #endif
        
        // Dati di esempio
        cpu.totalUsage = 25.0;  // 25% di utilizzo
        cpu.coreUsage.resize(4, 20.0);  // 4 core, ognuno al 20%
        cpu.temperature = 45.0;  // 45°C
        cpu.frequency = 3200.0;  // 3.2 GHz
        cpu.processCount = 100;
        cpu.threadCount = 1200;
        cpu.arch = "x86_64";
        cpu.model = "Intel Core i7";
    }
    
    /**
     * @brief Raccoglie informazioni sulla memoria.
     * @param memory Struttura da popolare
     */
    void collectMemoryInfo(MemoryUsage& memory) {
        // Implementazione per raccogliere informazioni sulla memoria
        // Questa è una versione semplificata/mock
        
        #ifdef _WIN32
            // Implementazione Windows
            // Potrebbe utilizzare API come GlobalMemoryStatusEx
        #elif defined(__linux__)
            // Implementazione Linux
            // Potrebbe leggere da /proc/meminfo
        #elif defined(__APPLE__)
            // Implementazione macOS
            // Potrebbe utilizzare sysctl, vm_statistics, ecc.
        #endif
        
        // Dati di esempio
        memory.totalPhysical = 16ULL * 1024 * 1024 * 1024;  // 16 GB
        memory.usedPhysical = 8ULL * 1024 * 1024 * 1024;    // 8 GB
        memory.availablePhysical = 8ULL * 1024 * 1024 * 1024; // 8 GB
        memory.totalVirtual = 32ULL * 1024 * 1024 * 1024;   // 32 GB
        memory.usedVirtual = 12ULL * 1024 * 1024 * 1024;    // 12 GB
        memory.availableVirtual = 20ULL * 1024 * 1024 * 1024; // 20 GB
        memory.totalSwap = 8ULL * 1024 * 1024 * 1024;       // 8 GB
        memory.usedSwap = 1ULL * 1024 * 1024 * 1024;        // 1 GB
    }
    
    /**
     * @brief Raccoglie informazioni sul disco.
     * @param disk Struttura da popolare
     */
    void collectDiskInfo(DiskUsage& disk) {
        // Implementazione per raccogliere informazioni sul disco
        // Questa è una versione semplificata/mock
        
        #ifdef _WIN32
            // Implementazione Windows
            // Potrebbe utilizzare API come GetDiskFreeSpaceEx, DeviceIoControl
        #elif defined(__linux__)
            // Implementazione Linux
            // Potrebbe leggere da /proc/diskstats, utilizzare statvfs, ecc.
        #elif defined(__APPLE__)
            // Implementazione macOS
            // Potrebbe utilizzare statvfs, IOKit, ecc.
        #endif
        
        // Dati di esempio
        DiskUsage::Disk sysDisk;
        sysDisk.name = "C:";
        sysDisk.mountPoint = "C:\\";
        sysDisk.fileSystem = "NTFS";
        sysDisk.totalSpace = 500ULL * 1024 * 1024 * 1024;  // 500 GB
        sysDisk.usedSpace = 250ULL * 1024 * 1024 * 1024;   // 250 GB
        sysDisk.availableSpace = 250ULL * 1024 * 1024 * 1024; // 250 GB
        sysDisk.readRate = 50 * 1024 * 1024;  // 50 MB/s
        sysDisk.writeRate = 30 * 1024 * 1024; // 30 MB/s
        sysDisk.busyTime = 5.0;  // 5%
        
        disk.disks.push_back(sysDisk);
    }
    
    /**
     * @brief Raccoglie informazioni sulla rete.
     * @param network Struttura da popolare
     */
    void collectNetworkInfo(NetworkUsage& network) {
        // Implementazione per raccogliere informazioni sulla rete
        // Questa è una versione semplificata/mock
        
        #ifdef _WIN32
            // Implementazione Windows
            // Potrebbe utilizzare API come GetIfTable2, GetIfEntry2
        #elif defined(__linux__)
            // Implementazione Linux
            // Potrebbe leggere da /proc/net/dev
        #elif defined(__APPLE__)
            // Implementazione macOS
            // Potrebbe utilizzare getifaddrs, IOKit, ecc.
        #endif
        
        // Dati di esempio
        NetworkUsage::Interface eth;
        eth.name = "eth0";
        eth.ipAddress = "192.168.1.100";
        eth.macAddress = "00:11:22:33:44:55";
        eth.bytesReceived = 1024 * 1024 * 100;  // 100 MB
        eth.bytesSent = 1024 * 1024 * 50;       // 50 MB
        eth.receiveRate = 1024 * 1024;  // 1 MB/s
        eth.sendRate = 512 * 1024;      // 512 KB/s
        eth.isUp = true;
        
        network.interfaces.push_back(eth);
    }
    
    /**
     * @brief Raccoglie informazioni sulla GPU.
     * @param gpu Struttura da popolare
     */
    void collectGPUInfo(GPUUsage& gpu) {
        // Implementazione per raccogliere informazioni sulla GPU
        // Questa è una versione semplificata/mock
        
        #ifdef _WIN32
            // Implementazione Windows
            // Potrebbe utilizzare API come NVAPI, AMD ADL
        #elif defined(__linux__)
            // Implementazione Linux
            // Potrebbe utilizzare NVML, AMD ADL, ecc.
        #elif defined(__APPLE__)
            // Implementazione macOS
            // Potrebbe utilizzare IOKit
        #endif
        
        // Dati di esempio
        GPUUsage::GPU gpuInfo;
        gpuInfo.name = "NVIDIA GeForce RTX 3080";
        gpuInfo.vendor = "NVIDIA Corporation";
        gpuInfo.driver = "512.15";
        gpuInfo.usage = 30.0;  // 30%
        gpuInfo.memoryTotal = 10ULL * 1024 * 1024 * 1024;  // 10 GB
        gpuInfo.memoryUsed = 3ULL * 1024 * 1024 * 1024;    // 3 GB
        gpuInfo.temperature = 65.0;  // 65°C
        gpuInfo.powerUsage = 180.0;  // 180W
        gpuInfo.coreFrequency = 1500.0;  // 1.5 GHz
        gpuInfo.memoryFrequency = 9000.0;  // 9 GHz
        
        gpu.gpus.push_back(gpuInfo);
    }
    
    /**
     * @brief Verifica le soglie di utilizzo.
     * @param info Informazioni sulle prestazioni
     */
    void checkThresholds(const SystemPerformanceInfo& info) {
        std::lock_guard<std::mutex> lock(m_thresholdMutex);
        
        // Verifica le soglie della CPU
        for (const auto& [id, threshold] : m_cpuThresholds) {
            if (info.cpu.totalUsage > threshold.first) {
                threshold.second(info.cpu.totalUsage);
            }
        }
        
        // Verifica le soglie della memoria
        for (const auto& [id, threshold] : m_memoryThresholds) {
            double memoryUsage = info.memory.getPhysicalUsagePercentage();
            if (memoryUsage > threshold.first) {
                threshold.second(memoryUsage);
            }
        }
    }
    
    /**
     * @brief Notifica i sottoscrittori delle prestazioni.
     * @param info Informazioni sulle prestazioni
     */
    void notifyPerformanceSubscribers(const SystemPerformanceInfo& info) {
        std::unordered_map<int, std::function<void(const SystemPerformanceInfo&)>> callbacksCopy;
        
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            callbacksCopy = m_performanceCallbacks;
        }
        
        for (const auto& [id, callback] : callbacksCopy) {
            callback(info);
        }
    }
    
    bool m_initialized;
    MonitoringInterval m_interval;
    bool m_enableCPU;
    bool m_enableMemory;
    bool m_enableDisk;
    bool m_enableNetwork;
    bool m_enableGPU;
    
    SystemPerformanceInfo m_currentPerformance;
    std::mutex m_performanceMutex;
    
    std::unordered_map<int, std::function<void(const SystemPerformanceInfo&)>> m_performanceCallbacks;
    std::mutex m_callbackMutex;
    int m_nextCallbackId;
    
    // Soglie di utilizzo
    std::unordered_map<int, std::pair<double, std::function<void(double)>>> m_cpuThresholds;
    std::unordered_map<int, std::pair<double, std::function<void(double)>>> m_memoryThresholds;
    std::mutex m_thresholdMutex;
    int m_nextThresholdId;
    
    bool m_isMonitoringActive;
    std::thread m_monitoringThread;
    std::mutex m_mutex;
};

/**
 * @brief Macro per registrare le statistiche di prestazione attuali.
 */
#define MONITOR_PERFORMANCE_SNAPSHOT() \
    do { \
        auto perfInfo = Core::Monitoring::PerformanceMonitor::getInstance().getCurrentPerformance(); \
        Core::Monitoring::MonitoringEvent event( \
            Core::Monitoring::MonitoringEventType::PERFORMANCE, \
            Core::Monitoring::MonitoringSeverity::INFO, \
            "PerformanceSnapshot", \
            "Current system performance snapshot" \
        ); \
        event.addNumericData("cpu_usage", perfInfo.cpu.totalUsage); \
        event.addNumericData("memory_physical_usage", perfInfo.memory.getPhysicalUsagePercentage()); \
        event.addNumericData("memory_virtual_usage", perfInfo.memory.getVirtualUsagePercentage()); \
        if (!perfInfo.gpu.gpus.empty()) { \
            event.addNumericData("gpu_usage", perfInfo.gpu.gpus[0].usage); \
            event.addNumericData("gpu_memory_usage", perfInfo.gpu.gpus[0].getMemoryUsagePercentage()); \
        } \
        Core::Monitoring::MonitoringSystem::getInstance().logEvent( \
            event.type, \
            event.severity, \
            event.name, \
            event.description, \
            __FILE__, \
            __LINE__, \
            __FUNCTION__ \
        ); \
    } while (0)

} // namespace Monitoring
} // namespace Core 