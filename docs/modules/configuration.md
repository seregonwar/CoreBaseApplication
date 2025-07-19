# Configuration Management

The Configuration Management module provides a flexible, type-safe configuration system with support for nested configurations, environment variable substitution, and runtime updates.

## Features

- **Type-Safe Access**: Get/set values with automatic type conversion
- **Nested Configuration**: Support for hierarchical configuration structures
- **Multiple Formats**: JSON, XML, INI, and YAML support
- **Environment Variables**: Automatic substitution and fallback
- **Runtime Updates**: Dynamic configuration changes without restart
- **Validation**: Schema-based configuration validation
- **Encryption**: Sensitive configuration data encryption
- **Hot Reload**: Automatic configuration file monitoring

## Quick Start

```cpp
#include "core/CoreClass/ConfigManager.h"

using namespace Core;

// Initialize configuration manager
ConfigManager& config = ConfigManager::getInstance();

// Load configuration from file
if (!config.loadConfig("config/application.json")) {
    std::cerr << "Failed to load configuration" << std::endl;
    return -1;
}

// Get configuration values
std::string dbHost = config.getConfigString("database.host");
int dbPort = config.getConfigInt("database.port");
bool enableLogging = config.getConfigBool("logging.enabled");
double timeout = config.getConfigDouble("network.timeout");

// Set configuration values
config.setConfigString("database.host", "localhost");
config.setConfigInt("database.port", 5432);
config.setConfigBool("logging.enabled", true);

// Save configuration
config.saveConfig("config/application.json");
```

## Configuration File Formats

### JSON Configuration

```json
{
  "application": {
    "name": "CoreBaseApplication",
    "version": "1.0.0",
    "environment": "${APP_ENV:development}"
  },
  "database": {
    "host": "${DB_HOST:localhost}",
    "port": 5432,
    "name": "myapp",
    "username": "${DB_USER:admin}",
    "password": "${DB_PASS:}",
    "pool": {
      "minSize": 5,
      "maxSize": 20,
      "timeout": 30.0
    }
  },
  "logging": {
    "enabled": true,
    "level": "INFO",
    "file": "logs/application.log",
    "rotation": {
      "enabled": true,
      "maxSize": "10MB",
      "maxFiles": 5
    }
  },
  "network": {
    "timeout": 30.0,
    "retries": 3,
    "endpoints": [
      "https://api.example.com",
      "https://backup.example.com"
    ]
  }
}
```

### YAML Configuration

```yaml
application:
  name: CoreBaseApplication
  version: 1.0.0
  environment: ${APP_ENV:development}

database:
  host: ${DB_HOST:localhost}
  port: 5432
  name: myapp
  username: ${DB_USER:admin}
  password: ${DB_PASS:}
  pool:
    minSize: 5
    maxSize: 20
    timeout: 30.0

logging:
  enabled: true
  level: INFO
  file: logs/application.log
  rotation:
    enabled: true
    maxSize: 10MB
    maxFiles: 5

network:
  timeout: 30.0
  retries: 3
  endpoints:
    - https://api.example.com
    - https://backup.example.com
```

## API Reference

### ConfigManager Class

```cpp
class ConfigManager {
public:
    // Singleton access
    static ConfigManager& getInstance();
    
    // Configuration loading/saving
    bool loadConfig(const std::string& filePath);
    bool saveConfig(const std::string& filePath);
    bool reloadConfig();
    
    // String values
    std::string getConfigString(const std::string& key, const std::string& defaultValue = "");
    void setConfigString(const std::string& key, const std::string& value);
    
    // Integer values
    int getConfigInt(const std::string& key, int defaultValue = 0);
    void setConfigInt(const std::string& key, int value);
    
    // Boolean values
    bool getConfigBool(const std::string& key, bool defaultValue = false);
    void setConfigBool(const std::string& key, bool value);
    
    // Double values
    double getConfigDouble(const std::string& key, double defaultValue = 0.0);
    void setConfigDouble(const std::string& key, double value);
    
    // Array values
    std::vector<std::string> getConfigArray(const std::string& key);
    void setConfigArray(const std::string& key, const std::vector<std::string>& values);
    
    // Configuration existence
    bool hasConfig(const std::string& key);
    void removeConfig(const std::string& key);
    
    // Configuration sections
    std::vector<std::string> getConfigKeys(const std::string& prefix = "");
    std::map<std::string, std::string> getConfigSection(const std::string& section);
    
    // Validation
    bool validateConfig(const std::string& schemaPath);
    std::vector<std::string> getValidationErrors();
    
    // Change notifications
    void addChangeListener(const std::string& key, std::function<void(const std::string&)> callback);
    void removeChangeListener(const std::string& key);
    
    // Environment variable substitution
    void enableEnvironmentSubstitution(bool enable = true);
    std::string substituteEnvironmentVariables(const std::string& value);
};
```

