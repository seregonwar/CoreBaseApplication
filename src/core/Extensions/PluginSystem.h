#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <mutex>
#include <optional>
#include <filesystem>

#include "EventSystem.h"

namespace Core {
namespace Extensions {

/**
 * @brief Livelli di isolamento per il sandboxing dei plugin.
 */
enum class SandboxLevel {
    NONE,       // Nessun isolamento
    MEMORY,     // Isolamento di memoria (buffer overflow protection)
    FILE,       // Isolamento di accesso al filesystem
    NETWORK,    // Isolamento di accesso alla rete
    PROCESS,    // Isolamento completo in un processo separato
    FULL        // Isolamento completo con vincoli rigidi
};

/**
 * @brief Metadati di un plugin.
 */
struct PluginMetadata {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::vector<std::string> dependencies;
    std::string apiVersion;
    std::string signature;
    bool isVerified;
};

/**
 * @brief Interfaccia base per tutti i plugin.
 */
class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    /**
     * @brief Inizializza il plugin.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Chiude il plugin.
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief Ottiene il nome del plugin.
     * @return Nome del plugin
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Ottiene la versione del plugin.
     * @return Versione del plugin
     */
    virtual std::string getVersion() const = 0;
    
    /**
     * @brief Ottiene i metadati completi del plugin.
     * @return Metadati del plugin
     */
    virtual PluginMetadata getMetadata() const = 0;
    
    /**
     * @brief Esegue un'operazione specifica del plugin.
     * @param operation Nome dell'operazione
     * @param params Parametri dell'operazione in formato JSON
     * @return Risultato dell'operazione in formato JSON, o empty se l'operazione fallisce
     */
    virtual std::optional<std::string> executeOperation(const std::string& operation, const std::string& params) = 0;
};

/**
 * @brief Delegate per la creazione di un plugin da una libreria dinamica.
 */
using PluginCreateFunction = IPlugin* (*)();

/**
 * @brief Delegate per la distruzione di un plugin.
 */
using PluginDestroyFunction = void (*)(IPlugin*);

/**
 * @brief Classe per la gestione dei plugin.
 */
class PluginManager {
public:
    /**
     * @brief Ottiene l'istanza singleton del gestore dei plugin.
     * @return Riferimento all'istanza singleton
     */
    static PluginManager& getInstance() {
        static PluginManager instance;
        return instance;
    }
    
    /**
     * @brief Carica un plugin da un file.
     * 
     * @param path Percorso del file del plugin
     * @param sandboxLevel Livello di isolamento da applicare
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     */
    bool loadPlugin(const std::string& path, SandboxLevel sandboxLevel = SandboxLevel::MEMORY);
    
    /**
     * @brief Scarica un plugin.
     * 
     * @param name Nome del plugin da scaricare
     * @return true se lo scaricamento è avvenuto con successo, false altrimenti
     */
    bool unloadPlugin(const std::string& name);
    
    /**
     * @brief Verifica se un plugin è caricato.
     * 
     * @param name Nome del plugin
     * @return true se il plugin è caricato, false altrimenti
     */
    bool isPluginLoaded(const std::string& name) const;
    
    /**
     * @brief Ottiene un'istanza di un plugin caricato.
     * 
     * @param name Nome del plugin
     * @return Puntatore al plugin, o nullptr se non è caricato
     */
    IPlugin* getPlugin(const std::string& name);
    
    /**
     * @brief Ottiene i metadati di un plugin.
     * 
     * @param name Nome del plugin
     * @return Metadati del plugin, o nullopt se il plugin non è caricato
     */
    std::optional<PluginMetadata> getPluginMetadata(const std::string& name) const;
    
    /**
     * @brief Ottiene i nomi di tutti i plugin caricati.
     * 
     * @return Vettore contenente i nomi dei plugin caricati
     */
    std::vector<std::string> getLoadedPlugins() const;
    
    /**
     * @brief Esegue un'operazione su un plugin.
     * 
     * @param pluginName Nome del plugin
     * @param operation Nome dell'operazione
     * @param params Parametri dell'operazione in formato JSON
     * @return Risultato dell'operazione in formato JSON, o empty se l'operazione fallisce
     */
    std::optional<std::string> executePluginOperation(const std::string& pluginName, 
                                              const std::string& operation, 
                                              const std::string& params);
    
