#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include <iomanip>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include "../../src/core/CoreAPI.h"

using namespace CoreNS;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

// Definizione dei colori per la console
enum ConsoleColor {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    BROWN = 6,
    LIGHTGRAY = 7,
    DARKGRAY = 8,
    LIGHTBLUE = 9,
    LIGHTGREEN = 10,
    LIGHTCYAN = 11,
    LIGHTRED = 12,
    LIGHTMAGENTA = 13,
    YELLOW = 14,
    WHITE = 15
};

// Funzione per impostare il colore del testo nella console
void setConsoleColor(ConsoleColor color) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, static_cast<int>(color));
#else
    // Codici di escape ANSI per i colori in sistemi Unix
    const char* colors[] = {
        "\033[30m", // BLACK
        "\033[34m", // BLUE
        "\033[32m", // GREEN
        "\033[36m", // CYAN
        "\033[31m", // RED
        "\033[35m", // MAGENTA
        "\033[33m", // BROWN
        "\033[37m", // LIGHTGRAY
        "\033[90m", // DARKGRAY
        "\033[94m", // LIGHTBLUE
        "\033[92m", // LIGHTGREEN
        "\033[96m", // LIGHTCYAN
        "\033[91m", // LIGHTRED
        "\033[95m", // LIGHTMAGENTA
        "\033[93m", // YELLOW
        "\033[97m"  // WHITE
    };
    std::cout << colors[static_cast<int>(color)];
#endif
}

// Funzione per ripristinare il colore predefinito nella console
void resetConsoleColor() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, static_cast<int>(ConsoleColor::LIGHTGRAY));
#else
    std::cout << "\033[0m";
#endif
}

// Callback per il monitoraggio delle risorse
void onResourceThreshold(const std::string& resourceType, int usage) {
    setConsoleColor(ConsoleColor::LIGHTRED);
    std::cout << "ALLARME: Utilizzo " << resourceType << " al " << usage << "%" << std::endl;
    resetConsoleColor();
}

// Funzione wrapper che può essere passata a registerResourceCallback
auto createResourceCallback = [](void (*fn)(const std::string&, int)) {
    return [fn](const std::string& resourceType, int usage) {
        fn(resourceType, usage);
    };
};

// Funzione per test di configurazione
void testConfiguration(CoreAPI& api) {
    setConsoleColor(ConsoleColor::CYAN);
    std::cout << "\n=== Test Configurazione ===\n" << std::endl;
    resetConsoleColor();
    
    // Imposta alcune configurazioni
    api.setConfig("app.name", "CoreTestApp");
    api.setConfig("app.version", "1.0.0");
    api.setConfig("app.log_level", 1);  // INFO
    api.setConfig("app.max_threads", 4);
    api.setConfig("network.timeout", 30.5);
    api.setConfig("debug.enabled", true);
    api.setConfig("python.enabled", true);
    api.setConfig("python.path", "C:\\Python310");
    api.setConfig("python.modules", "math,numpy,pandas");
    
    // Leggi e visualizza le configurazioni
    std::cout << "Configurazioni impostate:" << std::endl;
    std::cout << "  app.name = " << api.getConfigString("app.name") << std::endl;
    std::cout << "  app.version = " << api.getConfigString("app.version") << std::endl;
    std::cout << "  app.log_level = " << api.getConfigInt("app.log_level") << std::endl;
    std::cout << "  app.max_threads = " << api.getConfigInt("app.max_threads") << std::endl;
    std::cout << "  network.timeout = " << api.getConfigDouble("network.timeout") << std::endl;
    std::cout << "  debug.enabled = " << (api.getConfigBool("debug.enabled") ? "true" : "false") << std::endl;
    std::cout << "  python.enabled = " << (api.getConfigBool("python.enabled") ? "true" : "false") << std::endl;
    std::cout << "  python.path = " << api.getConfigString("python.path") << std::endl;
    std::cout << "  python.modules = " << api.getConfigString("python.modules") << std::endl;
    
    // Test di configurazione non esistente (dovrebbe restituire il valore di default)
    std::cout << "\nTest configurazione non esistente:" << std::endl;
    std::cout << "  unknown.key = " << api.getConfigString("unknown.key", "valore predefinito") << std::endl;
    std::cout << "  unknown.int = " << api.getConfigInt("unknown.int", 42) << std::endl;
    
    // Salva la configurazione su file
    std::string configFile = "config.json";
    if (api.saveConfig(configFile)) {
        std::cout << "\nConfigurazione salvata su " << configFile << std::endl;
    } else {
        std::cout << "\nImpossibile salvare la configurazione su " << configFile << std::endl;
    }
}

