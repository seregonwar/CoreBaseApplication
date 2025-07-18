#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
// Forward declaration delle classi necessarie
namespace CoreNS {
    class ErrorHandler;
    class ConfigManager;
    class ResourceManager;
    class ModuleManager;
    class IPCManager;
    class SystemMonitor; // Aggiunto
    class Core;
    enum class LogLevel;
    struct ChannelInfo;
    struct SystemResources;
    struct ModuleInfo;
    enum class IPCType;
    enum class IPCRole;
    enum class ResourceType;
}

#include "CoreClass/ResourceManager.h"
#include "CoreClass/Core.h"
#include "CoreClass/ModuleInfo.h"
#include "CoreClass/SystemResources.h"
#include "Monitoring/SystemMonitor.h" // Aggiunto
#ifdef USE_PYTHON_BINDINGS
#include "bindings/python/python_bindings.h"
#endif

namespace CoreNS {

/**
 * @brief Livelli di log supportati dall'API
 */
enum class APILogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERR,
    FATAL
};

/**
 * @brief Tipi di moduli supportati dall'API
 */
enum class APIModuleType {
    CPP,
    PYTHON,
    JAVA
};

/**
 * @brief Tipi di comunicazione IPC supportati dall'API
 */
enum class APIIPCType {
    SHARED_MEMORY,
    NAMED_PIPE,
    SOCKET
};

/**
 * @brief Ruolo in una comunicazione IPC
 */
enum class APIIPCRole {
    SERVER,
    CLIENT
};

/**
 * @brief Callback per la ricezione di messaggi IPC
 */
using APIMessageCallback = std::function<void(const std::string& channelName, const void* data, size_t dataSize)>;

/**
 * @brief Callback per le soglie di risorse
 */
using APIResourceCallback = std::function<void(const std::string&, int)>;

/**
 * @brief Struct che contiene informazioni sulle risorse di sistema
 */
struct APISystemResources {
    double cpuUsagePercent;
    uint64_t availableMemoryBytes;
    uint64_t totalMemoryBytes;
    uint64_t availableDiskBytes;
    uint64_t totalDiskBytes;
    double networkUsagePercent;
    double gpuUsagePercent;
};

/**
 * @brief Struct che contiene informazioni su un modulo
 */
struct APIModuleInfo {
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    std::string type;
    std::vector<std::string> dependencies;
    bool isLoaded;
};

// Forward declaration della classe nascosta che implementa l'API
class CoreAPIImpl;

class Core;

/**
 * @brief API semplificata per interagire con il Core
 */
class CoreAPI {
public:
    /**
     * @brief Costruttore dell'API
     */
    CoreAPI();
    
    /**
     * @brief Distruttore dell'API
     */
    ~CoreAPI();
    
    //----------------------------------------------
    // Funzioni di inizializzazione e gestione
    //----------------------------------------------
    
    /**
     * @brief Inizializza il Core e tutti i suoi componenti
     * @param configPath Percorso del file di configurazione
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize(const std::string& configPath);
    
    /**
     * @brief Arresta il Core e libera le risorse
     */
    void shutdown();
    
    //----------------------------------------------
    // Gestione moduli
    //----------------------------------------------
    
    /**
     * @brief Carica un modulo
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
     * @return Informazioni sul modulo
     */
    APIModuleInfo getModuleInfo(const std::string& modulePath) const;
    
    /**
     * @brief Ottiene tutti i moduli caricati
     * @return Vector contenente i percorsi dei moduli caricati
     */
    std::vector<std::string> getLoadedModules() const;
    
    //----------------------------------------------
    // Configuration Management
    //----------------------------------------------
    
    /**
     * @brief Carica la configurazione da un file
     * @param filePath Percorso del file di configurazione
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     */
    bool loadConfig(const std::string& filePath);
    
    /**
     * @brief Salva la configurazione su un file
     * @param filePath Percorso del file di configurazione
     * @return true se il salvataggio è avvenuto con successo, false altrimenti
     */
    bool saveConfig(const std::string& filePath);
    
