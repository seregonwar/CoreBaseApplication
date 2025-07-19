#include "system_monitor_wrapper.h"
#include <stdexcept>

/**
 * @file system_monitor_wrapper.cpp
 * @brief Implementation of Node.js wrapper for CoreBase SystemMonitor
 */

Napi::Object SystemMonitorWrapper::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "SystemMonitor", {
    InstanceMethod("initialize", &SystemMonitorWrapper::Initialize),
    InstanceMethod("shutdown", &SystemMonitorWrapper::Shutdown),
    InstanceMethod("startMonitoring", &SystemMonitorWrapper::StartMonitoring),
    InstanceMethod("stopMonitoring", &SystemMonitorWrapper::StopMonitoring),
    InstanceMethod("isMonitoring", &SystemMonitorWrapper::IsMonitoring),
    InstanceMethod("getSystemInfo", &SystemMonitorWrapper::GetSystemInfo),
    InstanceMethod("getCpuUsage", &SystemMonitorWrapper::GetCpuUsage),
    InstanceMethod("getMemoryUsage", &SystemMonitorWrapper::GetMemoryUsage),
    InstanceMethod("getDiskUsage", &SystemMonitorWrapper::GetDiskUsage),
    InstanceMethod("getNetworkUsage", &SystemMonitorWrapper::GetNetworkUsage),
    InstanceMethod("getProcessInfo", &SystemMonitorWrapper::GetProcessInfo),
    InstanceMethod("getSystemLoad", &SystemMonitorWrapper::GetSystemLoad),
    InstanceMethod("getTemperature", &SystemMonitorWrapper::GetTemperature),
    InstanceMethod("getPerformanceMetrics", &SystemMonitorWrapper::GetPerformanceMetrics),
    InstanceMethod("getHistoricalData", &SystemMonitorWrapper::GetHistoricalData),
    InstanceMethod("clearHistory", &SystemMonitorWrapper::ClearHistory),
    InstanceMethod("setMonitoringInterval", &SystemMonitorWrapper::SetMonitoringInterval),
    InstanceMethod("getMonitoringInterval", &SystemMonitorWrapper::GetMonitoringInterval),
    InstanceMethod("setThresholds", &SystemMonitorWrapper::SetThresholds),
    InstanceMethod("getThresholds", &SystemMonitorWrapper::GetThresholds),
    InstanceMethod("enableAlerts", &SystemMonitorWrapper::EnableAlerts),
    InstanceMethod("disableAlerts", &SystemMonitorWrapper::DisableAlerts),
    InstanceMethod("getAlerts", &SystemMonitorWrapper::GetAlerts),
    InstanceMethod("clearAlerts", &SystemMonitorWrapper::ClearAlerts),
    InstanceMethod("exportData", &SystemMonitorWrapper::ExportData),
    InstanceMethod("generateReport", &SystemMonitorWrapper::GenerateReport),
    StaticMethod("createNew", &SystemMonitorWrapper::CreateNew)
  });
  
  exports.Set("SystemMonitor", func);
  return exports;
}

Napi::Value SystemMonitorWrapper::CreateNew(const Napi::CallbackInfo& info) {
  return info.Env().Undefined();
}

