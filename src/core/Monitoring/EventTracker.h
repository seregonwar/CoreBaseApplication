#pragma once

#include "MonitoringSystem.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <memory>
#include <chrono>
#include <atomic>
#include <stack>
#include <thread>
#include <functional>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace Core {
namespace Monitoring {

/**
 * @brief Informazioni relative a un evento di funzione.
 */
struct FunctionEvent {
    std::string functionName;                 ///< Nome della funzione
    std::string file;                         ///< File dove la funzione è definita
    int line;                                 ///< Linea nel file
    std::chrono::steady_clock::time_point startTime;   ///< Tempo di inizio
    std::chrono::steady_clock::time_point endTime;     ///< Tempo di fine
    std::thread::id threadId;                 ///< ID del thread
    std::string args;                         ///< Argomenti (opzionali)
    std::string result;                       ///< Risultato (opzionale)
    int64_t eventId;                          ///< ID univoco dell'evento
    int64_t parentEventId;                    ///< ID dell'evento padre (chiamante)
    int depth;                                ///< Profondità nello stack delle chiamate
    std::string module;                       ///< Nome del modulo
    
    /**
     * @brief Costruttore di default.
     */
    FunctionEvent()
        : line(0), eventId(0), parentEventId(-1), depth(0) {}
    
    /**
     * @brief Costruttore.
     * @param name Nome della funzione
     * @param fileName File dove la funzione è definita
     * @param lineNum Linea nel file
     * @param threadId ID del thread
     * @param moduleStr Nome del modulo
     */
    FunctionEvent(const std::string& name, const std::string& fileName, int lineNum,
                 std::thread::id threadId, const std::string& moduleStr = "")
        : functionName(name), file(fileName), line(lineNum),
          startTime(std::chrono::steady_clock::now()), endTime(),
          threadId(threadId), eventId(0), parentEventId(-1),
          depth(0), module(moduleStr) {}
    
    /**
     * @brief Calcola la durata dell'evento in microsecondi.
     * @return Durata in microsecondi
     */
    int64_t getDurationMicros() const {
        if (endTime > startTime) {
            return std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        }
        return 0;
    }
    
    /**
     * @brief Calcola la durata dell'evento in millisecondi.
     * @return Durata in millisecondi
     */
    double getDurationMillis() const {
        return getDurationMicros() / 1000.0;
    }
    
    /**
     * @brief Rappresentazione dell'evento come stringa.
     * @return Stringa rappresentante l'evento
     */
    std::string toString() const {
        std::ostringstream oss;
        oss << "[" << std::fixed << std::setprecision(3) << getDurationMillis() << "ms] ";
        
        // Aggiungi indentazione in base alla profondità
        for (int i = 0; i < depth; ++i) {
            oss << "  ";
        }
        
        oss << functionName;
        
        if (!args.empty()) {
            oss << "(" << args << ")";
        }
        
        if (!file.empty()) {
            oss << " [" << file << ":" << line << "]";
        }
        
        if (!result.empty()) {
            oss << " -> " << result;
        }
        
        return oss.str();
    }
};

/**
 * @brief Statistiche per una funzione.
 */
struct FunctionStats {
    std::string functionName;             ///< Nome della funzione
    std::string module;                   ///< Nome del modulo
    uint64_t callCount;                   ///< Numero di chiamate
    int64_t totalDurationMicros;          ///< Durata totale in microsecondi
    int64_t minDurationMicros;            ///< Durata minima in microsecondi
    int64_t maxDurationMicros;            ///< Durata massima in microsecondi
    int64_t lastCallDurationMicros;       ///< Durata dell'ultima chiamata in microsecondi
    
    /**
     * @brief Costruttore.
     * @param name Nome della funzione
     * @param mod Nome del modulo
     */
    FunctionStats(const std::string& name, const std::string& mod = "")
        : functionName(name), module(mod), callCount(0),
          totalDurationMicros(0), minDurationMicros(INT64_MAX),
          maxDurationMicros(0), lastCallDurationMicros(0) {}
    
    /**
     * @brief Ottiene la durata media in microsecondi.
     * @return Durata media in microsecondi
     */
    double getAverageDurationMicros() const {
        return (callCount > 0) ? (static_cast<double>(totalDurationMicros) / callCount) : 0.0;
    }
    
    /**
     * @brief Aggiorna le statistiche con un nuovo evento.
     * @param duration Durata dell'evento in microsecondi
     */
    void update(int64_t duration) {
        callCount++;
        totalDurationMicros += duration;
        minDurationMicros = std::min(minDurationMicros, duration);
        maxDurationMicros = std::max(maxDurationMicros, duration);
        lastCallDurationMicros = duration;
    }
    