## Advanced Usage

### Environment Variable Substitution

```cpp
// Enable environment variable substitution
config.enableEnvironmentSubstitution(true);

// Configuration with environment variables
// Format: ${VAR_NAME:default_value}
config.setConfigString("database.host", "${DB_HOST:localhost}");
config.setConfigString("database.password", "${DB_PASS:}");

// Get resolved values
std::string dbHost = config.getConfigString("database.host");
// Returns value of DB_HOST environment variable, or "localhost" if not set
```

### Configuration Validation

```cpp
// Define JSON schema for validation
std::string schema = R"(
{
  "type": "object",
  "properties": {
    "database": {
      "type": "object",
      "properties": {
        "host": {"type": "string"},
        "port": {"type": "integer", "minimum": 1, "maximum": 65535},
        "name": {"type": "string", "minLength": 1}
      },
      "required": ["host", "port", "name"]
    }
  },
  "required": ["database"]
}
)";

// Validate configuration
if (!config.validateConfig(schema)) {
    auto errors = config.getValidationErrors();
    for (const auto& error : errors) {
        std::cerr << "Validation error: " << error << std::endl;
    }
}
```

### Change Notifications

```cpp
// Register change listener
config.addChangeListener("database.host", [](const std::string& newValue) {
    std::cout << "Database host changed to: " << newValue << std::endl;
    // Reconnect to database with new host
    reconnectDatabase();
});

// Register listener for entire section
config.addChangeListener("logging", [](const std::string& key) {
    std::cout << "Logging configuration changed: " << key << std::endl;
    // Reconfigure logging system
    reconfigureLogging();
});

// Change configuration (triggers listeners)
config.setConfigString("database.host", "new-host.example.com");
```

### Hot Reload

```cpp
// Enable automatic file monitoring
config.enableHotReload(true);

// Set reload interval (in seconds)
config.setReloadInterval(5);

// Register reload callback
config.setReloadCallback([]() {
    std::cout << "Configuration reloaded" << std::endl;
    // Reinitialize components that depend on configuration
    reinitializeComponents();
});
```

### Configuration Encryption

```cpp
// Enable encryption for sensitive data
config.enableEncryption(true);
config.setEncryptionKey("your-encryption-key");

// Set encrypted value
config.setConfigString("database.password", "sensitive-password", true);

// Get decrypted value
std::string password = config.getConfigString("database.password");
// Automatically decrypted when retrieved
```

### Configuration Profiles

```cpp
// Load different configurations based on environment
std::string env = std::getenv("APP_ENV") ? std::getenv("APP_ENV") : "development";

std::string configFile;
if (env == "production") {
    configFile = "config/production.json";
} else if (env == "staging") {
    configFile = "config/staging.json";
} else {
    configFile = "config/development.json";
}

config.loadConfig(configFile);

// Override with environment-specific settings
config.loadConfig("config/" + env + "-overrides.json", true); // merge = true
```

### Configuration Templates

