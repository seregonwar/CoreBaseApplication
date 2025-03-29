#include "CoreAPI.h"
#include "CoreClass/Core.h"
#include "CoreClass/ConfigManager.h"
#include "CoreClass/ResourceManager.h"
#include "CoreClass/ModuleManager.h"
#include "CoreClass/ErrorHandler.h"
#include "CoreClass/IPCManager.h"
#include "bindings/bridge/PythonBridge/PythonBridge.h"
#include "bindings/bridge/JavaBridge/JavaBridge.h"
#include <nlohmann/json.hpp>
#include "CoreClass/LogManager.h"

using json = nlohmann::json;

#include <unordered_map>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace CoreNS {

CoreAPI::CoreAPIImpl::CoreAPIImpl() : m_core(nullptr), m_configManager(nullptr), m_resourceManager(nullptr),
                   m_moduleManager(nullptr), m_errorHandler(nullptr), m_ipcManager(nullptr) {
    m_core = std::make_shared<Core>();
    if (m_core) {
        m_configManager = m_core->getConfigManager();
        m_resourceManager = m_core->getResourceManager();
        m_moduleManager = m_core->getModuleManager();
        m_errorHandler = m_core->getErrorHandler();
        m_ipcManager = m_core->getIPCManager();
    }
}

CoreAPI::CoreAPIImpl::~CoreAPIImpl() = default;

// Funzioni di conversione
namespace {
    APIModuleInfo convertToAPIModuleInfo(const ModuleInfo& info) {
        APIModuleInfo apiInfo;
        apiInfo.name = info.name;
        apiInfo.version = info.version;
        apiInfo.description = info.description;
        apiInfo.author = info.author;
        apiInfo.type = std::to_string(static_cast<int>(info.type));
        apiInfo.dependencies = info.dependencies;
        apiInfo.isLoaded = info.isLoaded;
        return apiInfo;
    }

    APISystemResources convertToAPISystemResources(const SystemResources& resources) {
        APISystemResources apiResources;
        apiResources.cpuUsagePercent = resources.cpuUsagePercent;
        apiResources.availableMemoryBytes = resources.availableMemoryBytes;
        apiResources.totalMemoryBytes = resources.totalMemoryBytes;
        apiResources.availableDiskBytes = resources.availableDiskBytes;
        apiResources.totalDiskBytes = resources.totalDiskBytes;
        apiResources.networkUsagePercent = resources.networkUsagePercent;
        apiResources.gpuUsagePercent = resources.gpuUsagePercent;
        return apiResources;
    }

    IPCRole convertFromAPIIPCRole(APIIPCRole role) {
        return static_cast<IPCRole>(role);
    }

    IPCType convertFromAPIIPCType(APIIPCType type) {
        return static_cast<IPCType>(type);
    }

    ResourceType getResourceTypeFromString(const std::string& type) {
        if (type == "cpu") return ResourceType::CPU;
        if (type == "memory") return ResourceType::MEMORY;
        if (type == "disk") return ResourceType::DISK;
        if (type == "network") return ResourceType::NETWORK;
        if (type == "gpu") return ResourceType::GPU;
        return ResourceType::UNKNOWN;
    }
}

CoreAPI::CoreAPI() : m_impl(std::make_unique<CoreAPIImpl>()) {
}

CoreAPI::~CoreAPI() = default;

bool CoreAPI::initialize(const std::string& configPath) {
    return m_impl->m_core->initialize(configPath);
}

void CoreAPI::shutdown() {
    m_impl->m_core->shutdown();
}

bool CoreAPI::loadConfig(const std::string& filePath) {
    if (m_impl == nullptr || m_impl->m_configManager == nullptr) {
        return false;
    }
    return m_impl->m_configManager->loadConfig(filePath);
}

bool CoreAPI::saveConfig(const std::string& filePath) {
    if (m_impl == nullptr || m_impl->m_configManager == nullptr) {
        return false;
    }
    return m_impl->m_configManager->saveConfig(filePath);
}

