# CoreAPI Module

The CoreAPI is the central orchestrator of the CoreBaseApplication framework. It provides the main interface for initializing, configuring, and managing all framework components.

## Overview

The CoreAPI follows the Facade pattern, providing a simplified interface to the complex subsystems of the framework. It manages the lifecycle of all components and ensures proper initialization and shutdown sequences.

## Key Features

- **Centralized Management**: Single point of control for all framework components
- **Lifecycle Management**: Proper initialization and shutdown sequences
- **Configuration Integration**: Seamless integration with the configuration system
- **Module Coordination**: Manages dynamic module loading and unloading
- **Error Handling**: Comprehensive error handling and reporting

## Basic Usage

### Initialization

```cpp
#include "core/CoreAPI.h"

int main() {
    // Configure initialization parameters
    Core::CoreAPI::ConfigParams params;
    params.configFilePath = "config.json";
    params.applicationName = "MyApplication";
    params.applicationVersion = "1.0.0";
    params.logLevel = Core::LogLevel::INFO;
    
    // Initialize the framework
    if (!Core::CoreAPI::initialize(params)) {
        std::cerr << "Failed to initialize CoreAPI" << std::endl;
        return 1;
    }
    
    // Get the core instance
    auto& core = Core::CoreAPI::getInstance();
    
    // Your application logic here
    
    // Shutdown gracefully
    Core::CoreAPI::shutdown();
    
    return 0;
}
```

### Configuration Parameters

```cpp
struct ConfigParams {
    std::string configFilePath;      // Path to configuration file
    std::string applicationName;     // Application name
    std::string applicationVersion;  // Application version
    LogLevel logLevel;              // Default log level
    bool enableMonitoring;          // Enable monitoring system
    bool enableSecurity;            // Enable security features
    std::vector<std::string> modules; // Modules to load
};
```

## Advanced Usage

### Custom Initialization

```cpp
// Custom initialization with specific subsystems
Core::CoreAPI::ConfigParams params;
params.configFilePath = "config.json";
params.enableMonitoring = true;
params.enableSecurity = false;  // Disable security for development

if (!Core::CoreAPI::initialize(params)) {
    // Handle initialization failure
    return false;
}

// Access specific subsystems
auto& core = Core::CoreAPI::getInstance();
auto& configManager = core.getConfigManager();
auto& logger = core.getLogger();
auto& monitoring = core.getMonitoringSystem();
```

### Module Management

```cpp
auto& core = Core::CoreAPI::getInstance();

// Load a dynamic module
if (core.loadModule("UserManagementModule")) {
    std::cout << "Module loaded successfully" << std::endl;
}

// Get loaded modules
auto modules = core.getLoadedModules();
for (const auto& module : modules) {
    std::cout << "Loaded module: " << module << std::endl;
}

// Unload a module
core.unloadModule("UserManagementModule");
```

### Configuration Access

```cpp
auto& core = Core::CoreAPI::getInstance();
auto& config = core.getConfigManager();

// Read configuration values
std::string dbHost = config.getConfigString("database.host");
int dbPort = config.getConfigInt("database.port");
bool enableSSL = config.getConfigBool("database.ssl");

// Update configuration
config.setConfigString("database.host", "new-host.example.com");
config.saveConfig();
```

## API Reference

### Static Methods

#### `initialize(const ConfigParams& params)`
Initializes the CoreAPI with the specified parameters.

**Parameters:**
- `params`: Configuration parameters for initialization

**Returns:**
- `bool`: `true` if initialization was successful, `false` otherwise

#### `shutdown()`
Shuts down the CoreAPI and all managed subsystems.

#### `getInstance()`
Returns the singleton instance of CoreAPI.

**Returns:**
- `CoreAPI&`: Reference to the CoreAPI instance

### Instance Methods

#### `getConfigManager()`
Returns a reference to the configuration manager.

**Returns:**
- `ConfigManager&`: Reference to the configuration manager

#### `getLogger()`
Returns a reference to the logging system.

**Returns:**
- `Logger&`: Reference to the logger

#### `getMonitoringSystem()`
Returns a reference to the monitoring system.

**Returns:**
- `MonitoringSystem&`: Reference to the monitoring system

#### `loadModule(const std::string& moduleName)`
Loads a dynamic module.

**Parameters:**
- `moduleName`: Name of the module to load

**Returns:**
- `bool`: `true` if the module was loaded successfully

#### `unloadModule(const std::string& moduleName)`
Unloads a dynamic module.

**Parameters:**
- `moduleName`: Name of the module to unload

**Returns:**
- `bool`: `true` if the module was unloaded successfully

#### `getLoadedModules()`
Returns a list of currently loaded modules.

**Returns:**
- `std::vector<std::string>`: List of loaded module names

## Error Handling

The CoreAPI provides comprehensive error handling:

```cpp
try {
    if (!Core::CoreAPI::initialize(params)) {
        // Check specific error conditions
        auto lastError = Core::CoreAPI::getLastError();
        std::cerr << "Initialization failed: " << lastError << std::endl;
        return 1;
    }
} catch (const Core::CoreException& e) {
    std::cerr << "Core exception: " << e.what() << std::endl;
    return 1;
} catch (const std::exception& e) {
    std::cerr << "Standard exception: " << e.what() << std::endl;
    return 1;
}
```

## Best Practices

1. **Always check return values** from `initialize()` and other methods
2. **Use RAII** for automatic cleanup in case of exceptions
3. **Initialize early** in your application's main function
4. **Shutdown gracefully** to ensure proper cleanup
5. **Handle exceptions** appropriately for your application context

## Thread Safety

The CoreAPI is thread-safe:
- Singleton instance creation is thread-safe
- All public methods can be called from multiple threads
- Internal state is protected with appropriate synchronization

## Performance Notes

- Initialization has a one-time cost but is optimized for speed
- Subsequent method calls have minimal overhead
- Module loading/unloading should be done during initialization/shutdown phases

## See Also

- [Configuration Management](../modules/configuration.md)
- [Logging System](../modules/logging.md)
- [Module System](../architecture/module-system.md)
- [Best Practices](../development/best-practices.md)