    /**
     * @brief Ottiene un valore di configurazione come stringa
     * @param key Chiave del valore
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore di configurazione
     */
    std::string getConfigString(const std::string& key, const std::string& defaultValue = "");
    
    /**
     * @brief Ottiene un valore di configurazione come intero
     * @param key Chiave del valore
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore di configurazione
     */
    int getConfigInt(const std::string& key, int defaultValue = 0);
    
    /**
     * @brief Ottiene un valore di configurazione come double
     * @param key Chiave del valore
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore di configurazione
     */
    double getConfigDouble(const std::string& key, double defaultValue = 0.0);
    
    /**
     * @brief Ottiene un valore di configurazione come booleano
     * @param key Chiave del valore
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore di configurazione
     */
    bool getConfigBool(const std::string& key, bool defaultValue = false);
    
    /**
     * @brief Imposta un valore di configurazione come stringa
     * @param key Chiave del valore
     * @param value Valore da impostare
     */
    void setConfig(const std::string& key, const std::string& value);
    
    /**
     * @brief Imposta un valore di configurazione come intero
     * @param key Chiave del valore
     * @param value Valore da impostare
     */
    void setConfig(const std::string& key, int value);
    
    /**
     * @brief Imposta un valore di configurazione come double
     * @param key Chiave del valore
     * @param value Valore da impostare
     */
    void setConfig(const std::string& key, double value);
    
    /**
     * @brief Imposta un valore di configurazione come booleano
     * @param key Chiave del valore
     * @param value Valore da impostare
     */
    void setConfig(const std::string& key, bool value);
    
    //----------------------------------------------
    // Configuration Management
    //----------------------------------------------
    
    /**
     * @brief Crea un canale di comunicazione IPC
     * @param name Nome del canale
     * @param type Tipo di canale
     * @param role Ruolo (server o client)
     * @param params Parametri specifici per il tipo di canale (es. host:port per socket)
     * @return true se la creazione è avvenuta con successo, false altrimenti
     */
    bool createIPCChannel(const std::string& name, APIIPCType type, APIIPCRole role = APIIPCRole::SERVER, 
                         const std::string& params = "");
    
    /**
     * @brief Invia dati su un canale IPC
     * @param channelName Nome del canale
     * @param data Dati da inviare
     * @param dataSize Dimensione dei dati
     * @return true se l'invio è avvenuto con successo, false altrimenti
     */
    bool sendData(const std::string& channelName, const void* data, size_t dataSize);
    
    /**
     * @brief Riceve dati da un canale IPC
     * @param channelName Nome del canale
     * @param buffer Buffer in cui leggere i dati
     * @param bufferSize Dimensione del buffer
     * @param bytesRead Riferimento in cui salvare il numero di bytes letti
     * @return true se la ricezione è avvenuta con successo, false altrimenti
     */
    bool receiveData(const std::string& channelName, void* buffer, size_t bufferSize, size_t& bytesRead);
    
    /**
     * @brief Registra una callback per i messaggi ricevuti
     * @param channelName Nome del canale IPC
     * @param callback Funzione da chiamare quando arriva un messaggio
     * @return ID della callback registrata
     */
    int registerMessageCallback(const std::string& channelName, APIMessageCallback callback);
    
    /**
     * @brief Chiude un canale IPC
     * @param channelName Nome del canale
     * @return true se la chiusura è avvenuta con successo, false altrimenti
     */
    bool closeIPCChannel(const std::string& channelName);
    
    //----------------------------------------------
    // Integrazione Python
    //----------------------------------------------
    
    /**
     * @brief Inizializza l'ambiente Python
     * @param pythonHome Percorso dell'installazione Python
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initializePython(const std::string& pythonHome = "");
    
    /**
     * @brief Importa un modulo Python (legacy)
     * @param moduleName Nome del modulo da importare
     * @return true se l'importazione è avvenuta con successo, false altrimenti
     */
    bool importPythonModuleLegacy(const std::string& moduleName);
    
