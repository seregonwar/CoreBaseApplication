#pragma once

#include <string>
#include <vector>

namespace CoreNS {

enum class ModuleType {
    CORE,
    PLUGIN,
    EXTENSION,
    TOOL
};

struct ModuleInfo {
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    ModuleType type;
    std::vector<std::string> dependencies;
    bool isLoaded;
};

} // namespace CoreNS 