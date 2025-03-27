#pragma once

#include "../MonitoringSystem.h"
#include "../PerformanceMonitor.h"
#include "../MemoryTracker.h"
#include "../EventTracker.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <memory>
#include <chrono>
#include <functional>
#include <deque>

namespace Core {
namespace Monitoring {
namespace GUI {

/**
 * @brief Struttura per memorizzare i dati di una serie temporale.
 */
struct TimeSeriesData {
    std::deque<std::pair<std::chrono::steady_clock::time_point, double>> points;
    std::string label;
    float min;
    float max;
    int color;
    bool visible;
    size_t maxPoints;
    
    /**
     * @brief Costruttore.
     * @param lbl Etichetta della serie
     * @param minVal Valore minimo
     * @param maxVal Valore massimo
     * @param col Colore (formato RGBA)
     * @param maxPts Numero massimo di punti da memorizzare
     */
    TimeSeriesData(const std::string& lbl, float minVal, float maxVal, int col, size_t maxPts = 100)
        : label(lbl), min(minVal), max(maxVal), color(col), visible(true), maxPoints(maxPts) {}
    
    /**
     * @brief Aggiunge un punto alla serie.
     * @param value Valore del punto
     */
    void addPoint(double value) {
        auto now = std::chrono::steady_clock::now();
        points.emplace_back(now, value);
        
        // Limita il numero di punti
        while (points.size() > maxPoints) {
            points.pop_front();
        }
        
        // Adatta min e max se necessario
        if (value < min) min = static_cast<float>(value);
        if (value > max) max = static_cast<float>(value);
    }
    
    /**
     * @brief Pulisce tutti i punti.
     */
    void clear() {
        points.clear();
    }
};

/**
 * @brief Struttura per memorizzare informazioni sulle funzioni più lente.
 */
struct SlowFunctionInfo {
    std::string name;           ///< Nome della funzione
    std::string module;         ///< Nome del modulo
    double totalTimeMs;         ///< Tempo totale in millisecondi
    double avgTimeMs;           ///< Tempo medio in millisecondi
    uint64_t callCount;         ///< Numero di chiamate
    
    /**
     * @brief Costruttore.
     * @param n Nome della funzione
     * @param mod Nome del modulo
     * @param total Tempo totale in millisecondi
     * @param avg Tempo medio in millisecondi
     * @param calls Numero di chiamate
     */
    SlowFunctionInfo(const std::string& n, const std::string& mod, 
                     double total, double avg, uint64_t calls)
        : name(n), module(mod), totalTimeMs(total), avgTimeMs(avg), callCount(calls) {}
};

/**
 * @brief Enum per i tipi di widget della dashboard.
 */
enum class DashboardWidgetType {
    CPU_USAGE,              ///< Utilizzo CPU
    MEMORY_USAGE,           ///< Utilizzo memoria
    DISK_USAGE,             ///< Utilizzo disco
    NETWORK_USAGE,          ///< Utilizzo rete
    GPU_USAGE,              ///< Utilizzo GPU
    PERFORMANCE_CHART,      ///< Grafico prestazioni
    MEMORY_CHART,           ///< Grafico memoria
    EVENT_LOG,              ///< Log eventi
    FUNCTION_STATS,         ///< Statistiche funzioni
    MEMORY_ALLOCATIONS,     ///< Allocazioni memoria
    CUSTOM                  ///< Widget personalizzato
};

/**
 * @brief Interfaccia base per un widget della dashboard.
 */
class IDashboardWidget {
public:
    /**
     * @brief Distruttore virtuale.
     */
    virtual ~IDashboardWidget() = default;
    
    /**
     * @brief Inizializza il widget.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Termina il widget.
     * @return true se la terminazione è avvenuta con successo, false altrimenti
     */
    virtual bool shutdown() = 0;
    
    /**
     * @brief Aggiorna il widget.
     * @param deltaTime Tempo passato dall'ultimo aggiornamento (in secondi)
     */
    virtual void update(float deltaTime) = 0;
    
    /**
     * @brief Disegna il widget.
     */
    virtual void render() = 0;
    
    /**
     * @brief Ottiene il nome del widget.
     * @return Nome del widget
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Ottiene il tipo del widget.
     * @return Tipo del widget
     */
    virtual DashboardWidgetType getType() const = 0;
    
