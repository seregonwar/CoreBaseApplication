#include "Core.h"
#include "ConfigManager.h"
#include "ResourceManager.h"
#include "ModuleManager.h"
#include "ErrorHandler.h"
#include "IPCManager.h"

namespace CoreNS {

Core::Core() : m_isInitialized(false) {
    m_configManager = std::make_shared<ConfigManager>();
    m_resourceManager = std::make_shared<ResourceManager>();
    m_moduleLoader = std::make_shared<ModuleManager>();
    m_errorHandler = std::make_shared<ErrorHandler>();
    m_ipcManager = std::make_shared<IPCManager>();
}

Core::~Core() {
    shutdown();
}

bool Core::initialize(const std::string& configPath) {
    if (m_isInitialized) {
        return true;
    }

    bool success = true;

    if (!m_configManager->loadConfig(configPath)) {
        success = false;
    }

    if (!m_resourceManager->initialize()) {
        success = false;
    }

    if (!m_moduleLoader->initialize()) {
        success = false;
    }

    m_errorHandler->initialize();

    ChannelInfo channelInfo;
    channelInfo.name = "default";
    channelInfo.type = IPCType::NAMED_PIPE;
    channelInfo.isServer = true;
    channelInfo.address = ".";
    channelInfo.port = 0;

    if (!m_ipcManager->initialize(channelInfo)) {
        success = false;
    }

    m_isInitialized = success;
    return success;
}

bool Core::shutdown() {
    if (!m_isInitialized) {
        return true;
    }

    bool success = true;

    if (m_ipcManager) {
        m_ipcManager->close();
    }

    if (m_errorHandler) {
        m_errorHandler->shutdown();
    }

    if (m_moduleLoader) {
        m_moduleLoader->shutdown();
    }

    if (m_resourceManager) {
        m_resourceManager->shutdown();
    }

    if (m_configManager) {
        m_configManager->saveConfig("config.json");
    }

    m_isInitialized = false;
    return success;
}

} // namespace CoreNS 