#include "CoreAPI.h"
#include "CoreClass/Core.h"
#include "CoreClass/ConfigManager.h"
#include "CoreClass/ResourceManager.h"
#include "CoreClass/ModuleLoader.h"
#include "CoreClass/ErrorHandler.h"
#include "CoreClass/IPCManager.h"
#include "bindings/bridge/PythonBridge/PythonBridge.h"
#include "bindings/bridge/JavaBridge/JavaBridge.h"

#include <unordered_map>
#include <iostream>
#include <json.hpp> // Assumiamo l'uso di nlohmann/json per la gestione JSON

// Alias per JSON library
using json = nlohmann::json;

/**
 * @brief Classe di implementazione nascosta (Pimpl idiom)
 */
class CoreAPIImpl {
public:
    CoreAPIImpl(const std::string& configPath)
        : m_core(configPath), 
          m_resourceCallbacks(), 
          m_lastCallbackId(0) {
    }

    ~CoreAPIImpl() {
        // Assicuriamoci che il core sia shutdown in caso non lo sia già
        if (m_isInitialized) {
            m_core.shutdown();
        }
    }

    // Puntatore al core
    Core m_core;
    
    // Stato dell'inizializzazione
    bool m_isInitialized = false;
    
    // Cache per le callback sulle risorse
    std::unordered_map<int, std::pair<std::string, APIResourceCallback>> m_resourceCallbacks;
    int m_lastCallbackId;
    
    // Converte un enum interno in un enum API
    APILogLevel convertToAPILogLevel(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return APILogLevel::DEBUG;
            case LogLevel::INFO: return APILogLevel::INFO;
            case LogLevel::WARNING: return APILogLevel::WARNING;
            case LogLevel::ERROR: return APILogLevel::ERROR;
            case LogLevel::FATAL: return APILogLevel::FATAL;
            default: return APILogLevel::INFO;
        }
    }
    
    // Converte un enum API in un enum interno
    LogLevel convertFromAPILogLevel(APILogLevel level) {
        switch (level) {
            case APILogLevel::DEBUG: return LogLevel::DEBUG;
            case APILogLevel::INFO: return LogLevel::INFO;
            case APILogLevel::WARNING: return LogLevel::WARNING;
            case APILogLevel::ERROR: return LogLevel::ERROR;
            case APILogLevel::FATAL: return LogLevel::FATAL;
            default: return LogLevel::INFO;
        }
    }
    
    // Converte un ModuleType in APIModuleType
    APIModuleType convertToAPIModuleType(ModuleType type) {
        switch (type) {
            case ModuleType::CPP: return APIModuleType::CPP;
            case ModuleType::PYTHON: return APIModuleType::PYTHON;
            case ModuleType::JAVA: return APIModuleType::JAVA;
            default: return APIModuleType::CPP;
        }
    }
    
    // Converte un IPCType in APIIPCType
    APIIPCType convertToAPIIPCType(IPCType type) {
        switch (type) {
            case IPCType::SHARED_MEMORY: return APIIPCType::SHARED_MEMORY;
            case IPCType::NAMED_PIPE: return APIIPCType::NAMED_PIPE;
            case IPCType::SOCKET: return APIIPCType::SOCKET;
            case IPCType::MESSAGE_QUEUE: return APIIPCType::MESSAGE_QUEUE;
            default: return APIIPCType::SHARED_MEMORY;
        }
    }
    
    // Converte un APIIPCType in IPCType
    IPCType convertFromAPIIPCType(APIIPCType type) {
        switch (type) {
            case APIIPCType::SHARED_MEMORY: return IPCType::SHARED_MEMORY;
            case APIIPCType::NAMED_PIPE: return IPCType::NAMED_PIPE;
            case APIIPCType::SOCKET: return IPCType::SOCKET;
            case APIIPCType::MESSAGE_QUEUE: return IPCType::MESSAGE_QUEUE;
            default: return IPCType::SHARED_MEMORY;
        }
    }
    
    // Converte un APIIPCRole in IPCRole
    IPCRole convertFromAPIIPCRole(APIIPCRole role) {
        switch (role) {
            case APIIPCRole::SERVER: return IPCRole::SERVER;
            case APIIPCRole::CLIENT: return IPCRole::CLIENT;
            default: return IPCRole::SERVER;
        }
    }
    
    // Converte una stringa in ResourceType
    ResourceType getResourceTypeFromString(const std::string& resourceType) {
        if (resourceType == "cpu") return ResourceType::CPU;
        if (resourceType == "memory") return ResourceType::MEMORY;
        if (resourceType == "disk") return ResourceType::DISK;
        if (resourceType == "network") return ResourceType::NETWORK;
        if (resourceType == "gpu") return ResourceType::GPU;
        
        // Default a CPU se non riconosciuto
        return ResourceType::CPU;
    }

    // Converte un ModuleInfo in APIModuleInfo
    APIModuleInfo convertToAPIModuleInfo(const ModuleInfo* info) {
        APIModuleInfo apiInfo;
        if (info) {
            apiInfo.name = info->name;
            apiInfo.version = info->version;
            apiInfo.author = info->author;
            apiInfo.description = info->description;
            apiInfo.type = convertToAPIModuleType(info->type);
            apiInfo.dependencies = info->dependencies;
            apiInfo.isLoaded = info->isLoaded;
        }
        return apiInfo;
    }
    
    // Converte un SystemResources in APISystemResources
    APISystemResources convertToAPISystemResources(const SystemResources& res) {
        APISystemResources apiRes;
        apiRes.cpuUsagePercent = res.cpuUsagePercent;
        apiRes.availableMemoryBytes = res.availableMemoryBytes;
        apiRes.totalMemoryBytes = res.totalMemoryBytes;
        apiRes.availableDiskBytes = res.availableDiskBytes;
        apiRes.totalDiskBytes = res.totalDiskBytes;
        apiRes.networkUsagePercent = res.networkUsagePercent;
        apiRes.gpuUsagePercent = res.gpuUsagePercent;
        return apiRes;
    }
};