    /**
     * @brief Imposta se il widget è visibile.
     * @param visible Se true, il widget è visibile
     */
    void setVisible(bool visible) {
        m_visible = visible;
    }
    
    /**
     * @brief Verifica se il widget è visibile.
     * @return true se il widget è visibile, false altrimenti
     */
    bool isVisible() const {
        return m_visible;
    }
    
protected:
    bool m_visible = true;    ///< Se il widget è visibile
};

/**
 * @brief Widget per mostrare il monitor delle prestazioni.
 */
class PerformanceWidget : public IDashboardWidget {
public:
    /**
     * @brief Costruttore.
     * @param name Nome del widget
     */
    explicit PerformanceWidget(const std::string& name = "Performance Monitor")
        : m_name(name), m_initialized(false), m_updateInterval(1.0f) {}
    
    /**
     * @brief Inizializza il widget.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize() override {
        if (m_initialized) return true;
        
        // Crea le serie temporali
        m_cpuSeries = std::make_shared<TimeSeriesData>("CPU Usage", 0.0f, 100.0f, 0xFF0000FF, 100);
        m_memorySeries = std::make_shared<TimeSeriesData>("Memory Usage", 0.0f, 100.0f, 0xFF00FF00, 100);
        m_gpuSeries = std::make_shared<TimeSeriesData>("GPU Usage", 0.0f, 100.0f, 0xFFFF0000, 100);
        
        // Aggiungi le serie
        m_series.push_back(m_cpuSeries);
        m_series.push_back(m_memorySeries);
        m_series.push_back(m_gpuSeries);
        
        m_initialized = true;
        return true;
    }
    
    /**
     * @brief Termina il widget.
     * @return true se la terminazione è avvenuta con successo, false altrimenti
     */
    bool shutdown() override {
        if (!m_initialized) return true;
        
        // Pulisci le serie
        for (auto& series : m_series) {
            series->clear();
        }
        
        m_series.clear();
        
        m_initialized = false;
        return true;
    }
    
    /**
     * @brief Aggiorna il widget.
     * @param deltaTime Tempo passato dall'ultimo aggiornamento (in secondi)
     */
    void update(float deltaTime) override {
        if (!m_initialized) return;
        
        // Accumula il tempo
        m_timeSinceLastUpdate += deltaTime;
        
        // Aggiorna le serie temporali ogni intervallo
        if (m_timeSinceLastUpdate >= m_updateInterval) {
            // Ottieni le informazioni sulle prestazioni
            auto perfInfo = PerformanceMonitor::getInstance().getCurrentPerformance();
            
            // Aggiungi i punti alle serie
            m_cpuSeries->addPoint(perfInfo.cpu.totalUsage);
            m_memorySeries->addPoint(perfInfo.memory.getPhysicalUsagePercentage());
            
            if (!perfInfo.gpu.gpus.empty()) {
                m_gpuSeries->addPoint(perfInfo.gpu.gpus[0].usage);
            }
            
            m_timeSinceLastUpdate = 0.0f;
        }
    }
    
    /**
     * @brief Disegna il widget.
     */
    void render() override {
        if (!m_initialized || !m_visible) return;
        
        // Qui verrebbe implementato il codice per disegnare il widget con ImGui
        // Esempio (pseudo-codice):
        /*
        if (ImGui::Begin(m_name.c_str())) {
            // Disegna le CPU core come barre
            ImGui::Text("CPU Usage: %.1f%%", m_cpuSeries->points.back().second);
            ImGui::ProgressBar(m_cpuSeries->points.back().second / 100.0f);
            
            // Disegna la memoria come barra
            ImGui::Text("Memory Usage: %.1f%%", m_memorySeries->points.back().second);
            ImGui::ProgressBar(m_memorySeries->points.back().second / 100.0f);
            
            // Disegna la GPU come barra
            if (!m_gpuSeries->points.empty()) {
                ImGui::Text("GPU Usage: %.1f%%", m_gpuSeries->points.back().second);
                ImGui::ProgressBar(m_gpuSeries->points.back().second / 100.0f);
            }
            
            // Disegna i grafici
            if (ImGui::BeginTabBar("PerformanceCharts")) {
                if (ImGui::BeginTabItem("CPU")) {
                    // Disegna il grafico CPU
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Memory")) {
                    // Disegna il grafico memoria
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("GPU")) {
                    // Disegna il grafico GPU
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
        */
    }
    
