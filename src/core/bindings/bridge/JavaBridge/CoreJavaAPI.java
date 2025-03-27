package com.corebaseapplication.api;

import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.ArrayList;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.io.File;
import org.json.JSONObject;
import org.json.JSONArray;

/**
 * Interfaccia Java per CoreBaseApplication.
 * Fornisce un'API semplificata per accedere alle funzionalità del CoreBaseApplication.
 */
public class CoreAPI {
    
    // Costanti per la gestione dell'API
    private static final String LIB_NAME = "CoreAPI";
    
    // Enumerazioni
    
    /**
     * Livelli di log supportati
     */
    public enum LogLevel {
        DEBUG(0),
        INFO(1),
        WARNING(2),
        ERROR(3),
        FATAL(4);
        
        private final int value;
        
        LogLevel(int value) {
            this.value = value;
        }
        
        public int getValue() {
            return value;
        }
    }
    
    /**
     * Tipi di moduli supportati
     */
    public enum ModuleType {
        CPP(0),
        PYTHON(1),
        JAVA(2);
        
        private final int value;
        
        ModuleType(int value) {
            this.value = value;
        }
        
        public int getValue() {
            return value;
        }
        
        public static ModuleType fromValue(int value) {
            for (ModuleType type : ModuleType.values()) {
                if (type.getValue() == value) {
                    return type;
                }
            }
            return CPP; // Default
        }
    }
    
    /**
     * Tipi di comunicazione IPC supportati
     */
    public enum IPCType {
        SHARED_MEMORY(0),
        NAMED_PIPE(1),
        SOCKET(2),
        MESSAGE_QUEUE(3);
        
        private final int value;
        
        IPCType(int value) {
            this.value = value;
        }
        
        public int getValue() {
            return value;
        }
    }
    
    /**
     * Ruolo in una comunicazione IPC
     */
    public enum IPCRole {
        SERVER(0),
        CLIENT(1);
        
        private final int value;
        
        IPCRole(int value) {
            this.value = value;
        }
        
        public int getValue() {
            return value;
        }
    }
    
    /**
     * Classe che rappresenta le informazioni sulle risorse di sistema
     */
    public static class SystemResources {
        private int cpuUsagePercent;
        private long availableMemoryBytes;
        private long totalMemoryBytes;
        private long availableDiskBytes;
        private long totalDiskBytes;
        private int networkUsagePercent;
        private int gpuUsagePercent;
        
        public SystemResources() {
            this.cpuUsagePercent = 0;
            this.availableMemoryBytes = 0;
            this.totalMemoryBytes = 0;
            this.availableDiskBytes = 0;
            this.totalDiskBytes = 0;
            this.networkUsagePercent = 0;
            this.gpuUsagePercent = 0;
        }
        
        public int getCpuUsagePercent() {
            return cpuUsagePercent;
        }
        
        public void setCpuUsagePercent(int cpuUsagePercent) {
            this.cpuUsagePercent = cpuUsagePercent;
        }
        
        public long getAvailableMemoryBytes() {
            return availableMemoryBytes;
        }
        
        public void setAvailableMemoryBytes(long availableMemoryBytes) {
            this.availableMemoryBytes = availableMemoryBytes;
        }
        
        public long getTotalMemoryBytes() {
            return totalMemoryBytes;
        }
        
        public void setTotalMemoryBytes(long totalMemoryBytes) {
            this.totalMemoryBytes = totalMemoryBytes;
        }
        
        public long getAvailableDiskBytes() {
            return availableDiskBytes;
        }
        
        public void setAvailableDiskBytes(long availableDiskBytes) {
            this.availableDiskBytes = availableDiskBytes;
        }
        
        public long getTotalDiskBytes() {
            return totalDiskBytes;
        }
        
        public void setTotalDiskBytes(long totalDiskBytes) {
            this.totalDiskBytes = totalDiskBytes;
        }
        
        public int getNetworkUsagePercent() {
            return networkUsagePercent;
        }
        
        public void setNetworkUsagePercent(int networkUsagePercent) {
            this.networkUsagePercent = networkUsagePercent;
        }
        
        public int getGpuUsagePercent() {
            return gpuUsagePercent;
        }
        
        public void setGpuUsagePercent(int gpuUsagePercent) {
            this.gpuUsagePercent = gpuUsagePercent;
        }
        
