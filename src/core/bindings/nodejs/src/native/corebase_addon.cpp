#include <napi.h>
#include "error_handler_wrapper.h"
#include "config_manager_wrapper.h"
#include "system_monitor_wrapper.h"
#include "network_manager_wrapper.h"

/**
 * @file corebase_addon.cpp
 * @brief Main entry point for CoreBase Node.js addon
 * 
 * This file initializes all the CoreBase functionality for Node.js,
 * providing cross-platform support for Windows, macOS, and Linux.
 */

namespace CoreBaseNodeJS {

/**
 * Initialize the CoreBase addon
 * @param env N-API environment
 * @param info Function call info
 * @return Promise that resolves when initialization is complete
 */
Napi::Value Initialize(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    // Initialize core components
    // This would typically initialize the C++ CoreBase library
    
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("Initialization failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

/**
 * Shutdown the CoreBase addon
 * @param env N-API environment
 * @param info Function call info
 * @return Promise that resolves when shutdown is complete
 */
Napi::Value Shutdown(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    // Cleanup core components
    // This would typically cleanup the C++ CoreBase library
    
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("Shutdown failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

/**
 * Get platform information
 * @param env N-API environment
 * @param info Function call info
 * @return Object containing platform details
 */
Napi::Value GetPlatformInfo(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Object result = Napi::Object::New(env);
  
#ifdef PLATFORM_WINDOWS
  result.Set("platform", "windows");
  result.Set("name", "Windows");
#elif defined(PLATFORM_MACOS)
  result.Set("platform", "macos");
  result.Set("name", "macOS");
#elif defined(PLATFORM_LINUX)
  result.Set("platform", "linux");
  result.Set("name", "Linux");
#else
  result.Set("platform", "unknown");
  result.Set("name", "Unknown");
#endif
  
  result.Set("version", "1.0.0");
  result.Set("architecture", sizeof(void*) == 8 ? "x64" : "x86");
  
  return result;
}

/**
 * Get addon version
 * @param env N-API environment
 * @param info Function call info
 * @return String containing version
 */
Napi::Value GetVersion(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::String::New(env, "1.0.0");
}

} // namespace CoreBaseNodeJS

/**
 * Initialize the Node.js addon
 * @param env N-API environment
 * @param exports Module exports object
 * @return Exports object with all bindings
 */
Napi::Object Init(Napi::Env env, Napi::Object exports) {
  // Core functions
  exports.Set("initialize", Napi::Function::New(env, CoreBaseNodeJS::Initialize));
  exports.Set("shutdown", Napi::Function::New(env, CoreBaseNodeJS::Shutdown));
  exports.Set("getPlatformInfo", Napi::Function::New(env, CoreBaseNodeJS::GetPlatformInfo));
  exports.Set("getVersion", Napi::Function::New(env, CoreBaseNodeJS::GetVersion));
  
  // Initialize component wrappers
  ErrorHandlerWrapper::Init(env, exports);
  ConfigManagerWrapper::Init(env, exports);
  SystemMonitorWrapper::Init(env, exports);
  NetworkManagerWrapper::Init(env, exports);
  
  return exports;
}

// Register the addon
NODE_API_MODULE(corebase_nodejs, Init)