    /**
     * @brief Ottiene il nome del widget.
     * @return Nome del widget
     */
    std::string getName() const override {
        return m_name;
    }
    
    /**
     * @brief Ottiene il tipo del widget.
     * @return Tipo del widget
     */
    DashboardWidgetType getType() const override {
        return DashboardWidgetType::PERFORMANCE_CHART;
    }
    
    /**
     * @brief Imposta l'intervallo di aggiornamento.
     * @param interval Intervallo in secondi
     */
    void setUpdateInterval(float interval) {
        m_updateInterval = interval;
    }
    
    /**
     * @brief Ottiene l'intervallo di aggiornamento.
     * @return Intervallo in secondi
     */
    float getUpdateInterval() const {
        return m_updateInterval;
    }

private:
    std::string m_name;                           ///< Nome del widget
    bool m_initialized;                           ///< Se il widget è inizializzato
    float m_updateInterval;                       ///< Intervallo di aggiornamento
    float m_timeSinceLastUpdate = 0.0f;           ///< Tempo dall'ultimo aggiornamento
    
    std::vector<std::shared_ptr<TimeSeriesData>> m_series;  ///< Serie temporali
    std::shared_ptr<TimeSeriesData> m_cpuSeries;            ///< Serie CPU
    std::shared_ptr<TimeSeriesData> m_memorySeries;         ///< Serie memoria
    std::shared_ptr<TimeSeriesData> m_gpuSeries;            ///< Serie GPU
};

/**
 * @brief Widget per mostrare il log degli eventi.
 */
class EventLogWidget : public IDashboardWidget {
public:
    /**
     * @brief Costruttore.
     * @param name Nome del widget
     */
    explicit EventLogWidget(const std::string& name = "Event Log")
        : m_name(name), m_initialized(false), m_maxEvents(1000), 
          m_selectedEvent(MonitoringEventType::SYSTEM, MonitoringSeverity::INFO, "Empty", "No event selected") {}
    
    /**
     * @brief Inizializza il widget.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize() override {
        if (m_initialized) return true;
        
        // Registra un callback per i nuovi eventi
        m_callbackId = MonitoringSystem::getInstance().registerEventCallback(
            [this](const MonitoringEvent& event) {
                this->onNewEvent(event);
            }
        );
        
        m_initialized = true;
        return true;
    }
    
    /**
     * @brief Termina il widget.
     * @return true se la terminazione è avvenuta con successo, false altrimenti
     */
    bool shutdown() override {
        if (!m_initialized) return true;
        
        // Annulla la registrazione del callback
        MonitoringSystem::getInstance().unregisterEventCallback(m_callbackId);
        
        // Pulisci gli eventi
        m_events.clear();
        
        m_initialized = false;
        return true;
    }
    
    /**
     * @brief Aggiorna il widget.
     * @param deltaTime Tempo passato dall'ultimo aggiornamento (in secondi)
     */
    void update(float deltaTime) override {
        // Non c'è bisogno di un aggiornamento periodico,
        // gli eventi vengono aggiunti tramite callback
    }
    
