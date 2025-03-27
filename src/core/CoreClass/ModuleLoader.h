#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

// Forward declarations
class ErrorHandler;
class ConfigManager;

/**
 * @brief Enum che definisce i tipi di moduli supportati
 */
enum class ModuleType {
    CPP,
    PYTHON,
    JAVA,
    UNKNOWN
};

/**
 * @brief Struct che rappresenta le informazioni di un modulo
 */
struct ModuleInfo {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    ModuleType type;
    std::vector<std::string> dependencies;
    bool isLoaded;
    void* handle;
};

/**
 * @brief Classe per il caricamento dinamico dei moduli
 */
class ModuleLoader {
public:
    /**
     * @brief Costruttore del ModuleLoader
     * @param errorHandler Riferimento all'ErrorHandler
     * @param configManager Riferimento al ConfigManager
     */
    ModuleLoader(ErrorHandler& errorHandler, ConfigManager& configManager);
    
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
     * @brief Carica un modulo da un percorso specificato
     * @param modulePath Percorso del modulo da caricare
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     */
    bool load(const std::string& modulePath);
    
    /**
     * @brief Scarica un modulo
     * @param moduleName Nome del modulo da scaricare
     * @return true se lo scaricamento è avvenuto con successo, false altrimenti
     */
    bool unload(const std::string& moduleName);
    
    /**
     * @brief Verifica se un modulo è caricato
     * @param moduleName Nome del modulo
     * @return true se il modulo è caricato, false altrimenti
     */
    bool isLoaded(const std::string& moduleName) const;
    
    /**
     * @brief Ottiene le informazioni di un modulo
     * @param moduleName Nome del modulo
     * @return Informazioni sul modulo, nullptr se il modulo non esiste
     */
    const ModuleInfo* getModuleInfo(const std::string& moduleName) const;
    
    /**
     * @brief Ottiene tutti i moduli caricati
     * @return Vector contenente i nomi dei moduli caricati
     */
    std::vector<std::string> getLoadedModules() const;
    
    /**
     * @brief Chiama una funzione esportata da un modulo C++
     * @param moduleName Nome del modulo
     * @param functionName Nome della funzione
     * @param args Argomenti da passare alla funzione
     * @return true se la chiamata è avvenuta con successo, false altrimenti
     */
    template<typename... Args>
    bool callFunction(const std::string& moduleName, const std::string& functionName, Args... args);
    
    /**
     * @brief Esegue un metodo in un modulo Python
     * @param moduleName Nome del modulo Python
     * @param functionName Nome della funzione
     * @param args Argomenti da passare alla funzione
     * @return true se la chiamata è avvenuta con successo, false altrimenti
     */
    template<typename... Args>
    bool executePythonMethod(const std::string& moduleName, const std::string& functionName, Args... args);
    
    /**
     * @brief Esegue un metodo in un modulo Java
     * @param className Nome della classe Java
     * @param methodName Nome del metodo
     * @param args Argomenti da passare al metodo
     * @return true se la chiamata è avvenuta con successo, false altrimenti
     */
    template<typename... Args>
    bool executeJavaMethod(const std::string& className, const std::string& methodName, Args... args);
    
private:
    ErrorHandler& m_errorHandler;
    ConfigManager& m_configManager;
    std::unordered_map<std::string, ModuleInfo> m_modules;
    
    /**
     * @brief Determina il tipo di modulo in base all'estensione del file
     * @param modulePath Percorso del modulo
     * @return Tipo del modulo
     */
    ModuleType getModuleType(const std::string& modulePath) const;
    
    /**
     * @brief Carica un modulo C++
     * @param modulePath Percorso del modulo
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     */
    bool loadCppModule(const std::string& modulePath);
    
    /**
     * @brief Carica un modulo Python
     * @param modulePath Percorso del modulo
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     */
    bool loadPythonModule(const std::string& modulePath);
    
    /**
     * @brief Carica un modulo Java
     * @param modulePath Percorso del modulo
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     */
    bool loadJavaModule(const std::string& modulePath);
    
    /**
     * @brief Verifica le dipendenze di un modulo
     * @param dependencies Lista delle dipendenze del modulo
     * @return true se tutte le dipendenze sono soddisfatte, false altrimenti
     */
    bool checkDependencies(const std::vector<std::string>& dependencies) const;
};