SystemMonitorWrapper::SystemMonitorWrapper(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<SystemMonitorWrapper>(info) {
  try {
    systemMonitor_ = std::make_unique<SystemMonitor>();
  } catch (const std::exception& e) {
    Napi::TypeError::New(info.Env(), std::string("Failed to create SystemMonitor: ") + e.what())
        .ThrowAsJavaScriptException();
  }
}

SystemMonitorWrapper::~SystemMonitorWrapper() {
  if (systemMonitor_) {
    try {
      systemMonitor_->shutdown();
    } catch (...) {
      // Ignore exceptions in destructor
    }
  }
}

Napi::Value SystemMonitorWrapper::Initialize(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    bool result = systemMonitor_->initialize();
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("Initialize failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::Shutdown(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    return env.Undefined();
  }
  
  try {
    systemMonitor_->shutdown();
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("Shutdown failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::StartMonitoring(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    bool result = systemMonitor_->startMonitoring();
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("StartMonitoring failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::StopMonitoring(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    systemMonitor_->stopMonitoring();
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("StopMonitoring failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::IsMonitoring(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    bool result = systemMonitor_->isMonitoring();
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("IsMonitoring failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetSystemInfo(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    SystemInfo sysInfo = systemMonitor_->getSystemInfo();
    return SystemInfoToJS(env, sysInfo);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetSystemInfo failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetCpuUsage(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    CpuInfo cpuInfo = systemMonitor_->getCpuUsage();
    return CpuInfoToJS(env, cpuInfo);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetCpuUsage failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetMemoryUsage(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    MemoryInfo memInfo = systemMonitor_->getMemoryUsage();
    return MemoryInfoToJS(env, memInfo);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetMemoryUsage failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetDiskUsage(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string path = "/";
    if (info.Length() > 0 && info[0].IsString()) {
      path = info[0].As<Napi::String>().Utf8Value();
    }
    
    DiskInfo diskInfo = systemMonitor_->getDiskUsage(path);
    return DiskInfoToJS(env, diskInfo);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetDiskUsage failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetNetworkUsage(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    NetworkInfo netInfo = systemMonitor_->getNetworkUsage();
    return NetworkInfoToJS(env, netInfo);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetNetworkUsage failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetProcessInfo(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    int pid = -1;
    if (info.Length() > 0 && info[0].IsNumber()) {
      pid = info[0].As<Napi::Number>().Int32Value();
    }
    
    ProcessInfo procInfo = systemMonitor_->getProcessInfo(pid);
    return ProcessInfoToJS(env, procInfo);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetProcessInfo failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetSystemLoad(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    double load = systemMonitor_->getSystemLoad();
    return Napi::Number::New(env, load);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetSystemLoad failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetTemperature(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    double temp = systemMonitor_->getTemperature();
    return Napi::Number::New(env, temp);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetTemperature failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetPerformanceMetrics(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    PerformanceMetrics metrics = systemMonitor_->getPerformanceMetrics();
    return PerformanceMetricsToJS(env, metrics);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetPerformanceMetrics failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetHistoricalData(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    int hours = 24;
    if (info.Length() > 0 && info[0].IsNumber()) {
      hours = info[0].As<Napi::Number>().Int32Value();
    }
    
    std::vector<PerformanceMetrics> history = systemMonitor_->getHistoricalData(hours);
    Napi::Array result = Napi::Array::New(env, history.size());
    
    for (size_t i = 0; i < history.size(); ++i) {
      result[i] = PerformanceMetricsToJS(env, history[i]);
    }
    
    return result;
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetHistoricalData failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::ClearHistory(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    systemMonitor_->clearHistory();
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("ClearHistory failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::SetMonitoringInterval(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsNumber()) {
    Napi::TypeError::New(env, "Expected interval in milliseconds")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    int interval = info[0].As<Napi::Number>().Int32Value();
    systemMonitor_->setMonitoringInterval(interval);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("SetMonitoringInterval failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetMonitoringInterval(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    int interval = systemMonitor_->getMonitoringInterval();
    return Napi::Number::New(env, interval);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetMonitoringInterval failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::SetThresholds(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsObject()) {
    Napi::TypeError::New(env, "Expected thresholds object")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    MonitoringThresholds thresholds = JSToThresholds(info[0]);
    systemMonitor_->setThresholds(thresholds);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("SetThresholds failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetThresholds(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    MonitoringThresholds thresholds = systemMonitor_->getThresholds();
    Napi::Object result = Napi::Object::New(env);
    result.Set("cpuThreshold", Napi::Number::New(env, thresholds.cpuThreshold));
    result.Set("memoryThreshold", Napi::Number::New(env, thresholds.memoryThreshold));
    result.Set("diskThreshold", Napi::Number::New(env, thresholds.diskThreshold));
    result.Set("temperatureThreshold", Napi::Number::New(env, thresholds.temperatureThreshold));
    return result;
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetThresholds failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::EnableAlerts(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    systemMonitor_->enableAlerts();
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("EnableAlerts failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::DisableAlerts(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    systemMonitor_->disableAlerts();
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("DisableAlerts failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GetAlerts(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::vector<std::string> alerts = systemMonitor_->getAlerts();
    Napi::Array result = Napi::Array::New(env, alerts.size());
    
    for (size_t i = 0; i < alerts.size(); ++i) {
      result[i] = Napi::String::New(env, alerts[i]);
    }
    
    return result;
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetAlerts failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::ClearAlerts(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    systemMonitor_->clearAlerts();
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("ClearAlerts failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::ExportData(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected file path")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string filePath = info[0].As<Napi::String>().Utf8Value();
    std::string format = "json";
    if (info.Length() > 1 && info[1].IsString()) {
      format = info[1].As<Napi::String>().Utf8Value();
    }
    
    bool result = systemMonitor_->exportData(filePath, format);
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("ExportData failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value SystemMonitorWrapper::GenerateReport(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!systemMonitor_) {
    Napi::TypeError::New(env, "SystemMonitor not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string reportType = "summary";
    if (info.Length() > 0 && info[0].IsString()) {
      reportType = info[0].As<Napi::String>().Utf8Value();
    }
    
    std::string report = systemMonitor_->generateReport(reportType);
    return Napi::String::New(env, report);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GenerateReport failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

// Helper methods
Napi::Value SystemMonitorWrapper::SystemInfoToJS(Napi::Env env, const SystemInfo& info) {
  Napi::Object result = Napi::Object::New(env);
  result.Set("osName", Napi::String::New(env, info.osName));
  result.Set("osVersion", Napi::String::New(env, info.osVersion));
  result.Set("architecture", Napi::String::New(env, info.architecture));
  result.Set("hostname", Napi::String::New(env, info.hostname));
  result.Set("totalMemory", Napi::Number::New(env, info.totalMemory));
  result.Set("cpuCores", Napi::Number::New(env, info.cpuCores));
  result.Set("cpuModel", Napi::String::New(env, info.cpuModel));
  return result;
}

Napi::Value SystemMonitorWrapper::CpuInfoToJS(Napi::Env env, const CpuInfo& info) {
  Napi::Object result = Napi::Object::New(env);
  result.Set("usage", Napi::Number::New(env, info.usage));
  result.Set("cores", Napi::Number::New(env, info.cores));
  result.Set("frequency", Napi::Number::New(env, info.frequency));
  result.Set("temperature", Napi::Number::New(env, info.temperature));
  return result;
}

Napi::Value SystemMonitorWrapper::MemoryInfoToJS(Napi::Env env, const MemoryInfo& info) {
  Napi::Object result = Napi::Object::New(env);
  result.Set("total", Napi::Number::New(env, info.total));
  result.Set("used", Napi::Number::New(env, info.used));
  result.Set("free", Napi::Number::New(env, info.free));
  result.Set("available", Napi::Number::New(env, info.available));
  result.Set("usage", Napi::Number::New(env, info.usage));
  return result;
}

Napi::Value SystemMonitorWrapper::DiskInfoToJS(Napi::Env env, const DiskInfo& info) {
  Napi::Object result = Napi::Object::New(env);
  result.Set("total", Napi::Number::New(env, info.total));
  result.Set("used", Napi::Number::New(env, info.used));
  result.Set("free", Napi::Number::New(env, info.free));
  result.Set("usage", Napi::Number::New(env, info.usage));
  result.Set("path", Napi::String::New(env, info.path));
  return result;
}

Napi::Value SystemMonitorWrapper::NetworkInfoToJS(Napi::Env env, const NetworkInfo& info) {
  Napi::Object result = Napi::Object::New(env);
  result.Set("bytesReceived", Napi::Number::New(env, info.bytesReceived));
  result.Set("bytesSent", Napi::Number::New(env, info.bytesSent));
  result.Set("packetsReceived", Napi::Number::New(env, info.packetsReceived));
  result.Set("packetsSent", Napi::Number::New(env, info.packetsSent));
  result.Set("downloadSpeed", Napi::Number::New(env, info.downloadSpeed));
  result.Set("uploadSpeed", Napi::Number::New(env, info.uploadSpeed));
  return result;
}

Napi::Value SystemMonitorWrapper::ProcessInfoToJS(Napi::Env env, const ProcessInfo& info) {
  Napi::Object result = Napi::Object::New(env);
  result.Set("pid", Napi::Number::New(env, info.pid));
  result.Set("name", Napi::String::New(env, info.name));
  result.Set("cpuUsage", Napi::Number::New(env, info.cpuUsage));
  result.Set("memoryUsage", Napi::Number::New(env, info.memoryUsage));
  result.Set("status", Napi::String::New(env, info.status));
  return result;
}

Napi::Value SystemMonitorWrapper::PerformanceMetricsToJS(Napi::Env env, const PerformanceMetrics& metrics) {
  Napi::Object result = Napi::Object::New(env);
  result.Set("timestamp", Napi::Number::New(env, metrics.timestamp));
  result.Set("cpuUsage", Napi::Number::New(env, metrics.cpuUsage));
  result.Set("memoryUsage", Napi::Number::New(env, metrics.memoryUsage));
  result.Set("diskUsage", Napi::Number::New(env, metrics.diskUsage));
  result.Set("networkUsage", Napi::Number::New(env, metrics.networkUsage));
  result.Set("temperature", Napi::Number::New(env, metrics.temperature));
  result.Set("systemLoad", Napi::Number::New(env, metrics.systemLoad));
  return result;
}

MonitoringThresholds SystemMonitorWrapper::JSToThresholds(const Napi::Value& value) {
  MonitoringThresholds thresholds;
  
  if (value.IsObject()) {
    Napi::Object obj = value.As<Napi::Object>();
    
    if (obj.Has("cpuThreshold") && obj.Get("cpuThreshold").IsNumber()) {
      thresholds.cpuThreshold = obj.Get("cpuThreshold").As<Napi::Number>().DoubleValue();
    }
    
    if (obj.Has("memoryThreshold") && obj.Get("memoryThreshold").IsNumber()) {
      thresholds.memoryThreshold = obj.Get("memoryThreshold").As<Napi::Number>().DoubleValue();
    }
    
    if (obj.Has("diskThreshold") && obj.Get("diskThreshold").IsNumber()) {
      thresholds.diskThreshold = obj.Get("diskThreshold").As<Napi::Number>().DoubleValue();
    }
    
    if (obj.Has("temperatureThreshold") && obj.Get("temperatureThreshold").IsNumber()) {
      thresholds.temperatureThreshold = obj.Get("temperatureThreshold").As<Napi::Number>().DoubleValue();
    }
  }
  
  return thresholds;
}