// Funzione per test di logging
void testLogging(CoreAPI& api) {
    setConsoleColor(ConsoleColor::YELLOW);
    std::cout << "\n=== Test Logging ===\n" << std::endl;
    resetConsoleColor();
    
    api.logDebug("Questo è un messaggio di debug");
    api.logInfo("Questo è un messaggio informativo");
    api.logWarning("Questo è un avviso");
    api.logError("Questo è un messaggio di errore");
    
    // Cambia il livello di log e verifica che i messaggi vengano filtrati correttamente
    std::cout << "\nImpostazione del livello di log a WARNING:" << std::endl;
    api.setLogLevel(APILogLevel::WARNING);
    std::cout << "Livello di log impostato a WARNING" << std::endl;
    
    api.logDebug("Questo messaggio di debug non dovrebbe essere mostrato");
    api.logInfo("Questo messaggio informativo non dovrebbe essere mostrato");
    api.logWarning("Questo avviso dovrebbe essere mostrato");
    api.logError("Questo errore dovrebbe essere mostrato");
    
    // Ripristina il livello di log a INFO
    api.setLogLevel(APILogLevel::INFO);
    std::cout << "Livello di log ripristinato a INFO" << std::endl;
}

// Funzione per test delle risorse di sistema
void testSystemResources(CoreAPI& api) {
    setConsoleColor(ConsoleColor::LIGHTGREEN);
    std::cout << "\n=== Test Risorse di Sistema ===\n" << std::endl;
    resetConsoleColor();
    
    // Registra callback per il monitoraggio delle risorse
    int cpuCallbackId = api.registerResourceCallback("cpu", 80, createResourceCallback(onResourceThreshold));
    int memCallbackId = api.registerResourceCallback("memory", 85, createResourceCallback(onResourceThreshold));
    int diskCallbackId = api.registerResourceCallback("disk", 90, createResourceCallback(onResourceThreshold));
    
    std::cout << "Callback registrate per il monitoraggio delle risorse:" << std::endl;
    std::cout << "  CPU > 80%: ID " << cpuCallbackId << std::endl;
    std::cout << "  Memoria > 85%: ID " << memCallbackId << std::endl;
    std::cout << "  Disco > 90%: ID " << diskCallbackId << std::endl;
    
    // Ottieni e visualizza le risorse di sistema
    APISystemResources resources = api.getSystemResources();
    
    std::cout << "\nRisorse di sistema attuali:" << std::endl;
    std::cout << "  CPU: " << std::fixed << std::setprecision(2) << resources.cpuUsagePercent << "%" << std::endl;
    std::cout << "  Memoria: " << resources.availableMemoryBytes / (1024 * 1024) << "MB / " 
              << resources.totalMemoryBytes / (1024 * 1024) << "MB" << std::endl;
    std::cout << "  Disco: " << resources.availableDiskBytes / (1024 * 1024 * 1024) << "GB / " 
              << resources.totalDiskBytes / (1024 * 1024 * 1024) << "GB" << std::endl;
    
    if (resources.networkUsagePercent >= 0) {
        std::cout << "  Rete: " << resources.networkUsagePercent << "%" << std::endl;
    }
    
    if (resources.gpuUsagePercent >= 0) {
        std::cout << "  GPU: " << resources.gpuUsagePercent << "%" << std::endl;
    }
}