std::string CoreAPI::getConfigString(const std::string& key, const std::string& defaultValue) {
    if (m_impl == nullptr || m_impl->m_configManager == nullptr) {
        return defaultValue;
    }
    return m_impl->m_configManager->getValue<std::string>(key, defaultValue);
}

int CoreAPI::getConfigInt(const std::string& key, int defaultValue) {
    if (m_impl == nullptr || m_impl->m_configManager == nullptr) {
        return defaultValue;
    }
    return m_impl->m_configManager->getValue<int>(key, defaultValue);
}

double CoreAPI::getConfigDouble(const std::string& key, double defaultValue) {
    if (m_impl == nullptr || m_impl->m_configManager == nullptr) {
        return defaultValue;
    }
    return m_impl->m_configManager->getValue<double>(key, defaultValue);
}

bool CoreAPI::getConfigBool(const std::string& key, bool defaultValue) {
    if (m_impl == nullptr || m_impl->m_configManager == nullptr) {
        return defaultValue;
    }
    return m_impl->m_configManager->getValue<bool>(key, defaultValue);
}

void CoreAPI::setConfig(const std::string& key, const std::string& value) {
    if (m_impl && m_impl->m_configManager) {
        m_impl->m_configManager->setValue<std::string>(key, value);
    }
}

void CoreAPI::setConfig(const std::string& key, int value) {
    if (m_impl && m_impl->m_configManager) {
        m_impl->m_configManager->setValue<int>(key, value);
    }
}

void CoreAPI::setConfig(const std::string& key, double value) {
    if (m_impl && m_impl->m_configManager) {
        m_impl->m_configManager->setValue<double>(key, value);
    }
}

void CoreAPI::setConfig(const std::string& key, bool value) {
    if (m_impl && m_impl->m_configManager) {
        m_impl->m_configManager->setValue<bool>(key, value);
    }
}

bool CoreAPI::loadModule(const std::string& moduleName) {
    if (m_impl->m_moduleManager) {
        return m_impl->m_moduleManager->loadModule(moduleName);
    }
    return false;
}

bool CoreAPI::unloadModule(const std::string& moduleName) {
    if (m_impl->m_moduleManager) {
        return m_impl->m_moduleManager->unloadModule(moduleName);
    }
    return false;
}

bool CoreAPI::isModuleLoaded(const std::string& moduleName) const {
    if (m_impl->m_moduleManager) {
        return m_impl->m_moduleManager->isModuleLoaded(moduleName);
    }
    return false;
}

std::vector<std::string> CoreAPI::getLoadedModules() const {
    if (m_impl->m_moduleManager) {
        return m_impl->m_moduleManager->getLoadedModules();
    }
    return std::vector<std::string>();
}

APIModuleInfo CoreAPI::getModuleInfo(const std::string& moduleName) const {
    APIModuleInfo info;
    if (m_impl->m_moduleManager) {
        auto moduleInfo = m_impl->m_moduleManager->getModuleInfo(moduleName);
        if (moduleInfo) {
            info.name = moduleInfo->name;
            info.version = moduleInfo->version;
            info.description = moduleInfo->description;
            info.author = moduleInfo->author;
            info.type = std::to_string(static_cast<int>(moduleInfo->type));
            info.dependencies = moduleInfo->dependencies;
            info.isLoaded = moduleInfo->isLoaded;
        }
    }
    return info;
}

void CoreAPI::log(APILogLevel level, const std::string& message, const std::string& source, int line) {
    if (m_impl && m_impl->m_errorHandler) {
        m_impl->m_errorHandler->log(static_cast<LogLevel>(level), message);
    }
}

void CoreAPI::logError(const std::string& errorMessage) {
    log(static_cast<APILogLevel>(3), errorMessage); // ERROR = 3
}

void CoreAPI::logWarning(const std::string& warningMessage) {
    log(static_cast<APILogLevel>(2), warningMessage); // WARNING = 2
}

void CoreAPI::logInfo(const std::string& infoMessage) {
    log(static_cast<APILogLevel>(1), infoMessage); // INFO = 1
}