//------------------------------------------------------------------
// Implementazione di CoreAPI
//------------------------------------------------------------------

CoreAPI::CoreAPI(const std::string& configPath)
    : m_impl(std::make_unique<CoreAPIImpl>(configPath)) {
}

CoreAPI::~CoreAPI() {
    // L'implementazione Pimpl si occupa di gestire la pulizia
}

bool CoreAPI::initialize() {
    if (!m_impl->m_isInitialized) {
        m_impl->m_isInitialized = m_impl->m_core.initialize();
    }
    return m_impl->m_isInitialized;
}

void CoreAPI::shutdown() {
    if (m_impl->m_isInitialized) {
        m_impl->m_core.shutdown();
        m_impl->m_isInitialized = false;
    }
}

//------------------------------------------------------------------
// Gestione configurazioni
//------------------------------------------------------------------

bool CoreAPI::loadConfig(const std::string& filePath) {
    return m_impl->m_core.getConfigManager().load(filePath);
}

bool CoreAPI::saveConfig(const std::string& filePath) {
    return m_impl->m_core.getConfigManager().save(filePath);
}

std::string CoreAPI::getConfigString(const std::string& key, const std::string& defaultValue) {
    return m_impl->m_core.getConfigManager().get<std::string>(key, defaultValue);
}

int CoreAPI::getConfigInt(const std::string& key, int defaultValue) {
    return m_impl->m_core.getConfigManager().get<int>(key, defaultValue);
}

double CoreAPI::getConfigDouble(const std::string& key, double defaultValue) {
    return m_impl->m_core.getConfigManager().get<double>(key, defaultValue);
}

bool CoreAPI::getConfigBool(const std::string& key, bool defaultValue) {
    return m_impl->m_core.getConfigManager().get<bool>(key, defaultValue);
}

void CoreAPI::setConfig(const std::string& key, const std::string& value) {
    m_impl->m_core.getConfigManager().set(key, value);
}

void CoreAPI::setConfig(const std::string& key, int value) {
    m_impl->m_core.getConfigManager().set(key, value);
}

void CoreAPI::setConfig(const std::string& key, double value) {
    m_impl->m_core.getConfigManager().set(key, value);
}

void CoreAPI::setConfig(const std::string& key, bool value) {
    m_impl->m_core.getConfigManager().set(key, value);
}

//------------------------------------------------------------------
// Gestione moduli
//------------------------------------------------------------------

bool CoreAPI::loadModule(const std::string& modulePath) {
    return m_impl->m_core.getModuleLoader().load(modulePath);
}

bool CoreAPI::unloadModule(const std::string& moduleName) {
    return m_impl->m_core.getModuleLoader().unload(moduleName);
}

