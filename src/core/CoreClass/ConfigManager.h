#pragma once

#include <string>
#include <unordered_map>
#include <any>
#include <mutex>
#include <memory>
#include <nlohmann/json.hpp>

namespace CoreNS {

/**
 * @brief Classe per la gestione delle configurazioni dell'applicazione
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    bool initialize();
    void shutdown();

    bool loadConfig(const std::string& configFile);
    bool saveConfig(const std::string& configFile) const;

    template<typename T>
    void setValue(const std::string& key, const T& value);

    template<typename T>
    T getValue(const std::string& key, const T& defaultValue = T()) const;

    bool hasValue(const std::string& key) const;
    void removeValue(const std::string& key);
    void clear();

    // Metodi specifici per tipo
    std::string getConfigString(const std::string& key, const std::string& defaultValue = "") const;
    int getConfigInt(const std::string& key, int defaultValue = 0) const;
    double getConfigDouble(const std::string& key, double defaultValue = 0.0) const;
    bool getConfigBool(const std::string& key, bool defaultValue = false) const;

    void setConfigString(const std::string& key, const std::string& value);
    void setConfigInt(const std::string& key, int value);
    void setConfigDouble(const std::string& key, double value);
    void setConfigBool(const std::string& key, bool value);

    bool hasKey(const std::string& key) const;
    void removeKey(const std::string& key);
    std::vector<std::string> getKeys() const;

private:
    std::unordered_map<std::string, std::any> m_config;
    std::unordered_map<std::string, std::string> m_values;
    std::string m_configPath;
    mutable std::mutex m_mutex;
    
    // Metodo helper per appiattire JSON annidati
    void flattenJson(const nlohmann::json& j, const std::string& prefix);
};

} // namespace CoreNS