void CoreAPI::logDebug(const std::string& debugMessage) {
    log(static_cast<APILogLevel>(0), debugMessage); // DEBUG = 0
}

void CoreAPI::setLogLevel(APILogLevel level) {
    if (m_impl && m_impl->m_errorHandler) {
        m_impl->m_errorHandler->setLogLevel(static_cast<LogLevel>(level));
    }
}

double CoreAPI::getCpuUsage() const {
    if (m_impl == nullptr || m_impl->m_resourceManager == nullptr) {
        return 0.0;
    }
    return m_impl->m_resourceManager->getCpuUsage();
}

double CoreAPI::getMemoryUsage() const {
    if (m_impl == nullptr || m_impl->m_resourceManager == nullptr) {
        return 0.0;
    }
    return m_impl->m_resourceManager->getMemoryUsage();
}

double CoreAPI::getDiskUsage() const {
    if (m_impl == nullptr || m_impl->m_resourceManager == nullptr) {
        return 0.0;
    }
    return m_impl->m_resourceManager->getDiskUsage();
}

double CoreAPI::getNetworkUsage() const {
    if (m_impl == nullptr || m_impl->m_resourceManager == nullptr) {
        return 0.0;
    }
    return m_impl->m_resourceManager->getNetworkUsage();
}

double CoreAPI::getGpuUsage() const {
    if (m_impl == nullptr || m_impl->m_resourceManager == nullptr) {
        return 0.0;
    }
    return m_impl->m_resourceManager->getGpuUsage();
}

APISystemResources CoreAPI::getSystemResources() const {
    APISystemResources resources;
    if (m_impl != nullptr && m_impl->m_resourceManager != nullptr) {
        auto sysResources = m_impl->m_resourceManager->getSystemResources();
        resources.cpuUsagePercent = sysResources.cpuUsagePercent;
        resources.availableMemoryBytes = sysResources.availableMemoryBytes;
        resources.totalMemoryBytes = sysResources.totalMemoryBytes;
        resources.availableDiskBytes = sysResources.availableDiskBytes;
        resources.totalDiskBytes = sysResources.totalDiskBytes;
        resources.networkUsagePercent = sysResources.networkUsagePercent;
        resources.gpuUsagePercent = sysResources.gpuUsagePercent;
    }
    return resources;
}

bool CoreAPI::initializeIPC() {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }

    ChannelInfo channelInfo;
    channelInfo.name = "default";
    channelInfo.type = IPCType::NAMED_PIPE;
    channelInfo.isServer = true;
    channelInfo.address = ".";
    channelInfo.port = 0;

    return m_impl->m_ipcManager->initialize(channelInfo);
}

void CoreAPI::closeIPC() {
    if (m_impl && m_impl->m_ipcManager) {
        m_impl->m_ipcManager->close();
    }
}

bool CoreAPI::isIPCChannelOpen(const std::string& channelName) const {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->isChannelOpen(channelName);
}

bool CoreAPI::createIPCChannel(const std::string& name, APIIPCType type, APIIPCRole role, const std::string& params) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }

    ChannelInfo channelInfo;
    channelInfo.name = name;
    channelInfo.type = convertFromAPIIPCType(type);
    channelInfo.isServer = (role == APIIPCRole::SERVER);
    channelInfo.address = params;
    channelInfo.port = 0;

    return m_impl->m_ipcManager->initialize(channelInfo);
}

bool CoreAPI::sendData(const std::string& channelName, const void* data, size_t dataSize) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->sendData(channelName, data, dataSize);
}

bool CoreAPI::receiveData(const std::string& channelName, void* buffer, size_t bufferSize, size_t& bytesRead) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->receiveData(channelName, buffer, bufferSize, bytesRead);
}

int CoreAPI::registerMessageCallback(const std::string& channelName, APIMessageCallback callback) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return -1;
    }
    return m_impl->m_ipcManager->registerCallback(channelName, callback);
}

bool CoreAPI::unregisterMessageCallback(const std::string& channelName, int callbackId) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->unregisterCallback(channelName, callbackId);
}

bool CoreAPI::closeIPCChannel(const std::string& channelName) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->closeChannel(channelName);
}

