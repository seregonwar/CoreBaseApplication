#pragma once

#include <string>
#include <memory>
#include "ConfigManager.h"
#include "ResourceManager.h"
#include "ModuleManager.h"
#include "ErrorHandler.h"
#include "IPCManager.h"

namespace CoreNS {

// Forward declarations
class ModuleLoader;
class ConfigManager;
class ResourceManager;
class ErrorHandler;
class IPCManager;

/**
 * @brief Classe principale del Core che gestisce l'intero ciclo di vita dell'applicazione.
 */
class Core {
public:
    /**
     * @brief Costruttore del Core
     */
    Core();
    
    /**
     * @brief Distruttore del Core
     */
    ~Core();
    
    /**
     * @brief Inizializza il Core e i suoi componenti
     * @param configPath Percorso del file di configurazione
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize(const std::string& configPath);
    
    /**
     * @brief Arresta il Core e libera le risorse
     */
    bool shutdown();
    
    /**
     * @brief Ottiene l'istanza del ConfigManager
     * @return Riferimento al ConfigManager
     */
    std::shared_ptr<ConfigManager> getConfigManager() const { return m_configManager; }
    
    /**
     * @brief Ottiene l'istanza del ResourceManager
     * @return Riferimento al ResourceManager
     */
    std::shared_ptr<ResourceManager> getResourceManager() const { return m_resourceManager; }
    
    /**
     * @brief Ottiene l'istanza del ModuleLoader
     * @return Riferimento al ModuleLoader
     */
    std::shared_ptr<ModuleManager> getModuleManager() const { return m_moduleLoader; }
    
    /**
     * @brief Ottiene l'istanza dell'ErrorHandler
     * @return Riferimento all'ErrorHandler
     */
    std::shared_ptr<ErrorHandler> getErrorHandler() const { return m_errorHandler; }
    
    /**
     * @brief Ottiene l'istanza dell'IPCManager
     * @return Riferimento all'IPCManager
     */
    std::shared_ptr<IPCManager> getIPCManager() const { return m_ipcManager; }
    
private:
    bool m_isInitialized{false};
    
    std::shared_ptr<ConfigManager> m_configManager;
    std::shared_ptr<ResourceManager> m_resourceManager;
    std::shared_ptr<ModuleManager> m_moduleLoader;
    std::shared_ptr<ErrorHandler> m_errorHandler;
    std::shared_ptr<IPCManager> m_ipcManager;
};

} // namespace CoreNS
