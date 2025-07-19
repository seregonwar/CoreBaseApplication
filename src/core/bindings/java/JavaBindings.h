#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include "../../CoreClass/ErrorHandler.h"
#include "../../CoreClass/ConfigManager.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <jni.h>
#ifdef __cplusplus
}
#endif

/**
 * @brief Classe per i bindings tra C++ e Java
 */
class JavaBindings {
public:
    JavaBindings(CoreNS::ErrorHandler& errorHandler, CoreNS::ConfigManager& configManager);
    ~JavaBindings();

    // Funzioni di ErrorHandler esposte a Java
    void initialize();
    void shutdown();
    void handleError(const std::string& message, const std::string& file, int line, const std::string& function);
    void setLogLevel(int level); // int per compatibilità JNI
    int getLogLevel() const;
    void log(int level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);

    // ConfigManager wrapper methods
    bool loadConfig(const std::string& filename);
    std::string getValue(const std::string& key);
    void setValue(const std::string& key, const std::string& value);
    bool saveConfig(const std::string& filename);

    // JNI setup (scheletro)
    static void registerNatives(JNIEnv* env);

private:
    CoreNS::ErrorHandler& m_errorHandler;
    CoreNS::ConfigManager& m_configManager;
};
