#pragma once

#include "../CoreClass/SystemResources.h"

namespace CoreNS {

class SystemMonitor {
public:
    SystemMonitor();
    ~SystemMonitor();

    SystemResources getSystemResourceUsage();

private:
    // Metodi privati per ottenere le singole metriche
    double getCpuUsage();
    void getMemoryUsage(double& available, double& total);
    void getDiskUsage(double& available, double& total);
    double getNetworkUsage();
    double getGpuUsage();

    // Membri per il calcolo dell'utilizzo della CPU
#ifdef _WIN32
    unsigned long long m_previousTotalTicks;
    unsigned long long m_previousIdleTicks;
#endif
};

} // namespace CoreNS