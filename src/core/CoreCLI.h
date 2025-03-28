#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "CoreAPI.h"

namespace CoreNS {

class CoreCLI {
public:
    struct Command {
        std::string name;
        std::string description;
        std::function<void(const std::vector<std::string>&)> handler;
    };

    CoreCLI(CoreAPI& api);
    
    void addCommand(const std::string& name, 
                   const std::string& description,
                   std::function<void(const std::vector<std::string>&)> handler);
    
    void run();
    void executeCommand(const std::string& commandLine);
    
private:
    CoreAPI& m_api;
    std::vector<Command> m_commands;
    
    void registerDefaultCommands();
    void printHelp();
    std::vector<std::string> parseCommandLine(const std::string& line);
};

} // namespace CoreNS 