    /**
     * @brief Configura il percorso di ricerca per i plugin.
     * 
     * @param searchPath Percorso di ricerca
     */
    void setPluginSearchPath(const std::string& searchPath);
    
    /**
     * @brief Cerca plugin nel percorso di ricerca.
     * 
     * @return Vettore contenente i metadati dei plugin trovati
     */
    std::vector<PluginMetadata> discoverPlugins();
    
    /**
     * @brief Verifica la firma di un plugin.
     * 
     * @param path Percorso del file del plugin
     * @return true se la firma è valida, false altrimenti
     */
    bool verifyPluginSignature(const std::string& path);
    
    /**
     * @brief Registra una callback da eseguire quando un plugin viene caricato.
     * 
     * @param callback Funzione da eseguire
     * @return ID della callback registrata
     */
    int registerPluginLoadedCallback(std::function<void(const std::string&)> callback);
    
    /**
     * @brief Registra una callback da eseguire quando un plugin viene scaricato.
     * 
     * @param callback Funzione da eseguire
     * @return ID della callback registrata
     */
    int registerPluginUnloadedCallback(std::function<void(const std::string&)> callback);

private:
    // Costruttore privato per pattern Singleton
    PluginManager();
    
    // Distruttore privato
    ~PluginManager();
    
    // Disabilita copia e assegnazione
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    
    /**
     * @brief Crea un sandbox per un plugin.
     * 
     * @param pluginName Nome del plugin
     * @param level Livello di isolamento
     * @return true se la creazione è avvenuta con successo, false altrimenti
     */
    bool createSandbox(const std::string& pluginName, SandboxLevel level);
    
    /**
     * @brief Distrugge il sandbox di un plugin.
     * 
     * @param pluginName Nome del plugin
     */
    void destroySandbox(const std::string& pluginName);
    
    /**
     * @brief Carica una libreria dinamica.
     * 
     * @param path Percorso del file della libreria
     * @return Handle della libreria, o nullptr se il caricamento fallisce
     */
    void* loadLibrary(const std::string& path);
    
    /**
     * @brief Scarica una libreria dinamica.
     * 
     * @param handle Handle della libreria
     * @return true se lo scaricamento è avvenuto con successo, false altrimenti
     */
    bool unloadLibrary(void* handle);
    
    /**
     * @brief Ottiene un simbolo da una libreria dinamica.
     * 
     * @param handle Handle della libreria
     * @param symbolName Nome del simbolo
     * @return Puntatore al simbolo, o nullptr se non è trovato
     */
    void* getSymbol(void* handle, const std::string& symbolName);
    
    /**
     * @brief Estrae i metadati da un file plugin.
     * 
     * @param path Percorso del file del plugin
     * @return Metadati del plugin, o nullopt se l'estrazione fallisce
     */
    std::optional<PluginMetadata> extractMetadata(const std::string& path);

    std::string m_pluginSearchPath;
    std::mutex m_mutex;
    
    struct PluginInfo {
        std::unique_ptr<IPlugin> plugin;
        void* libraryHandle;
        SandboxLevel sandboxLevel;
        PluginMetadata metadata;
        bool isInitialized;
    };
    
    std::unordered_map<std::string, PluginInfo> m_plugins;
    std::unordered_map<std::string, void*> m_sandboxHandles;
    
    std::vector<std::function<void(const std::string&)>> m_loadCallbacks;
    std::vector<std::function<void(const std::string&)>> m_unloadCallbacks;
};

/**
 * @brief Evento emesso quando un plugin viene caricato.
 */
struct PluginLoadedEvent : public BaseEvent {
    std::string pluginName;
    std::string version;
    
    PluginLoadedEvent(const std::string& name, const std::string& ver)
        : pluginName(name), version(ver) {}
};

/**
 * @brief Evento emesso quando un plugin viene scaricato.
 */
struct PluginUnloadedEvent : public BaseEvent {
    std::string pluginName;
    
    PluginUnloadedEvent(const std::string& name) : pluginName(name) {}
};

} // namespace Extensions
} // namespace Core 