bool CoreAPI::sendIPCData(const std::string& data) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->sendData("default", data.c_str(), data.size());
}

std::string CoreAPI::receiveIPCData() {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return "";
    }
    char buffer[4096];
    size_t bytesRead;
    if (m_impl->m_ipcManager->receiveData("default", buffer, sizeof(buffer), bytesRead)) {
        return std::string(buffer, bytesRead);
    }
    return "";
}

int CoreAPI::registerIPCCallback(const std::string& messageName, std::function<void(const std::string&)> callback) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return -1;
    }
    return m_impl->m_ipcManager->registerCallback(messageName, 
        [callback](const std::string& channelName, const void* data, size_t size) {
            if (data && size > 0) {
                std::string message(static_cast<const char*>(data), size);
                callback(message);
            }
        });
}

bool CoreAPI::unregisterIPCCallback(const std::string& messageName, int callbackId) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->unregisterCallback(messageName, callbackId);
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
        log(static_cast<APILogLevel>(1), "Inizializzazione Python con Python Home di default"); // INFO = 1
    } else {
        log(static_cast<APILogLevel>(1), "Inizializzazione Python con Python Home: " + pythonHome); // INFO = 1
    }
    
    return true;
}

bool CoreAPI::importPythonModule(const std::string& moduleName) {
    // Simile a initializePython, qui dovremmo avere accesso a PythonBridge
    
    log(static_cast<APILogLevel>(1), "Importazione modulo Python: " + moduleName); // INFO = 1
    return true;
}

bool CoreAPI::executePythonFunction(const std::string& moduleName, const std::string& funcName, 
                                   const std::string& args) {
    // Qui dovremmo interpretare gli argomenti JSON e passarli alla funzione Python
    
    try {
        // Parsificare gli argomenti JSON
        json jsonArgs = json::parse(args);
        
        // Chiamare la funzione Python (implementazione dimostrativa)
        log(static_cast<APILogLevel>(1), "Esecuzione funzione Python: " + moduleName + "." + funcName); // INFO = 1
        
        return true;
    } catch (const json::exception& e) {
        log(static_cast<APILogLevel>(3), "Errore nel parsing JSON degli argomenti: " + std::string(e.what())); // ERROR = 3
        return false;
    } catch (...) {
        log(static_cast<APILogLevel>(3), "Errore sconosciuto nell'esecuzione della funzione Python"); // ERROR = 3
        return false;
    }
}

bool CoreAPI::executePythonCode(const std::string& code) {
    // Eseguire il codice Python attraverso PythonBridge
    
    log(static_cast<APILogLevel>(1), "Esecuzione codice Python"); // INFO = 1
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
        log(static_cast<APILogLevel>(1), "Inizializzazione JVM con percorso di default"); // INFO = 1
    } else {
        log(static_cast<APILogLevel>(1), "Inizializzazione JVM con percorso: " + jvmPath); // INFO = 1
    }
    
    if (!classPath.empty()) {
        log(static_cast<APILogLevel>(1), "ClassPath specificato: " + classPath); // INFO = 1
    }
    
    return true;
}

bool CoreAPI::loadJavaClass(const std::string& className) {
    // Simile a initializeJava, qui dovremmo avere accesso a JavaBridge
    
    log(static_cast<APILogLevel>(1), "Caricamento classe Java: " + className); // INFO = 1
    return true;
}