    /**
     * @brief Rappresentazione delle statistiche come stringa.
     * @return Stringa rappresentante le statistiche
     */
    std::string toString() const {
        std::ostringstream oss;
        oss << functionName;
        
        if (!module.empty()) {
            oss << " [" << module << "]";
        }
        
        oss << ": calls=" << callCount
            << ", total=" << std::fixed << std::setprecision(3) << (totalDurationMicros / 1000.0) << "ms"
            << ", avg=" << std::fixed << std::setprecision(3) << (getAverageDurationMicros() / 1000.0) << "ms"
            << ", min=" << std::fixed << std::setprecision(3) << (minDurationMicros / 1000.0) << "ms"
            << ", max=" << std::fixed << std::setprecision(3) << (maxDurationMicros / 1000.0) << "ms";
        
        return oss.str();
    }
};

/**
 * @brief Rappresenta il contesto di tracciamento per un thread.
 */
class ThreadTrackingContext {
public:
    /**
     * @brief Costruttore.
     */
    ThreadTrackingContext()
        : m_currentDepth(0), m_nextEventId(0) {}
    
    /**
     * @brief Inizia il tracciamento di una funzione.
     * @param functionName Nome della funzione
     * @param file File dove la funzione è definita
     * @param line Linea nel file
     * @param module Nome del modulo
     * @param args Argomenti (opzionali)
     * @return ID dell'evento
     */
    int64_t beginFunction(const std::string& functionName, const std::string& file,
                         int line, const std::string& module, const std::string& args = "") {
        std::thread::id threadId = std::this_thread::get_id();
        
        FunctionEvent event(functionName, file, line, threadId, module);
        event.args = args;
        event.depth = m_currentDepth;
        event.eventId = m_nextEventId++;
        
        if (!m_callStack.empty()) {
            event.parentEventId = m_callStack.top();
        }
        
        m_callStack.push(event.eventId);
        m_currentDepth++;
        
        m_activeEvents[event.eventId] = event;
        
        return event.eventId;
    }
    
    /**
     * @brief Termina il tracciamento di una funzione.
     * @param eventId ID dell'evento
     * @param result Risultato (opzionale)
     * @return Evento completato
     */
    FunctionEvent endFunction(int64_t eventId, const std::string& result = "") {
        if (!m_callStack.empty()) {
            m_callStack.pop();
        }
        
        m_currentDepth = std::max(0, m_currentDepth - 1);
        
        auto it = m_activeEvents.find(eventId);
        if (it == m_activeEvents.end()) {
            // Evento non trovato, ritorna un evento vuoto
            return FunctionEvent("UNKNOWN", "", 0, std::this_thread::get_id());
        }
        
        FunctionEvent& event = it->second;
        event.endTime = std::chrono::steady_clock::now();
        event.result = result;
        
        FunctionEvent completedEvent = event;
        m_activeEvents.erase(it);
        
        return completedEvent;
    }
    
    /**
     * @brief Aggiunge un evento completato al contesto.
     * @param event Evento da aggiungere
     */
    void addCompletedEvent(const FunctionEvent& event) {
        m_completedEvents.push_back(event);
        
        // Mantieni al massimo 1000 eventi completati per thread
        if (m_completedEvents.size() > 1000) {
            m_completedEvents.erase(m_completedEvents.begin());
        }
    }
    
    /**
     * @brief Ottiene tutti gli eventi completati.
     * @return Vettore di eventi completati
     */
    std::vector<FunctionEvent> getCompletedEvents() const {
        return m_completedEvents;
    }
    
    /**
     * @brief Cancella gli eventi completati.
     */
    void clearCompletedEvents() {
        m_completedEvents.clear();
    }

private:
    int m_currentDepth;                               ///< Profondità corrente
    std::stack<int64_t> m_callStack;                  ///< Stack delle chiamate
    std::unordered_map<int64_t, FunctionEvent> m_activeEvents;  ///< Eventi attivi
    std::vector<FunctionEvent> m_completedEvents;     ///< Eventi completati
    int64_t m_nextEventId;                            ///< Prossimo ID evento
};

/**
 * @brief Tracciatore di eventi per monitorare le chiamate a funzioni.
 */
class EventTracker {
public:
    /**
     * @brief Ottiene l'istanza singleton del tracciatore di eventi.
     * @return Riferimento all'istanza singleton
     */
    static EventTracker& getInstance() {
        static EventTracker instance;
        return instance;
    }
    