    /**
     * @brief Esegue una funzione Python
     * @param moduleName Nome del modulo
     * @param funcName Nome della funzione
     * @param args Argomenti come stringa in formato JSON (es. "[1, 'test', true]")
     * @return true se l'esecuzione è avvenuta con successo, false altrimenti
     */
    bool executePythonFunction(const std::string& moduleName, const std::string& funcName, 
                             const std::string& args = "[]");
    
    /**
     * @brief Esegue un codice Python
     * @param code Codice Python da eseguire
     * @return true se l'esecuzione è avvenuta con successo, false altrimenti
     */
    bool executePythonCode(const std::string& code);
    
    /**
     * @brief Esegue uno script Python e restituisce l'output
     * @param code Codice Python da eseguire
     * @return Output dello script
     */
    std::string executePythonScriptWithOutput(const std::string& code);
    
    // Gestione interprete Python avanzato
    bool initializePythonEngine();
    void finalizePythonEngine();
    std::string executePythonString(const std::string& code);
    std::string executePythonFile(const std::string& filename);
    bool executePythonStringQuiet(const std::string& code);
    bool executePythonFileQuiet(const std::string& filename);
    
    // Gestione variabili Python
    bool setPythonVariable(const std::string& name, const std::string& value);
    bool setPythonVariable(const std::string& name, int value);
    bool setPythonVariable(const std::string& name, double value);
    bool setPythonVariable(const std::string& name, bool value);
    std::string getPythonVariable(const std::string& name);
    
    // Gestione moduli Python
    bool importPythonModule(const std::string& moduleName);
    bool importPythonModuleAs(const std::string& moduleName, const std::string& alias);
    std::vector<std::string> getLoadedPythonModules();
    
    // Chiamata funzioni Python
    std::string callPythonFunction(const std::string& functionName, const std::vector<std::string>& args = {});
    std::string callPythonModuleFunction(const std::string& moduleName, const std::string& functionName, const std::vector<std::string>& args = {});
    
    // Gestione path Python
    bool addToPythonPath(const std::string& path);
    std::vector<std::string> getPythonPath();
    
    // Informazioni interprete Python
    std::string getPythonVersion();
    std::string getPythonExecutable();
    bool isPythonInitialized();
    std::string getPythonLastError();
    void clearPythonError();
    
    // Salva/carica stato interprete Python
    bool savePythonState(const std::string& filename);
    bool loadPythonState(const std::string& filename);
    bool resetPythonEngine();
    
    //----------------------------------------------
    // Integrazione Java
    //----------------------------------------------
    
    /**
     * @brief Inizializza l'ambiente Java (JVM)
     * @param jvmPath Percorso della JVM
     * @param classPath Classpath per la JVM
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initializeJava(const std::string& jvmPath = "", const std::string& classPath = "");
    
    /**
     * @brief Carica una classe Java
     * @param className Nome completo della classe (con package)
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     */
    bool loadJavaClass(const std::string& className);
    
    /**
     * @brief Crea un'istanza di una classe Java
     * @param className Nome completo della classe (con package)
     * @param args Argomenti del costruttore come stringa in formato JSON
     * @return ID dell'oggetto creato, -1 in caso di errore
     */
    int createJavaObject(const std::string& className, const std::string& args = "[]");
    
    /**
     * @brief Chiama un metodo di un oggetto Java
     * @param objectId ID dell'oggetto
     * @param methodName Nome del metodo
     * @param args Argomenti del metodo come stringa in formato JSON
     * @param result Riferimento in cui salvare il risultato come stringa JSON
     * @return true se la chiamata è avvenuta con successo, false altrimenti
     */
    bool callJavaMethod(int objectId, const std::string& methodName, 
                       const std::string& args, std::string& result);
    
    /**
     * @brief Chiama un metodo statico di una classe Java
     * @param className Nome completo della classe (con package)
     * @param methodName Nome del metodo
     * @param args Argomenti del metodo come stringa in formato JSON
     * @param result Riferimento in cui salvare il risultato come stringa JSON
     * @return true se la chiamata è avvenuta con successo, false altrimenti
     */
    bool callJavaStaticMethod(const std::string& className, const std::string& methodName, 
                            const std::string& args, std::string& result);
    
