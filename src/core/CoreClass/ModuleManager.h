#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "ConfigManager.h"
#include "ModuleInfo.h"

namespace CoreNS {

class ErrorHandler;

class ModuleManager {
public:
    ModuleManager();
    ~ModuleManager();

    bool initialize();
    void shutdown();

    bool loadModule(const std::string& moduleName);
    bool unloadModule(const std::string& moduleName);
    bool isModuleLoaded(const std::string& moduleName) const;
    std::vector<std::string> getLoadedModules() const;
    std::shared_ptr<ModuleInfo> getModuleInfo(const std::string& moduleName) const;

private:
    std::shared_ptr<ConfigManager> m_configManager;
    std::shared_ptr<ErrorHandler> m_errorHandler;
    std::unordered_map<std::string, std::shared_ptr<ModuleInfo>> m_loadedModules;
};

} // namespace CoreNS 