#include "SystemMonitor.h"

#ifdef _WIN32
#include <windows.h>
#else
// Implementazione per altri sistemi operativi (es. Linux)
#endif

namespace CoreNS {

SystemMonitor::SystemMonitor() {
#ifdef _WIN32
    m_previousTotalTicks = 0;
    m_previousIdleTicks = 0;
#endif
}

SystemMonitor::~SystemMonitor() = default;

SystemResources SystemMonitor::getSystemResourceUsage() {
    SystemResources resources;
    resources.cpuUsagePercent = getCpuUsage();
    getMemoryUsage(resources.availableMemoryBytes, resources.totalMemoryBytes);
    // TODO: Implementare le altre metriche
    resources.availableDiskBytes = 0;
    resources.totalDiskBytes = 0;
    resources.networkUsagePercent = 0;
    resources.gpuUsagePercent = 0;
    return resources;
}

double SystemMonitor::getCpuUsage() {
#ifdef _WIN32
    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return -1.0; // Errore
    }

    ULARGE_INTEGER idle, kernel, user;
    idle.LowPart = idleTime.dwLowDateTime;
    idle.HighPart = idleTime.dwHighDateTime;
    kernel.LowPart = kernelTime.dwLowDateTime;
    kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart = userTime.dwLowDateTime;
    user.HighPart = userTime.dwHighDateTime;

    unsigned long long totalTicks = kernel.QuadPart + user.QuadPart;
    unsigned long long idleTicks = idle.QuadPart;

    unsigned long long totalTicksSinceLastTime = totalTicks - m_previousTotalTicks;
    unsigned long long idleTicksSinceLastTime = idleTicks - m_previousIdleTicks;

    double ret = 1.0 - (totalTicksSinceLastTime > 0 ? ((double)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

    m_previousTotalTicks = totalTicks;
    m_previousIdleTicks = idleTicks;

    return ret * 100.0;
#else
    return 0.0; // Non implementato per questo OS
#endif
}

void SystemMonitor::getMemoryUsage(double& available, double& total) {
#ifdef _WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    available = (double)statex.ullAvailPhys;
    total = (double)statex.ullTotalPhys;
#else
    available = 0;
    total = 0;
#endif
}

} // namespace CoreNS