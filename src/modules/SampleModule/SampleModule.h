#pragma once

#include "core/CoreClass/ModuleInfo.h"

#ifdef _WIN32
#define MODULE_API __declspec(dllexport)
#else
#define MODULE_API
#endif

extern "C" MODULE_API void getModuleInfo(CoreNS::ModuleInfo* info);
extern "C" MODULE_API void initializeModule();
extern "C" MODULE_API void shutdownModule();