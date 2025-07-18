#ifdef USE_PYTHON_BINDINGS
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <Python.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>
#include "../../CoreClass/ErrorHandler.h"
#include "python_bindings.h"

namespace py = pybind11;

// Funzione di esempio per testare pybind11
int add(int i, int j) {
    return i + j;
}

// Funzione per eseguire script Python con output catturato
std::string execute_python_script_with_output(const std::string& script) {
    try {
        // Inizializza l'interprete Python se non è già inizializzato
        py::scoped_interpreter guard{};
        
        // Cattura stdout e stderr
        py::object sys = py::module::import("sys");
        py::object io = py::module::import("io");
        py::object string_io = io.attr("StringIO")();
        
        // Reindirizza stdout e stderr
        sys.attr("stdout") = string_io;
        sys.attr("stderr") = string_io;
        
        // Esegue lo script
        py::exec(script);
        
        // Ottiene l'output
        py::object output = string_io.attr("getvalue")();
        return py::cast<std::string>(output);
    }
    catch (const py::error_already_set& e) {
        return "Errore Python: " + std::string(e.what());
    }
    catch (const std::exception& e) {
        return "Errore: " + std::string(e.what());
    }
}

// Implementazione PythonScriptingEngine
PythonScriptingEngine::PythonScriptingEngine() : m_initialized(false) {}

PythonScriptingEngine::~PythonScriptingEngine() {
    if (m_initialized) {
        finalize();
    }
}

bool PythonScriptingEngine::initialize() {
    try {
        if (!m_initialized) {
            if (!Py_IsInitialized()) {
                Py_Initialize();
            }
            m_initialized = true;
            clearError();
            
            // Importa moduli base
            importModule("sys");
            importModule("os");
            importModule("io");
            
            return true;
        }
        return true;
    }
    catch (const std::exception& e) {
        setError("Errore inizializzazione interprete: " + std::string(e.what()));
        return false;
    }
}

void PythonScriptingEngine::finalize() {
    if (m_initialized) {
        try {
            m_loadedModules.clear();
            clearError();
            // Non chiamiamo Py_Finalize() per evitare problemi con pybind11
            m_initialized = false;
        }
        catch (const std::exception& e) {
            setError("Errore finalizzazione interprete: " + std::string(e.what()));
        }
    }
}

std::string PythonScriptingEngine::execString(const std::string& code) {
    if (!m_initialized && !initialize()) {
        return "Errore: Interprete non inizializzato";
    }
    
    return captureOutput([&]() {
        try {
            py::exec(code);
        }
        catch (const py::error_already_set& e) {
            setError("Errore esecuzione Python: " + std::string(e.what()));
            throw;
        }
    });
}

