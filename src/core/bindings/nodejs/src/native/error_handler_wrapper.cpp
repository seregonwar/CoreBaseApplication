#include "error_handler_wrapper.h"
#include <stdexcept>

/**
 * @file error_handler_wrapper.cpp
 * @brief Implementation of Node.js wrapper for CoreBase ErrorHandler
 */

Napi::Object ErrorHandlerWrapper::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "ErrorHandler", {
    InstanceMethod("initialize", &ErrorHandlerWrapper::Initialize),
    InstanceMethod("shutdown", &ErrorHandlerWrapper::Shutdown),
    InstanceMethod("setLogLevel", &ErrorHandlerWrapper::SetLogLevel),
    InstanceMethod("getLogLevel", &ErrorHandlerWrapper::GetLogLevel),
    InstanceMethod("logError", &ErrorHandlerWrapper::LogError),
    InstanceMethod("logWarning", &ErrorHandlerWrapper::LogWarning),
    InstanceMethod("logInfo", &ErrorHandlerWrapper::LogInfo),
    InstanceMethod("logDebug", &ErrorHandlerWrapper::LogDebug),
    InstanceMethod("handleError", &ErrorHandlerWrapper::HandleError),
    InstanceMethod("handleException", &ErrorHandlerWrapper::HandleException),
    StaticMethod("getLogLevelConstants", &ErrorHandlerWrapper::GetLogLevelConstants),
    StaticMethod("createNew", &ErrorHandlerWrapper::CreateNew)
  });
  
  exports.Set("ErrorHandler", func);
  return exports;
}

Napi::Value ErrorHandlerWrapper::CreateNew(const Napi::CallbackInfo& info) {
  return info.Env().Undefined();
}

ErrorHandlerWrapper::ErrorHandlerWrapper(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<ErrorHandlerWrapper>(info) {
  try {
    errorHandler_ = std::make_unique<ErrorHandler>();
  } catch (const std::exception& e) {
    Napi::TypeError::New(info.Env(), std::string("Failed to create ErrorHandler: ") + e.what())
        .ThrowAsJavaScriptException();
  }
}

ErrorHandlerWrapper::~ErrorHandlerWrapper() {
  if (errorHandler_) {
    try {
      errorHandler_->shutdown();
    } catch (...) {
      // Ignore exceptions in destructor
    }
  }
}

Napi::Value ErrorHandlerWrapper::Initialize(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!errorHandler_) {
    Napi::TypeError::New(env, "ErrorHandler not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string logFile = "";
    if (info.Length() > 0 && info[0].IsString()) {
      logFile = info[0].As<Napi::String>().Utf8Value();
    }
    
    bool result = errorHandler_->initialize(logFile);
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("Initialize failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ErrorHandlerWrapper::Shutdown(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!errorHandler_) {
    return env.Undefined();
  }
  
  try {
    errorHandler_->shutdown();
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("Shutdown failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ErrorHandlerWrapper::SetLogLevel(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!errorHandler_) {
    Napi::TypeError::New(env, "ErrorHandler not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Expected log level argument")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    LogLevel level = JSToLogLevel(info[0]);
    errorHandler_->setLogLevel(level);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("SetLogLevel failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ErrorHandlerWrapper::GetLogLevel(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!errorHandler_) {
    Napi::TypeError::New(env, "ErrorHandler not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    LogLevel level = errorHandler_->getLogLevel();
    return LogLevelToJS(env, level);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetLogLevel failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ErrorHandlerWrapper::LogError(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!errorHandler_) {
    Napi::TypeError::New(env, "ErrorHandler not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected string message")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string message = info[0].As<Napi::String>().Utf8Value();
    errorHandler_->logError(message);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("LogError failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ErrorHandlerWrapper::LogWarning(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!errorHandler_) {
    Napi::TypeError::New(env, "ErrorHandler not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected string message")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string message = info[0].As<Napi::String>().Utf8Value();
    errorHandler_->logWarning(message);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("LogWarning failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ErrorHandlerWrapper::LogInfo(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!errorHandler_) {
    Napi::TypeError::New(env, "ErrorHandler not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected string message")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string message = info[0].As<Napi::String>().Utf8Value();
    errorHandler_->logInfo(message);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("LogInfo failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ErrorHandlerWrapper::LogDebug(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!errorHandler_) {
    Napi::TypeError::New(env, "ErrorHandler not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected string message")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string message = info[0].As<Napi::String>().Utf8Value();
    errorHandler_->logDebug(message);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("LogDebug failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ErrorHandlerWrapper::HandleError(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!errorHandler_) {
    Napi::TypeError::New(env, "ErrorHandler not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsString()) {
    Napi::TypeError::New(env, "Expected error code and message")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    int errorCode = info[0].As<Napi::Number>().Int32Value();
    std::string message = info[1].As<Napi::String>().Utf8Value();
    errorHandler_->handleError(errorCode, message);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("HandleError failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ErrorHandlerWrapper::HandleException(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!errorHandler_) {
    Napi::TypeError::New(env, "ErrorHandler not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected exception message")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string message = info[0].As<Napi::String>().Utf8Value();
    std::exception ex;
    errorHandler_->handleException(ex, message);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("HandleException failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ErrorHandlerWrapper::GetLogLevelConstants(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Object constants = Napi::Object::New(env);
  
  constants.Set("DEBUG", Napi::Number::New(env, static_cast<int>(LogLevel::DEBUG)));
  constants.Set("INFO", Napi::Number::New(env, static_cast<int>(LogLevel::INFO)));
  constants.Set("WARNING", Napi::Number::New(env, static_cast<int>(LogLevel::WARNING)));
  constants.Set("ERROR", Napi::Number::New(env, static_cast<int>(LogLevel::ERROR)));
  constants.Set("CRITICAL", Napi::Number::New(env, static_cast<int>(LogLevel::CRITICAL)));
  
  return constants;
}

LogLevel ErrorHandlerWrapper::JSToLogLevel(const Napi::Value& value) {
  if (value.IsNumber()) {
    int level = value.As<Napi::Number>().Int32Value();
    switch (level) {
      case 0: return LogLevel::DEBUG;
      case 1: return LogLevel::INFO;
      case 2: return LogLevel::WARNING;
      case 3: return LogLevel::ERROR;
      case 4: return LogLevel::CRITICAL;
      default: throw std::invalid_argument("Invalid log level");
    }
  } else if (value.IsString()) {
    std::string levelStr = value.As<Napi::String>().Utf8Value();
    if (levelStr == "DEBUG") return LogLevel::DEBUG;
    if (levelStr == "INFO") return LogLevel::INFO;
    if (levelStr == "WARNING") return LogLevel::WARNING;
    if (levelStr == "ERROR") return LogLevel::ERROR;
    if (levelStr == "CRITICAL") return LogLevel::CRITICAL;
    throw std::invalid_argument("Invalid log level string");
  }
  throw std::invalid_argument("Log level must be number or string");
}

Napi::Value ErrorHandlerWrapper::LogLevelToJS(Napi::Env env, LogLevel level) {
  return Napi::Number::New(env, static_cast<int>(level));
}