```cpp
// Use configuration templates for common patterns
class DatabaseConfig {
public:
    DatabaseConfig(ConfigManager& config, const std::string& prefix = "database") 
        : m_config(config), m_prefix(prefix) {}
    
    std::string getHost() const {
        return m_config.getConfigString(m_prefix + ".host", "localhost");
    }
    
    int getPort() const {
        return m_config.getConfigInt(m_prefix + ".port", 5432);
    }
    
    std::string getDatabase() const {
        return m_config.getConfigString(m_prefix + ".name");
    }
    
    std::string getConnectionString() const {
        return "host=" + getHost() + " port=" + std::to_string(getPort()) + 
               " dbname=" + getDatabase();
    }
    
private:
    ConfigManager& m_config;
    std::string m_prefix;
};

// Usage
DatabaseConfig dbConfig(config);
std::string connectionString = dbConfig.getConnectionString();
```

## Configuration Best Practices

### 1. Use Hierarchical Structure

```cpp
// Good: Organized hierarchy
config.setConfigString("database.primary.host", "db1.example.com");
config.setConfigString("database.replica.host", "db2.example.com");
config.setConfigString("cache.redis.host", "redis.example.com");

// Avoid: Flat structure
config.setConfigString("db_primary_host", "db1.example.com");
config.setConfigString("db_replica_host", "db2.example.com");
config.setConfigString("redis_host", "redis.example.com");
```

### 2. Provide Sensible Defaults

```cpp
// Always provide defaults for optional settings
int maxConnections = config.getConfigInt("database.pool.maxSize", 20);
double timeout = config.getConfigDouble("network.timeout", 30.0);
bool enableCache = config.getConfigBool("cache.enabled", true);
```

### 3. Use Environment Variables for Deployment

```cpp
// Use environment variables for deployment-specific settings
config.setConfigString("database.host", "${DB_HOST:localhost}");
config.setConfigString("database.password", "${DB_PASS:}");
config.setConfigString("api.key", "${API_KEY:}");
```

### 4. Validate Critical Configuration

```cpp
// Validate critical configuration at startup
if (!config.hasConfig("database.host")) {
    throw std::runtime_error("Database host not configured");
}

if (config.getConfigString("database.password").empty()) {
    throw std::runtime_error("Database password not configured");
}
```

### 5. Use Configuration Objects

```cpp
// Create configuration objects for complex settings
struct ServerConfig {
    std::string host;
    int port;
    bool sslEnabled;
    int maxConnections;
    
    static ServerConfig fromConfig(ConfigManager& config, const std::string& prefix) {
        ServerConfig serverConfig;
        serverConfig.host = config.getConfigString(prefix + ".host", "localhost");
        serverConfig.port = config.getConfigInt(prefix + ".port", 8080);
        serverConfig.sslEnabled = config.getConfigBool(prefix + ".ssl.enabled", false);
        serverConfig.maxConnections = config.getConfigInt(prefix + ".maxConnections", 100);
        return serverConfig;
    }
};

// Usage
ServerConfig webServer = ServerConfig::fromConfig(config, "server.web");
ServerConfig apiServer = ServerConfig::fromConfig(config, "server.api");
```

## Troubleshooting

### Common Issues

**Configuration file not found:**
```cpp
if (!config.loadConfig("config.json")) {
    std::cerr << "Failed to load config.json" << std::endl;
    // Check file path and permissions
}
```

**Invalid JSON format:**
```cpp
// Use validation to catch format errors
if (!config.validateConfig(schemaPath)) {
    auto errors = config.getValidationErrors();
    for (const auto& error : errors) {
        std::cerr << "Config error: " << error << std::endl;
    }
}
```

**Environment variable not substituted:**
```cpp
// Ensure environment substitution is enabled
config.enableEnvironmentSubstitution(true);

// Check environment variable exists
if (!std::getenv("DB_HOST")) {
    std::cerr << "DB_HOST environment variable not set" << std::endl;
}
```

## Performance Considerations

- Configuration values are cached for fast access
- File monitoring uses efficient OS-specific mechanisms
- Environment variable substitution is performed once at load time
- Change notifications use minimal overhead event system

## Thread Safety

- All ConfigManager operations are thread-safe
- Multiple threads can read configuration concurrently
- Configuration updates are atomic
- Change listeners are called synchronously

## See Also

- [Advanced Logging](logging.md)
- [Monitoring System](monitoring.md)
- [Development Guide](../development/setup.md)
- [Best Practices](../development/best-practices.md)