    /**
     * @brief Disegna il widget.
     */
    void render() override {
        if (!m_initialized || !m_visible) return;
        
        // Qui verrebbe implementato il codice per disegnare il widget con ImGui
        // Esempio (pseudo-codice):
        /*
        if (ImGui::Begin(m_name.c_str())) {
            // Controlli per il filtro
            static ImGuiTextFilter filter;
            filter.Draw("Filter", -100.0f);
            
            // Controlli per il tipo di evento
            static bool showSystem = true, showPerformance = true, showMemory = true;
            ImGui::Checkbox("System", &showSystem);
            ImGui::SameLine();
            ImGui::Checkbox("Performance", &showPerformance);
            ImGui::SameLine();
            ImGui::Checkbox("Memory", &showMemory);
            
            // Pulsante per pulire
            ImGui::SameLine();
            if (ImGui::Button("Clear")) {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_events.clear();
            }
            
            // Lista eventi
            ImGui::BeginChild("EventList", ImVec2(0, 0), true);
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                for (const auto& event : m_events) {
                    // Filtra per tipo
                    if ((!showSystem && event.type == MonitoringEventType::SYSTEM) ||
                        (!showPerformance && event.type == MonitoringEventType::PERFORMANCE) ||
                        (!showMemory && event.type == MonitoringEventType::MEMORY)) {
                        continue;
                    }
                    
                    // Filtra per testo
                    std::string eventText = event.toString();
                    if (!filter.PassFilter(eventText.c_str())) {
                        continue;
                    }
                    
                    // Colora in base alla severità
                    ImVec4 color;
                    switch (event.severity) {
                        case MonitoringSeverity::CRITICAL:
                        case MonitoringSeverity::ERROR:
                            color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                            break;
                        case MonitoringSeverity::WARNING:
                            color = ImVec4(1.0f, 0.7f, 0.0f, 1.0f);
                            break;
                        case MonitoringSeverity::INFO:
                            color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                            break;
                        default:
                            color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
                            break;
                    }
                    
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::TextWrapped("%s", eventText.c_str());
                    ImGui::PopStyleColor();
                    
                    // Se si fa click su una riga, mostra i dettagli
                    if (ImGui::IsItemClicked()) {
                        m_selectedEvent = event;
                        ImGui::OpenPopup("EventDetails");
                    }
                }
            }
            ImGui::EndChild();
            
            // Popup per i dettagli dell'evento
            if (ImGui::BeginPopup("EventDetails")) {
                ImGui::Text("Event Details");
                ImGui::Separator();
                ImGui::Text("Name: %s", m_selectedEvent.name.c_str());
                ImGui::Text("Type: %s", toString(m_selectedEvent.type).c_str());
                ImGui::Text("Severity: %s", toString(m_selectedEvent.severity).c_str());
                ImGui::Text("Description: %s", m_selectedEvent.description.c_str());
                
                if (!m_selectedEvent.file.empty()) {
                    ImGui::Text("File: %s:%d", m_selectedEvent.file.c_str(), m_selectedEvent.line);
                }
                
                if (!m_selectedEvent.function.empty()) {
                    ImGui::Text("Function: %s", m_selectedEvent.function.c_str());
                }
                
                // Mostra i dati numerici
                if (!m_selectedEvent.getNumericData().empty()) {
                    ImGui::Separator();
                    ImGui::Text("Numeric Data:");
                    for (const auto& [key, value] : m_selectedEvent.getNumericData()) {
                        ImGui::Text("%s: %f", key.c_str(), value);
                    }
                }
                
                // Mostra i dati stringa
                if (!m_selectedEvent.getStringData().empty()) {
                    ImGui::Separator();
                    ImGui::Text("String Data:");
                    for (const auto& [key, value] : m_selectedEvent.getStringData()) {
                        ImGui::Text("%s: %s", key.c_str(), value.c_str());
                    }
                }
                
                ImGui::EndPopup();
            }
        }
        ImGui::End();
        */
    }
    
    /**
     * @brief Ottiene il nome del widget.
     * @return Nome del widget
     */
    std::string getName() const override {
        return m_name;
    }
    
    /**
     * @brief Ottiene il tipo del widget.
     * @return Tipo del widget
     */
    DashboardWidgetType getType() const override {
        return DashboardWidgetType::EVENT_LOG;
    }
    
    /**
     * @brief Imposta il numero massimo di eventi da memorizzare.
     * @param maxEvents Numero massimo di eventi
     */
    void setMaxEvents(size_t maxEvents) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxEvents = maxEvents;
        
        // Riduci gli eventi se necessario
        while (m_events.size() > m_maxEvents) {
            m_events.pop_front();
        }
    }
    
    /**
     * @brief Ottiene il numero massimo di eventi da memorizzare.
     * @return Numero massimo di eventi
     */
    size_t getMaxEvents() const {
        return m_maxEvents;
    }
    
    /**
     * @brief Pulisce tutti gli eventi.
     */
    void clearEvents() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_events.clear();
    }

private:
    /**
     * @brief Callback per i nuovi eventi.
     * @param event Nuovo evento
     */
    void onNewEvent(const MonitoringEvent& event) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Aggiungi l'evento
        m_events.push_back(event);
        
