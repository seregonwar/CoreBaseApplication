# Basic Usage Examples

This document provides practical examples of how to use CoreBaseApplication (CBA) for common scenarios. Each example includes complete, runnable code with explanations.

## Table of Contents

- [Hello World Application](#hello-world-application)
- [Configuration Management](#configuration-management)
- [Logging Examples](#logging-examples)
- [HTTP Client Usage](#http-client-usage)
- [System Monitoring](#system-monitoring)
- [WebSocket Communication](#websocket-communication)
- [Error Handling](#error-handling)
- [Performance Monitoring](#performance-monitoring)

## Hello World Application

### Simple Console Application

```cpp
#include "CoreAPI.h"
#include <iostream>

int main() {
    try {
        // Initialize the framework
        if (!CoreAPI::initialize()) {
            std::cerr << "Failed to initialize CoreAPI" << std::endl;
            return -1;
        }
        
        std::cout << "CoreBaseApplication " << CoreAPI::getVersion() << " initialized successfully!" << std::endl;
        
        // Get the singleton instance
        auto& api = CoreAPI::getInstance();
        
        // Start the application
        if (api.start()) {
            std::cout << "Application started successfully!" << std::endl;
            
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::seconds(2));
            
            // Stop the application
            api.stop();
            std::cout << "Application stopped." << std::endl;
        }
        
        // Clean shutdown
        CoreAPI::shutdown();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
```

### Event-Driven Application

```cpp
#include "CoreAPI.h"
#include <iostream>
#include <atomic>

std::atomic<bool> running{true};

void handleEvent(const Event& event) {
    std::cout << "Received event: " << event.getType() << std::endl;
    
    if (event.getType() == "shutdown") {
        running = false;
    }
}

int main() {
    try {
        // Initialize with custom config
        if (!CoreAPI::initialize("config/app.json")) {
            std::cerr << "Failed to initialize CoreAPI" << std::endl;
            return -1;
        }
        
        auto& api = CoreAPI::getInstance();
        
        // Set event callback
        api.setEventCallback(handleEvent);
        
        // Start the application
        if (!api.start()) {
            std::cerr << "Failed to start application" << std::endl;
            return -1;
        }
        
        std::cout << "Application running. Press Ctrl+C to exit." << std::endl;
        
        // Main event loop
        while (running && api.isRunning()) {
            api.processEvents();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        api.stop();
        CoreAPI::shutdown();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
```

## Configuration Management

### Loading and Using Configuration

```cpp
#include "ConfigManager.h"
#include <iostream>

int main() {
    try {
        auto& config = ConfigManager::getInstance();
        
        // Load configuration from file
        if (!config.loadConfig("config/settings.json")) {
            std::cerr << "Failed to load configuration" << std::endl;
            return -1;
        }
        
        // Read various configuration values
        std::string appName = config.getConfigString("application.name", "DefaultApp");
        int port = config.getConfigInt("server.port", 8080);
        bool debugMode = config.getConfigBool("application.debug", false);
        double timeout = config.getConfigDouble("network.timeout", 30.0);
        
        std::cout << "Application: " << appName << std::endl;
        std::cout << "Port: " << port << std::endl;
        std::cout << "Debug Mode: " << (debugMode ? "enabled" : "disabled") << std::endl;
        std::cout << "Timeout: " << timeout << " seconds" << std::endl;
        
        // Read array configuration
        auto servers = config.getConfigArray("database.servers");
        std::cout << "Database servers:" << std::endl;
        for (const auto& server : servers) {
            std::cout << "  - " << server << std::endl;
        }
        
        // Modify configuration
        config.setConfigString("application.status", "running");
        config.setConfigInt("runtime.pid", getpid());
        
        // Save updated configuration
        config.saveConfig("config/runtime.json");
        
    } catch (const ConfigException& e) {
        std::cerr << "Configuration error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
```

### Configuration with Change Notifications

```cpp
#include "ConfigManager.h"
#include <iostream>

void onConfigChange(const std::string& key) {
    std::cout << "Configuration changed: " << key << std::endl;
    
    auto& config = ConfigManager::getInstance();
    
    if (key == "logging.level") {
        std::string newLevel = config.getConfigString(key);
        std::cout << "New logging level: " << newLevel << std::endl;
        // Update logger level here
    }
}

int main() {
    auto& config = ConfigManager::getInstance();
    
    // Set change notification callback
    config.setChangeCallback(onConfigChange);
    
    // Load initial configuration
    config.loadConfig("config/app.json");
    
    // Simulate configuration changes
    config.setConfigString("logging.level", "DEBUG");
    config.setConfigBool("features.newFeature", true);
    
    return 0;
}
```

## Logging Examples

### Basic Logging

```cpp
#include "Logger.h"
#include <iostream>

int main() {
    try {
        // Initialize logger with configuration
        Logger::initialize("config/logging.json");
        
        auto& logger = Logger::getInstance();
        
        // Set logging level
        logger.setLevel(Logger::Level::DEBUG);
        
        // Basic logging
        LOG_INFO("Application started");
        LOG_DEBUG("Debug information");
        LOG_WARNING("This is a warning");
        LOG_ERROR("An error occurred");
        
        // Logging with categories
        LOG_INFO_CAT("User logged in", "AUTH");
        LOG_DEBUG_CAT("Database query executed", "DB");
        LOG_ERROR_CAT("Network connection failed", "NETWORK");
        
        // Direct logging with custom formatting
        logger.log(Logger::Level::INFO, "Processing request #123", "HTTP");
        
        // Ensure all logs are written
        logger.flush();
        
    } catch (const std::exception& e) {
        std::cerr << "Logging error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
```

### Advanced Logging with Custom Appenders

```cpp
#include "Logger.h"
#include "LogAppenders.h"
#include <memory>

int main() {
    auto& logger = Logger::getInstance();
    
    // Add console appender with colors
    auto consoleAppender = std::make_unique<ConsoleAppender>("console");
    consoleAppender->setColorEnabled(true);
    logger.addAppender(std::move(consoleAppender));
    
    // Add file appender
    auto fileAppender = std::make_unique<FileAppender>("logs/app.log", "file");
    logger.addAppender(std::move(fileAppender));
    
    // Add rotating file appender (10MB max, keep 5 files)
    auto rotatingAppender = std::make_unique<RotatingFileAppender>(
        "logs/app_rotating.log", 
        10 * 1024 * 1024,  // 10MB
        5,                  // Keep 5 files
        "rotating"
    );
    logger.addAppender(std::move(rotatingAppender));
    
    // Test logging
    for (int i = 0; i < 1000; ++i) {
        LOG_INFO("Log message #" + std::to_string(i));
        
        if (i % 100 == 0) {
            LOG_WARNING("Checkpoint reached: " + std::to_string(i));
        }
    }
    
    return 0;
}
```

## HTTP Client Usage

### Simple HTTP Requests

```cpp
#include "HttpClient.h"
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
    try {
        // Create HTTP client
        HttpClient client("https://api.example.com");
        
        // Set default headers
        client.setDefaultHeaders({
            {"User-Agent", "CoreBaseApplication/1.0"},
            {"Accept", "application/json"}
        });
        
        // Simple GET request
        auto response = client.get("/users/123");
        
        if (response.success && response.statusCode == 200) {
            std::cout << "User data: " << response.body << std::endl;
            
            // Parse JSON response
            auto userData = nlohmann::json::parse(response.body);
            std::cout << "User name: " << userData["name"] << std::endl;
        } else {
            std::cerr << "Request failed: " << response.statusCode 
                      << " " << response.statusMessage << std::endl;
        }
        
        // POST request with JSON data
        nlohmann::json newUser = {
            {"name", "John Doe"},
            {"email", "john@example.com"},
            {"age", 30}
        };
        
        auto postResponse = client.post("/users", newUser.dump(), {
            {"Content-Type", "application/json"}
        });
        
        if (postResponse.success && postResponse.statusCode == 201) {
            std::cout << "User created successfully" << std::endl;
            std::cout << "Response: " << postResponse.body << std::endl;
        }
        
        // PUT request to update user
        nlohmann::json updateData = {
            {"age", 31}
        };
        
        auto putResponse = client.put("/users/123", updateData.dump(), {
            {"Content-Type", "application/json"}
        });
        
        if (putResponse.success) {
            std::cout << "User updated successfully" << std::endl;
        }
        
        // DELETE request
        auto deleteResponse = client.del("/users/123");
        
        if (deleteResponse.success && deleteResponse.statusCode == 204) {
            std::cout << "User deleted successfully" << std::endl;
        }
        
    } catch (const NetworkException& e) {
        std::cerr << "Network error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
```

### RESTful API Client

```cpp
#include "RestClient.h"
#include <iostream>
#include <nlohmann/json.hpp>

// User model
struct User {
    std::string id;
    std::string name;
    std::string email;
    int age;
    
    // JSON serialization
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(User, id, name, email, age)
};

int main() {
    try {
        // Create REST client for User resources
        RestClient<User> userClient("https://api.example.com/users");
        
        // Set authentication
        userClient.setAuthToken("your-jwt-token-here");
        
        // Get all users
        auto allUsers = userClient.getAll();
        std::cout << "Found " << allUsers.size() << " users" << std::endl;
        
        // Get specific user
        auto user = userClient.get("123");
        if (user) {
            std::cout << "User: " << user->name << " (" << user->email << ")" << std::endl;
        }
        
        // Create new user
        User newUser{
            "",  // ID will be assigned by server
            "Jane Smith",
            "jane@example.com",
            28
        };
        
        auto createdUser = userClient.create(newUser);
        if (createdUser) {
            std::cout << "Created user with ID: " << createdUser->id << std::endl;
        }
        
        // Update user
        if (createdUser) {
            createdUser->age = 29;
            auto updatedUser = userClient.update(createdUser->id, *createdUser);
            if (updatedUser) {
                std::cout << "Updated user age to: " << updatedUser->age << std::endl;
            }
        }
        
        // Search users
        auto searchResults = userClient.search("jane");
        std::cout << "Search found " << searchResults.size() << " users" << std::endl;
        
        // Paginated results
        auto page = userClient.getPage(1, 10);  // Page 1, 10 items per page
        std::cout << "Page 1: " << page.items.size() << " items" << std::endl;
        std::cout << "Total: " << page.totalCount << " users" << std::endl;
        std::cout << "Has next page: " << (page.hasNext ? "yes" : "no") << std::endl;
        
        // Delete user
        if (createdUser) {
            bool deleted = userClient.remove(createdUser->id);
            if (deleted) {
                std::cout << "User deleted successfully" << std::endl;
            }
        }
        
    } catch (const NetworkException& e) {
        std::cerr << "Network error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
```

## System Monitoring

### Basic System Metrics

```cpp
#include "SystemMonitor.h"
#include <iostream>
#include <iomanip>

int main() {
    try {
        auto& monitor = SystemMonitor::getInstance();
        
        // Get current system metrics
        auto metrics = monitor.getCurrentMetrics();
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "=== System Metrics ===" << std::endl;
        std::cout << "CPU Usage: " << metrics.cpuUsage << "%" << std::endl;
        std::cout << "Memory Usage: " << metrics.memoryUsagePercent << "% "
                  << "(" << (metrics.memoryUsed / 1024 / 1024) << " MB / "
                  << (metrics.memoryTotal / 1024 / 1024) << " MB)" << std::endl;
        std::cout << "Disk Usage: " << metrics.diskUsagePercent << "% "
                  << "(" << (metrics.diskUsed / 1024 / 1024 / 1024) << " GB / "
                  << (metrics.diskTotal / 1024 / 1024 / 1024) << " GB)" << std::endl;
        std::cout << "Network In: " << (metrics.networkBytesIn / 1024) << " KB/s" << std::endl;
        std::cout << "Network Out: " << (metrics.networkBytesOut / 1024) << " KB/s" << std::endl;
        
        // Individual metric queries
        double cpuUsage = monitor.getCpuUsage();
        size_t memoryUsage = monitor.getMemoryUsage();
        double diskUsage = monitor.getDiskUsagePercent("/");
        
        std::cout << "\n=== Individual Metrics ===" << std::endl;
        std::cout << "CPU: " << cpuUsage << "%" << std::endl;
        std::cout << "Memory: " << (memoryUsage / 1024 / 1024) << " MB" << std::endl;
        std::cout << "Disk: " << diskUsage << "%" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Monitoring error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
```

### Continuous Monitoring

```cpp
#include "SystemMonitor.h"
#include <iostream>
#include <atomic>
#include <thread>

std::atomic<bool> monitoring{true};

void onMetricsUpdate(const SystemMonitor::SystemMetrics& metrics) {
    static int counter = 0;
    counter++;
    
    std::cout << "[" << counter << "] "
              << "CPU: " << std::fixed << std::setprecision(1) << metrics.cpuUsage << "% "
              << "MEM: " << metrics.memoryUsagePercent << "% "
              << "DISK: " << metrics.diskUsagePercent << "%" << std::endl;
    
    // Alert on high resource usage
    if (metrics.cpuUsage > 80.0) {
        std::cout << "WARNING: High CPU usage detected!" << std::endl;
    }
    
    if (metrics.memoryUsagePercent > 90.0) {
        std::cout << "WARNING: High memory usage detected!" << std::endl;
    }
    
    if (metrics.diskUsagePercent > 95.0) {
        std::cout << "CRITICAL: Disk space critically low!" << std::endl;
    }
}

int main() {
    try {
        auto& monitor = SystemMonitor::getInstance();
        
        // Set metrics callback
        monitor.setMetricsCallback(onMetricsUpdate);
        
        // Start monitoring every 2 seconds
        monitor.startMonitoring(std::chrono::milliseconds(2000));
        
        std::cout << "System monitoring started. Press Enter to stop..." << std::endl;
        
        // Wait for user input
        std::cin.get();
        
        // Stop monitoring
        monitor.stopMonitoring();
        
        // Get historical data
        auto history = monitor.getHistoricalMetrics(10);  // Last 10 measurements
        
        std::cout << "\n=== Historical Data (Last 10 measurements) ===" << std::endl;
        for (size_t i = 0; i < history.size(); ++i) {
            const auto& m = history[i];
            std::cout << "[" << i << "] CPU: " << m.cpuUsage 
                      << "% MEM: " << m.memoryUsagePercent << "%" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Monitoring error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
```

## WebSocket Communication

### WebSocket Client

```cpp
#include "WebSocketClient.h"
#include <iostream>
#include <atomic>
#include <thread>

std::atomic<bool> connected{false};

void onConnect() {
    std::cout << "Connected to WebSocket server" << std::endl;
    connected = true;
}

void onDisconnect(int code, const std::string& reason) {
    std::cout << "Disconnected from WebSocket server: " << code << " - " << reason << std::endl;
    connected = false;
}

void onMessage(const WebSocketClient::Message& message) {
    if (message.type == WebSocketClient::MessageType::TEXT) {
        std::string text(message.data.begin(), message.data.end());
        std::cout << "Received: " << text << std::endl;
        
        // Echo the message back
        // (This would be done in the main loop)
    } else if (message.type == WebSocketClient::MessageType::BINARY) {
        std::cout << "Received binary message (" << message.data.size() << " bytes)" << std::endl;
    }
}

void onError(const std::string& error) {
    std::cerr << "WebSocket error: " << error << std::endl;
}

int main() {
    try {
        // Create WebSocket client
        WebSocketClient client("ws://localhost:8080/websocket");
        
        // Set event handlers
        client.setOnConnect(onConnect);
        client.setOnDisconnect(onDisconnect);
        client.setOnMessage(onMessage);
        client.setOnError(onError);
        
        // Set custom headers
        client.setHeaders({
            {"Authorization", "Bearer your-token-here"},
            {"User-Agent", "CoreBaseApplication/1.0"}
        });
        
        // Connect to server
        std::cout << "Connecting to WebSocket server..." << std::endl;
        if (!client.connect(std::chrono::milliseconds(5000))) {
            std::cerr << "Failed to connect to WebSocket server" << std::endl;
            return -1;
        }
        
        // Wait for connection
        while (!connected && client.getState() == WebSocketClient::State::CONNECTING) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        if (!connected) {
            std::cerr << "Connection failed" << std::endl;
            return -1;
        }
        
        // Send messages
        client.sendText("Hello, WebSocket server!");
        client.sendText("{\"type\": \"greeting\", \"message\": \"Hello from CBA\"}");
        
        // Send binary data
        std::vector<uint8_t> binaryData = {0x01, 0x02, 0x03, 0x04, 0x05};
        client.sendBinary(binaryData);
        
        // Send ping
        client.ping("ping-payload");
        
        // Keep connection alive for a while
        std::cout << "WebSocket client running. Press Enter to disconnect..." << std::endl;
        std::cin.get();
        
        // Disconnect
        client.disconnect();
        
        // Wait for disconnection
        while (client.getState() != WebSocketClient::State::DISCONNECTED) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "WebSocket client disconnected" << std::endl;
        
    } catch (const NetworkException& e) {
        std::cerr << "WebSocket error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
```

## Error Handling

### Exception Handling Patterns

```cpp
#include "CoreAPI.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "HttpClient.h"
#include <iostream>

int main() {
    try {
        // Initialize framework with error handling
        if (!CoreAPI::initialize("config/app.json")) {
            throw CoreException("Failed to initialize CoreAPI", "INIT");
        }
        
        auto& config = ConfigManager::getInstance();
        auto& logger = Logger::getInstance();
        
        // Configuration error handling
        try {
            std::string dbUrl = config.getConfigString("database.url");
            int dbPort = config.getConfigInt("database.port");
            
            LOG_INFO("Database configuration loaded: " + dbUrl + ":" + std::to_string(dbPort));
            
        } catch (const ConfigException& e) {
            LOG_ERROR("Configuration error: " + std::string(e.what()));
            
            // Use default values
            std::string dbUrl = "localhost";
            int dbPort = 5432;
            
            LOG_WARNING("Using default database configuration");
        }
        
        // Network error handling with retry
        HttpClient client("https://api.example.com");
        
        int maxRetries = 3;
        int retryCount = 0;
        bool success = false;
        
        while (retryCount < maxRetries && !success) {
            try {
                auto response = client.get("/health");
                
                if (response.success && response.statusCode == 200) {
                    LOG_INFO("Health check successful");
                    success = true;
                } else {
                    throw NetworkException("Health check failed: " + std::to_string(response.statusCode));
                }
                
            } catch (const NetworkException& e) {
                retryCount++;
                LOG_WARNING("Network error (attempt " + std::to_string(retryCount) + "/" + 
                           std::to_string(maxRetries) + "): " + e.what());
                
                if (retryCount < maxRetries) {
                    std::this_thread::sleep_for(std::chrono::seconds(retryCount * 2));  // Exponential backoff
                } else {
                    LOG_ERROR("Max retries exceeded, giving up");
                    throw;  // Re-throw the exception
                }
            }
        }
        
        // File operation error handling
        try {
            std::string content = FileUtils::readTextFile("data/important.txt");
            LOG_INFO("File read successfully");
            
        } catch (const FileException& e) {
            LOG_ERROR("File error: " + std::string(e.what()));
            
            // Try to create the file with default content
            try {
                FileUtils::createDirectories("data");
                FileUtils::writeTextFile("data/important.txt", "Default content");
                LOG_INFO("Created file with default content");
                
            } catch (const FileException& createError) {
                LOG_CRITICAL("Cannot create file: " + std::string(createError.what()));
                throw;  // This is critical, re-throw
            }
        }
        
        // Validation error handling
        try {
            std::string email = config.getConfigString("user.email");
            
            if (!StringUtils::contains(email, "@")) {
                throw ValidationException("Invalid email format", "user.email");
            }
            
            LOG_INFO("Email validation passed: " + email);
            
        } catch (const ValidationException& e) {
            LOG_ERROR("Validation error in field '" + e.getField() + "': " + e.what());
            
            // Set a default email
            config.setConfigString("user.email", "default@example.com");
            LOG_WARNING("Using default email address");
        }
        
        CoreAPI::shutdown();
        
    } catch (const CoreException& e) {
        std::cerr << "Core error [" << e.getCategory() << "]: " << e.what() << std::endl;
        return -1;
        
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return -1;
        
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return -1;
    }
    
    return 0;
}
```

## Performance Monitoring

### Performance Counters

```cpp
#include "PerformanceCounter.h"
#include "Logger.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>

// Simulate some work
void doWork(int iterations) {
    std::vector<int> data;
    for (int i = 0; i < iterations; ++i) {
        data.push_back(i * i);
    }
    std::sort(data.begin(), data.end());
}

int main() {
    try {
        Logger::initialize();
        auto& logger = Logger::getInstance();
        
        // Create performance counters
        PerformanceCounter totalCounter("total_execution");
        PerformanceCounter workCounter("work_phase");
        PerformanceCounter sortCounter("sorting_phase");
        
        totalCounter.start();
        
        LOG_INFO("Starting performance test");
        
        // Phase 1: Data generation
        workCounter.start();
        doWork(100000);
        workCounter.stop();
        
        LOG_INFO("Work phase completed in " + 
                std::to_string(workCounter.getElapsedMilliseconds()) + " ms");
        
        // Phase 2: More intensive work
        sortCounter.start();
        doWork(500000);
        sortCounter.stop();
        
        LOG_INFO("Sorting phase completed in " + 
                std::to_string(sortCounter.getElapsedMilliseconds()) + " ms");
        
        totalCounter.stop();
        
        // Performance summary
        std::cout << "\n=== Performance Summary ===" << std::endl;
        std::cout << "Total execution time: " << totalCounter.getElapsedMilliseconds() << " ms" << std::endl;
        std::cout << "Work phase: " << workCounter.getElapsedMilliseconds() << " ms" << std::endl;
        std::cout << "Sorting phase: " << sortCounter.getElapsedMilliseconds() << " ms" << std::endl;
        
        // Calculate percentages
        double totalMs = totalCounter.getElapsedMilliseconds();
        double workPercent = (workCounter.getElapsedMilliseconds() / totalMs) * 100.0;
        double sortPercent = (sortCounter.getElapsedMilliseconds() / totalMs) * 100.0;
        
        std::cout << "\nTime distribution:" << std::endl;
        std::cout << "  Work phase: " << std::fixed << std::setprecision(1) << workPercent << "%" << std::endl;
        std::cout << "  Sorting phase: " << sortPercent << "%" << std::endl;
        
        // Log performance metrics
        LOG_INFO("Performance test completed - Total: " + 
                std::to_string(totalCounter.getElapsedMilliseconds()) + "ms");
        
    } catch (const std::exception& e) {
        std::cerr << "Performance monitoring error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
```

### Benchmarking Functions

```cpp
#include "PerformanceCounter.h"
#include <iostream>
#include <functional>
#include <vector>
#include <numeric>

class Benchmark {
public:
    struct Result {
        std::string name;
        double averageMs;
        double minMs;
        double maxMs;
        double stdDevMs;
        int iterations;
    };
    
    static Result run(const std::string& name, std::function<void()> func, int iterations = 100) {
        std::vector<double> times;
        times.reserve(iterations);
        
        std::cout << "Running benchmark: " << name << " (" << iterations << " iterations)..." << std::endl;
        
        for (int i = 0; i < iterations; ++i) {
            PerformanceCounter counter(name + "_" + std::to_string(i));
            
            counter.start();
            func();
            counter.stop();
            
            times.push_back(counter.getElapsedMilliseconds());
        }
        
        // Calculate statistics
        double sum = std::accumulate(times.begin(), times.end(), 0.0);
        double average = sum / times.size();
        
        double minTime = *std::min_element(times.begin(), times.end());
        double maxTime = *std::max_element(times.begin(), times.end());
        
        // Calculate standard deviation
        double variance = 0.0;
        for (double time : times) {
            variance += (time - average) * (time - average);
        }
        variance /= times.size();
        double stdDev = std::sqrt(variance);
        
        return Result{
            name,
            average,
            minTime,
            maxTime,
            stdDev,
            iterations
        };
    }
    
    static void printResult(const Result& result) {
        std::cout << "\n=== Benchmark Results: " << result.name << " ===" << std::endl;
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Iterations: " << result.iterations << std::endl;
        std::cout << "Average: " << result.averageMs << " ms" << std::endl;
        std::cout << "Min: " << result.minMs << " ms" << std::endl;
        std::cout << "Max: " << result.maxMs << " ms" << std::endl;
        std::cout << "Std Dev: " << result.stdDevMs << " ms" << std::endl;
        std::cout << "Coefficient of Variation: " << (result.stdDevMs / result.averageMs * 100.0) << "%" << std::endl;
    }
};

// Test functions
void vectorPushBack() {
    std::vector<int> vec;
    for (int i = 0; i < 10000; ++i) {
        vec.push_back(i);
    }
}

void vectorReserveAndPushBack() {
    std::vector<int> vec;
    vec.reserve(10000);
    for (int i = 0; i < 10000; ++i) {
        vec.push_back(i);
    }
}

void stringConcatenation() {
    std::string result;
    for (int i = 0; i < 1000; ++i) {
        result += "test" + std::to_string(i) + " ";
    }
}

void stringStreamConcatenation() {
    std::stringstream ss;
    for (int i = 0; i < 1000; ++i) {
        ss << "test" << i << " ";
    }
    std::string result = ss.str();
}

int main() {
    try {
        std::cout << "Starting performance benchmarks..." << std::endl;
        
        // Run benchmarks
        auto result1 = Benchmark::run("Vector Push Back", vectorPushBack, 1000);
        Benchmark::printResult(result1);
        
        auto result2 = Benchmark::run("Vector Reserve + Push Back", vectorReserveAndPushBack, 1000);
        Benchmark::printResult(result2);
        
        auto result3 = Benchmark::run("String Concatenation", stringConcatenation, 100);
        Benchmark::printResult(result3);
        
        auto result4 = Benchmark::run("StringStream Concatenation", stringStreamConcatenation, 100);
        Benchmark::printResult(result4);
        
        // Compare results
        std::cout << "\n=== Performance Comparison ===" << std::endl;
        std::cout << "Vector operations:" << std::endl;
        std::cout << "  Reserve speedup: " << std::fixed << std::setprecision(2) 
                  << (result1.averageMs / result2.averageMs) << "x faster" << std::endl;
        
        std::cout << "String operations:" << std::endl;
        std::cout << "  StringStream speedup: " << std::fixed << std::setprecision(2) 
                  << (result3.averageMs / result4.averageMs) << "x faster" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Benchmark error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
```

---

## Building and Running Examples

### CMake Configuration

Add this to your `CMakeLists.txt` to build the examples:

```cmake
# Examples
file(GLOB EXAMPLE_SOURCES "examples/*.cpp")

foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
    target_link_libraries(${EXAMPLE_NAME} CoreBaseApplication)
    set_target_properties(${EXAMPLE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
    )
endforeach()
```

### Running Examples

```bash
# Build all examples
cmake --build build --target all

# Run specific example
./build/examples/hello_world
./build/examples/config_example
./build/examples/logging_example
./build/examples/http_client_example
```

---

## Next Steps

- Explore the [API Reference](../api/reference.md) for detailed documentation
- Check out [Advanced Examples](advanced-usage.md) for more complex scenarios
- Read the [Development Guide](../development/setup.md) to contribute to the project
- Visit the [Architecture Overview](../architecture/overview.md) to understand the framework design

---

*These examples demonstrate the core functionality of CoreBaseApplication. For production use, always include proper error handling, logging, and configuration management.*