        /**
         * Crea un oggetto SystemResources da un oggetto JSON
         * @param json JSON contenente i dati delle risorse
         * @return Oggetto SystemResources
         */
        public static SystemResources fromJson(JSONObject json) {
            SystemResources resources = new SystemResources();
            
            if (json.has("cpuUsagePercent")) {
                resources.setCpuUsagePercent(json.getInt("cpuUsagePercent"));
            }
            
            if (json.has("availableMemoryBytes")) {
                resources.setAvailableMemoryBytes(json.getLong("availableMemoryBytes"));
            }
            
            if (json.has("totalMemoryBytes")) {
                resources.setTotalMemoryBytes(json.getLong("totalMemoryBytes"));
            }
            
            if (json.has("availableDiskBytes")) {
                resources.setAvailableDiskBytes(json.getLong("availableDiskBytes"));
            }
            
            if (json.has("totalDiskBytes")) {
                resources.setTotalDiskBytes(json.getLong("totalDiskBytes"));
            }
            
            if (json.has("networkUsagePercent")) {
                resources.setNetworkUsagePercent(json.getInt("networkUsagePercent"));
            }
            
            if (json.has("gpuUsagePercent")) {
                resources.setGpuUsagePercent(json.getInt("gpuUsagePercent"));
            }
            
            return resources;
        }
        
        /**
         * Converte l'oggetto in JSON
         * @return JSONObject
         */
        public JSONObject toJson() {
            JSONObject json = new JSONObject();
            
            json.put("cpuUsagePercent", cpuUsagePercent);
            json.put("availableMemoryBytes", availableMemoryBytes);
            json.put("totalMemoryBytes", totalMemoryBytes);
            json.put("availableDiskBytes", availableDiskBytes);
            json.put("totalDiskBytes", totalDiskBytes);
            json.put("networkUsagePercent", networkUsagePercent);
            json.put("gpuUsagePercent", gpuUsagePercent);
            
            return json;
        }
        
        @Override
        public String toString() {
            return String.format(
                "CPU: %d%% | Memory: %.1fMB / %.1fMB | Disk: %.1fGB / %.1fGB | Network: %d%% | GPU: %d%%",
                cpuUsagePercent,
                availableMemoryBytes / (1024.0 * 1024.0),
                totalMemoryBytes / (1024.0 * 1024.0),
                availableDiskBytes / (1024.0 * 1024.0 * 1024.0),
                totalDiskBytes / (1024.0 * 1024.0 * 1024.0),
                networkUsagePercent,
                gpuUsagePercent
            );
        }
    }
    
    /**
     * Classe che rappresenta le informazioni di un modulo
     */
    public static class ModuleInfo {
        private String name;
        private String version;
        private String author;
        private String description;
        private ModuleType type;
        private List<String> dependencies;
        private boolean isLoaded;
        
        public ModuleInfo() {
            this.name = "";
            this.version = "";
            this.author = "";
            this.description = "";
            this.type = ModuleType.CPP;
            this.dependencies = new ArrayList<>();
            this.isLoaded = false;
        }
        
        public String getName() {
            return name;
        }
        
        public void setName(String name) {
            this.name = name;
        }
        
        public String getVersion() {
            return version;
        }
        
        public void setVersion(String version) {
            this.version = version;
        }
        
        public String getAuthor() {
            return author;
        }
        
        public void setAuthor(String author) {
            this.author = author;
        }
        
        public String getDescription() {
            return description;
        }
        
        public void setDescription(String description) {
            this.description = description;
        }
        
        public ModuleType getType() {
            return type;
        }
        
        public void setType(ModuleType type) {
            this.type = type;
        }
        
        public List<String> getDependencies() {
            return dependencies;
        }
        
        public void setDependencies(List<String> dependencies) {
            this.dependencies = dependencies;
        }
        
        public boolean isLoaded() {
            return isLoaded;
        }
        
        public void setLoaded(boolean loaded) {
            isLoaded = loaded;
        }
        