    /**
     * @brief Inizializza il tracciatore di eventi.
     * @param enabled Se true, abilita il tracciamento
     * @param autoRegisterWithMonitoring Se true, registra automaticamente con il sistema di monitoraggio
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize(bool enabled = true, bool autoRegisterWithMonitoring = true) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_initialized) return true;
        
        m_enabled = enabled;
        
        if (autoRegisterWithMonitoring) {
            // Registra con il sistema di monitoraggio
            MonitoringSystem::getInstance().logEvent(
                MonitoringEventType::FUNCTION,
                MonitoringSeverity::INFO,
                "EventTrackerInitialized",
                "Event tracking initialized, " + 
                std::string(m_enabled ? "enabled" : "disabled")
            );
        }
        
        m_initialized = true;
        return true;
    }
    
    /**
     * @brief Termina il tracciatore di eventi.
     * @return true se la terminazione è avvenuta con successo, false altrimenti
     */
    bool shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) return true;
        
        // Crea un report finale
        createPerformanceReport();
        
        // Registra la terminazione
        MonitoringSystem::getInstance().logEvent(
            MonitoringEventType::FUNCTION,
            MonitoringSeverity::INFO,
            "EventTrackerShutdown",
            "Event tracking shutting down"
        );
        
        // Resetta le strutture dati
        m_functionStats.clear();
        m_threadContexts.clear();
        
