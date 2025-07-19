#pragma once
#include <napi.h>
#include "../../../../CoreClass/ConfigManager.h"
#include <memory>

/**
 * @file config_manager_wrapper.h
 * @brief Node.js wrapper for CoreBase ConfigManager
 */

class ConfigManagerWrapper : public Napi::ObjectWrap<ConfigManagerWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::Value CreateNew(const Napi::CallbackInfo& info);
    
    ConfigManagerWrapper(const Napi::CallbackInfo& info);
    virtual ~ConfigManagerWrapper();

private:
    // Core methods
    Napi::Value LoadConfig(const Napi::CallbackInfo& info);
    Napi::Value SaveConfig(const Napi::CallbackInfo& info);
    Napi::Value GetValue(const Napi::CallbackInfo& info);
    Napi::Value SetValue(const Napi::CallbackInfo& info);
    Napi::Value HasKey(const Napi::CallbackInfo& info);
    Napi::Value RemoveKey(const Napi::CallbackInfo& info);
    Napi::Value Clear(const Napi::CallbackInfo& info);
    Napi::Value GetAllKeys(const Napi::CallbackInfo& info);
    Napi::Value GetSection(const Napi::CallbackInfo& info);
    Napi::Value SetSection(const Napi::CallbackInfo& info);
    Napi::Value Reload(const Napi::CallbackInfo& info);
    Napi::Value IsModified(const Napi::CallbackInfo& info);
    Napi::Value GetConfigPath(const Napi::CallbackInfo& info);
    
    // Validation methods
    Napi::Value ValidateConfig(const Napi::CallbackInfo& info);
    Napi::Value SetSchema(const Napi::CallbackInfo& info);
    
    // Utility methods
    Napi::Value Merge(const Napi::CallbackInfo& info);
    Napi::Value Clone(const Napi::CallbackInfo& info);
    Napi::Value Export(const Napi::CallbackInfo& info);
    Napi::Value Import(const Napi::CallbackInfo& info);
    
    // Helper methods
    static Napi::Value JsonToJS(Napi::Env env, const nlohmann::json& json);
    static nlohmann::json JSToJson(const Napi::Value& value);
    static std::string JSToString(const Napi::Value& value);
    
    std::unique_ptr<ConfigManager> configManager_;
};