// Funzione per test dell'integrazione con Python
void testPythonIntegration(CoreAPI& api) {
    setConsoleColor(ConsoleColor::LIGHTMAGENTA);
    std::cout << "\n=== Test Integrazione Python ===\n" << std::endl;
    resetConsoleColor();
    
    // Ottieni il percorso Python dalla configurazione
    std::string pythonPath = api.getConfigString("python.path", "");
    
    if (api.initializePython(pythonPath)) {
        std::cout << "✅ Interprete Python inizializzato correttamente" << std::endl;
        
        // Test importazione modulo math (dovrebbe essere disponibile in qualsiasi installazione Python)
        if (api.importPythonModule("math")) {
            std::cout << "✅ Modulo 'math' importato correttamente" << std::endl;
            
            // Test esecuzione funzione
            std::string args = "[3.14159]";
            if (api.executePythonFunction("math", "sin", args)) {
                std::cout << "✅ Funzione 'math.sin' eseguita correttamente" << std::endl;
            } else {
                std::cout << "❌ Errore nell'esecuzione della funzione 'math.sin'" << std::endl;
            }
            
            // Test con altre funzioni matematiche
            if (api.executePythonFunction("math", "cos", "[1.5707]")) {
                std::cout << "✅ Funzione 'math.cos' eseguita correttamente" << std::endl;
            }
            
            if (api.executePythonFunction("math", "sqrt", "[16]")) {
                std::cout << "✅ Funzione 'math.sqrt' eseguita correttamente" << std::endl;
            }
        } else {
            std::cout << "❌ Errore nell'importazione del modulo 'math'" << std::endl;
        }
        
        // Crea un file Python temporaneo per testare l'esecuzione di codice
        std::string pythonTempFile = "temp_test.py";
        {
            std::ofstream file(pythonTempFile);
            file << "def test_function():\n";
            file << "    print('Test della funzione Python')\n";
            file << "    return 'Success!'\n\n";
            file << "print('File Python caricato con successo!')\n";
        }
        
        // Test importazione del modulo temporaneo
        if (api.importPythonModule("temp_test")) {
            std::cout << "✅ Modulo 'temp_test' importato correttamente" << std::endl;
            
            if (api.executePythonFunction("temp_test", "test_function", "[]")) {
                std::cout << "✅ Funzione 'test_function' eseguita correttamente" << std::endl;
            } else {
                std::cout << "❌ Errore nell'esecuzione della funzione 'test_function'" << std::endl;
            }
        } else {
            std::cout << "❌ Errore nell'importazione del modulo 'temp_test'" << std::endl;
        }
        
        // Test esecuzione codice Python
        std::string code = "print('Hello from Python!')\nresult = 42 * 2\nprint(f'Il risultato è {result}')";
        if (api.executePythonCode(code)) {
            std::cout << "✅ Codice Python eseguito correttamente" << std::endl;
        } else {
            std::cout << "❌ Errore nell'esecuzione del codice Python" << std::endl;
        }
        
        // Prova con un esempio di calcolo più complesso
        std::string complexCode = R"(
import math
from datetime import datetime

# Calcolo di una serie di numeri
nums = [i**2 for i in range(10)]
print(f'Quadrati: {nums}')

# Calcolo della data e ora corrente
now = datetime.now()
print(f'Data e ora: {now.strftime("%Y-%m-%d %H:%M:%S")}')

# Calcolo della serie di Fibonacci
def fib(n):
    if n <= 1:
        return n
    return fib(n-1) + fib(n-2)

print(f'Fibonacci(10): {fib(10)}')
)";
        
        if (api.executePythonCode(complexCode)) {
            std::cout << "✅ Codice Python complesso eseguito correttamente" << std::endl;
        } else {
            std::cout << "❌ Errore nell'esecuzione del codice Python complesso" << std::endl;
        }
        
        // Elimina il file temporaneo
        fs::remove(pythonTempFile);
        
    } else {
        std::cout << "❌ Errore nell'inizializzazione dell'interprete Python" << std::endl;
        std::cout << "   Verifica che Python sia installato e che il percorso sia corretto." << std::endl;
        if (!pythonPath.empty()) {
            std::cout << "   Percorso configurato: " << pythonPath << std::endl;
        }
    }
}

// Funzione per simulazione di carico
void testLoadSimulation() {
    setConsoleColor(ConsoleColor::LIGHTRED);
    std::cout << "\n=== Test Simulazione Carico ===\n" << std::endl;
    resetConsoleColor();
    
    std::cout << "Avvio simulazione di carico (3 secondi)..." << std::endl;
    
    // Simulazione di carico CPU per 3 secondi
    auto startTime = std::chrono::steady_clock::now();
    double result = 0.0;
    
    while (std::chrono::steady_clock::now() - startTime < 3s) {
        // Simulazione di carico CPU
        for (int i = 0; i < 500000; ++i) {
            result += std::sin(static_cast<double>(i) * 0.01);
        }
        
        // Aggiornamento progresso
        int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count();
        int percent = static_cast<int>((elapsed / 3000.0) * 100);
        
        std::cout << "\rProgresso: [";
        for (int i = 0; i < 20; ++i) {
            if (i < percent / 5) std::cout << "=";
            else std::cout << " ";
        }
        std::cout << "] " << percent << "%" << std::flush;
        
        std::this_thread::sleep_for(100ms);
    }
    
    std::cout << "\nSimulazione di carico completata!" << std::endl;
}