bool CoreAPI::isModuleLoaded(const std::string& moduleName) const {
    return m_impl->m_core.getModuleLoader().isLoaded(moduleName);
}

APIModuleInfo CoreAPI::getModuleInfo(const std::string& moduleName) const {
    const ModuleInfo* info = m_impl->m_core.getModuleLoader().getModuleInfo(moduleName);
    return m_impl->convertToAPIModuleInfo(info);
}

std::vector<std::string> CoreAPI::getLoadedModules() const {
    return m_impl->m_core.getModuleLoader().getLoadedModules();
}

//------------------------------------------------------------------
// Logging e gestione errori
//------------------------------------------------------------------

void CoreAPI::log(APILogLevel level, const std::string& message, 
                 const std::string& sourceFile, int lineNumber, 
                 const std::string& functionName) {
    m_impl->m_core.getErrorHandler().log(
        m_impl->convertFromAPILogLevel(level),
        message,
        sourceFile,
        lineNumber,
        functionName
    );
}

void CoreAPI::logError(const std::string& errorMessage) {
    ErrorHandler::logError(errorMessage);
}

void CoreAPI::logWarning(const std::string& warningMessage) {
    ErrorHandler::logWarning(warningMessage);
}

void CoreAPI::logInfo(const std::string& infoMessage) {
    ErrorHandler::logInfo(infoMessage);
}

void CoreAPI::logDebug(const std::string& debugMessage) {
    ErrorHandler::logDebug(debugMessage);
}

void CoreAPI::setLogLevel(APILogLevel level) {
    m_impl->m_core.getErrorHandler().setLogLevel(m_impl->convertFromAPILogLevel(level));
}

//------------------------------------------------------------------
// Gestione risorse
//------------------------------------------------------------------

APISystemResources CoreAPI::getSystemResources() const {
    SystemResources res = m_impl->m_core.getResourceManager().getSystemResources();
    return m_impl->convertToAPISystemResources(res);
}

uint64_t CoreAPI::getAvailableMemory() const {
    return m_impl->m_core.getResourceManager().getAvailableMemory();
}

int CoreAPI::getCpuUsage() const {
    return m_impl->m_core.getResourceManager().getCpuUsage();
}

int CoreAPI::registerResourceCallback(const std::string& resourceType, int thresholdPercent, 
                                     APIResourceCallback callback) {
    // Convertire il tipo di risorsa
    ResourceType type = m_impl->getResourceTypeFromString(resourceType);
    
    // Wrapper per la callback
    auto wrapper = [callback](int usage) {
        callback(usage);
    };
    
    // Registrare la callback nel ResourceManager
    int id = m_impl->m_core.getResourceManager().registerThresholdCallback(type, thresholdPercent, wrapper);
    
    // Memorizzare la callback nel nostro registro interno per gestione memoria
    if (id >= 0) {
        m_impl->m_resourceCallbacks[id] = std::make_pair(resourceType, callback);
    }
    
    return id;
}

//------------------------------------------------------------------
// Comunicazione IPC
//------------------------------------------------------------------

bool CoreAPI::createIPCChannel(const std::string& name, APIIPCType type, APIIPCRole role, 
                              const std::string& params) {
    IPCType ipcType = m_impl->convertFromAPIIPCType(type);
    IPCRole ipcRole = m_impl->convertFromAPIIPCRole(role);
    
    IPCManager& ipcManager = m_impl->m_core.getIPCManager();
    
    switch (ipcType) {
        case IPCType::SHARED_MEMORY: {
            size_t size = 1024; // Default size
            if (!params.empty()) {
                try {
                    size = std::stoul(params);
                } catch (...) {
                    // Uso default se params non è un numero valido
                }
            }
            return ipcManager.createSharedMemory(name, size);
        }
        
        case IPCType::NAMED_PIPE: {
            return ipcManager.createNamedPipe(name, ipcRole);
        }
        
        case IPCType::SOCKET: {
            std::string host = "";
            int port = 0;
            
            if (!params.empty()) {
                // Formato previsto: "host:port"
                size_t pos = params.find(':');
                if (pos != std::string::npos) {
                    host = params.substr(0, pos);
                    try {
                        port = std::stoi(params.substr(pos + 1));
                    } catch (...) {
                        // Usa default se non valido
                    }
                }
            }
            
            return ipcManager.createSocket(name, ipcRole, host, port);
        }
        
        case IPCType::MESSAGE_QUEUE: {
            return ipcManager.createMessageQueue(name);
        }
        
        default:
            return false;
    }
}

