#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

// Forward declarations
class ErrorHandler;
class ConfigManager;

/**
 * @brief Classe per il bridge tra C++ e Java
 */
class JavaBridge {
public:
    /**
     * @brief Costruttore del JavaBridge
     * @param errorHandler Riferimento all'ErrorHandler
     * @param configManager Riferimento al ConfigManager
     */
    JavaBridge(ErrorHandler& errorHandler, ConfigManager& configManager);
    
    /**
     * @brief Distruttore del JavaBridge
     */
    ~JavaBridge();
    
    /**
     * @brief Inizializza la JVM (Java Virtual Machine)
     * @param jvmPath Percorso della JVM
     * @param classPath Classpath per la JVM
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize(const std::string& jvmPath = "", const std::string& classPath = "");
    
    /**
     * @brief Finalizza la JVM
     */
    void finalize();
    
    /**
     * @brief Carica una classe Java
     * @param className Nome completo della classe (con package)
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     */
    bool loadClass(const std::string& className);
    
    /**
     * @brief Crea un'istanza di una classe Java
     * @param className Nome completo della classe (con package)
     * @param constructorArgs Argomenti del costruttore
     * @return ID dell'oggetto creato, -1 in caso di errore
     */
    template<typename... Args>
    int createObject(const std::string& className, Args... constructorArgs);
    
    /**
     * @brief Chiama un metodo di un oggetto Java
     * @param objectId ID dell'oggetto
     * @param methodName Nome del metodo
     * @param args Argomenti del metodo
     * @return true se la chiamata è avvenuta con successo, false altrimenti
     */
    template<typename Ret, typename... Args>
    bool callMethod(int objectId, const std::string& methodName, Ret& returnValue, Args... args);
    
    /**
     * @brief Chiama un metodo statico di una classe Java
     * @param className Nome completo della classe (con package)
     * @param methodName Nome del metodo
     * @param args Argomenti del metodo
     * @return true se la chiamata è avvenuta con successo, false altrimenti
     */
    template<typename Ret, typename... Args>
    bool callStaticMethod(const std::string& className, const std::string& methodName, 
                         Ret& returnValue, Args... args);
    
    /**
     * @brief Ottiene il valore di un campo di un oggetto Java
     * @param objectId ID dell'oggetto
     * @param fieldName Nome del campo
     * @param value Riferimento in cui salvare il valore
     * @return true se l'ottenimento è avvenuto con successo, false altrimenti
     */
    template<typename T>
    bool getField(int objectId, const std::string& fieldName, T& value);
    
    /**
     * @brief Imposta il valore di un campo di un oggetto Java
     * @param objectId ID dell'oggetto
     * @param fieldName Nome del campo
     * @param value Valore da impostare
     * @return true se l'impostazione è avvenuta con successo, false altrimenti
     */
    template<typename T>
    bool setField(int objectId, const std::string& fieldName, const T& value);
    
    /**
     * @brief Ottiene il valore di un campo statico di una classe Java
     * @param className Nome completo della classe (con package)
     * @param fieldName Nome del campo
     * @param value Riferimento in cui salvare il valore
     * @return true se l'ottenimento è avvenuto con successo, false altrimenti
     */
    template<typename T>
    bool getStaticField(const std::string& className, const std::string& fieldName, T& value);
    
    /**
     * @brief Imposta il valore di un campo statico di una classe Java
     * @param className Nome completo della classe (con package)
     * @param fieldName Nome del campo
     * @param value Valore da impostare
     * @return true se l'impostazione è avvenuta con successo, false altrimenti
     */
    template<typename T>
    bool setStaticField(const std::string& className, const std::string& fieldName, const T& value);
    
    /**
     * @brief Distrugge un oggetto Java
     * @param objectId ID dell'oggetto
     * @return true se la distruzione è avvenuta con successo, false altrimenti
     */
    bool destroyObject(int objectId);
    
    /**
     * @brief Registra una funzione C++ in Java
     * @param className Nome completo della classe (con package)
     * @param methodName Nome del metodo
     * @param func Funzione C++ da registrare
     * @return true se la registrazione è avvenuta con successo, false altrimenti
     */
    template<typename Ret, typename... Args>
    bool registerNativeMethod(const std::string& className, const std::string& methodName, 
                             std::function<Ret(Args...)> func);
    
    /**
     * @brief Gestisce le eccezioni Java
     * @return true se c'è stata un'eccezione, false altrimenti
     */
    bool handleJavaExceptions();
    
private:
    ErrorHandler& m_errorHandler;
    ConfigManager& m_configManager;
    void* m_jvm;
    void* m_env;
    bool m_isInitialized;
    std::unordered_map<std::string, void*> m_classes;
    std::unordered_map<int, void*> m_objects;
    int m_nextObjectId;
    
    /**
     * @brief Converte un tipo C++ in un tipo Java
     * @param value Valore C++
     * @return Puntatore all'oggetto Java
     */
    template<typename T>
    void* toJavaObject(const T& value);
    
    /**
     * @brief Converte un tipo Java in un tipo C++
     * @param obj Oggetto Java
     * @param value Riferimento in cui salvare il valore C++
     * @return true se la conversione è avvenuta con successo, false altrimenti
     */
    template<typename T>
    bool fromJavaObject(void* obj, T& value);
};
