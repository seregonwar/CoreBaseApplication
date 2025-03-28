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

ResourceManager::ResourceManager() : m_stopMonitoring(false), m_errorHandler(nullptr) {
    // L'ErrorHandler verrà inizializzato durante initialize()
}

ResourceManager::~ResourceManager() {
    shutdown();
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
            // TODO: Implement actual resource monitoring
            m_currentResources.cpuUsagePercent = 0.0;
            m_currentResources.availableMemoryBytes = 0.0;
            m_currentResources.totalMemoryBytes = 0.0;
            m_currentResources.availableDiskBytes = 0.0;
            m_currentResources.totalDiskBytes = 0.0;
            m_currentResources.networkUsagePercent = 0.0;
            m_currentResources.gpuUsagePercent = 0.0;
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

} // namespace CoreNS 