bool CoreAPI::sendData(const std::string& channelName, const void* data, size_t dataSize) {
    IPCManager& ipcManager = m_impl->m_core.getIPCManager();
    
    // Determiniamo automaticamente il tipo di canale
    IPCChannelInfo* channelInfo = nullptr;
    // Nota: Assumiamo che esista un metodo per ottenere le informazioni sul canale
    // In un'implementazione reale, questo dovrebbe essere corretto per funzionare
    
    if (channelInfo) {
        switch (channelInfo->type) {
            case IPCType::SHARED_MEMORY: {
                void* sharedMem = ipcManager.getSharedMemory(channelName);
                if (sharedMem && dataSize <= channelInfo->size) {
                    std::memcpy(sharedMem, data, dataSize);
                    return true;
                }
                return false;
            }
            
            case IPCType::NAMED_PIPE:
                return ipcManager.writeToNamedPipe(channelName, data, dataSize);
                
            case IPCType::SOCKET:
                return ipcManager.writeToSocket(channelName, data, dataSize);
                
            case IPCType::MESSAGE_QUEUE:
                return ipcManager.sendMessage(channelName, data, dataSize);
                
            default:
                return false;
        }
    }
    
    // Se non troviamo info sul canale, proviamo tutti i metodi
    if (ipcManager.writeToNamedPipe(channelName, data, dataSize)) {
        return true;
    }
    
    if (ipcManager.writeToSocket(channelName, data, dataSize)) {
        return true;
    }
    
    return ipcManager.sendMessage(channelName, data, dataSize);
}

bool CoreAPI::receiveData(const std::string& channelName, void* buffer, size_t bufferSize, size_t& bytesRead) {
    IPCManager& ipcManager = m_impl->m_core.getIPCManager();
    
    // Simile a sendData, determiniamo il tipo di canale
    // In un'implementazione reale, questo dovrebbe essere implementato correttamente
    
    // Prova tutti i metodi di ricezione
    if (ipcManager.readFromNamedPipe(channelName, buffer, bufferSize, bytesRead)) {
        return true;
    }
    
    if (ipcManager.readFromSocket(channelName, buffer, bufferSize, bytesRead)) {
        return true;
    }
    
    return ipcManager.receiveMessage(channelName, buffer, bufferSize, bytesRead);
}

int CoreAPI::registerMessageCallback(const std::string& channelName, APIMessageCallback callback) {
    IPCManager& ipcManager = m_impl->m_core.getIPCManager();
    
    // Convertiamo la callback API nella callback interna
    IPCMessageCallback internalCallback = [callback](const std::string& name, const void* data, size_t size) {
        callback(name, data, size);
    };
    
    return ipcManager.registerMessageCallback(channelName, internalCallback);
}

bool CoreAPI::closeIPCChannel(const std::string& channelName) {
    IPCManager& ipcManager = m_impl->m_core.getIPCManager();
    
    // Prova tutti i metodi di chiusura
    if (ipcManager.releaseSharedMemory(channelName)) {
        return true;
    }
    
    if (ipcManager.closeNamedPipe(channelName)) {
        return true;
    }
    
    if (ipcManager.closeSocket(channelName)) {
        return true;
    }
    
    return ipcManager.closeMessageQueue(channelName);
}

//------------------------------------------------------------------
// Integrazione Python
//------------------------------------------------------------------

bool CoreAPI::initializePython(const std::string& pythonHome) {
    // Nota: poiché PythonBridge non è un membro diretto di Core,
    // assumiamo che sia accessibile tramite ModuleLoader o un altro meccanismo
    // Questo dovrebbe essere adattato all'implementazione reale
    
    // Esempio: ottieni l'istanza di PythonBridge
    // PythonBridge* pythonBridge = getPythonBridge();
    
    // Per ora, forniamo un'implementazione finta
    if (pythonHome.empty()) {
        logInfo("Inizializzazione Python con Python Home di default");
    } else {
        logInfo("Inizializzazione Python con Python Home: " + pythonHome);
    }
    
    return true;
}

bool CoreAPI::importPythonModule(const std::string& moduleName) {
    // Simile a initializePython, qui dovremmo avere accesso a PythonBridge
    
    logInfo("Importazione modulo Python: " + moduleName);
    return true;
}