int CoreAPI::createJavaObject(const std::string& className, const std::string& args) {
    // Qui dovremmo interpretare gli argomenti JSON e passarli al costruttore Java
    
    try {
        // Parsificare gli argomenti JSON
        json jsonArgs = json::parse(args);
        
        // Creare l'oggetto Java (implementazione dimostrativa)
        log(static_cast<APILogLevel>(1), "Creazione oggetto Java: " + className); // INFO = 1
        
        // Restituire un ID finto per l'oggetto
        return 1;
    } catch (const json::exception& e) {
        log(static_cast<APILogLevel>(3), "Errore nel parsing JSON degli argomenti: " + std::string(e.what())); // ERROR = 3
        return -1;
    } catch (...) {
        log(static_cast<APILogLevel>(3), "Errore sconosciuto nella creazione dell'oggetto Java"); // ERROR = 3
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
        log(static_cast<APILogLevel>(1), "Chiamata metodo Java: " + methodName + " sull'oggetto " + std::to_string(objectId)); // INFO = 1
        
        // Imposta un risultato di prova
        result = R"({"status": "success", "result": null})";
        
        return true;
    } catch (const json::exception& e) {
        log(static_cast<APILogLevel>(3), "Errore nel parsing JSON degli argomenti: " + std::string(e.what())); // ERROR = 3
        return false;
    } catch (...) {
        log(static_cast<APILogLevel>(3), "Errore sconosciuto nella chiamata del metodo Java"); // ERROR = 3
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
        log(static_cast<APILogLevel>(1), "Chiamata metodo statico Java: " + className + "." + methodName); // INFO = 1
        
        // Imposta un risultato di prova
        result = R"({"status": "success", "result": null})";
        
        return true;
    } catch (const json::exception& e) {
        log(static_cast<APILogLevel>(3), "Errore nel parsing JSON degli argomenti: " + std::string(e.what())); // ERROR = 3
        return false;
    } catch (...) {
        log(static_cast<APILogLevel>(3), "Errore sconosciuto nella chiamata del metodo statico Java"); // ERROR = 3
        return false;
    }
}

bool CoreAPI::createSharedMemory(const std::string& name, size_t size) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return (*m_impl->m_ipcManager).createSharedMemory(name, size);
}

bool CoreAPI::releaseSharedMemory(const std::string& name) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return (*m_impl->m_ipcManager).releaseSharedMemory(name);
}

bool CoreAPI::createNamedPipe(const std::string& name) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return (*m_impl->m_ipcManager).createNamedPipe(name);
}

bool CoreAPI::writeToNamedPipe(const std::string& name, const void* data, size_t size) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return (*m_impl->m_ipcManager).writeToNamedPipe(name, data, size);
}

bool CoreAPI::readFromNamedPipe(const std::string& name, void* buffer, size_t size) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return (*m_impl->m_ipcManager).readFromNamedPipe(name, buffer, size);
}

bool CoreAPI::closeNamedPipe(const std::string& name) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return (*m_impl->m_ipcManager).closeNamedPipe(name);
}

bool CoreAPI::createSocket(const std::string& address, int port) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->createSocket(address, port);
}

bool CoreAPI::writeToSocket(const std::string& address, const void* data, size_t size) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->writeToSocket(address, data, size);
}

bool CoreAPI::readFromSocket(const std::string& address, void* buffer, size_t size) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->readFromSocket(address, buffer, size);
}

bool CoreAPI::closeSocket(const std::string& address) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->closeSocket(address);
}

bool CoreAPI::createMessageQueue(const std::string& name) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->createMessageQueue(name);
}

bool CoreAPI::sendMessage(const std::string& name, const void* data, size_t size) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->sendMessage(name, data, size);
}

bool CoreAPI::receiveMessage(const std::string& name, void* buffer, size_t size) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->receiveMessage(name, buffer, size);
}

bool CoreAPI::closeMessageQueue(const std::string& name) {
    if (m_impl == nullptr || m_impl->m_ipcManager == nullptr) {
        return false;
    }
    return m_impl->m_ipcManager->closeMessageQueue(name);
}

int CoreAPI::registerResourceCallback(const std::string& resourceType, int thresholdPercent, APIResourceCallback callback) {
    if (m_impl == nullptr) {
        return -1;
    }
    
    static int nextCallbackId = 0;
    int id = ++nextCallbackId;
    
    if (m_impl->m_resourceCallbacks.find(resourceType) == m_impl->m_resourceCallbacks.end()) {
        m_impl->m_resourceCallbacks[resourceType] = std::vector<std::pair<int, APIResourceCallback>>();
    }
    
    m_impl->m_resourceCallbacks[resourceType].push_back(std::make_pair(id, callback));
    return id;
}

} // namespace CoreNS 