        // Limita il numero di eventi
        while (m_events.size() > m_maxEvents) {
            m_events.pop_front();
        }
    }
    
    std::string m_name;                   ///< Nome del widget
    bool m_initialized;                   ///< Se il widget è inizializzato
    size_t m_maxEvents;                   ///< Numero massimo di eventi
    std::deque<MonitoringEvent> m_events; ///< Eventi
    int m_callbackId;                     ///< ID del callback
    MonitoringEvent m_selectedEvent;      ///< Evento selezionato
    std::mutex m_mutex;                   ///< Mutex per accesso thread-safe
};

/**
 * @brief Widget per mostrare le statistiche delle funzioni.
 */
class FunctionStatsWidget : public IDashboardWidget {
public:
    /**
     * @brief Costruttore.
     * @param name Nome del widget
     */
    explicit FunctionStatsWidget(const std::string& name = "Function Statistics")
        : m_name(name), m_initialized(false), m_updateInterval(1.0f) {}
    
    /**
     * @brief Inizializza il widget.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize() override {
        if (m_initialized) return true;
        
        m_initialized = true;
        return true;
    }
    
    /**
     * @brief Termina il widget.
     * @return true se la terminazione è avvenuta con successo, false altrimenti
     */
    bool shutdown() override {
        if (!m_initialized) return true;
        
        m_initialized = false;
        return true;
    }
    
    /**
     * @brief Aggiorna il widget.
     * @param deltaTime Tempo passato dall'ultimo aggiornamento (in secondi)
     */
    void update(float deltaTime) override {
        if (!m_initialized) return;
        
        // Accumula il tempo
        m_timeSinceLastUpdate += deltaTime;
        
        // Aggiorna le statistiche ogni intervallo
        if (m_timeSinceLastUpdate >= m_updateInterval) {
            updateFunctionStats();
            m_timeSinceLastUpdate = 0.0f;
        }
    }
    
    /**
     * @brief Disegna il widget.
     */
    void render() override {
        if (!m_initialized || !m_visible) return;
        
        // Qui verrebbe implementato il codice per disegnare il widget con ImGui
        // Esempio (pseudo-codice):
        /*
        if (ImGui::Begin(m_name.c_str())) {
            // Pulsante per aggiornare manualmente
            if (ImGui::Button("Refresh")) {
                updateFunctionStats();
            }
            
            // Controlli per ordinamento
            static int sortIndex = 0;
            const char* sortOptions[] = { "Total Time", "Average Time", "Call Count" };
            ImGui::SameLine();
            ImGui::Combo("Sort By", &sortIndex, sortOptions, IM_ARRAYSIZE(sortOptions));
            
            // Ordina le funzioni
            std::vector<SlowFunctionInfo> sortedFunctions;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                sortedFunctions = m_slowFunctions;
            }
            
            switch (sortIndex) {
                case 0: // Total Time
                    std::sort(sortedFunctions.begin(), sortedFunctions.end(),
                              [](const SlowFunctionInfo& a, const SlowFunctionInfo& b) {
                                  return a.totalTimeMs > b.totalTimeMs;
                              });
                    break;
                case 1: // Average Time
                    std::sort(sortedFunctions.begin(), sortedFunctions.end(),
                              [](const SlowFunctionInfo& a, const SlowFunctionInfo& b) {
                                  return a.avgTimeMs > b.avgTimeMs;
                              });
                    break;
                case 2: // Call Count
                    std::sort(sortedFunctions.begin(), sortedFunctions.end(),
                              [](const SlowFunctionInfo& a, const SlowFunctionInfo& b) {
                                  return a.callCount > b.callCount;
                              });
                    break;
            }
            
            // Tabella delle funzioni
            if (ImGui::BeginTable("FunctionStats", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Module", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Total Time (ms)", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Avg Time (ms)", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Calls", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();
                
                for (const auto& func : sortedFunctions) {
                    ImGui::TableNextRow();
                    
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(func.name.c_str());
                    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(func.module.c_str());
                    
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.3f", func.totalTimeMs);
                    
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.3f", func.avgTimeMs);
                    
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%llu", func.callCount);
                }
                
                ImGui::EndTable();
            }
        }
        ImGui::End();
        */
    }
    
