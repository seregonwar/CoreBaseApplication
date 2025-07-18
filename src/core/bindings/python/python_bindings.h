#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#ifdef USE_PYTHON_BINDINGS
#include <pybind11/pybind11.h>
#include <Python.h>
#endif
#include "../../CoreClass/ErrorHandler.h"

// Forward declarations
class ConfigManager;

/**
 * @brief Classe per i bindings tra C++ e Python
 */
class PythonBindings {
public:
    PythonBindings(CoreNS::ErrorHandler& errorHandler, ConfigManager& configManager);
    ~PythonBindings();

    // Funzioni di ErrorHandler esposte a Python
    void initialize();
    void shutdown();
    void handleError(const std::string& message, const std::string& file, int line, const std::string& function);
    void setLogLevel(int level);
    int getLogLevel() const;
    void log(int level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);

    // pybind11 setup (scheletro)
#ifdef USE_PYTHON_BINDINGS
    static void bind(pybind11::module_& m);
#endif

private:
    CoreNS::ErrorHandler& m_errorHandler;
    ConfigManager& m_configManager;
};

#ifdef USE_PYTHON_BINDINGS
/**
 * @brief Motore di scripting Python integrato completo
 */
class PythonScriptingEngine {
public:
    PythonScriptingEngine();
    ~PythonScriptingEngine();

    // Inizializza l'interprete Python
    bool initialize();
    // Finalizza l'interprete Python
    void finalize();

    // Esegue uno script Python da stringa con output catturato
    std::string execString(const std::string& code);
    // Esegue uno script Python da file con output catturato
    std::string execFile(const std::string& filename);
    
    // Esegue codice Python senza catturare output
    bool execStringQuiet(const std::string& code);
    bool execFileQuiet(const std::string& filename);

    // Gestione variabili Python
    bool setVariable(const std::string& name, const std::string& value);
    bool setVariable(const std::string& name, int value);
    bool setVariable(const std::string& name, double value);
    bool setVariable(const std::string& name, bool value);
    std::string getVariable(const std::string& name);
    
    // Gestione moduli Python
    bool importModule(const std::string& moduleName);
    bool importModuleAs(const std::string& moduleName, const std::string& alias);
    std::vector<std::string> getLoadedModules();
    
    // Chiamata funzioni Python
    std::string callFunction(const std::string& functionName, const std::vector<std::string>& args = {});
    std::string callModuleFunction(const std::string& moduleName, const std::string& functionName, const std::vector<std::string>& args = {});
    
    // Gestione path Python
    bool addToPath(const std::string& path);
    std::vector<std::string> getPythonPath();
    
    // Informazioni interprete
    std::string getPythonVersion();
    std::string getPythonExecutable();
    bool isInitialized() const { return m_initialized; }
    
    // Gestione errori Python
    std::string getLastError() const { return m_lastError; }
    void clearError() { m_lastError.clear(); }
    
    // Espone un oggetto C++ a Python (placeholder, da specializzare)
    template<typename T>
    void exposeObject(const std::string& name, T& object);

    // Espone una funzione C++ a Python (placeholder, da specializzare)
    template<typename Func>
    void exposeFunction(const std::string& name, Func&& func);

    // Gestione errori Python
    bool handlePythonErrors();
    
    // Salva/carica stato interprete
    bool saveState(const std::string& filename);
    bool loadState(const std::string& filename);
    
    // Reset interprete
    bool reset();

private:
    bool m_initialized;
    std::string m_lastError;
    std::vector<std::string> m_loadedModules;
    
    // Helper per catturare output
    std::string captureOutput(const std::function<void()>& func);
    void setError(const std::string& error);
};
#endif

#ifdef USE_PYTHON_BINDINGS
std::string execute_python_script_with_output(const std::string& code);
#endif
