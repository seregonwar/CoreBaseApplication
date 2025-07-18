#include "ModuleManager.h"
#include "ConfigManager.h"
#include "ErrorHandler.h"
#include "ModuleInfo.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

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

#ifdef _WIN32
    std::string libPath = moduleName + ".dll";
    HMODULE handle = LoadLibrary(libPath.c_str());
#else
    std::string libPath = "./lib" + moduleName + ".so";
    void* handle = dlopen(libPath.c_str(), RTLD_LAZY);
#endif

    if (!handle) {
        m_errorHandler->handleError("Failed to load module: " + moduleName, __FILE__, __LINE__, __FUNCTION__);
        return false;
    }

#ifdef _WIN32
    using GetModuleInfoFunc = void(*)(ModuleInfo*);
    auto getModuleInfo = (GetModuleInfoFunc)GetProcAddress(handle, "getModuleInfo");
#else
    using GetModuleInfoFunc = void(*)(ModuleInfo*);
    auto getModuleInfo = (GetModuleInfoFunc)dlsym(handle, "getModuleInfo");
#endif

    if (!getModuleInfo) {
        m_errorHandler->handleError("Failed to find getModuleInfo in module: " + moduleName, __FILE__, __LINE__, __FUNCTION__);
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }

    auto moduleInfo = std::make_shared<ModuleInfo>();
    getModuleInfo(moduleInfo.get());

    moduleInfo->name = moduleName;
    moduleInfo->handle = handle;
    moduleInfo->isLoaded = true;

    m_loadedModules[moduleName] = moduleInfo;
    return true;
}

bool ModuleManager::unloadModule(const std::string& moduleName) {
    auto it = m_loadedModules.find(moduleName);
    if (it == m_loadedModules.end()) {
        return false;
    }

    auto moduleInfo = it->second;

#ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(moduleInfo->handle));
#else
    dlclose(moduleInfo->handle);
#endif

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