        m_initialized = false;
        return true;
    }
    
    /**
     * @brief Abilita o disabilita il tracciamento.
     * @param enabled Se true, abilita il tracciamento
     */
    void setEnabled(bool enabled) {
        m_enabled = enabled;
    }
    
    /**
     * @brief Verifica se il tracciamento è abilitato.
     * @return true se il tracciamento è abilitato, false altrimenti
     */
    bool isEnabled() const {
        return m_enabled;
    }
    
    /**
     * @brief Inizia il tracciamento di una funzione.
     * @param functionName Nome della funzione
     * @param file File dove la funzione è definita
     * @param line Linea nel file
     * @param module Nome del modulo
     * @param args Argomenti (opzionali)
     * @return ID dell'evento
     */
    int64_t beginFunction(const std::string& functionName, const std::string& file,
                         int line, const std::string& module = "", const std::string& args = "") {
        if (!m_initialized || !m_enabled) {
            return -1;
        }
        
        std::thread::id threadId = std::this_thread::get_id();
        
        // Ottieni o crea il contesto per questo thread
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& context = m_threadContexts[threadId];
        
        return context.beginFunction(functionName, file, line, module, args);
    }
    
    /**
     * @brief Termina il tracciamento di una funzione.
     * @param eventId ID dell'evento
     * @param result Risultato (opzionale)
     */
    void endFunction(int64_t eventId, const std::string& result = "") {
        if (!m_initialized || !m_enabled || eventId < 0) {
            return;
        }
        
        std::thread::id threadId = std::this_thread::get_id();
        
        FunctionEvent completedEvent;
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_threadContexts.find(threadId);
            if (it == m_threadContexts.end()) {
                return;
            }
            
            ThreadTrackingContext& context = it->second;
            completedEvent = context.endFunction(eventId, result);
            context.addCompletedEvent(completedEvent);
            
            // Aggiorna le statistiche
            std::string key = completedEvent.functionName + ":" + completedEvent.module;
            auto statsIt = m_functionStats.find(key);
            if (statsIt == m_functionStats.end()) {
                m_functionStats[key] = FunctionStats(completedEvent.functionName, completedEvent.module);
            }
            
            m_functionStats[key].update(completedEvent.getDurationMicros());
        }
        
        // Registra l'evento nel sistema di monitoraggio se supera una certa soglia (ad es. 1ms)
        if (completedEvent.getDurationMillis() >= 1.0) {
            MonitoringEvent event(
                MonitoringEventType::FUNCTION,
                MonitoringSeverity::DEBUG,
                "FunctionExecution",
                "Function " + completedEvent.functionName + " executed in " + 
                std::to_string(completedEvent.getDurationMillis()) + "ms"
            );
            
            event.addNumericData("duration_ms", completedEvent.getDurationMillis());
            event.addStringData("function", completedEvent.functionName);
            event.addStringData("module", completedEvent.module);
            event.addStringData("file", completedEvent.file);
            event.addNumericData("line", completedEvent.line);
            
            if (!completedEvent.args.empty()) {
                event.addStringData("args", completedEvent.args);
            }
            
            if (!completedEvent.result.empty()) {
                event.addStringData("result", completedEvent.result);
            }
            
            MonitoringSystem::getInstance().logEvent(event);
        }
    }
    
    /**
     * @brief Ottiene le statistiche per tutte le funzioni.
     * @return Mappa di statistiche per funzione
     */
    std::unordered_map<std::string, FunctionStats> getFunctionStats() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_functionStats;
    }
    
    /**
     * @brief Crea un report delle prestazioni.
     * @param includeAll Se true, include tutte le funzioni, anche quelle con poche chiamate
     * @param minCallCount Numero minimo di chiamate per includere una funzione
     */
    void createPerformanceReport(bool includeAll = false, uint64_t minCallCount = 5) {
        if (!m_initialized) return;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Crea un report di base
        MonitoringEvent event(
            MonitoringEventType::FUNCTION,
            MonitoringSeverity::INFO,
            "FunctionPerformanceReport",
            "Function performance report"
        );
        
        // Ordina le funzioni per tempo totale
        struct FunctionStatWithKey {
            std::string key;
            FunctionStats stats;
        };
        
        std::vector<FunctionStatWithKey> sortedStats;
        for (const auto& [key, stats] : m_functionStats) {
            if (includeAll || stats.callCount >= minCallCount) {
                sortedStats.push_back({key, stats});
            }
        }
        
        std::sort(sortedStats.begin(), sortedStats.end(),
                 [](const FunctionStatWithKey& a, const FunctionStatWithKey& b) {
                     return a.stats.totalDurationMicros > b.stats.totalDurationMicros;
                 });
        
        // Formatta il report
        std::ostringstream reportSummary;
        reportSummary << "Top functions by total time:\n";
        
        size_t reportLimit = std::min(static_cast<size_t>(20), sortedStats.size());
        
        for (size_t i = 0; i < reportLimit; ++i) {
            const auto& item = sortedStats[i];
            reportSummary << (i + 1) << ". " << item.stats.toString() << "\n";
        }
        
        event.addStringData("summary", reportSummary.str());
        
        // Aggiungi le top 10 funzioni come dati numerici
        for (size_t i = 0; i < std::min(static_cast<size_t>(10), sortedStats.size()); ++i) {
            const auto& item = sortedStats[i];
            std::string prefix = "func" + std::to_string(i+1) + "_";
            
            event.addStringData(prefix + "name", item.stats.functionName);
            event.addStringData(prefix + "module", item.stats.module);
            event.addNumericData(prefix + "calls", item.stats.callCount);
            event.addNumericData(prefix + "total_ms", item.stats.totalDurationMicros / 1000.0);
            event.addNumericData(prefix + "avg_ms", item.stats.getAverageDurationMicros() / 1000.0);
        }
        
        // Invia il report al sistema di monitoraggio
        MonitoringSystem::getInstance().logEvent(event);
    }
    
    /**
     * @brief Registra una threshold per il tempo di esecuzione di una funzione.
     * @param functionName Nome della funzione
     * @param thresholdMs Soglia in millisecondi
     * @param callback Funzione da chiamare quando la soglia viene superata
     * @return ID della threshold
     */
    int setExecutionThreshold(const std::string& functionName, 
                             double thresholdMs, 
                             std::function<void(const std::string&, double)> callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        int id = m_nextThresholdId++;
        m_executionThresholds[id] = {functionName, thresholdMs * 1000.0, callback};
        
        return id;
    }
    
    /**
     * @brief Rimuove una threshold.
     * @param thresholdId ID della threshold
     * @return true se la threshold è stata rimossa, false altrimenti
     */
    bool removeExecutionThreshold(int thresholdId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_executionThresholds.find(thresholdId);
        if (it == m_executionThresholds.end()) {
            return false;
        }
        
        m_executionThresholds.erase(it);
        
        return true;
    }
    
    /**
     * @brief Ottiene la call tree per un thread specifico.
     * @param threadId ID del thread
     * @return Stringa rappresentante l'albero delle chiamate
     */
    std::string getCallTree(std::thread::id threadId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_threadContexts.find(threadId);
        if (it == m_threadContexts.end()) {
            return "No call tree available for thread";
        }
        
        const auto& events = it->second.getCompletedEvents();
        
        // Costruisci un albero delle chiamate
        std::ostringstream oss;
        oss << "Call tree for thread " << threadId << ":\n";
        
        for (const auto& event : events) {
            oss << event.toString() << "\n";
        }
        
        return oss.str();
    }
    
    /**
     * @brief Ottiene la call tree per il thread corrente.
     * @return Stringa rappresentante l'albero delle chiamate
     */
    std::string getCurrentThreadCallTree() {
        return getCallTree(std::this_thread::get_id());
    }
    
    /**
     * @brief Ottiene una lista di tutti i thread tracciati.
     * @return Vettore di ID thread
     */
    std::vector<std::thread::id> getTrackedThreads() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<std::thread::id> threads;
        for (const auto& [threadId, context] : m_threadContexts) {
            threads.push_back(threadId);
        }
        
        return threads;
    }
    
    /**
     * @brief Cancella la call tree per un thread specifico.
     * @param threadId ID del thread
     */
    void clearCallTree(std::thread::id threadId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_threadContexts.find(threadId);
        if (it != m_threadContexts.end()) {
            it->second.clearCompletedEvents();
        }
    }
    
    /**
     * @brief Cancella tutte le call tree.
     */
    void clearAllCallTrees() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto& [threadId, context] : m_threadContexts) {
            context.clearCompletedEvents();
        }
    }