    //----------------------------------------------
    // Logging e gestione errori
    //----------------------------------------------
    
    /**
     * @brief Registra un messaggio di log
     * @param level Livello del log
     * @param message Messaggio da registrare
     * @param file Nome del file sorgente (opzionale)
     * @param line Numero di riga nel file sorgente (opzionale)
     * @param function Nome della funzione (opzionale)
     */
    void log(APILogLevel level, const std::string& message, const std::string& source = "", int line = 0);
    
    /**
     * @brief Registra un errore
     * @param errorMessage Messaggio di errore
     */
    void logError(const std::string& errorMessage);
    
    /**
     * @brief Registra un avviso
     * @param warningMessage Messaggio di avviso
     */
    void logWarning(const std::string& warningMessage);
    
    /**
     * @brief Registra un messaggio informativo
     * @param infoMessage Messaggio informativo
     */
    void logInfo(const std::string& infoMessage);
    
    /**
     * @brief Registra un messaggio di debug
     * @param debugMessage Messaggio di debug
     */
    void logDebug(const std::string& debugMessage);
    
    /**
     * @brief Imposta il livello minimo di log da registrare
     * @param level Livello minimo di log
     */
    void setLogLevel(APILogLevel level);
    
    //----------------------------------------------
    // Shared Memory
    //----------------------------------------------
    
    /**
     * @brief Crea una memoria condivisa
     * @param name Nome della memoria condivisa
     * @param size Dimensione della memoria condivisa
     * @return true se la creazione è avvenuta con successo, false altrimenti
     */
    bool createSharedMemory(const std::string& name, size_t size);
    
    /**
     * @brief Rilascia una memoria condivisa
     * @param name Nome della memoria condivisa
     * @return true se la rilascio è avvenuto con successo, false altrimenti
     */
    bool releaseSharedMemory(const std::string& name);
    
    //----------------------------------------------
    // Named Pipes
    //----------------------------------------------
    
    /**
     * @brief Crea una named pipe
     * @param name Nome della named pipe
     * @return true se la creazione è avvenuta con successo, false altrimenti
     */
    bool createNamedPipe(const std::string& name);
    
    /**
     * @brief Scrive dati su una named pipe
     * @param name Nome della named pipe
     * @param data Dati da scrivere
     * @param size Dimensione dei dati
     * @return true se la scrittura è avvenuta con successo, false altrimenti
     */
    bool writeToNamedPipe(const std::string& name, const void* data, size_t size);
    
    /**
     * @brief Legge dati da una named pipe
     * @param name Nome della named pipe
     * @param buffer Buffer in cui leggere i dati
     * @param size Dimensione del buffer
     * @return true se la lettura è avvenuta con successo, false altrimenti
     */
    bool readFromNamedPipe(const std::string& name, void* buffer, size_t size);
    
    /**
     * @brief Chiude una named pipe
     * @param name Nome della named pipe
     * @return true se la chiusura è avvenuta con successo, false altrimenti
     */
    bool closeNamedPipe(const std::string& name);
    
    //----------------------------------------------
    // Sockets
    //----------------------------------------------
    
    /**
     * @brief Crea una socket
     * @param address Indirizzo della socket
     * @param port Porta della socket
     * @return true se la creazione è avvenuta con successo, false altrimenti
     */
    bool createSocket(const std::string& address, int port);
    
    /**
     * @brief Scrive dati su una socket
     * @param address Indirizzo della socket
     * @param data Dati da scrivere
     * @param size Dimensione dei dati
     * @return true se la scrittura è avvenuta con successo, false altrimenti
     */
    bool writeToSocket(const std::string& address, const void* data, size_t size);
    
    /**
     * @brief Legge dati da una socket
     * @param address Indirizzo della socket
     * @param buffer Buffer in cui leggere i dati
     * @param size Dimensione del buffer
     * @return true se la lettura è avvenuta con successo, false altrimenti
     */
    bool readFromSocket(const std::string& address, void* buffer, size_t size);
    
