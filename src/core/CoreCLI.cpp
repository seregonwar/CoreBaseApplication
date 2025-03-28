#include "CoreCLI.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace CoreNS {

CoreCLI::CoreCLI(CoreAPI& api) : m_api(api) {
    registerDefaultCommands();
}

void CoreCLI::addCommand(const std::string& name,
                        const std::string& description,
                        std::function<void(const std::vector<std::string>&)> handler) {
    m_commands.push_back({name, description, handler});
}

void CoreCLI::run() {
    std::string line;
    std::cout << "Core CLI (v1.0.0)\n";
    std::cout << "Digita 'help' per vedere i comandi disponibili\n";
    std::cout << "Digita 'exit' per uscire\n";
    
    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, line);
        
        if (line == "exit") {
            break;
        }
        
        executeCommand(line);
    }
}

void CoreCLI::executeCommand(const std::string& commandLine) {
    auto args = parseCommandLine(commandLine);
    if (args.empty()) {
        return;
    }
    
    std::string cmd = args[0];
    args.erase(args.begin());
    
    auto it = std::find_if(m_commands.begin(), m_commands.end(),
                          [&cmd](const Command& c) { return c.name == cmd; });
    
    if (it != m_commands.end()) {
        try {
            it->handler(args);
        } catch (const std::exception& e) {
            std::cerr << "Errore nell'esecuzione del comando: " << e.what() << std::endl;
        }
    } else {
        std::cout << "Comando non trovato: " << cmd << std::endl;
        std::cout << "Digita 'help' per vedere i comandi disponibili\n";
    }
}

void CoreCLI::registerDefaultCommands() {
    addCommand("help", "Mostra l'aiuto", [this](const auto&) { printHelp(); });
    
    addCommand("init", "Inizializza il core", [this](const auto&) {
        if (m_api.initialize("config.json")) {
            std::cout << "Core inizializzato con successo\n";
        } else {
            std::cout << "Errore nell'inizializzazione del core\n";
        }
    });
    
    addCommand("shutdown", "Arresta il core", [this](const auto&) {
        m_api.shutdown();
        std::cout << "Core arrestato\n";
    });
    
    addCommand("config", "Gestisce la configurazione", [this](const auto& args) {
        if (args.size() < 2) {
            std::cout << "Uso: config <get|set> <chiave> [valore]\n";
            return;
        }
        
        if (args[0] == "get") {
            std::string value = m_api.getConfigString(args[1]);
            std::cout << args[1] << " = " << value << std::endl;
        } else if (args[0] == "set" && args.size() > 2) {
            m_api.setConfig(args[1], args[2]);
            std::cout << "Configurazione aggiornata\n";
        }
    });
    
    addCommand("module", "Gestisce i moduli", [this](const auto& args) {
        if (args.size() < 2) {
            std::cout << "Uso: module <load|unload|list> <nome_modulo>\n";
            return;
        }
        
        if (args[0] == "load") {
            if (m_api.loadModule(args[1])) {
                std::cout << "Modulo caricato con successo\n";
            } else {
                std::cout << "Errore nel caricamento del modulo\n";
            }
        } else if (args[0] == "unload") {
            if (m_api.unloadModule(args[1])) {
                std::cout << "Modulo scaricato con successo\n";
            } else {
                std::cout << "Errore nello scaricamento del modulo\n";
            }
        }
    });
}

void CoreCLI::printHelp() {
    std::cout << "\nComandi disponibili:\n";
    for (const auto& cmd : m_commands) {
        std::cout << "  " << cmd.name << " - " << cmd.description << std::endl;
    }
}

std::vector<std::string> CoreCLI::parseCommandLine(const std::string& line) {
    std::vector<std::string> args;
    std::istringstream iss(line);
    std::string arg;
    
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    return args;
}

} // namespace CoreNS 