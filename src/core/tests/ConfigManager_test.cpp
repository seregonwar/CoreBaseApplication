#include <gtest/gtest.h>
#include "CoreClass/ConfigManager.h"

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup per ogni test
    }
    
    void TearDown() override {
        // Cleanup per ogni test
    }
    
    CoreNS::ConfigManager configManager;
};

TEST_F(ConfigManagerTest, LoadConfig) {
    // Test caricamento configurazione valida
    bool result = configManager.loadConfig("src/core/tests/test_config.json");
    EXPECT_TRUE(result);
    
    // Verifica valori caricati
    EXPECT_EQ(configManager.getConfigString("test.name"), "CoreBaseApplication Test");
    EXPECT_EQ(configManager.getConfigString("test.version"), "1.0.0-test");
    EXPECT_EQ(configManager.getConfigString("test.environment"), "testing");
}

TEST_F(ConfigManagerTest, LoadInvalidConfig) {
    // Test caricamento file inesistente
    bool result = configManager.loadConfig("nonexistent_config.json");
    EXPECT_FALSE(result);
}

TEST_F(ConfigManagerTest, GetAndSetValues) {
    // Carica configurazione di test
    configManager.loadConfig("src/core/tests/test_config.json");
    
    // Test getValue con chiavi esistenti
    EXPECT_EQ(configManager.getConfigString("logging.level"), "DEBUG");
    EXPECT_EQ(configManager.getConfigString("network.default_timeout_ms"), "5000");
    
    // Test getValue con chiave inesistente
    EXPECT_EQ(configManager.getConfigString("nonexistent.key"), "");
    
    // Test setValue
    configManager.setConfigString("test.new_key", "new_value");
    EXPECT_EQ(configManager.getConfigString("test.new_key"), "new_value");
    
    // Test modifica valore esistente
    configManager.setConfigString("test.name", "Modified Test Name");
    EXPECT_EQ(configManager.getConfigString("test.name"), "Modified Test Name");
}

TEST_F(ConfigManagerTest, SaveConfig) {
    // Carica configurazione di test
    configManager.loadConfig("src/core/tests/test_config.json");
    
    // Modifica alcuni valori
    configManager.setConfigString("test.modified", "true");
    configManager.setConfigString("test.timestamp", "2024-01-01T00:00:00Z");
    
    // Salva configurazione
    bool result = configManager.saveConfig("src/core/tests/test_config_output.json");
    EXPECT_TRUE(result);
    
    // Verifica che il file sia stato creato e contenga i valori modificati
    CoreNS::ConfigManager newConfigManager;
    bool loadResult = newConfigManager.loadConfig("src/core/tests/test_config_output.json");
    EXPECT_TRUE(loadResult);
    EXPECT_EQ(newConfigManager.getConfigString("test.modified"), "true");
    EXPECT_EQ(newConfigManager.getConfigString("test.timestamp"), "2024-01-01T00:00:00Z");
}

TEST_F(ConfigManagerTest, NestedKeys) {
    // Carica configurazione di test
    configManager.loadConfig("src/core/tests/test_config.json");
    
    // Test accesso a chiavi annidate
    EXPECT_EQ(configManager.getConfigString("network.protocols.tcp.enabled"), "true");
    EXPECT_EQ(configManager.getConfigString("network.protocols.tcp.buffer_size"), "1024");
    EXPECT_EQ(configManager.getConfigString("security.authentication.enabled"), "false");
    
    // Test modifica di chiavi annidate
    configManager.setConfigString("network.protocols.tcp.buffer_size", "2048");
    EXPECT_EQ(configManager.getConfigString("network.protocols.tcp.buffer_size"), "2048");
}

TEST_F(ConfigManagerTest, BooleanValues) {
    // Carica configurazione di test
    configManager.loadConfig("src/core/tests/test_config.json");
    
    // Test valori booleani
    EXPECT_EQ(configManager.getConfigString("logging.console_output"), "true");
    EXPECT_EQ(configManager.getConfigString("logging.file_output"), "false");
    EXPECT_EQ(configManager.getConfigString("development.debug_mode"), "true");
}

TEST_F(ConfigManagerTest, NumericValues) {
    // Carica configurazione di test
    configManager.loadConfig("src/core/tests/test_config.json");
    
    // Test valori numerici
    EXPECT_EQ(configManager.getConfigString("network.default_timeout_ms"), "5000");
    EXPECT_EQ(configManager.getConfigString("network.max_connections"), "10");
    EXPECT_EQ(configManager.getConfigString("monitoring.report_interval_seconds"), "10");
}

TEST_F(ConfigManagerTest, EmptyAndNullValues) {
    // Test gestione valori vuoti e null
    configManager.setConfigString("test.empty_string", "");
    configManager.setConfigString("test.null_value", "null");
    
    EXPECT_EQ(configManager.getConfigString("test.empty_string"), "");
    EXPECT_EQ(configManager.getConfigString("test.null_value"), "null");
    
    // Test chiave inesistente
    EXPECT_EQ(configManager.getConfigString("test.nonexistent"), "");
}