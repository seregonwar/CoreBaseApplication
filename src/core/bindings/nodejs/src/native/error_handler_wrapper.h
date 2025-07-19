#pragma once

#include <napi.h>
#include "CoreClass/ErrorHandler.h"

/**
 * @file error_handler_wrapper.h
 * @brief Node.js wrapper for CoreBase ErrorHandler
 * 
 * Provides Node.js bindings for error handling and logging functionality
 * with cross-platform support.
 */

class ErrorHandlerWrapper : public Napi::ObjectWrap<ErrorHandlerWrapper> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Value CreateNew(const Napi::CallbackInfo& info);
  
  ErrorHandlerWrapper(const Napi::CallbackInfo& info);
  ~ErrorHandlerWrapper();

private:
  // Instance methods
  Napi::Value Initialize(const Napi::CallbackInfo& info);
  Napi::Value Shutdown(const Napi::CallbackInfo& info);
  Napi::Value SetLogLevel(const Napi::CallbackInfo& info);
  Napi::Value GetLogLevel(const Napi::CallbackInfo& info);
  Napi::Value LogError(const Napi::CallbackInfo& info);
  Napi::Value LogWarning(const Napi::CallbackInfo& info);
  Napi::Value LogInfo(const Napi::CallbackInfo& info);
  Napi::Value LogDebug(const Napi::CallbackInfo& info);
  Napi::Value HandleError(const Napi::CallbackInfo& info);
  Napi::Value HandleException(const Napi::CallbackInfo& info);
  
  // Static methods
  static Napi::Value GetLogLevelConstants(const Napi::CallbackInfo& info);
  
  // Member variables
  std::unique_ptr<ErrorHandler> errorHandler_;
  
  // Helper methods
  static LogLevel JSToLogLevel(const Napi::Value& value);
  static Napi::Value LogLevelToJS(Napi::Env env, LogLevel level);
};