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
 * @brief Classe per il bridge tra C++ e Python
 */
class PythonBridge {
public:
    /**
     * @brief Costruttore del PythonBridge
     * @param errorHandler Riferimento all'ErrorHandler
     * @param configManager Riferimento al ConfigManager
     */
    PythonBridge(ErrorHandler& errorHandler, ConfigManager& configManager);
    
    /**
     * @brief Distruttore del PythonBridge
     */
    ~PythonBridge();
    
    /**
     * @brief Inizializza l'ambiente Python
     * @param pythonHome Percorso dell'installazione Python
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize(const std::string& pythonHome = "");
    
    /**
     * @brief Finalizza l'ambiente Python
     */
    void finalize();
    
    /**
     * @brief Aggiunge un percorso al sys.path di Python
     * @param path Percorso da aggiungere
     * @return true se l'aggiunta è avvenuta con successo, false altrimenti
     */
    bool addPythonPath(const std::string& path);
    
    /**
     * @brief Importa un modulo Python
     * @param moduleName Nome del modulo da importare
     * @return true se l'importazione è avvenuta con successo, false altrimenti
     */
    bool importModule(const std::string& moduleName);
    
    /**
     * @brief Esegue una funzione Python
     * @param moduleName Nome del modulo
     * @param funcName Nome della funzione
     * @param args Argomenti da passare alla funzione
     * @return true se l'esecuzione è avvenuta con successo, false altrimenti
     */
    template<typename... Args>
    bool callPythonFunction(const std::string& moduleName, const std::string& funcName, Args... args);
    
    /**
     * @brief Ottiene un valore da una variabile Python
     * @param moduleName Nome del modulo
     * @param varName Nome della variabile
     * @param value Riferimento in cui salvare il valore
     * @return true se l'ottenimento è avvenuto con successo, false altrimenti
     */
    template<typename T>
    bool getPythonVariable(const std::string& moduleName, const std::string& varName, T& value);
    
    /**
     * @brief Imposta un valore in una variabile Python
     * @param moduleName Nome del modulo
     * @param varName Nome della variabile
     * @param value Valore da impostare
     * @return true se l'impostazione è avvenuta con successo, false altrimenti
     */
    template<typename T>
    bool setPythonVariable(const std::string& moduleName, const std::string& varName, const T& value);
    
    /**
     * @brief Esegue un codice Python
     * @param code Codice Python da eseguire
     * @return true se l'esecuzione è avvenuta con successo, false altrimenti
     */
    bool executeString(const std::string& code);
    
    /**
     * @brief Registra una funzione C++ in Python
     * @param moduleName Nome del modulo Python
     * @param funcName Nome della funzione
     * @param func Funzione C++ da registrare
     * @return true se la registrazione è avvenuta con successo, false altrimenti
     */
    template<typename Ret, typename... Args>
    bool registerFunction(const std::string& moduleName, const std::string& funcName, 
                         std::function<Ret(Args...)> func);
    
    /**
     * @brief Gestisce gli errori Python
     * @return true se c'è stato un errore, false altrimenti
     */
    bool handlePythonErrors();
    
private:
    ErrorHandler& m_errorHandler;
    ConfigManager& m_configManager;
    void* m_pythonState;
    bool m_isInitialized;
    std::unordered_map<std::string, void*> m_modules;
    
    /**
     * @brief Converte un tipo C++ in un tipo Python
     * @param value Valore C++
     * @return Puntatore all'oggetto Python
     */
    template<typename T>
    void* toPythonObject(const T& value);
    
    /**
     * @brief Converte un tipo Python in un tipo C++
     * @param obj Oggetto Python
     * @param value Riferimento in cui salvare il valore C++
     * @return true se la conversione è avvenuta con successo, false altrimenti
     */
    template<typename T>
    bool fromPythonObject(void* obj, T& value);
};
