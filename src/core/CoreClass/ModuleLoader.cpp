#include "ModuleLoader.h"
#include "ConfigManager.h"
#include "./ErrorHandler.h"
#include <format>
#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace CoreNS {

// Forward declaration
class ErrorHandler;

ModuleLoader::ModuleLoader()
    : m_configManager(std::make_shared<ConfigManager>()),
      m_errorHandler(*new ErrorHandler()),
      m_initialized(false),
      m_ownsErrorHandler(true) {
    m_errorHandler.initialize();
}

ModuleLoader::ModuleLoader(ErrorHandler& errorHandler)
    : m_configManager(std::make_shared<ConfigManager>()),
      m_errorHandler(errorHandler),
      m_initialized(false),
      m_ownsErrorHandler(false) {
}

ModuleLoader::~ModuleLoader() {
    shutdown();
    if (m_ownsErrorHandler) {
        delete &m_errorHandler;
    }
}

bool ModuleLoader::initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_initialized) {
        return true;
    }

    m_initialized = true;
    return true;
}

void ModuleLoader::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized) {
        return;
    }

    for (const auto& [path, module] : m_loadedModules) {
        unloadModule(path);
    }

    m_initialized = false;
}

bool ModuleLoader::loadModule(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return false;
    }
    
    // Verifica se il modulo è già caricato
    if (m_loadedModules.find(path) != m_loadedModules.end()) {
        return false;
    }

    // Valida il modulo
    if (!validateModule(path)) {
        return false;
    }

    // Carica la libreria dinamica
    void* handle = nullptr;
    #ifdef _WIN32
    handle = LoadLibraryA(path.c_str());
    #else
    handle = dlopen(path.c_str(), RTLD_NOW);
    #endif

    if (!handle) {
        return false;
    }

    // Ottieni i simboli necessari
    GetModuleInfoFunc getInfo = nullptr;
    InitializeModuleFunc initialize = nullptr;
    #ifdef _WIN32
    getInfo = reinterpret_cast<GetModuleInfoFunc>(GetProcAddress(static_cast<HMODULE>(handle), "getModuleInfo"));
    initialize = reinterpret_cast<InitializeModuleFunc>(GetProcAddress(static_cast<HMODULE>(handle), "initializeModule"));
    #else
    getInfo = reinterpret_cast<GetModuleInfoFunc>(dlsym(handle, "getModuleInfo"));
    initialize = reinterpret_cast<InitializeModuleFunc>(dlsym(handle, "initializeModule"));
    #endif

    if (!getInfo || !initialize) {
        #ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(handle));
        #else
        dlclose(handle);
        #endif
        return false;
    }

    // Ottieni le informazioni del modulo
    ModuleInfo info = getInfo();
    info.isLoaded = true;
    
    // Verifica le dipendenze
    if (!checkDependencies(info)) {
        #ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(handle));
        #else
        dlclose(handle);
        #endif
        return false;
    }
    
    // Inizializza il modulo
    if (!initialize()) {
        #ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(handle));
        #else
        dlclose(handle);
        #endif
        return false;
    }

    // Salva le informazioni del modulo
    m_loadedModules[path] = std::make_pair(handle, info);
    return true;
}

bool ModuleLoader::unloadModule(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return false;
    }
    
    auto it = m_loadedModules.find(path);
    if (it == m_loadedModules.end()) {
        return false;
    }

    // Ottieni l'handle del modulo
    void* handle = it->second.first;

    // Chiama la funzione di cleanup se disponibile
    #ifdef _WIN32
    auto cleanup = (CleanupModuleFunc)GetProcAddress(static_cast<HMODULE>(handle), "cleanupModule");
    #else
    auto cleanup = (CleanupModuleFunc)dlsym(handle, "cleanupModule");
    #endif

    if (cleanup) {
        cleanup();
    }

    // Scarica la libreria
    #ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(handle));
    #else
    dlclose(handle);
    #endif

    // Rimuovi il modulo dalla mappa
    m_loadedModules.erase(it);
    return true;
}

bool ModuleLoader::isModuleLoaded(const std::string& modulePath) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_loadedModules.find(modulePath) != m_loadedModules.end();
}

const ModuleInfo* ModuleLoader::getModuleInfo(const std::string& modulePath) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_loadedModules.find(modulePath);
    if (it != m_loadedModules.end()) {
        return &it->second.second;
    }
    return nullptr;
}

std::vector<std::string> ModuleLoader::getLoadedModules() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> modules;
    for (const auto& [path, _] : m_loadedModules) {
        modules.push_back(path);
    }
    return modules;
}

bool ModuleLoader::reloadAll() {
    if (!m_initialized) {
        return false;
    }

    std::vector<std::string> modulePaths;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& [path, _] : m_loadedModules) {
            modulePaths.push_back(path);
        }
    }
    
    bool success = true;
    for (const auto& path : modulePaths) {
        if (!unloadModule(path) || !loadModule(path)) {
            success = false;
        }
    }
    
    return success;
}

bool ModuleLoader::validateModule(const std::string& modulePath) const {
    if (!std::filesystem::exists(modulePath)) {
        return false;
    }

    auto extension = std::filesystem::path(modulePath).extension();
#ifdef _WIN32
    return extension == ".dll";
#else
    return extension == ".so";
#endif
}

bool ModuleLoader::checkDependencies(const ModuleInfo& moduleInfo) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& dependency : moduleInfo.dependencies) {
        bool found = false;
        for (const auto& [_, info] : m_loadedModules) {
            if (info.second.name == dependency) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

void* ModuleLoader::getSymbol(const std::string& modulePath, const std::string& symbolName) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return nullptr;
    }
    
    auto it = m_loadedModules.find(modulePath);
    if (it == m_loadedModules.end() || !it->second.first) {
        return nullptr;
    }
    
#ifdef _WIN32
    return GetProcAddress(static_cast<HMODULE>(it->second.first), symbolName.c_str());
#else
    return dlsym(it->second.first, symbolName.c_str());
#endif
}

} // namespace CoreNS