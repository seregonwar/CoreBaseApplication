#include "JavaBindings.h"
#include "../../CoreClass/ErrorHandler.h"
#include "../../CoreClass/ConfigManager.h"
#include "../../CoreClass/LogLevel.h"
#include <iostream>
#include <stdexcept>

// Global reference to JavaVM for callbacks
static JavaVM* g_jvm = nullptr;
static JavaBindings* g_javaBindings = nullptr;

// JNI method signatures
extern "C" {
    // ErrorHandler methods
    JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_initialize(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_shutdown(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_handleError(JNIEnv* env, jobject obj, jstring message, jstring file, jint line, jstring function);
    JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_setLogLevel(JNIEnv* env, jobject obj, jint level);
    JNIEXPORT jint JNICALL Java_com_corebase_ErrorHandler_getLogLevel(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_log(JNIEnv* env, jobject obj, jint level, jstring message);
    JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_debug(JNIEnv* env, jobject obj, jstring message);
    JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_info(JNIEnv* env, jobject obj, jstring message);
    JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_warning(JNIEnv* env, jobject obj, jstring message);
    JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_error(JNIEnv* env, jobject obj, jstring message);
    JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_critical(JNIEnv* env, jobject obj, jstring message);
    
    // ConfigManager methods
    JNIEXPORT jboolean JNICALL Java_com_corebase_ConfigManager_loadConfig(JNIEnv* env, jobject obj, jstring filename);
    JNIEXPORT jstring JNICALL Java_com_corebase_ConfigManager_getValue(JNIEnv* env, jobject obj, jstring key);
    JNIEXPORT void JNICALL Java_com_corebase_ConfigManager_setValue(JNIEnv* env, jobject obj, jstring key, jstring value);
    JNIEXPORT jboolean JNICALL Java_com_corebase_ConfigManager_saveConfig(JNIEnv* env, jobject obj, jstring filename);
    
    // JVM lifecycle
    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved);
    JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved);
}

// Utility functions for string conversion
std::string jstringToString(JNIEnv* env, jstring jstr) {
    if (!jstr) return "";
    
    const char* chars = env->GetStringUTFChars(jstr, nullptr);
    if (!chars) return "";
    
    std::string result(chars);
    env->ReleaseStringUTFChars(jstr, chars);
    return result;
}

jstring stringToJstring(JNIEnv* env, const std::string& str) {
    return env->NewStringUTF(str.c_str());
}

// JavaBindings implementation
JavaBindings::JavaBindings(CoreNS::ErrorHandler& errorHandler, CoreNS::ConfigManager& configManager)
    : m_errorHandler(errorHandler), m_configManager(configManager) {
    g_javaBindings = this;
}

JavaBindings::~JavaBindings() {
    g_javaBindings = nullptr;
}

void JavaBindings::initialize() {
    m_errorHandler.initialize();
}

void JavaBindings::shutdown() {
    m_errorHandler.shutdown();
}

void JavaBindings::handleError(const std::string& message, const std::string& file, int line, const std::string& function) {
    m_errorHandler.handleError(message, file, line, function);
}

void JavaBindings::setLogLevel(int level) {
    m_errorHandler.setLogLevel(static_cast<CoreNS::LogLevel>(level));
}

int JavaBindings::getLogLevel() const {
    return static_cast<int>(m_errorHandler.getLogLevel());
}

void JavaBindings::log(int level, const std::string& message) {
    m_errorHandler.log(static_cast<CoreNS::LogLevel>(level), message);
}

void JavaBindings::debug(const std::string& message) {
    m_errorHandler.debug(message);
}

void JavaBindings::info(const std::string& message) {
    m_errorHandler.info(message);
}

void JavaBindings::warning(const std::string& message) {
    m_errorHandler.warning(message);
}

void JavaBindings::error(const std::string& message) {
    m_errorHandler.error(message);
}

void JavaBindings::critical(const std::string& message) {
    m_errorHandler.critical(message);
}

// ConfigManager wrapper methods
bool JavaBindings::loadConfig(const std::string& filename) {
    return m_configManager.loadConfig(filename);
}

std::string JavaBindings::getValue(const std::string& key) {
    return m_configManager.getConfigString(key, "");
}

void JavaBindings::setValue(const std::string& key, const std::string& value) {
    m_configManager.setConfigString(key, value);
}

bool JavaBindings::saveConfig(const std::string& filename) {
    return m_configManager.saveConfig(filename);
}

void JavaBindings::registerNatives(JNIEnv* env) {
    // Find the ErrorHandler class
    jclass errorHandlerClass = env->FindClass("com/corebase/ErrorHandler");
    if (!errorHandlerClass) {
        std::cerr << "Failed to find ErrorHandler class" << std::endl;
        return;
    }
    
    // Define native methods for ErrorHandler
    JNINativeMethod errorHandlerMethods[] = {
        {"initialize", "()V", (void*)Java_com_corebase_ErrorHandler_initialize},
        {"shutdown", "()V", (void*)Java_com_corebase_ErrorHandler_shutdown},
        {"handleError", "(Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;)V", (void*)Java_com_corebase_ErrorHandler_handleError},
        {"setLogLevel", "(I)V", (void*)Java_com_corebase_ErrorHandler_setLogLevel},
        {"getLogLevel", "()I", (void*)Java_com_corebase_ErrorHandler_getLogLevel},
        {"log", "(ILjava/lang/String;)V", (void*)Java_com_corebase_ErrorHandler_log},
        {"debug", "(Ljava/lang/String;)V", (void*)Java_com_corebase_ErrorHandler_debug},
        {"info", "(Ljava/lang/String;)V", (void*)Java_com_corebase_ErrorHandler_info},
        {"warning", "(Ljava/lang/String;)V", (void*)Java_com_corebase_ErrorHandler_warning},
        {"error", "(Ljava/lang/String;)V", (void*)Java_com_corebase_ErrorHandler_error},
        {"critical", "(Ljava/lang/String;)V", (void*)Java_com_corebase_ErrorHandler_critical}
    };
    
    // Register ErrorHandler methods
    if (env->RegisterNatives(errorHandlerClass, errorHandlerMethods, 
                           sizeof(errorHandlerMethods) / sizeof(errorHandlerMethods[0])) < 0) {
        std::cerr << "Failed to register ErrorHandler native methods" << std::endl;
    }
    
    // Find the ConfigManager class
    jclass configManagerClass = env->FindClass("com/corebase/ConfigManager");
    if (!configManagerClass) {
        std::cerr << "Failed to find ConfigManager class" << std::endl;
        return;
    }
    
    // Define native methods for ConfigManager
    JNINativeMethod configManagerMethods[] = {
        {"loadConfig", "(Ljava/lang/String;)Z", (void*)Java_com_corebase_ConfigManager_loadConfig},
        {"getValue", "(Ljava/lang/String;)Ljava/lang/String;", (void*)Java_com_corebase_ConfigManager_getValue},
        {"setValue", "(Ljava/lang/String;Ljava/lang/String;)V", (void*)Java_com_corebase_ConfigManager_setValue},
        {"saveConfig", "(Ljava/lang/String;)Z", (void*)Java_com_corebase_ConfigManager_saveConfig}
    };
    
    // Register ConfigManager methods
    if (env->RegisterNatives(configManagerClass, configManagerMethods, 
                           sizeof(configManagerMethods) / sizeof(configManagerMethods[0])) < 0) {
        std::cerr << "Failed to register ConfigManager native methods" << std::endl;
    }
}

// JNI method implementations
JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_initialize(JNIEnv* env, jobject obj) {
    if (g_javaBindings) {
        g_javaBindings->initialize();
    }
}

JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_shutdown(JNIEnv* env, jobject obj) {
    if (g_javaBindings) {
        g_javaBindings->shutdown();
    }
}

JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_handleError(JNIEnv* env, jobject obj, 
                                                                  jstring message, jstring file, 
                                                                  jint line, jstring function) {
    if (g_javaBindings) {
        std::string msgStr = jstringToString(env, message);
        std::string fileStr = jstringToString(env, file);
        std::string funcStr = jstringToString(env, function);
        g_javaBindings->handleError(msgStr, fileStr, line, funcStr);
    }
}

JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_setLogLevel(JNIEnv* env, jobject obj, jint level) {
    if (g_javaBindings) {
        g_javaBindings->setLogLevel(level);
    }
}

JNIEXPORT jint JNICALL Java_com_corebase_ErrorHandler_getLogLevel(JNIEnv* env, jobject obj) {
    if (g_javaBindings) {
        return g_javaBindings->getLogLevel();
    }
    return 0;
}

JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_log(JNIEnv* env, jobject obj, jint level, jstring message) {
    if (g_javaBindings) {
        std::string msgStr = jstringToString(env, message);
        g_javaBindings->log(level, msgStr);
    }
}

JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_debug(JNIEnv* env, jobject obj, jstring message) {
    if (g_javaBindings) {
        std::string msgStr = jstringToString(env, message);
        g_javaBindings->debug(msgStr);
    }
}

JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_info(JNIEnv* env, jobject obj, jstring message) {
    if (g_javaBindings) {
        std::string msgStr = jstringToString(env, message);
        g_javaBindings->info(msgStr);
    }
}

JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_warning(JNIEnv* env, jobject obj, jstring message) {
    if (g_javaBindings) {
        std::string msgStr = jstringToString(env, message);
        g_javaBindings->warning(msgStr);
    }
}

JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_error(JNIEnv* env, jobject obj, jstring message) {
    if (g_javaBindings) {
        std::string msgStr = jstringToString(env, message);
        g_javaBindings->error(msgStr);
    }
}

JNIEXPORT void JNICALL Java_com_corebase_ErrorHandler_critical(JNIEnv* env, jobject obj, jstring message) {
    if (g_javaBindings) {
        std::string msgStr = jstringToString(env, message);
        g_javaBindings->critical(msgStr);
    }
}

// ConfigManager JNI implementations
JNIEXPORT jboolean JNICALL Java_com_corebase_ConfigManager_loadConfig(JNIEnv* env, jobject obj, jstring filename) {
    if (g_javaBindings) {
        std::string filenameStr = jstringToString(env, filename);
        return g_javaBindings->loadConfig(filenameStr);
    }
    return JNI_FALSE;
}

JNIEXPORT jstring JNICALL Java_com_corebase_ConfigManager_getValue(JNIEnv* env, jobject obj, jstring key) {
    if (g_javaBindings) {
        std::string keyStr = jstringToString(env, key);
        std::string value = g_javaBindings->getValue(keyStr);
        return stringToJstring(env, value);
    }
    return stringToJstring(env, "");
}

JNIEXPORT void JNICALL Java_com_corebase_ConfigManager_setValue(JNIEnv* env, jobject obj, jstring key, jstring value) {
    if (g_javaBindings) {
        std::string keyStr = jstringToString(env, key);
        std::string valueStr = jstringToString(env, value);
        g_javaBindings->setValue(keyStr, valueStr);
    }
}

JNIEXPORT jboolean JNICALL Java_com_corebase_ConfigManager_saveConfig(JNIEnv* env, jobject obj, jstring filename) {
    if (g_javaBindings) {
        std::string filenameStr = jstringToString(env, filename);
        return g_javaBindings->saveConfig(filenameStr);
    }
    return JNI_FALSE;
}

// JVM lifecycle functions
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_jvm = vm;
    
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    
    // Register native methods
    JavaBindings::registerNatives(env);
    
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
    g_jvm = nullptr;
    g_javaBindings = nullptr;
}