    /**
     * @brief Chiude una socket
     * @param address Indirizzo della socket
     * @return true se la chiusura è avvenuta con successo, false altrimenti
     */
    bool closeSocket(const std::string& address);
    
    //----------------------------------------------
    // Message Queue
    //----------------------------------------------
    
    /**
     * @brief Crea una coda di messaggi
     * @param name Nome della coda di messaggi
     * @return true se la creazione è avvenuta con successo, false altrimenti
     */
    bool createMessageQueue(const std::string& name);
    
    /**
     * @brief Invia un messaggio a una coda di messaggi
     * @param name Nome della coda di messaggi
     * @param data Dati da inviare
     * @param size Dimensione dei dati
     * @return true se l'invio è avvenuto con successo, false altrimenti
     */
    bool sendMessage(const std::string& name, const void* data, size_t size);
    
    /**
     * @brief Riceve un messaggio da una coda di messaggi
     * @param name Nome della coda di messaggi
     * @param buffer Buffer in cui leggere il messaggio
     * @param size Dimensione del buffer
     * @return true se la ricezione è avvenuta con successo, false altrimenti
     */
    bool receiveMessage(const std::string& name, void* buffer, size_t size);
    
    /**
     * @brief Chiude una coda di messaggi
     * @param name Nome della coda di messaggi
     * @return true se la chiusura è avvenuta con successo, false altrimenti
     */
    bool closeMessageQueue(const std::string& name);
    
    //----------------------------------------------
    // Message Callbacks
    //----------------------------------------------
    
    /**
     * @brief Unregistra una callback per i messaggi ricevuti
     * @param channelName Nome del canale IPC
     * @param callbackId ID della callback da unregistrar
     * @return true se la unregistrazione è avvenuta con successo, false altrimenti
     */
    bool unregisterMessageCallback(const std::string& channelName, int callbackId);
    
    //----------------------------------------------
    // Logging
    //----------------------------------------------
    
    /**
     * @brief Ottiene il buffer di log corrente
     * @return Vector contenente i messaggi di log
     */
    std::vector<std::string> getLogBuffer() const;
    
    /**
     * @brief Pulisce il buffer di log
     */
    void clearLogBuffer();
    
    //----------------------------------------------
    // Static logging methods
    //----------------------------------------------
    
    /**
     * @brief Registra un messaggio di debug
     * @param message Messaggio di debug
     * @param file Nome del file sorgente (opzionale)
     * @param line Numero di riga nel file sorgente (opzionale)
     * @param function Nome della funzione (opzionale)
     */
    void debug(const std::string& message, const char* file = nullptr, int line = 0, const char* function = nullptr);
    
    /**
     * @brief Registra un messaggio informativo
     * @param message Messaggio informativo
     * @param file Nome del file sorgente (opzionale)
     * @param line Numero di riga nel file sorgente (opzionale)
     * @param function Nome della funzione (opzionale)
     */
    void info(const std::string& message, const char* file = nullptr, int line = 0, const char* function = nullptr);
    
    /**
     * @brief Registra un messaggio di avviso
     * @param message Messaggio di avviso
     * @param file Nome del file sorgente (opzionale)
     * @param line Numero di riga nel file sorgente (opzionale)
     * @param function Nome della funzione (opzionale)
     */
    void warning(const std::string& message, const char* file = nullptr, int line = 0, const char* function = nullptr);
    
    /**
     * @brief Registra un messaggio di errore
     * @param message Messaggio di errore
     * @param file Nome del file sorgente (opzionale)
     * @param line Numero di riga nel file sorgente (opzionale)
     * @param function Nome della funzione (opzionale)
     */
    void error(const std::string& message, const char* file = nullptr, int line = 0, const char* function = nullptr);
    