// Interfaccia a riga di comando per l'interazione con l'utente
void runCLI(CoreAPI& api) {
    setConsoleColor(ConsoleColor::WHITE);
    std::cout << "\n=== CoreTest CLI ===\n" << std::endl;
    resetConsoleColor();
    
    std::cout << "Core CLI (v1.0.0)" << std::endl;
    std::cout << "Digita 'help' per vedere i comandi disponibili" << std::endl;
    std::cout << "Digita 'exit' per uscire" << std::endl;
    
    // Mappatura dei comandi alle funzioni disponibili
    std::unordered_map<std::string, std::function<void()>> commands = {
        {"config", [&api](){ testConfiguration(api); }},
        {"log", [&api](){ testLogging(api); }},
        {"resources", [&api](){ testSystemResources(api); }},
        {"python", [&api](){ testPythonIntegration(api); }},
        {"load", []{ testLoadSimulation(); }}
    };
    
    // Descrizioni dei comandi
    std::unordered_map<std::string, std::string> commandDescriptions = {
        {"config", "Testa le funzionalità di configurazione"},
        {"log", "Testa le funzionalità di logging"},
        {"resources", "Mostra le risorse di sistema"},
        {"python", "Testa l'integrazione con Python"},
        {"load", "Esegue una simulazione di carico"},
        {"help", "Mostra l'elenco dei comandi disponibili"},
        {"exit", "Esce dall'applicazione"}
    };
    
    std::string input;
        while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, input);
        
        if (input == "exit") {
            break;
        } else if (input == "help") {
            setConsoleColor(ConsoleColor::CYAN);
            std::cout << "\nComandi disponibili:\n" << std::endl;
            resetConsoleColor();
            
            for (const auto& [cmd, desc] : commandDescriptions) {
                std::cout << "  " << std::left << std::setw(10) << cmd << " - " << desc << std::endl;
            }
        } else if (commands.find(input) != commands.end()) {
            try {
                commands[input]();
            } catch (const std::exception& e) {
                setConsoleColor(ConsoleColor::LIGHTRED);
                std::cout << "Errore durante l'esecuzione del comando: " << e.what() << std::endl;
                resetConsoleColor();
            }
        } else {
            std::cout << "Comando non riconosciuto. Digita 'help' per vedere i comandi disponibili." << std::endl;
        }
    }
}

// Thread per il monitoraggio continuo delle risorse
void resourceMonitoringThread(CoreAPI& api, std::atomic<bool>& running) {
    constexpr auto updateInterval = 5s;
    
    while (running) {
        APISystemResources resources = api.getSystemResources();
        
        // Log delle risorse
        api.logDebug("Monitoraggio risorse - CPU: " + std::to_string(resources.cpuUsagePercent) + "%");
        
        std::this_thread::sleep_for(updateInterval);
    }
}

int main() {
#ifdef _WIN32
    // Configura la console Windows per supportare l'output UTF-8
    SetConsoleOutputCP(CP_UTF8);
#endif

    setConsoleColor(ConsoleColor::LIGHTGREEN);
    std::cout << "====================================" << std::endl;
    std::cout << "     CoreBaseApplication Test      " << std::endl;
    std::cout << "     Versione con supporto Python  " << std::endl;
    std::cout << "====================================" << std::endl;
    resetConsoleColor();
    
    try {
        // Inizializza l'API del Core
        CoreAPI api;
        
        // Inizializza con un file di configurazione di default
        fs::path configPath = fs::current_path() / "config.json";
        
        if (!api.initialize(configPath.string())) {
            std::cout << "Avviso: Inizializzazione con file di configurazione non riuscita." << std::endl;
            std::cout << "Verrà utilizzata la configurazione predefinita." << std::endl;
        }
        
        // Imposta il livello di log a INFO per default
        api.setLogLevel(APILogLevel::INFO);
        
        // Avvia il thread di monitoraggio risorse
        std::atomic<bool> monitoringRunning{true};
        std::thread monitoringThread(resourceMonitoringThread, std::ref(api), std::ref(monitoringRunning));
        
        // Mostra le risorse di sistema iniziali
        testSystemResources(api);
        
        // Esegui l'interfaccia CLI
        runCLI(api);
        
        // Fermata del thread di monitoraggio
        monitoringRunning = false;
        if (monitoringThread.joinable()) {
            monitoringThread.join();
        }
        
        // Chiusura ordinata
        api.shutdown();
        
        std::cout << "\nApplicazione terminata correttamente." << std::endl;
        
    } catch (const std::exception& e) {
        setConsoleColor(ConsoleColor::LIGHTRED);
        std::cout << "Errore fatale: " << e.what() << std::endl;
        resetConsoleColor();
        return 1;
    }
    
    return 0;
} 