bool CoreAPI::executePythonFunction(const std::string& moduleName, const std::string& funcName, 
                                   const std::string& args) {
    // Qui dovremmo interpretare gli argomenti JSON e passarli alla funzione Python
    
    try {
        // Parsificare gli argomenti JSON
        json jsonArgs = json::parse(args);
        
        // Chiamare la funzione Python (implementazione dimostrativa)
        logInfo("Esecuzione funzione Python: " + moduleName + "." + funcName);
        
        return true;
    } catch (const json::exception& e) {
        logError("Errore nel parsing JSON degli argomenti: " + std::string(e.what()));
        return false;
    } catch (...) {
        logError("Errore sconosciuto nell'esecuzione della funzione Python");
        return false;
    }
}

bool CoreAPI::executePythonCode(const std::string& code) {
    // Eseguire il codice Python attraverso PythonBridge
    
    logInfo("Esecuzione codice Python");
    return true;
}

//------------------------------------------------------------------
// Integrazione Java
//------------------------------------------------------------------

bool CoreAPI::initializeJava(const std::string& jvmPath, const std::string& classPath) {
    // Nota: poiché JavaBridge non è un membro diretto di Core,
    // assumiamo che sia accessibile tramite ModuleLoader o un altro meccanismo
    // Questo dovrebbe essere adattato all'implementazione reale
    
    // Esempio: ottieni l'istanza di JavaBridge
    // JavaBridge* javaBridge = getJavaBridge();
    
    // Per ora, forniamo un'implementazione finta
    if (jvmPath.empty()) {
        logInfo("Inizializzazione JVM con percorso di default");
    } else {
        logInfo("Inizializzazione JVM con percorso: " + jvmPath);
    }
    
    if (!classPath.empty()) {
        logInfo("ClassPath specificato: " + classPath);
    }
    
    return true;
}

bool CoreAPI::loadJavaClass(const std::string& className) {
    // Simile a initializeJava, qui dovremmo avere accesso a JavaBridge
    
    logInfo("Caricamento classe Java: " + className);
    return true;
}

int CoreAPI::createJavaObject(const std::string& className, const std::string& args) {
    // Qui dovremmo interpretare gli argomenti JSON e passarli al costruttore Java
    
    try {
        // Parsificare gli argomenti JSON
        json jsonArgs = json::parse(args);
        
        // Creare l'oggetto Java (implementazione dimostrativa)
        logInfo("Creazione oggetto Java: " + className);
        
        // Restituire un ID finto per l'oggetto
        return 1;
    } catch (const json::exception& e) {
        logError("Errore nel parsing JSON degli argomenti: " + std::string(e.what()));
        return -1;
    } catch (...) {
        logError("Errore sconosciuto nella creazione dell'oggetto Java");
        return -1;
    }
}

bool CoreAPI::callJavaMethod(int objectId, const std::string& methodName, 
                           const std::string& args, std::string& result) {
    // Qui dovremmo interpretare gli argomenti JSON e passarli al metodo Java
    
    try {
        // Parsificare gli argomenti JSON
        json jsonArgs = json::parse(args);
        
        // Chiamare il metodo Java (implementazione dimostrativa)
        logInfo("Chiamata metodo Java: " + methodName + " sull'oggetto " + std::to_string(objectId));
        
        // Imposta un risultato di prova
        result = R"({"status": "success", "result": null})";
        
        return true;
    } catch (const json::exception& e) {
        logError("Errore nel parsing JSON degli argomenti: " + std::string(e.what()));
        return false;
    } catch (...) {
        logError("Errore sconosciuto nella chiamata del metodo Java");
        return false;
    }
}

bool CoreAPI::callJavaStaticMethod(const std::string& className, const std::string& methodName, 
                                 const std::string& args, std::string& result) {
    // Qui dovremmo interpretare gli argomenti JSON e passarli al metodo statico Java
    
    try {
        // Parsificare gli argomenti JSON
        json jsonArgs = json::parse(args);
        
        // Chiamare il metodo statico Java (implementazione dimostrativa)
        logInfo("Chiamata metodo statico Java: " + className + "." + methodName);
        
        // Imposta un risultato di prova
        result = R"({"status": "success", "result": null})";
        
        return true;
    } catch (const json::exception& e) {
        logError("Errore nel parsing JSON degli argomenti: " + std::string(e.what()));
        return false;
    } catch (...) {
        logError("Errore sconosciuto nella chiamata del metodo statico Java");
        return false;
    }
} 