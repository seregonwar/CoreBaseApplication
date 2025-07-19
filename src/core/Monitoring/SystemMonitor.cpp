#include "SystemMonitor.h"
#include <chrono>

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
    getDiskUsage(resources.availableDiskBytes, resources.totalDiskBytes);
    resources.networkUsagePercent = getNetworkUsage();
    resources.gpuUsagePercent = getGpuUsage();
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

void SystemMonitor::getDiskUsage(double& available, double& total) {
#ifdef _WIN32
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
    if (GetDiskFreeSpaceExW(L"C:\\", &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        available = (double)freeBytesAvailable.QuadPart;
        total = (double)totalNumberOfBytes.QuadPart;
    } else {
        available = 0;
        total = 0;
    }
#else
    // Implementazione per Linux/macOS usando statvfs
    available = 0;
    total = 0;
#endif
}

double SystemMonitor::getNetworkUsage() {
#ifdef _WIN32
    // Implementazione semplificata - in un'implementazione reale
    // si dovrebbe monitorare il traffico di rete nel tempo
    static double lastBytesReceived = 0;
    static double lastBytesSent = 0;
    static auto lastTime = std::chrono::steady_clock::now();
    
    // Per ora restituiamo un valore simulato
    // In una implementazione reale si userebbe GetIfTable2 o WMI
    return 0.0;
#else
    return 0.0;
#endif
}

double SystemMonitor::getGpuUsage() {
#ifdef _WIN32
    // Implementazione semplificata - in un'implementazione reale
    // si dovrebbe usare NVML per NVIDIA, ADL per AMD, o WMI
    // Per ora restituiamo un valore simulato
    return 0.0;
#else
    return 0.0;
#endif
}

} // namespace CoreNS