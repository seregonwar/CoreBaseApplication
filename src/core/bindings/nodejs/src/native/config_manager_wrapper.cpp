#include "config_manager_wrapper.h"
#include <stdexcept>

/**
 * @file config_manager_wrapper.cpp
 * @brief Implementation of Node.js wrapper for CoreBase ConfigManager
 */

Napi::Object ConfigManagerWrapper::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "ConfigManager", {
    InstanceMethod("loadConfig", &ConfigManagerWrapper::LoadConfig),
    InstanceMethod("saveConfig", &ConfigManagerWrapper::SaveConfig),
    InstanceMethod("getValue", &ConfigManagerWrapper::GetValue),
    InstanceMethod("setValue", &ConfigManagerWrapper::SetValue),
    InstanceMethod("hasKey", &ConfigManagerWrapper::HasKey),
    InstanceMethod("removeKey", &ConfigManagerWrapper::RemoveKey),
    InstanceMethod("clear", &ConfigManagerWrapper::Clear),
    InstanceMethod("getAllKeys", &ConfigManagerWrapper::GetAllKeys),
    InstanceMethod("getSection", &ConfigManagerWrapper::GetSection),
    InstanceMethod("setSection", &ConfigManagerWrapper::SetSection),
    InstanceMethod("reload", &ConfigManagerWrapper::Reload),
    InstanceMethod("isModified", &ConfigManagerWrapper::IsModified),
    InstanceMethod("getConfigPath", &ConfigManagerWrapper::GetConfigPath),
    InstanceMethod("validateConfig", &ConfigManagerWrapper::ValidateConfig),
    InstanceMethod("setSchema", &ConfigManagerWrapper::SetSchema),
    InstanceMethod("merge", &ConfigManagerWrapper::Merge),
    InstanceMethod("clone", &ConfigManagerWrapper::Clone),
    InstanceMethod("export", &ConfigManagerWrapper::Export),
    InstanceMethod("import", &ConfigManagerWrapper::Import),
    StaticMethod("createNew", &ConfigManagerWrapper::CreateNew)
  });
  
  exports.Set("ConfigManager", func);
  return exports;
}

Napi::Value ConfigManagerWrapper::CreateNew(const Napi::CallbackInfo& info) {
  return info.Env().Undefined();
}