    /**
     * @brief Ottiene il nome del widget.
     * @return Nome del widget
     */
    std::string getName() const override {
        return m_name;
    }
    
    /**
     * @brief Ottiene il tipo del widget.
     * @return Tipo del widget
     */
    DashboardWidgetType getType() const override {
        return DashboardWidgetType::FUNCTION_STATS;
    }
    
    /**
     * @brief Imposta l'intervallo di aggiornamento.
     * @param interval Intervallo in secondi
     */
    void setUpdateInterval(float interval) {
        m_updateInterval = interval;
    }
    
    /**
     * @brief Ottiene l'intervallo di aggiornamento.
     * @return Intervallo in secondi
     */
    float getUpdateInterval() const {
        return m_updateInterval;
    }

private:
    /**
     * @brief Aggiorna le statistiche delle funzioni.
     */
    void updateFunctionStats() {
        // Ottieni le statistiche delle funzioni
        auto stats = EventTracker::getInstance().getFunctionStats();
        
        std::lock_guard<std::mutex> lock(m_mutex);
        m_slowFunctions.clear();
        
        // Converti in SlowFunctionInfo
        for (const auto& [key, funcStats] : stats) {
            // Ignora le funzioni con poche chiamate
            if (funcStats.callCount < 5) continue;
            
            // Aggiungi alla lista
            m_slowFunctions.emplace_back(
                funcStats.functionName,
                funcStats.module,
                funcStats.totalDurationMicros / 1000.0,
                funcStats.getAverageDurationMicros() / 1000.0,
                funcStats.callCount
            );
        }
        
        // Limita a 100 funzioni
        if (m_slowFunctions.size() > 100) {
            // Ordina per tempo totale
            std::sort(m_slowFunctions.begin(), m_slowFunctions.end(),
                     [](const SlowFunctionInfo& a, const SlowFunctionInfo& b) {
                         return a.totalTimeMs > b.totalTimeMs;
                     });
            
            // Mantieni solo le prime 100
            m_slowFunctions.resize(100);
        }
    }
    
    std::string m_name;                   ///< Nome del widget
    bool m_initialized;                   ///< Se il widget è inizializzato
    float m_updateInterval;               ///< Intervallo di aggiornamento
    float m_timeSinceLastUpdate = 0.0f;   ///< Tempo dall'ultimo aggiornamento
    
    std::vector<SlowFunctionInfo> m_slowFunctions;  ///< Funzioni più lente
    std::mutex m_mutex;                   ///< Mutex per accesso thread-safe
};

/**
 * @brief Classe principale della dashboard di monitoraggio.
 */
class MonitoringDashboard {
public:
    /**
     * @brief Costruttore.
     * @param title Titolo della dashboard
     */
    explicit MonitoringDashboard(const std::string& title = "Monitoring Dashboard")
        : m_title(title), m_initialized(false) {}
    
    /**
     * @brief Distruttore.
     */
    ~MonitoringDashboard() {
        if (m_initialized) {
            shutdown();
        }
    }
    
    /**
     * @brief Inizializza la dashboard.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize() {
        if (m_initialized) return true;
        
        // Qui verrebbe inizializzato ImGui
        
        // Aggiungi widget predefiniti
        auto perfWidget = std::make_shared<PerformanceWidget>();
        addWidget(perfWidget);
        
        auto eventLogWidget = std::make_shared<EventLogWidget>();
        addWidget(eventLogWidget);
        
        auto funcStatsWidget = std::make_shared<FunctionStatsWidget>();
        addWidget(funcStatsWidget);
        
        m_initialized = true;
        return true;
    }
    
    /**
     * @brief Termina la dashboard.
     * @return true se la terminazione è avvenuta con successo, false altrimenti
     */
    bool shutdown() {
        if (!m_initialized) return true;
        
        // Termina tutti i widget
        for (auto& widget : m_widgets) {
            widget->shutdown();
        }
        
        // Pulisci i widget
        m_widgets.clear();
        
        // Qui verrebbe terminato ImGui
        
        m_initialized = false;
        return true;
    }
    