        /**
         * Crea un oggetto ModuleInfo da un oggetto JSON
         * @param json JSON contenente i dati del modulo
         * @return Oggetto ModuleInfo
         */
        public static ModuleInfo fromJson(JSONObject json) {
            ModuleInfo moduleInfo = new ModuleInfo();
            
            if (json.has("name")) {
                moduleInfo.setName(json.getString("name"));
            }
            
            if (json.has("version")) {
                moduleInfo.setVersion(json.getString("version"));
            }
            
            if (json.has("author")) {
                moduleInfo.setAuthor(json.getString("author"));
            }
            
            if (json.has("description")) {
                moduleInfo.setDescription(json.getString("description"));
            }
            
            if (json.has("type")) {
                moduleInfo.setType(ModuleType.fromValue(json.getInt("type")));
            }
            
            if (json.has("dependencies")) {
                JSONArray dependencies = json.getJSONArray("dependencies");
                List<String> dependencyList = new ArrayList<>();
                
                for (int i = 0; i < dependencies.length(); i++) {
                    dependencyList.add(dependencies.getString(i));
                }
                
                moduleInfo.setDependencies(dependencyList);
            }
            
            if (json.has("isLoaded")) {
                moduleInfo.setLoaded(json.getBoolean("isLoaded"));
            }
            
            return moduleInfo;
        }
        
        /**
         * Converte l'oggetto in JSON
         * @return JSONObject
         */
        public JSONObject toJson() {
            JSONObject json = new JSONObject();
            
            json.put("name", name);
            json.put("version", version);
            json.put("author", author);
            json.put("description", description);
            json.put("type", type.getValue());
            
            JSONArray dependenciesArray = new JSONArray();
            for (String dependency : dependencies) {
                dependenciesArray.put(dependency);
            }
            
            json.put("dependencies", dependenciesArray);
            json.put("isLoaded", isLoaded);
            
            return json;
        }
        
        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            
            sb.append("Module: ").append(name).append(" (v").append(version).append(")\n");
            sb.append("Author: ").append(author).append("\n");
            sb.append("Type: ").append(type.name()).append("\n");
            sb.append("Description: ").append(description).append("\n");
            
            sb.append("Dependencies: ");
            if (dependencies.isEmpty()) {
                sb.append("None");
            } else {
                sb.append(String.join(", ", dependencies));
            }
            sb.append("\n");
            
            sb.append("Loaded: ").append(isLoaded ? "Yes" : "No");
            