ConfigManagerWrapper::ConfigManagerWrapper(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<ConfigManagerWrapper>(info) {
  try {
    configManager_ = std::make_unique<ConfigManager>();
  } catch (const std::exception& e) {
    Napi::TypeError::New(info.Env(), std::string("Failed to create ConfigManager: ") + e.what())
        .ThrowAsJavaScriptException();
  }
}

ConfigManagerWrapper::~ConfigManagerWrapper() {
  // ConfigManager destructor handles cleanup
}

Napi::Value ConfigManagerWrapper::LoadConfig(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected config file path")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string configPath = info[0].As<Napi::String>().Utf8Value();
    bool result = configManager_->loadConfig(configPath);
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("LoadConfig failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::SaveConfig(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string configPath = "";
    if (info.Length() > 0 && info[0].IsString()) {
      configPath = info[0].As<Napi::String>().Utf8Value();
    }
    
    bool result = configManager_->saveConfig(configPath);
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("SaveConfig failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::GetValue(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected key string")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string key = info[0].As<Napi::String>().Utf8Value();
    
    if (info.Length() > 1) {
      // With default value
      nlohmann::json defaultValue = JSToJson(info[1]);
      nlohmann::json result = configManager_->getValue(key, defaultValue);
      return JsonToJS(env, result);
    } else {
      // Without default value
      nlohmann::json result = configManager_->getValue(key);
      return JsonToJS(env, result);
    }
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetValue failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::SetValue(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 2 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected key and value")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string key = info[0].As<Napi::String>().Utf8Value();
    nlohmann::json value = JSToJson(info[1]);
    configManager_->setValue(key, value);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("SetValue failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::HasKey(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected key string")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string key = info[0].As<Napi::String>().Utf8Value();
    bool result = configManager_->hasKey(key);
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("HasKey failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::RemoveKey(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected key string")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string key = info[0].As<Napi::String>().Utf8Value();
    bool result = configManager_->removeKey(key);
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("RemoveKey failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::Clear(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    configManager_->clear();
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("Clear failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::GetAllKeys(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::vector<std::string> keys = configManager_->getAllKeys();
    Napi::Array result = Napi::Array::New(env, keys.size());
    
    for (size_t i = 0; i < keys.size(); ++i) {
      result[i] = Napi::String::New(env, keys[i]);
    }
    
    return result;
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetAllKeys failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::GetSection(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected section name")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string section = info[0].As<Napi::String>().Utf8Value();
    nlohmann::json result = configManager_->getSection(section);
    return JsonToJS(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetSection failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::SetSection(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 2 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected section name and data")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string section = info[0].As<Napi::String>().Utf8Value();
    nlohmann::json data = JSToJson(info[1]);
    configManager_->setSection(section, data);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("SetSection failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::Reload(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    bool result = configManager_->reload();
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("Reload failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::IsModified(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    bool result = configManager_->isModified();
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("IsModified failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::GetConfigPath(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string path = configManager_->getConfigPath();
    return Napi::String::New(env, path);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("GetConfigPath failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::ValidateConfig(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    bool result = configManager_->validateConfig();
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("ValidateConfig failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::SetSchema(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Expected schema object")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    nlohmann::json schema = JSToJson(info[0]);
    configManager_->setSchema(schema);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("SetSchema failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::Merge(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Expected config object to merge")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    nlohmann::json other = JSToJson(info[0]);
    configManager_->merge(other);
    return env.Undefined();
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("Merge failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::Clone(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::unique_ptr<ConfigManager> cloned = configManager_->clone();
    // Return a new wrapper instance
    Napi::Function constructor = env.GetInstanceData<Napi::FunctionReference>()->Value();
    Napi::Object instance = constructor.New({});
    ConfigManagerWrapper* wrapper = Napi::ObjectWrap<ConfigManagerWrapper>::Unwrap(instance);
    wrapper->configManager_ = std::move(cloned);
    return instance;
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("Clone failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::Export(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected export file path")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string filePath = info[0].As<Napi::String>().Utf8Value();
    std::string format = "json";
    if (info.Length() > 1 && info[1].IsString()) {
      format = info[1].As<Napi::String>().Utf8Value();
    }
    
    bool result = configManager_->exportConfig(filePath, format);
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("Export failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

Napi::Value ConfigManagerWrapper::Import(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!configManager_) {
    Napi::TypeError::New(env, "ConfigManager not initialized")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected import file path")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  try {
    std::string filePath = info[0].As<Napi::String>().Utf8Value();
    bool merge = false;
    if (info.Length() > 1 && info[1].IsBoolean()) {
      merge = info[1].As<Napi::Boolean>().Value();
    }
    
    bool result = configManager_->importConfig(filePath, merge);
    return Napi::Boolean::New(env, result);
  } catch (const std::exception& e) {
    Napi::TypeError::New(env, std::string("Import failed: ") + e.what())
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
}

// Helper methods
Napi::Value ConfigManagerWrapper::JsonToJS(Napi::Env env, const nlohmann::json& json) {
  if (json.is_null()) {
    return env.Null();
  } else if (json.is_boolean()) {
    return Napi::Boolean::New(env, json.get<bool>());
  } else if (json.is_number_integer()) {
    return Napi::Number::New(env, json.get<int64_t>());
  } else if (json.is_number_float()) {
    return Napi::Number::New(env, json.get<double>());
  } else if (json.is_string()) {
    return Napi::String::New(env, json.get<std::string>());
  } else if (json.is_array()) {
    Napi::Array arr = Napi::Array::New(env, json.size());
    for (size_t i = 0; i < json.size(); ++i) {
      arr[i] = JsonToJS(env, json[i]);
    }
    return arr;
  } else if (json.is_object()) {
    Napi::Object obj = Napi::Object::New(env);
    for (auto& [key, value] : json.items()) {
      obj.Set(key, JsonToJS(env, value));
    }
    return obj;
  }
  return env.Undefined();
}

nlohmann::json ConfigManagerWrapper::JSToJson(const Napi::Value& value) {
  if (value.IsNull() || value.IsUndefined()) {
    return nlohmann::json();
  } else if (value.IsBoolean()) {
    return value.As<Napi::Boolean>().Value();
  } else if (value.IsNumber()) {
    double num = value.As<Napi::Number>().DoubleValue();
    if (num == std::floor(num)) {
      return static_cast<int64_t>(num);
    }
    return num;
  } else if (value.IsString()) {
    return value.As<Napi::String>().Utf8Value();
  } else if (value.IsArray()) {
    Napi::Array arr = value.As<Napi::Array>();
    nlohmann::json jsonArr = nlohmann::json::array();
    for (uint32_t i = 0; i < arr.Length(); ++i) {
      jsonArr.push_back(JSToJson(arr[i]));
    }
    return jsonArr;
  } else if (value.IsObject()) {
    Napi::Object obj = value.As<Napi::Object>();
    nlohmann::json jsonObj = nlohmann::json::object();
    Napi::Array keys = obj.GetPropertyNames();
    for (uint32_t i = 0; i < keys.Length(); ++i) {
      std::string key = keys.Get(i).As<Napi::String>().Utf8Value();
      jsonObj[key] = JSToJson(obj.Get(key));
    }
    return jsonObj;
  }
  return nlohmann::json();
}

std::string ConfigManagerWrapper::JSToString(const Napi::Value& value) {
  if (value.IsString()) {
    return value.As<Napi::String>().Utf8Value();
  }
  return "";
}