    /**
     * @brief Aggiorna la dashboard.
     * @param deltaTime Tempo passato dall'ultimo aggiornamento (in secondi)
     */
    void update(float deltaTime) {
        if (!m_initialized) return;
        
        // Aggiorna tutti i widget
        for (auto& widget : m_widgets) {
            if (widget->isVisible()) {
                widget->update(deltaTime);
            }
        }
    }
    
    /**
     * @brief Disegna la dashboard.
     */
    void render() {
        if (!m_initialized) return;
        
        // Qui verrebbe iniziato un nuovo frame ImGui
        
        // Disegna la finestra principale
        /*
        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1200, 800), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(m_title.c_str())) {
            // Menu principale
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Save Layout", "Ctrl+S")) {
                        // TODO: Salva layout
                    }
                    if (ImGui::MenuItem("Load Layout", "Ctrl+L")) {
                        // TODO: Carica layout
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit", "Alt+F4")) {
                        // TODO: Chiudi
                    }
                    ImGui::EndMenu();
                }
                
                if (ImGui::BeginMenu("View")) {
                    for (auto& widget : m_widgets) {
                        bool visible = widget->isVisible();
                        if (ImGui::MenuItem(widget->getName().c_str(), nullptr, &visible)) {
                            widget->setVisible(visible);
                        }
                    }
                    ImGui::EndMenu();
                }
                
                ImGui::EndMainMenuBar();
            }
            
            // Layout a dock
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
            ImGuiID dockspace_id = ImGui::GetID("DashboardDockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            
            // Disegna tutti i widget
            for (auto& widget : m_widgets) {
                if (widget->isVisible()) {
                    widget->render();
                }
            }
        }
        ImGui::End();
        
        // Qui verrebbe renderizzato ImGui
        */
    }
    
    /**
     * @brief Aggiunge un widget alla dashboard.
     * @param widget Widget da aggiungere
     * @return true se l'aggiunta è avvenuta con successo, false altrimenti
     */
    bool addWidget(std::shared_ptr<IDashboardWidget> widget) {
        if (!widget) return false;
        
        // Inizializza il widget
        if (!widget->initialize()) {
            return false;
        }
        
        // Aggiungi il widget
        m_widgets.push_back(widget);
        
        return true;
    }
    
    /**
     * @brief Rimuove un widget dalla dashboard.
     * @param name Nome del widget da rimuovere
     * @return true se la rimozione è avvenuta con successo, false altrimenti
     */
    bool removeWidget(const std::string& name) {
        // Cerca il widget
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
                              [&name](const std::shared_ptr<IDashboardWidget>& widget) {
                                  return widget->getName() == name;
                              });
        
        if (it == m_widgets.end()) {
            return false;
        }
        
        // Termina il widget
        (*it)->shutdown();
        
        // Rimuovi il widget
        m_widgets.erase(it);
        
        return true;
    }
    
    /**
     * @brief Ottiene un widget per nome.
     * @param name Nome del widget
     * @return Puntatore al widget, o nullptr se non trovato
     */
    std::shared_ptr<IDashboardWidget> getWidget(const std::string& name) {
        // Cerca il widget
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
                              [&name](const std::shared_ptr<IDashboardWidget>& widget) {
                                  return widget->getName() == name;
                              });
        
        if (it == m_widgets.end()) {
            return nullptr;
        }
        
        return *it;
    }
    
    /**
     * @brief Ottiene un widget per tipo.
     * @param type Tipo del widget
     * @return Puntatore al widget, o nullptr se non trovato
     */
    std::shared_ptr<IDashboardWidget> getWidgetByType(DashboardWidgetType type) {
        // Cerca il widget
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
                              [type](const std::shared_ptr<IDashboardWidget>& widget) {
                                  return widget->getType() == type;
                              });
        
        if (it == m_widgets.end()) {
            return nullptr;
        }
        
        return *it;
    }

private:
    std::string m_title;                           ///< Titolo della dashboard
    bool m_initialized;                            ///< Se la dashboard è inizializzata
    std::vector<std::shared_ptr<IDashboardWidget>> m_widgets;  ///< Widget
};

} // namespace GUI
} // namespace Monitoring
} // namespace Core 