std::string PythonScriptingEngine::execFile(const std::string& filename) {
    if (!m_initialized && !initialize()) {
        return "Errore: Interprete non inizializzato";
    }
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        setError("Impossibile aprire il file: " + filename);
        return "Errore: Impossibile aprire il file: " + filename;
    }
    
    std::string code((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();
    
    return execString(code);
}

bool PythonScriptingEngine::execStringQuiet(const std::string& code) {
    if (!m_initialized && !initialize()) {
        return false;
    }
    
    try {
        py::exec(code);
        return true;
    }
    catch (const py::error_already_set& e) {
        setError("Errore esecuzione Python: " + std::string(e.what()));
        return false;
    }
}

bool PythonScriptingEngine::execFileQuiet(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        setError("Impossibile aprire il file: " + filename);
        return false;
    }
    
    std::string code((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();
    
    return execStringQuiet(code);
}

bool PythonScriptingEngine::setVariable(const std::string& name, const std::string& value) {
    if (!m_initialized && !initialize()) {
        return false;
    }
    
    try {
        py::globals()[name.c_str()] = value;
        return true;
    }
    catch (const py::error_already_set& e) {
        setError("Errore impostazione variabile: " + std::string(e.what()));
        return false;
    }
}

bool PythonScriptingEngine::setVariable(const std::string& name, int value) {
    if (!m_initialized && !initialize()) {
        return false;
    }
    
    try {
        py::globals()[name.c_str()] = value;
        return true;
    }
    catch (const py::error_already_set& e) {
        setError("Errore impostazione variabile: " + std::string(e.what()));
        return false;
    }
}

bool PythonScriptingEngine::setVariable(const std::string& name, double value) {
    if (!m_initialized && !initialize()) {
        return false;
    }
    
    try {
        py::globals()[name.c_str()] = value;
        return true;
    }
    catch (const py::error_already_set& e) {
        setError("Errore impostazione variabile: " + std::string(e.what()));
        return false;
    }
}

bool PythonScriptingEngine::setVariable(const std::string& name, bool value) {
    if (!m_initialized && !initialize()) {
        return false;
    }
    
    try {
        py::globals()[name.c_str()] = value;
        return true;
    }
    catch (const py::error_already_set& e) {
        setError("Errore impostazione variabile: " + std::string(e.what()));
        return false;
    }
}

std::string PythonScriptingEngine::getVariable(const std::string& name) {
    if (!m_initialized && !initialize()) {
        return "";
    }
    
    try {
        py::object var = py::globals()[name.c_str()];
        return py::str(var);
    }
    catch (const py::error_already_set& e) {
        setError("Errore lettura variabile: " + std::string(e.what()));
        return "";
    }
}

bool PythonScriptingEngine::importModule(const std::string& moduleName) {
    if (!m_initialized && !initialize()) {
        return false;
    }
    
    try {
        py::module::import(moduleName.c_str());
        if (std::find(m_loadedModules.begin(), m_loadedModules.end(), moduleName) == m_loadedModules.end()) {
            m_loadedModules.push_back(moduleName);
        }
        return true;
    }
    catch (const py::error_already_set& e) {
        setError("Errore importazione modulo: " + std::string(e.what()));
        return false;
    }
}

bool PythonScriptingEngine::importModuleAs(const std::string& moduleName, const std::string& alias) {
    if (!m_initialized && !initialize()) {
        return false;
    }
    
    try {
        py::object module = py::module::import(moduleName.c_str());
        py::globals()[alias.c_str()] = module;
        if (std::find(m_loadedModules.begin(), m_loadedModules.end(), moduleName) == m_loadedModules.end()) {
            m_loadedModules.push_back(moduleName);
        }
        return true;
    }
    catch (const py::error_already_set& e) {
        setError("Errore importazione modulo con alias: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::string> PythonScriptingEngine::getLoadedModules() {
    return m_loadedModules;
}

std::string PythonScriptingEngine::callFunction(const std::string& functionName, const std::vector<std::string>& args) {
    if (!m_initialized && !initialize()) {
        return "Errore: Interprete non inizializzato";
    }
    
    return captureOutput([&]() {
        try {
            py::object func = py::globals()[functionName.c_str()];
            if (args.empty()) {
                func();
            } else {
                py::list py_args;
                for (const auto& arg : args) {
                    py_args.append(arg);
                }
                func(*py_args);
            }
        }
        catch (const py::error_already_set& e) {
            setError("Errore chiamata funzione: " + std::string(e.what()));
            throw;
        }
    });
}

std::string PythonScriptingEngine::callModuleFunction(const std::string& moduleName, const std::string& functionName, const std::vector<std::string>& args) {
    if (!m_initialized && !initialize()) {
        return "Errore: Interprete non inizializzato";
    }
    
    return captureOutput([&]() {
        try {
            py::object module = py::module::import(moduleName.c_str());
            py::object func = module.attr(functionName.c_str());
            if (args.empty()) {
                func();
            } else {
                py::list py_args;
                for (const auto& arg : args) {
                    py_args.append(arg);
                }
                func(*py_args);
            }
        }
        catch (const py::error_already_set& e) {
            setError("Errore chiamata funzione modulo: " + std::string(e.what()));
            throw;
        }
    });
}

bool PythonScriptingEngine::addToPath(const std::string& path) {
    if (!m_initialized && !initialize()) {
        return false;
    }
    
    try {
        py::object sys = py::module::import("sys");
        py::list sys_path = sys.attr("path");
        sys_path.append(path);
        return true;
    }
    catch (const py::error_already_set& e) {
        setError("Errore aggiunta path: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::string> PythonScriptingEngine::getPythonPath() {
    std::vector<std::string> paths;
    if (!m_initialized && !initialize()) {
        return paths;
    }
    
    try {
        py::object sys = py::module::import("sys");
        py::list sys_path = sys.attr("path");
        for (auto item : sys_path) {
            paths.push_back(py::str(item));
        }
    }
    catch (const py::error_already_set& e) {
        setError("Errore lettura path: " + std::string(e.what()));
    }
    
    return paths;
}

std::string PythonScriptingEngine::getPythonVersion() {
    if (!m_initialized && !initialize()) {
        return "";
    }
    
    try {
        py::object sys = py::module::import("sys");
        py::object version = sys.attr("version");
        return py::str(version);
    }
    catch (const py::error_already_set& e) {
        setError("Errore lettura versione: " + std::string(e.what()));
        return "";
    }
}

std::string PythonScriptingEngine::getPythonExecutable() {
    if (!m_initialized && !initialize()) {
        return "";
    }
    
    try {
        py::object sys = py::module::import("sys");
        py::object executable = sys.attr("executable");
        return py::str(executable);
    }
    catch (const py::error_already_set& e) {
        setError("Errore lettura eseguibile: " + std::string(e.what()));
        return "";
    }
}

bool PythonScriptingEngine::saveState(const std::string& filename) {
    if (!m_initialized) {
        return false;
    }
    
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            setError("Impossibile aprire il file per scrittura: " + filename);
            return false;
        }
        
        // Salva le variabili globali
        py::dict globals = py::globals();
        for (auto item : globals) {
            std::string key = py::str(item.first);
            if (key.find("__") != 0) { // Ignora variabili built-in
                file << "# Variable: " << key << "\n";
                std::string value = py::str(py::repr(item.second));
                file << key << " = " << value << "\n\n";
            }
        }
        
        file.close();
        return true;
    }
    catch (const std::exception& e) {
        setError("Errore salvataggio stato: " + std::string(e.what()));
        return false;
    }
}

bool PythonScriptingEngine::loadState(const std::string& filename) {
    return execFileQuiet(filename);
}

bool PythonScriptingEngine::reset() {
    try {
        finalize();
        return initialize();
    }
    catch (const std::exception& e) {
        setError("Errore reset interprete: " + std::string(e.what()));
        return false;
    }
}

std::string PythonScriptingEngine::captureOutput(const std::function<void()>& func) {
    try {
        py::object sys = py::module::import("sys");
        py::object io = py::module::import("io");
        py::object string_io = io.attr("StringIO")();
        
        // Salva stdout/stderr originali
        py::object original_stdout = sys.attr("stdout");
        py::object original_stderr = sys.attr("stderr");
        
        // Reindirizza stdout e stderr
        sys.attr("stdout") = string_io;
        sys.attr("stderr") = string_io;
        
        // Esegue la funzione
        func();
        
        // Ripristina stdout/stderr
        sys.attr("stdout") = original_stdout;
        sys.attr("stderr") = original_stderr;
        
        // Ottiene l'output
        py::object output = string_io.attr("getvalue")();
        return py::cast<std::string>(output);
    }
    catch (const py::error_already_set& e) {
        return "Errore Python: " + std::string(e.what());
    }
    catch (const std::exception& e) {
        return "Errore: " + std::string(e.what());
    }
}

void PythonScriptingEngine::setError(const std::string& error) {
    m_lastError = error;
}

bool PythonScriptingEngine::handlePythonErrors() {
    if (PyErr_Occurred()) {
        PyErr_Print();
        PyErr_Clear();
        return false;
    }
    return true;
}

// Modulo pybind11
PYBIND11_MODULE(python_bindings, m) {
    m.doc() = "CBA Python Bindings";
    
    m.def("add", &add, "A function which adds two numbers");
    m.def("execute_python_script_with_output", &execute_python_script_with_output, "Execute Python script and return output");
    
    // Espone la classe ErrorHandler
    py::class_<CoreNS::ErrorHandler>(m, "ErrorHandler")
        .def(py::init<>())
        .def("error", &CoreNS::ErrorHandler::error)
        .def("warning", &CoreNS::ErrorHandler::warning)
        .def("info", &CoreNS::ErrorHandler::info)
        .def("debug", &CoreNS::ErrorHandler::debug)
        .def("critical", &CoreNS::ErrorHandler::critical);
        
    // Espone la classe PythonScriptingEngine
    py::class_<PythonScriptingEngine>(m, "PythonScriptingEngine")
        .def(py::init<>())
        .def("initialize", &PythonScriptingEngine::initialize)
        .def("finalize", &PythonScriptingEngine::finalize)
        .def("execString", &PythonScriptingEngine::execString)
        .def("execFile", &PythonScriptingEngine::execFile)
        .def("execStringQuiet", &PythonScriptingEngine::execStringQuiet)
        .def("execFileQuiet", &PythonScriptingEngine::execFileQuiet)
        .def("setVariable", py::overload_cast<const std::string&, const std::string&>(&PythonScriptingEngine::setVariable))
        .def("setVariable", py::overload_cast<const std::string&, int>(&PythonScriptingEngine::setVariable))
        .def("setVariable", py::overload_cast<const std::string&, double>(&PythonScriptingEngine::setVariable))
        .def("setVariable", py::overload_cast<const std::string&, bool>(&PythonScriptingEngine::setVariable))
        .def("getVariable", &PythonScriptingEngine::getVariable)
        .def("importModule", &PythonScriptingEngine::importModule)
        .def("importModuleAs", &PythonScriptingEngine::importModuleAs)
        .def("getLoadedModules", &PythonScriptingEngine::getLoadedModules)
        .def("callFunction", &PythonScriptingEngine::callFunction)
        .def("callModuleFunction", &PythonScriptingEngine::callModuleFunction)
        .def("addToPath", &PythonScriptingEngine::addToPath)
        .def("getPythonPath", &PythonScriptingEngine::getPythonPath)
        .def("getPythonVersion", &PythonScriptingEngine::getPythonVersion)
        .def("getPythonExecutable", &PythonScriptingEngine::getPythonExecutable)
        .def("isInitialized", &PythonScriptingEngine::isInitialized)
        .def("getLastError", &PythonScriptingEngine::getLastError)
        .def("clearError", &PythonScriptingEngine::clearError)
        .def("saveState", &PythonScriptingEngine::saveState)
        .def("loadState", &PythonScriptingEngine::loadState)
        .def("reset", &PythonScriptingEngine::reset);
}

#endif // USE_PYTHON_BINDINGS