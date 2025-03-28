#include "ModuleManager.h"
#include "ConfigManager.h"
#include "ErrorHandler.h"
#include "ModuleInfo.h"

namespace CoreNS {

ModuleManager::ModuleManager() {
    m_configManager = std::make_shared<ConfigManager>();
    m_errorHandler = std::make_shared<ErrorHandler>();
}

ModuleManager::~ModuleManager() {
    shutdown();
}

bool ModuleManager::initialize() {
    return true;
}

void ModuleManager::shutdown() {
    for (const auto& module : m_loadedModules) {
        unloadModule(module.first);
    }
    m_loadedModules.clear();
}

bool ModuleManager::loadModule(const std::string& moduleName) {
    if (isModuleLoaded(moduleName)) {
        return false;
    }

    auto moduleInfo = std::make_shared<ModuleInfo>();
    moduleInfo->name = moduleName;
    moduleInfo->version = "1.0.0"; // TODO: Get from module metadata
    moduleInfo->description = "Module description"; // TODO: Get from module metadata
    moduleInfo->author = "Unknown"; // TODO: Get from module metadata
    moduleInfo->type = ModuleType::PLUGIN; // TODO: Get from module metadata
    moduleInfo->isLoaded = true;

    m_loadedModules[moduleName] = moduleInfo;
    return true;
}

bool ModuleManager::unloadModule(const std::string& moduleName) {
    auto it = m_loadedModules.find(moduleName);
    if (it == m_loadedModules.end()) {
        return false;
    }

    m_loadedModules.erase(it);
    return true;
}

bool ModuleManager::isModuleLoaded(const std::string& moduleName) const {
    return m_loadedModules.find(moduleName) != m_loadedModules.end();
}

std::vector<std::string> ModuleManager::getLoadedModules() const {
    std::vector<std::string> modules;
    for (const auto& module : m_loadedModules) {
        modules.push_back(module.first);
    }
    return modules;
}

std::shared_ptr<ModuleInfo> ModuleManager::getModuleInfo(const std::string& moduleName) const {
    auto it = m_loadedModules.find(moduleName);
    if (it == m_loadedModules.end()) {
        return nullptr;
    }
    return it->second;
}

} // namespace CoreNS 