private:
    // Costruttore privato per pattern Singleton
    EventTracker()
        : m_initialized(false), m_enabled(true), m_nextThresholdId(0) {}
    
    // Distruttore privato
    ~EventTracker() {
        if (m_initialized) {
            shutdown();
        }
    }
    
    // Disabilita copia e assegnazione
    EventTracker(const EventTracker&) = delete;
    EventTracker& operator=(const EventTracker&) = delete;
    
    bool m_initialized;
    std::atomic<bool> m_enabled;
    std::unordered_map<std::thread::id, ThreadTrackingContext> m_threadContexts;
    std::unordered_map<std::string, FunctionStats> m_functionStats;
    
    struct ExecutionThreshold {
        std::string functionName;
        double thresholdMicros;
        std::function<void(const std::string&, double)> callback;
    };
    
    std::unordered_map<int, ExecutionThreshold> m_executionThresholds;
    int m_nextThresholdId;
    
    std::mutex m_mutex;
};

/**
 * @brief Classe per il tracciamento automatico di una funzione.
 */
class ScopedFunctionTracer {
public:
    /**
     * @brief Costruttore.
     * @param functionName Nome della funzione
     * @param file File dove la funzione è definita
     * @param line Linea nel file
     * @param module Nome del modulo
     * @param args Argomenti (opzionali)
     */
    ScopedFunctionTracer(const std::string& functionName, const std::string& file,
                        int line, const std::string& module = "", const std::string& args = "")
        : m_functionName(functionName), m_eventId(-1), m_result("") {
        m_eventId = EventTracker::getInstance().beginFunction(functionName, file, line, module, args);
    }
    
    /**
     * @brief Distruttore.
     */
    ~ScopedFunctionTracer() {
        EventTracker::getInstance().endFunction(m_eventId, m_result);
    }
    
    /**
     * @brief Imposta il risultato della funzione.
     * @param result Risultato
     */
    void setResult(const std::string& result) {
        m_result = result;
    }

private:
    std::string m_functionName;
    int64_t m_eventId;
    std::string m_result;
};

/**
 * @brief Macro per tracciare una funzione.
 */
#define TRACK_FUNCTION() \
    Core::Monitoring::ScopedFunctionTracer _funcTracer(__FUNCTION__, __FILE__, __LINE__)

/**
 * @brief Macro per tracciare una funzione con modulo.
 * @param module Nome del modulo
 */
#define TRACK_FUNCTION_MODULE(module) \
    Core::Monitoring::ScopedFunctionTracer _funcTracer(__FUNCTION__, __FILE__, __LINE__, module)

/**
 * @brief Macro per tracciare una funzione con modulo e argomenti.
 * @param module Nome del modulo
 * @param args Argomenti
 */
#define TRACK_FUNCTION_FULL(module, args) \
    Core::Monitoring::ScopedFunctionTracer _funcTracer(__FUNCTION__, __FILE__, __LINE__, module, args)

/**
 * @brief Macro per tracciare un blocco di codice.
 * @param name Nome del blocco
 */
#define TRACK_BLOCK(name) \
    Core::Monitoring::ScopedFunctionTracer _blockTracer(name, __FILE__, __LINE__)

/**
 * @brief Macro per tracciare una sezione di codice con performance critica.
 * @param name Nome della sezione
 */
#define TRACK_PERFORMANCE_CRITICAL(name) \
    Core::Monitoring::ScopedFunctionTracer _perfTracer(std::string("PERF_CRITICAL:") + name, __FILE__, __LINE__)

/**
 * @brief Macro per impostare il risultato del tracciamento corrente.
 * @param result Risultato
 */
#define TRACK_RESULT(result) \
    _funcTracer.setResult(result)

} // namespace Monitoring
} // namespace Core 