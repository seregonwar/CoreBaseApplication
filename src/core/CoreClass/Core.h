#pragma once

#include <string>
#include <memory>
#include <vector>

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
     * @param configPath Percorso del file di configurazione
     */
    Core(const std::string& configPath = "config.json");
    
    /**
     * @brief Distruttore del Core
     */
    ~Core();
    
    /**
     * @brief Inizializza il Core e i suoi componenti
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize();
    
    /**
     * @brief Avvia il ciclo principale dell'applicazione
     * @return Codice di uscita dell'applicazione
     */
    int run();
    
    /**
     * @brief Arresta il Core e libera le risorse
     */
    void shutdown();
    
    /**
     * @brief Ottiene l'istanza del ConfigManager
     * @return Riferimento al ConfigManager
     */
    ConfigManager& getConfigManager() const;
    
    /**
     * @brief Ottiene l'istanza del ResourceManager
     * @return Riferimento al ResourceManager
     */
    ResourceManager& getResourceManager() const;
    
    /**
     * @brief Ottiene l'istanza del ModuleLoader
     * @return Riferimento al ModuleLoader
     */
    ModuleLoader& getModuleLoader() const;
    
    /**
     * @brief Ottiene l'istanza dell'ErrorHandler
     * @return Riferimento all'ErrorHandler
     */
    ErrorHandler& getErrorHandler() const;
    
    /**
     * @brief Ottiene l'istanza dell'IPCManager
     * @return Riferimento all'IPCManager
     */
    IPCManager& getIPCManager() const;
    
private:
    std::string m_configPath;
    bool m_isRunning;
    
    std::unique_ptr<ConfigManager> m_configManager;
    std::unique_ptr<ResourceManager> m_resourceManager;
    std::unique_ptr<ModuleLoader> m_moduleLoader;
    std::unique_ptr<ErrorHandler> m_errorHandler;
    std::unique_ptr<IPCManager> m_ipcManager;
};
