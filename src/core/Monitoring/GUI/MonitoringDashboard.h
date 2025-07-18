#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <imgui.h>
#include "../../CoreAPI.h"
#include <deque>

namespace CoreNS {
namespace Monitoring {

class MonitoringDashboard {
public:
    MonitoringDashboard();
    ~MonitoringDashboard();

    void initialize();
    void run();
    void shutdown();

private:
    void updateResources();
    void render();
    void handleEvents();

    // --- UI State ---
    int m_selectedTab = 0; // 0: Monitoraggio, 1: CBA API
    std::deque<float> m_cpuHistory; // storico CPU
    std::deque<float> m_memHistory; // storico memoria
    static constexpr size_t HISTORY_SIZE = 120;
    std::vector<std::string> m_logLines; // log/output delle chiamate API
    std::string m_apiInputStr; // input generico per funzioni API
    std::string m_apiOutputStr; // output generico
    // ---

    std::thread m_updateThread;
    std::atomic<bool> m_running;
    CoreAPI m_api;
    // Risorse da mostrare
    float m_cpuUsage = 0.0f;
    float m_memoryMB = 0.0f;
};

} // namespace Monitoring
} // namespace CoreNS 