            return sb.toString();
        }
    }
    
    /**
     * Interfaccia per la gestione delle callback di messaggi IPC
     */
    public interface MessageCallback {
        void onMessage(String channelName, byte[] data, int dataSize);
    }
    
    /**
     * Interfaccia per la gestione delle callback di risorse
     */
    public interface ResourceCallback {
        void onThresholdExceeded(int currentUsage);
    }
    
    // Eccezioni
    
    /**
     * Eccezione base per gli errori di CoreAPI
     */
    public static class CoreAPIException extends Exception {
        public CoreAPIException(String message) {
            super(message);
        }
        
        public CoreAPIException(String message, Throwable cause) {
            super(message, cause);
        }
    }
    
    /**
     * Eccezione per errori di inizializzazione
     */
    public static class InitializationException extends CoreAPIException {
        public InitializationException(String message) {
            super(message);
        }
        
        public InitializationException(String message, Throwable cause) {
            super(message, cause);
        }
    }
    
    /**
     * Eccezione per errori di caricamento moduli
     */
    public static class ModuleLoadException extends CoreAPIException {
        public ModuleLoadException(String message) {
            super(message);
        }
        
        public ModuleLoadException(String message, Throwable cause) {
            super(message, cause);
        }
    }
    
    /**
     * Eccezione per errori di configurazione
     */
    public static class ConfigException extends CoreAPIException {
        public ConfigException(String message) {
            super(message);
        }
        
        public ConfigException(String message, Throwable cause) {
            super(message, cause);
        }
    }
    
    /**
     * Eccezione per errori di comunicazione IPC
     */
    public static class IPCException extends CoreAPIException {
        public IPCException(String message) {
            super(message);
        }
        
        public IPCException(String message, Throwable cause) {
            super(message, cause);
        }
    }
    
    // JNI metodi nativi
    private native long createCoreAPI(String configPath);
    private native void destroyCoreAPI(long handle);
    private native boolean initialize(long handle);
    private native void shutdown(long handle);
    
    // Configurazione
    private native boolean loadConfig(long handle, String filePath);
    private native boolean saveConfig(long handle, String filePath);
    private native String getConfigString(long handle, String key, String defaultValue);
    private native int getConfigInt(long handle, String key, int defaultValue);
    private native double getConfigDouble(long handle, String key, double defaultValue);
    private native boolean getConfigBool(long handle, String key, boolean defaultValue);
    private native void setConfigString(long handle, String key, String value);
    private native void setConfigInt(long handle, String key, int value);
    private native void setConfigDouble(long handle, String key, double value);
    private native void setConfigBool(long handle, String key, boolean value);
    
    // Gestione moduli
    private native boolean loadModule(long handle, String modulePath);
    private native boolean unloadModule(long handle, String moduleName);
    private native boolean isModuleLoaded(long handle, String moduleName);
    private native String getModuleInfo(long handle, String moduleName);
    private native String getLoadedModules(long handle);
    
    // Logging
    private native void log(long handle, int level, String message, String sourceFile, int lineNumber, String functionName);
    private native void logError(long handle, String message);
    private native void logWarning(long handle, String message);
    private native void logInfo(long handle, String message);
    private native void logDebug(long handle, String message);
    private native void setLogLevel(long handle, int level);
    
    // Risorse
    private native String getSystemResources(long handle);
    private native long getAvailableMemory(long handle);
    private native int getCpuUsage(long handle);
    private native int registerResourceCallback(long handle, String resourceType, int thresholdPercent, Object callback);
    
    // IPC
    private native boolean createIPCChannel(long handle, String name, int type, int role, String params);
    private native boolean sendData(long handle, String channelName, byte[] data, int dataSize);
    private native Object[] receiveData(long handle, String channelName, int bufferSize);
    private native int registerMessageCallback(long handle, String channelName, Object callback);
    private native boolean closeIPCChannel(long handle, String channelName);
    
    // Java specifiche
    private native int createJavaObject(long handle, String className, String argsJson);
    private native String callJavaMethod(long handle, int objectId, String methodName, String argsJson);
    private native String callJavaStaticMethod(long handle, String className, String methodName, String argsJson);
    
    // Attributi dell'istanza
    private long handle;
    private boolean initialized;
    private Map<Integer, ResourceCallback> resourceCallbacks;
    private Map<Integer, MessageCallback> messageCallbacks;
    
    static {
        try {
            System.loadLibrary(LIB_NAME);
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Impossibile caricare la libreria nativa: " + e.getMessage());
        }
    }
    
    /**
     * Costruttore dell'API
     * @param configPath Percorso del file di configurazione
     * @throws InitializationException Se l'inizializzazione fallisce
     */
    public CoreAPI(String configPath) throws InitializationException {
        try {
            this.handle = createCoreAPI(configPath);
            
            if (this.handle == 0) {
                throw new InitializationException("Impossibile creare l'istanza di CoreAPI");
            }
            
            this.initialized = false;
            this.resourceCallbacks = new HashMap<>();
            this.messageCallbacks = new HashMap<>();
            
        } catch (UnsatisfiedLinkError e) {
            throw new InitializationException("Errore nel caricamento delle funzioni native", e);
        }
    }
    
    /**
     * Costruttore dell'API con configurazione predefinita
     * @throws InitializationException Se l'inizializzazione fallisce
     */
    public CoreAPI() throws InitializationException {
        this("config.json");
    }
    
    /**
     * Finalize che libera le risorse native
     */
    @Override
    protected void finalize() throws Throwable {
        try {
            if (handle != 0) {
                shutdown();
                destroyCoreAPI(handle);
                handle = 0;
            }
        } finally {
            super.finalize();
        }
    }
    
    /**
     * Inizializza il Core e tutti i suoi componenti
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    public boolean initialize() {
        if (!initialized) {
            initialized = initialize(handle);
        }
        return initialized;
    }
    
    /**
     * Arresta il Core e libera le risorse
     */
    public void shutdown() {
        if (initialized) {
            shutdown(handle);
            initialized = false;
        }
    }
    
    // Implementazione delle API pubbliche
    
    // Configurazione
    
    /**
     * Carica una configurazione da file
     * @param filePath Percorso del file di configurazione
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     * @throws ConfigException Se si verifica un errore di configurazione
     */
    public boolean loadConfig(String filePath) throws ConfigException {
        try {
            return loadConfig(handle, filePath);
        } catch (Exception e) {
            throw new ConfigException("Errore nel caricamento della configurazione", e);
        }
    }
    
    /**
     * Salva la configurazione corrente su file
     * @param filePath Percorso del file (se vuoto, usa l'ultimo file caricato)
     * @return true se il salvataggio è avvenuto con successo, false altrimenti
     * @throws ConfigException Se si verifica un errore di configurazione
     */
    public boolean saveConfig(String filePath) throws ConfigException {
        try {
            return saveConfig(handle, filePath);
        } catch (Exception e) {
            throw new ConfigException("Errore nel salvataggio della configurazione", e);
        }
    }
    
    /**
     * Ottiene un valore di configurazione come stringa
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore della configurazione
     */
    public String getConfigString(String key, String defaultValue) {
        return getConfigString(handle, key, defaultValue);
    }
    
    /**
     * Ottiene un valore di configurazione come intero
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore della configurazione
     */
    public int getConfigInt(String key, int defaultValue) {
        return getConfigInt(handle, key, defaultValue);
    }
    
    /**
     * Ottiene un valore di configurazione come double
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore della configurazione
     */
    public double getConfigDouble(String key, double defaultValue) {
        return getConfigDouble(handle, key, defaultValue);
    }
    
    /**
     * Ottiene un valore di configurazione come boolean
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore della configurazione
     */
    public boolean getConfigBool(String key, boolean defaultValue) {
        return getConfigBool(handle, key, defaultValue);
    }
    
    /**
     * Imposta un valore di configurazione
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param value Valore da impostare
     */
    public void setConfig(String key, String value) {
        setConfigString(handle, key, value);
    }
    
    /**
     * Imposta un valore di configurazione
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param value Valore da impostare
     */
    public void setConfig(String key, int value) {
        setConfigInt(handle, key, value);
    }
    
    /**
     * Imposta un valore di configurazione
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param value Valore da impostare
     */
    public void setConfig(String key, double value) {
        setConfigDouble(handle, key, value);
    }
    
    /**
     * Imposta un valore di configurazione
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param value Valore da impostare
     */
    public void setConfig(String key, boolean value) {
        setConfigBool(handle, key, value);
    }
    
    // Gestione moduli
    
    /**
     * Carica un modulo
     * @param modulePath Percorso del modulo da caricare
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     * @throws ModuleLoadException Se il caricamento fallisce
     */
    public boolean loadModule(String modulePath) throws ModuleLoadException {
        boolean result = loadModule(handle, modulePath);
        if (!result) {
            throw new ModuleLoadException("Impossibile caricare il modulo: " + modulePath);
        }
        return result;
    }
    
    /**
     * Scarica un modulo
     * @param moduleName Nome del modulo da scaricare
     * @return true se lo scaricamento è avvenuto con successo, false altrimenti
     */
    public boolean unloadModule(String moduleName) {
        return unloadModule(handle, moduleName);
    }
    
    /**
     * Verifica se un modulo è caricato
     * @param moduleName Nome del modulo
     * @return true se il modulo è caricato, false altrimenti
     */
    public boolean isModuleLoaded(String moduleName) {
        return isModuleLoaded(handle, moduleName);
    }
    
    /**
     * Ottiene le informazioni di un modulo
     * @param moduleName Nome del modulo
     * @return Informazioni sul modulo
     */
    public ModuleInfo getModuleInfo(String moduleName) {
        String jsonStr = getModuleInfo(handle, moduleName);
        if (jsonStr == null || jsonStr.isEmpty()) {
            return new ModuleInfo();
        }
        
        try {
            JSONObject json = new JSONObject(jsonStr);
            return ModuleInfo.fromJson(json);
        } catch (Exception e) {
            return new ModuleInfo();
        }
    }
    
    /**
     * Ottiene tutti i moduli caricati
     * @return Lista contenente i nomi dei moduli caricati
     */
    public List<String> getLoadedModules() {
        String jsonStr = getLoadedModules(handle);
        List<String> modules = new ArrayList<>();
        
        if (jsonStr == null || jsonStr.isEmpty()) {
            return modules;
        }
        
        try {
            JSONArray json = new JSONArray(jsonStr);
            for (int i = 0; i < json.length(); i++) {
                modules.add(json.getString(i));
            }
        } catch (Exception e) {
            // Ignora l'errore
        }
        
        return modules;
    }
    
    // Logging
    
    /**
     * Registra un messaggio di log
     * @param level Livello del log
     * @param message Messaggio da registrare
     * @param sourceFile Nome del file sorgente (opzionale)
     * @param lineNumber Numero di riga nel file sorgente (opzionale)
     * @param functionName Nome della funzione (opzionale)
     */
    public void log(LogLevel level, String message, String sourceFile, int lineNumber, String functionName) {
        log(handle, level.getValue(), message, sourceFile, lineNumber, functionName);
    }
    
    /**
     * Registra un errore
     * @param message Messaggio di errore
     */
    public void logError(String message) {
        logError(handle, message);
    }
    
    /**
     * Registra un avviso
     * @param message Messaggio di avviso
     */
    public void logWarning(String message) {
        logWarning(handle, message);
    }
    
    /**
     * Registra un messaggio informativo
     * @param message Messaggio informativo
     */
    public void logInfo(String message) {
        logInfo(handle, message);
    }
    
    /**
     * Registra un messaggio di debug
     * @param message Messaggio di debug
     */
    public void logDebug(String message) {
        logDebug(handle, message);
    }
    
    /**
     * Imposta il livello minimo di log da registrare
     * @param level Livello minimo di log
     */
    public void setLogLevel(LogLevel level) {
        setLogLevel(handle, level.getValue());
    }
    
    // Risorse
    
    /**
     * Ottiene le informazioni sulle risorse di sistema
     * @return Oggetto SystemResources
     */
    public SystemResources getSystemResources() {
        String jsonStr = getSystemResources(handle);
        
        if (jsonStr == null || jsonStr.isEmpty()) {
            return new SystemResources();
        }
        
        try {
            JSONObject json = new JSONObject(jsonStr);
            return SystemResources.fromJson(json);
        } catch (Exception e) {
            return new SystemResources();
        }
    }
    
    /**
     * Ottiene la quantità di memoria disponibile
     * @return Memoria disponibile in bytes
     */
    public long getAvailableMemory() {
        return getAvailableMemory(handle);
    }
    
    /**
     * Ottiene l'utilizzo della CPU
     * @return Percentuale di utilizzo della CPU (0-100)
     */
    public int getCpuUsage() {
        return getCpuUsage(handle);
    }
    
    /**
     * Registra una callback da eseguire quando l'utilizzo di una risorsa supera una soglia
     * @param resourceType Tipo di risorsa da monitorare (cpu, memory, disk, network, gpu)
     * @param thresholdPercent Soglia percentuale (0-100)
     * @param callback Funzione da eseguire quando la soglia viene superata
     * @return ID della callback registrata
     */
    public int registerResourceCallback(String resourceType, int thresholdPercent, ResourceCallback callback) {
        int id = registerResourceCallback(handle, resourceType, thresholdPercent, callback);
        
        if (id >= 0) {
            resourceCallbacks.put(id, callback);
        }
        
        return id;
    }
    
    // IPC
    
    /**
     * Crea un canale di comunicazione IPC
     * @param name Nome del canale
     * @param type Tipo di canale
     * @param role Ruolo (server o client)
     * @param params Parametri specifici per il tipo di canale (es. host:port per socket)
     * @return true se la creazione è avvenuta con successo, false altrimenti
     * @throws IPCException Se la creazione fallisce
     */
    public boolean createIPCChannel(String name, IPCType type, IPCRole role, String params) throws IPCException {
        boolean result = createIPCChannel(handle, name, type.getValue(), role.getValue(), params);
        
        if (!result) {
            throw new IPCException("Impossibile creare il canale IPC: " + name);
        }
        
        return result;
    }
    
    /**
     * Crea un canale di comunicazione IPC con ruolo server
     * @param name Nome del canale
     * @param type Tipo di canale
     * @param params Parametri specifici per il tipo di canale
     * @return true se la creazione è avvenuta con successo, false altrimenti
     * @throws IPCException Se la creazione fallisce
     */
    public boolean createIPCChannel(String name, IPCType type, String params) throws IPCException {
        return createIPCChannel(name, type, IPCRole.SERVER, params);
    }
    
    /**
     * Invia dati su un canale IPC
     * @param channelName Nome del canale
     * @param data Dati da inviare
     * @return true se l'invio è avvenuto con successo, false altrimenti
     */
    public boolean sendData(String channelName, byte[] data) {
        return sendData(handle, channelName, data, data.length);
    }
    
    /**
     * Invia una stringa su un canale IPC
     * @param channelName Nome del canale
     * @param message Messaggio da inviare
     * @return true se l'invio è avvenuto con successo, false altrimenti
     */
    public boolean sendString(String channelName, String message) {
        byte[] data = message.getBytes(StandardCharsets.UTF_8);
        return sendData(channelName, data);
    }
    
    /**
     * Riceve dati da un canale IPC
     * @param channelName Nome del canale
     * @param bufferSize Dimensione del buffer
     * @return Array contenente [success, data, bytesRead]
     */
    public Object[] receiveData(String channelName, int bufferSize) {
        return receiveData(handle, channelName, bufferSize);
    }
    
    /**
     * Riceve una stringa da un canale IPC
     * @param channelName Nome del canale
     * @param bufferSize Dimensione del buffer
     * @return La stringa ricevuta, o stringa vuota in caso di errore
     */
    public String receiveString(String channelName, int bufferSize) {
        Object[] result = receiveData(channelName, bufferSize);
        
        if ((Boolean)result[0] && result[1] != null) {
            byte[] data = (byte[])result[1];
            int bytesRead = (Integer)result[2];
            
            return new String(data, 0, bytesRead, StandardCharsets.UTF_8);
        }
        
        return "";
    }
    
    /**
     * Registra una callback per i messaggi ricevuti
     * @param channelName Nome del canale IPC
     * @param callback Funzione da chiamare quando arriva un messaggio
     * @return ID della callback registrata
     */
    public int registerMessageCallback(String channelName, MessageCallback callback) {
        int id = registerMessageCallback(handle, channelName, callback);
        
        if (id >= 0) {
            messageCallbacks.put(id, callback);
        }
        
        return id;
    }
    
    /**
     * Chiude un canale IPC
     * @param channelName Nome del canale
     * @return true se la chiusura è avvenuta con successo, false altrimenti
     */
    public boolean closeIPCChannel(String channelName) {
        return closeIPCChannel(handle, channelName);
    }
    
    // Java specifiche
    
    /**
     * Crea un'istanza di una classe Java
     * @param className Nome completo della classe (con package)
     * @param args Argomenti del costruttore
     * @return ID dell'oggetto creato, -1 in caso di errore
     */
    public int createJavaObject(String className, Object... args) {
        JSONArray jsonArgs = new JSONArray();
        
        for (Object arg : args) {
            jsonArgs.put(arg);
        }
        
        return createJavaObject(handle, className, jsonArgs.toString());
    }
    
    /**
     * Chiama un metodo di un oggetto Java
     * @param objectId ID dell'oggetto
     * @param methodName Nome del metodo
     * @param args Argomenti del metodo
     * @return Risultato del metodo come oggetto JSON
     */
    public JSONObject callJavaMethod(int objectId, String methodName, Object... args) {
        JSONArray jsonArgs = new JSONArray();
        
        for (Object arg : args) {
            jsonArgs.put(arg);
        }
        
        String result = callJavaMethod(handle, objectId, methodName, jsonArgs.toString());
        
        if (result == null || result.isEmpty()) {
            return new JSONObject();
        }
        
        try {
            return new JSONObject(result);
        } catch (Exception e) {
            return new JSONObject();
        }
    }
    
    /**
     * Chiama un metodo statico di una classe Java
     * @param className Nome completo della classe (con package)
     * @param methodName Nome del metodo
     * @param args Argomenti del metodo
     * @return Risultato del metodo come oggetto JSON
     */
    public JSONObject callJavaStaticMethod(String className, String methodName, Object... args) {
        JSONArray jsonArgs = new JSONArray();
        
        for (Object arg : args) {
            jsonArgs.put(arg);
        }
        
        String result = callJavaStaticMethod(handle, className, methodName, jsonArgs.toString());
        
        if (result == null || result.isEmpty()) {
            return new JSONObject();
        }
        
        try {
            return new JSONObject(result);
        } catch (Exception e) {
            return new JSONObject();
        }
    }
} 