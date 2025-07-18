#include "SampleModule.h"
#include <iostream>

CoreNS::ModuleInfo g_moduleInfo = {
    "SampleModule",
    "1.0.0",
    "A sample module for demonstration purposes.",
    "Trae AI",
    CoreNS::ModuleType::PLUGIN,
    {},
    false
};

extern "C" MODULE_API void getModuleInfo(CoreNS::ModuleInfo* info) {
    if (info) {
        *info = g_moduleInfo;
    }
}

extern "C" MODULE_API void initializeModule() {
    std::cout << "SampleModule initialized!" << std::endl;
    g_moduleInfo.isLoaded = true;
}

extern "C" MODULE_API void shutdownModule() {
    std::cout << "SampleModule shutdown!" << std::endl;
    g_moduleInfo.isLoaded = false;
}