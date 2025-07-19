#include "ConfigManager.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace CoreNS {

ConfigManager::ConfigManager() {
}

ConfigManager::~ConfigManager() {
    shutdown();
}

bool ConfigManager::initialize() {
    return true;
}

void ConfigManager::shutdown() {
    clear();
}

bool ConfigManager::loadConfig(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }

        nlohmann::json j;
        file >> j;
        
        // Converti il JSON in coppie chiave-valore con supporto per chiavi annidate
        m_config.clear();
        flattenJson(j, "");
        
        m_configPath = filePath;
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

void ConfigManager::flattenJson(const nlohmann::json& j, const std::string& prefix) {
    for (auto it = j.begin(); it != j.end(); ++it) {
        std::string key = prefix.empty() ? it.key() : prefix + "." + it.key();
        
        if (it->is_object()) {
            // Ricorsione per oggetti annidati
            flattenJson(*it, key);
        } else if (it->is_string()) {
            m_config[key] = std::any(it->get<std::string>());
        } else if (it->is_number_integer()) {
            m_config[key] = std::any(it->get<int>());
        } else if (it->is_number_float()) {
            m_config[key] = std::any(it->get<double>());
        } else if (it->is_boolean()) {
            m_config[key] = std::any(it->get<bool>());
        }
    }
}

bool ConfigManager::saveConfig(const std::string& filePath) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return false;
        }

        nlohmann::json j;
        // Converti le coppie chiave-valore in JSON
        for (const auto& [key, value] : m_config) {
            try {
                if (value.type() == typeid(std::string)) {
                    j[key] = std::any_cast<std::string>(value);
                } else if (value.type() == typeid(int)) {
                    j[key] = std::any_cast<int>(value);
                } else if (value.type() == typeid(double)) {
                    j[key] = std::any_cast<double>(value);
                } else if (value.type() == typeid(bool)) {
                    j[key] = std::any_cast<bool>(value);
                }
            } catch (const std::bad_any_cast&) {
                // Ignora i valori che non possono essere convertiti
            }
        }
        
        file << j.dump(4);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

std::string ConfigManager::getConfigString(const std::string& key, const std::string& defaultValue) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        try {
            // Prova prima come stringa
            if (it->second.type() == typeid(std::string)) {
                return std::any_cast<std::string>(it->second);
            }
            // Converti numeri interi in stringa
            else if (it->second.type() == typeid(int)) {
                return std::to_string(std::any_cast<int>(it->second));
            }
            // Converti numeri decimali in stringa
            else if (it->second.type() == typeid(double)) {
                return std::to_string(std::any_cast<double>(it->second));
            }
            // Converti booleani in stringa
            else if (it->second.type() == typeid(bool)) {
                return std::any_cast<bool>(it->second) ? "true" : "false";
            }
        } catch (const std::bad_any_cast&) {}
    }
    return defaultValue;
}

int ConfigManager::getConfigInt(const std::string& key, int defaultValue) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        try {
            return std::any_cast<int>(it->second);
        } catch (const std::bad_any_cast&) {}
    }
    return defaultValue;
}

double ConfigManager::getConfigDouble(const std::string& key, double defaultValue) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        try {
            return std::any_cast<double>(it->second);
        } catch (const std::bad_any_cast&) {}
    }
    return defaultValue;
}

bool ConfigManager::getConfigBool(const std::string& key, bool defaultValue) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        try {
            return std::any_cast<bool>(it->second);
        } catch (const std::bad_any_cast&) {}
    }
    return defaultValue;
}

void ConfigManager::setConfigString(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config[key] = std::any(value);
}

void ConfigManager::setConfigInt(const std::string& key, int value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config[key] = std::any(value);
}

void ConfigManager::setConfigDouble(const std::string& key, double value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config[key] = std::any(value);
}

void ConfigManager::setConfigBool(const std::string& key, bool value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config[key] = std::any(value);
}

bool ConfigManager::hasKey(const std::string& key) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_config.find(key) != m_config.end();
}

void ConfigManager::removeKey(const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.erase(key);
}

std::vector<std::string> ConfigManager::getKeys() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> keys;
    for (const auto& [key, _] : m_config) {
        keys.push_back(key);
    }
    return keys;
}

void ConfigManager::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config.clear();
    m_values.clear();
}

template<typename T>
void ConfigManager::setValue(const std::string& key, const T& value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::stringstream ss;
    ss << value;
    m_values[key] = ss.str();
}

template<typename T>
T ConfigManager::getValue(const std::string& key, const T& defaultValue) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_values.find(key);
    if (it == m_values.end()) {
        return defaultValue;
    }

    T value;
    std::stringstream ss(it->second);
    ss >> value;
    return value;
}

bool ConfigManager::hasValue(const std::string& key) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_values.find(key) != m_values.end();
}

void ConfigManager::removeValue(const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_values.erase(key);
}

// Explicit template instantiations
template void ConfigManager::setValue<std::string>(const std::string&, const std::string&);
template void ConfigManager::setValue<int>(const std::string&, const int&);
template void ConfigManager::setValue<double>(const std::string&, const double&);
template void ConfigManager::setValue<bool>(const std::string&, const bool&);

template std::string ConfigManager::getValue<std::string>(const std::string&, const std::string&) const;
template int ConfigManager::getValue<int>(const std::string&, const int&) const;
template double ConfigManager::getValue<double>(const std::string&, const double&) const;
template bool ConfigManager::getValue<bool>(const std::string&, const bool&) const;

} // namespace CoreNS