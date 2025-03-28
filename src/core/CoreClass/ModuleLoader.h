#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "ConfigManager.h"

namespace CoreNS {

// Forward declarations
class ErrorHandler;

/**
 * @brief Enum che definisce i tipi di moduli supportati
 */
enum class ModuleType {
    CPP,
    PYTHON,
    LUA,
    JAVASCRIPT,
    JAVA
};

/**
 * @brief Struct che rappresenta le informazioni di un modulo
 */
struct ModuleInfo {
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    ModuleType type;
    std::vector<std::string> dependencies;
    bool isLoaded;
    void* handle;
};

// Definizioni dei tipi di funzione per i simboli esportati
using GetModuleInfoFunc = ModuleInfo(*)();
using InitializeModuleFunc = bool(*)();
using CleanupModuleFunc = void(*)();

/**
 * @brief Classe per il caricamento dinamico dei moduli
 */
class ModuleLoader {
public:
    /**
     * @brief Costruttore del ModuleLoader
     */
    ModuleLoader();
    
    /**
     * @brief Costruttore del ModuleLoader con ErrorHandler esistente
     * @param errorHandler Riferimento all'ErrorHandler da utilizzare
     */
    ModuleLoader(ErrorHandler& errorHandler);
    
    /**
     * @brief Distruttore del ModuleLoader
     */
    ~ModuleLoader();
    
    /**
     * @brief Inizializza il ModuleLoader
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize();
    
    /**
     * @brief Chiude il ModuleLoader
     */
    void shutdown();
    
    /**
     * @brief Carica un modulo da un percorso specificato
     * @param modulePath Percorso del modulo da caricare
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     */
    bool loadModule(const std::string& modulePath);
    
    /**
     * @brief Scarica un modulo
     * @param modulePath Percorso del modulo da scaricare
     * @return true se lo scaricamento è avvenuto con successo, false altrimenti
     */
    bool unloadModule(const std::string& modulePath);
    
    /**
     * @brief Verifica se un modulo è caricato
     * @param modulePath Percorso del modulo
     * @return true se il modulo è caricato, false altrimenti
     */
    bool isModuleLoaded(const std::string& modulePath) const;
    
    /**
     * @brief Ottiene le informazioni di un modulo
     * @param modulePath Percorso del modulo
     * @return Informazioni sul modulo, nullptr se il modulo non esiste
     */
    const ModuleInfo* getModuleInfo(const std::string& modulePath) const;
    
    /**
     * @brief Ottiene tutti i moduli caricati
     * @return Vector contenente i percorsi dei moduli caricati
     */
    std::vector<std::string> getLoadedModules() const;
    
    /**
     * @brief Ricarica tutti i moduli
     * @return true se il ricaricamento è avvenuto con successo, false altrimenti
     */
    bool reloadAll();
    
    /**
     * @brief Ottiene un simbolo esportato da un modulo
     * @param modulePath Percorso del modulo
     * @param symbolName Nome del simbolo da ottenere
     * @return Puntatore al simbolo esportato, nullptr se il simbolo non esiste
     */
    void* getSymbol(const std::string& modulePath, const std::string& symbolName) const;

private:
    bool validateModule(const std::string& modulePath) const;
    bool checkDependencies(const ModuleInfo& moduleInfo) const;

    std::unordered_map<std::string, std::pair<void*, ModuleInfo>> m_loadedModules;
    std::shared_ptr<ConfigManager> m_configManager;
    ErrorHandler& m_errorHandler;
    mutable std::mutex m_mutex;
    bool m_initialized{false};
    bool m_ownsErrorHandler{false};
};

} // namespace CoreNS