    /**
     * @brief Registra un messaggio di errore fatale
     * @param message Messaggio di errore fatale
     * @param file Nome del file sorgente (opzionale)
     * @param line Numero di riga nel file sorgente (opzionale)
     * @param function Nome della funzione (opzionale)
     */
    void fatal(const std::string& message, const char* file = nullptr, int line = 0, const char* function = nullptr);
    
    /**
     * @brief Ottiene il livello minimo di log da registrare
     * @return Livello minimo di log
     */
    APILogLevel getLogLevel() const;

    //----------------------------------------------
    // Gestione risorse
    //----------------------------------------------
    
    /**
     * @brief Ottiene le informazioni sulle risorse di sistema
     * @return Struct contenente le informazioni sulle risorse
     */
    APISystemResources getSystemResources() const;
    
    /**
     * @brief Ottiene la memoria disponibile in bytes
     * @return Quantità di memoria disponibile
     */
    uint64_t getAvailableMemory() const;
    
    /**
     * @brief Ottiene l'utilizzo della CPU in percentuale
     * @return Percentuale di utilizzo della CPU
     */
    double getCpuUsage() const;
    
    /**
     * @brief Ottiene l'utilizzo della rete in percentuale
     * @return Percentuale di utilizzo della rete
     */
    double getNetworkUsage() const;
    
    /**
     * @brief Ottiene l'utilizzo della GPU in percentuale
     * @return Percentuale di utilizzo della GPU
     */
    double getGpuUsage() const;
    
    /**
     * @brief Ottiene la memoria disponibile su disco in bytes
     * @return Quantità di memoria disponibile su disco
     */
    uint64_t getAvailableDisk() const;
    
    /**
     * @brief Registra una callback per il monitoraggio delle risorse
     * @param resourceType Tipo di risorsa da monitorare
     * @param thresholdPercent Soglia percentuale per la notifica
     * @param callback Funzione di callback da chiamare
     * @return ID della callback registrata, -1 in caso di errore
     */
    int registerResourceCallback(const std::string& resourceType, int thresholdPercent, APIResourceCallback callback);
    
    /**
     * @brief Ottiene l'utilizzo della memoria in percentuale
     * @return Percentuale di utilizzo della memoria
     */
    double getMemoryUsage() const;
    
    /**
     * @brief Ottiene l'utilizzo del disco in percentuale
     * @return Percentuale di utilizzo del disco
     */
    double getDiskUsage() const;

    //----------------------------------------------
    // Monitoraggio
    //----------------------------------------------

    /**
     * @brief Ottiene l'utilizzo delle risorse di sistema
     * @return Struct contenente le informazioni sull'utilizzo delle risorse
     */
    APISystemResources getSystemResourceUsage();
    
    // IPC Methods
    bool initializeIPC();
    void closeIPC();
    bool isIPCChannelOpen(const std::string& channelName) const;
    bool sendIPCData(const std::string& data);
    std::string receiveIPCData();
    int registerIPCCallback(const std::string& messageName, std::function<void(const std::string&)> callback);
    bool unregisterIPCCallback(const std::string& messageName, int callbackId);

private:
    class CoreAPIImpl {
    public:
        CoreAPIImpl();
        ~CoreAPIImpl();

        std::shared_ptr<CoreNS::Core> m_core;
        std::shared_ptr<CoreNS::ConfigManager> m_configManager;
        std::shared_ptr<CoreNS::ResourceManager> m_resourceManager;
        std::shared_ptr<CoreNS::ModuleManager> m_moduleManager;
        std::shared_ptr<CoreNS::ErrorHandler> m_errorHandler;
        std::shared_ptr<CoreNS::IPCManager> m_ipcManager;
        std::shared_ptr<CoreNS::SystemMonitor> m_systemMonitor;
        std::unordered_map<std::string, std::vector<std::pair<int, APIResourceCallback>>> m_resourceCallbacks;
#ifdef USE_PYTHON_BINDINGS
        std::unique_ptr<PythonScriptingEngine> m_pythonEngine;
#endif
    };
    std::unique_ptr<CoreAPIImpl> m_impl;
};

} // namespace CoreNS