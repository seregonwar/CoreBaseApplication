#pragma once
#include <napi.h>
#include "../../../../SystemMonitor.h"
#include <memory>

/**
 * @file system_monitor_wrapper.h
 * @brief Node.js wrapper for CoreBase SystemMonitor
 */

class SystemMonitorWrapper : public Napi::ObjectWrap<SystemMonitorWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::Value CreateNew(const Napi::CallbackInfo& info);
    
    SystemMonitorWrapper(const Napi::CallbackInfo& info);
    virtual ~SystemMonitorWrapper();

private:
    // Core monitoring methods
    Napi::Value Initialize(const Napi::CallbackInfo& info);
    Napi::Value Shutdown(const Napi::CallbackInfo& info);
    Napi::Value StartMonitoring(const Napi::CallbackInfo& info);
    Napi::Value StopMonitoring(const Napi::CallbackInfo& info);
    Napi::Value IsMonitoring(const Napi::CallbackInfo& info);
    
    // System information methods
    Napi::Value GetSystemInfo(const Napi::CallbackInfo& info);
    Napi::Value GetCpuUsage(const Napi::CallbackInfo& info);
    Napi::Value GetMemoryUsage(const Napi::CallbackInfo& info);
    Napi::Value GetDiskUsage(const Napi::CallbackInfo& info);
    Napi::Value GetNetworkUsage(const Napi::CallbackInfo& info);
    Napi::Value GetProcessInfo(const Napi::CallbackInfo& info);
    Napi::Value GetSystemLoad(const Napi::CallbackInfo& info);
    Napi::Value GetTemperature(const Napi::CallbackInfo& info);
    
    // Performance metrics
    Napi::Value GetPerformanceMetrics(const Napi::CallbackInfo& info);
    Napi::Value GetHistoricalData(const Napi::CallbackInfo& info);
    Napi::Value ClearHistory(const Napi::CallbackInfo& info);
    
    // Configuration methods
    Napi::Value SetMonitoringInterval(const Napi::CallbackInfo& info);
    Napi::Value GetMonitoringInterval(const Napi::CallbackInfo& info);
    Napi::Value SetThresholds(const Napi::CallbackInfo& info);
    Napi::Value GetThresholds(const Napi::CallbackInfo& info);
    
    // Alert methods
    Napi::Value EnableAlerts(const Napi::CallbackInfo& info);
    Napi::Value DisableAlerts(const Napi::CallbackInfo& info);
    Napi::Value GetAlerts(const Napi::CallbackInfo& info);
    Napi::Value ClearAlerts(const Napi::CallbackInfo& info);
    
    // Export methods
    Napi::Value ExportData(const Napi::CallbackInfo& info);
    Napi::Value GenerateReport(const Napi::CallbackInfo& info);
    
    // Helper methods
    static Napi::Value SystemInfoToJS(Napi::Env env, const SystemInfo& info);
    static Napi::Value CpuInfoToJS(Napi::Env env, const CpuInfo& info);
    static Napi::Value MemoryInfoToJS(Napi::Env env, const MemoryInfo& info);
    static Napi::Value DiskInfoToJS(Napi::Env env, const DiskInfo& info);
    static Napi::Value NetworkInfoToJS(Napi::Env env, const NetworkInfo& info);
    static Napi::Value ProcessInfoToJS(Napi::Env env, const ProcessInfo& info);
    static Napi::Value PerformanceMetricsToJS(Napi::Env env, const PerformanceMetrics& metrics);
    static MonitoringThresholds JSToThresholds(const Napi::Value& value);
    
    std::unique_ptr<SystemMonitor> systemMonitor_;
};