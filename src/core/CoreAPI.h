#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

/**
 * @brief Livelli di log supportati dall'API
 */
enum class APILogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
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
    SOCKET,
    MESSAGE_QUEUE
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
using APIResourceCallback = std::function<void(int currentUsage)>;

/**
 * @brief Struct che contiene informazioni sulle risorse di sistema
 */
struct APISystemResources {
    int cpuUsagePercent;
    uint64_t availableMemoryBytes;
    uint64_t totalMemoryBytes;
    uint64_t availableDiskBytes;
    uint64_t totalDiskBytes;
    int networkUsagePercent;
    int gpuUsagePercent;
};

/**
 * @brief Struct che contiene informazioni su un modulo
 */
struct APIModuleInfo {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    APIModuleType type;
    std::vector<std::string> dependencies;
    bool isLoaded;
};

// Forward declaration della classe nascosta che implementa l'API
class CoreAPIImpl;

/**
 * @brief API semplificata per interagire con il Core
 */
class CoreAPI {
public:
    /**
     * @brief Costruttore dell'API
     * @param configPath Percorso del file di configurazione
     */
    CoreAPI(const std::string& configPath = "config.json");
    
    /**
     * @brief Distruttore dell'API
     */
    ~CoreAPI();
    
    //----------------------------------------------
    // Funzioni di inizializzazione e gestione
    //----------------------------------------------
    
    /**
     * @brief Inizializza il Core e tutti i suoi componenti
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize();
    
    /**
     * @brief Arresta il Core e libera le risorse
     */
    void shutdown();
    
    //----------------------------------------------
    // Gestione configurazioni
    //----------------------------------------------
    
    /**
     * @brief Carica una configurazione da file
     * @param filePath Percorso del file di configurazione
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     */
    bool loadConfig(const std::string& filePath);
    
    /**
     * @brief Salva la configurazione corrente su file
     * @param filePath Percorso del file (se vuoto, usa l'ultimo file caricato)
     * @return true se il salvataggio è avvenuto con successo, false altrimenti
     */
    bool saveConfig(const std::string& filePath = "");
    
    /**
     * @brief Ottiene un valore di configurazione come stringa
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore della configurazione
     */
    std::string getConfigString(const std::string& key, const std::string& defaultValue = "");
    
    /**
     * @brief Ottiene un valore di configurazione come intero
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore della configurazione
     */
    int getConfigInt(const std::string& key, int defaultValue = 0);
    
    /**
     * @brief Ottiene un valore di configurazione come double
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore della configurazione
     */
    double getConfigDouble(const std::string& key, double defaultValue = 0.0);
    
    /**
     * @brief Ottiene un valore di configurazione come boolean
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore della configurazione
     */
    bool getConfigBool(const std::string& key, bool defaultValue = false);
    
    /**
     * @brief Imposta un valore di configurazione
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param value Valore da impostare
     */
    void setConfig(const std::string& key, const std::string& value);
    
    /**
     * @brief Imposta un valore di configurazione
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param value Valore da impostare
     */
    void setConfig(const std::string& key, int value);
    
    /**
     * @brief Imposta un valore di configurazione
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param value Valore da impostare
     */
    void setConfig(const std::string& key, double value);
    
    /**
     * @brief Imposta un valore di configurazione
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param value Valore da impostare
     */
    void setConfig(const std::string& key, bool value);
    
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
     * @param moduleName Nome del modulo da scaricare
     * @return true se lo scaricamento è avvenuto con successo, false altrimenti
     */
    bool unloadModule(const std::string& moduleName);
    
    /**
     * @brief Verifica se un modulo è caricato
     * @param moduleName Nome del modulo
     * @return true se il modulo è caricato, false altrimenti
     */
    bool isModuleLoaded(const std::string& moduleName) const;
    
    /**
     * @brief Ottiene le informazioni di un modulo
     * @param moduleName Nome del modulo
     * @return Informazioni sul modulo
     */
    APIModuleInfo getModuleInfo(const std::string& moduleName) const;
    
    /**
     * @brief Ottiene tutti i moduli caricati
     * @return Vector contenente i nomi dei moduli caricati
     */
    std::vector<std::string> getLoadedModules() const;
    
    //----------------------------------------------
    // Logging e gestione errori
    //----------------------------------------------
    
    /**
     * @brief Registra un messaggio di log
     * @param level Livello del log
     * @param message Messaggio da registrare
     * @param sourceFile Nome del file sorgente (opzionale)
     * @param lineNumber Numero di riga nel file sorgente (opzionale)
     * @param functionName Nome della funzione (opzionale)
     */
    void log(APILogLevel level, const std::string& message, 
            const std::string& sourceFile = "", int lineNumber = 0, 
            const std::string& functionName = "");
    
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
    // Gestione risorse
    //----------------------------------------------
    
    /**
     * @brief Ottiene le informazioni sulle risorse di sistema
     * @return Struct contenente informazioni sulle risorse
     */
    APISystemResources getSystemResources() const;
    
    /**
     * @brief Ottiene la quantità di memoria disponibile
     * @return Memoria disponibile in bytes
     */
    uint64_t getAvailableMemory() const;
    
    /**
     * @brief Ottiene l'utilizzo della CPU
     * @return Percentuale di utilizzo della CPU (0-100)
     */
    int getCpuUsage() const;
    
    /**
     * @brief Registra una callback da eseguire quando l'utilizzo di una risorsa supera una soglia
     * @param resourceType Tipo di risorsa da monitorare (cpu, memory, disk, network, gpu)
     * @param thresholdPercent Soglia percentuale (0-100)
     * @param callback Funzione da eseguire quando la soglia viene superata
     * @return ID della callback registrata
     */
    int registerResourceCallback(const std::string& resourceType, int thresholdPercent, 
                               APIResourceCallback callback);
    
    //----------------------------------------------
    // Comunicazione IPC
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
     * @brief Importa un modulo Python
     * @param moduleName Nome del modulo da importare
     * @return true se l'importazione è avvenuta con successo, false altrimenti
     */
    bool importPythonModule(const std::string& moduleName);
    
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
    
private:
    // Implementazione nascosta (Pimpl idiom)
    std::unique_ptr<CoreAPIImpl> m_impl;
}; 