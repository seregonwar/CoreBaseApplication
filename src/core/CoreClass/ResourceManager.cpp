#include "ResourceManager.h"
#include "SystemResources.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>
#include <functional>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <pdh.h>
#pragma comment(lib, "pdh.lib")
#elif defined(__linux__)
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <fstream>
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <sys/mount.h>
#endif

namespace CoreNS {

ResourceManager::ResourceManager() : m_stopMonitoring(false), m_errorHandler(nullptr), m_nextCallbackId(0) {
    // L'ErrorHandler verrà inizializzato durante initialize()
    
    #ifdef _WIN32
    // Inizializzazione contatori PDH per CPU
    PdhOpenQuery(NULL, 0, &m_cpuQuery);
    PdhAddCounterA(m_cpuQuery, "\\Processor(_Total)\\% Processor Time", 0, &m_cpuTotal);
    PdhCollectQueryData(m_cpuQuery);
    #endif
}

ResourceManager::~ResourceManager() {
    shutdown();
    
    #ifdef _WIN32
    // Chiusura contatori PDH
    PdhCloseQuery(m_cpuQuery);
    #endif
}

bool ResourceManager::initialize() {
    if (!m_stopMonitoring) {
        m_monitoringThread = std::thread(&ResourceManager::updateResources, this);
    }
    return true;
}

void ResourceManager::shutdown() {
    m_stopMonitoring = true;
    if (m_monitoringThread.joinable()) {
        m_monitoringThread.join();
    }
}

double ResourceManager::getCpuUsage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentResources.cpuUsagePercent;
}

double ResourceManager::getMemoryUsage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentResources.availableMemoryBytes;
}

double ResourceManager::getDiskUsage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentResources.availableDiskBytes;
}

double ResourceManager::getNetworkUsage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentResources.networkUsagePercent;
}

double ResourceManager::getGpuUsage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentResources.gpuUsagePercent;
}

SystemResources ResourceManager::getSystemResources() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentResources;
}

void ResourceManager::registerCallback(const ResourceCallback& callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    int id = ++m_nextCallbackId;
    m_callbacks[id] = std::make_pair("", callback);
}

void ResourceManager::unregisterCallback(const ResourceCallback& callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto it = m_callbacks.begin(); it != m_callbacks.end(); ) {
        if (&(it->second.second) == &callback) {
            it = m_callbacks.erase(it);
        } else {
            ++it;
        }
    }
}

void ResourceManager::updateResources() {
    while (!m_stopMonitoring) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            #ifdef _WIN32
            // Aggiorna CPU Usage
            PDH_FMT_COUNTERVALUE counterVal;
            PdhCollectQueryData(m_cpuQuery);
            PdhGetFormattedCounterValue(m_cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
            m_currentResources.cpuUsagePercent = counterVal.doubleValue;
            
            // Aggiorna Memory Usage
            MEMORYSTATUSEX memInfo;
            memInfo.dwLength = sizeof(MEMORYSTATUSEX);
            GlobalMemoryStatusEx(&memInfo);
            m_currentResources.totalMemoryBytes = static_cast<double>(memInfo.ullTotalPhys);
            m_currentResources.availableMemoryBytes = static_cast<double>(memInfo.ullAvailPhys);
            
            // Aggiorna Disk Usage
            ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
            GetDiskFreeSpaceExA("C:\\", &freeBytesAvailable, &totalBytes, &totalFreeBytes);
            m_currentResources.totalDiskBytes = static_cast<double>(totalBytes.QuadPart);
            m_currentResources.availableDiskBytes = static_cast<double>(totalFreeBytes.QuadPart);
            
            // Network e GPU restano a 0 per ora (richiedono librerie aggiuntive)
            m_currentResources.networkUsagePercent = 0.0;
            m_currentResources.gpuUsagePercent = 0.0;
            #else
            // Implementazione di default per altri OS o non implementato
            m_currentResources.cpuUsagePercent = 0.0;
            m_currentResources.availableMemoryBytes = 0.0;
            m_currentResources.totalMemoryBytes = 0.0;
            m_currentResources.availableDiskBytes = 0.0;
            m_currentResources.totalDiskBytes = 0.0;
            m_currentResources.networkUsagePercent = 0.0;
            m_currentResources.gpuUsagePercent = 0.0;
            #endif
        }

        checkThresholds();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void ResourceManager::checkThresholds() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& [id, callbackPair] : m_callbacks) {
        callbackPair.second(m_currentResources);
    }
}

uint64_t ResourceManager::getAvailableMemory() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<uint64_t>(m_currentResources.availableMemoryBytes);
}

} // namespace CoreNS 