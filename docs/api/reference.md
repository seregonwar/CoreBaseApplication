# API Reference

This document provides a comprehensive reference for all public APIs in CoreBaseApplication (CBA). The framework is organized into several core modules, each providing specific functionality.

## Table of Contents

- [Core API](#core-api)
- [Logging System](#logging-system)
- [Configuration Management](#configuration-management)
- [System Monitoring](#system-monitoring)
- [Network Management](#network-management)
- [Error Handling](#error-handling)
- [Utilities](#utilities)

## Core API

### CoreAPI Class

The main entry point for the CoreBaseApplication framework.

```cpp
class CoreAPI {
public:
    // Static methods
    static bool initialize(const std::string& configPath = "config.json");
    static void shutdown();
    static bool isInitialized();
    static std::string getVersion();
    static CoreAPI& getInstance();
    
    // Instance methods
    bool start();
    void stop();
    bool isRunning() const;
    void processEvents();
    void setEventCallback(std::function<void(const Event&)> callback);
};
```

#### Static Methods

##### `initialize(configPath)`

Initializes the CoreBaseApplication framework.

**Parameters:**
- `configPath` (string, optional): Path to configuration file. Defaults to "config.json".

**Returns:**
- `bool`: `true` if initialization successful, `false` otherwise.

**Example:**
```cpp
if (!CoreAPI::initialize("my_config.json")) {
    std::cerr << "Failed to initialize CoreAPI" << std::endl;
    return -1;
}
```

##### `shutdown()`

Cleanly shuts down the framework and releases all resources.

**Example:**
```cpp
CoreAPI::shutdown();
```

##### `isInitialized()`

Checks if the framework has been initialized.

**Returns:**
- `bool`: `true` if initialized, `false` otherwise.

##### `getVersion()`

Returns the framework version string.

**Returns:**
- `string`: Version in format "major.minor.patch".

##### `getInstance()`

Returns the singleton instance of CoreAPI.

**Returns:**
- `CoreAPI&`: Reference to the singleton instance.

**Throws:**
- `std::runtime_error`: If framework not initialized.

#### Instance Methods

##### `start()`

Starts the main application loop.

**Returns:**
- `bool`: `true` if started successfully, `false` otherwise.

##### `stop()`

Stops the main application loop.

##### `isRunning()`

Checks if the application is currently running.

**Returns:**
- `bool`: `true` if running, `false` otherwise.

##### `processEvents()`

Processes pending events in the event queue.

##### `setEventCallback(callback)`

Sets a callback function to handle events.

**Parameters:**
- `callback` (function): Function to call when events occur.

## Logging System

### Logger Class

```cpp
class Logger {
public:
    enum class Level {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARNING = 3,
        ERROR = 4,
        CRITICAL = 5
    };
    
    static Logger& getInstance();
    static void initialize(const std::string& configPath);
    
    void setLevel(Level level);
    Level getLevel() const;
    
    void log(Level level, const std::string& message, const std::string& category = "");
    void trace(const std::string& message, const std::string& category = "");
    void debug(const std::string& message, const std::string& category = "");
    void info(const std::string& message, const std::string& category = "");
    void warning(const std::string& message, const std::string& category = "");
    void error(const std::string& message, const std::string& category = "");
    void critical(const std::string& message, const std::string& category = "");
    
    void addAppender(std::unique_ptr<LogAppender> appender);
    void removeAppender(const std::string& name);
    void flush();
};
```

### LogAppender Classes

#### ConsoleAppender

```cpp
class ConsoleAppender : public LogAppender {
public:
    explicit ConsoleAppender(const std::string& name = "console");
    void setColorEnabled(bool enabled);
    bool isColorEnabled() const;
};
```

#### FileAppender

```cpp
class FileAppender : public LogAppender {
public:
    explicit FileAppender(const std::string& filename, const std::string& name = "file");
    void setFilename(const std::string& filename);
    std::string getFilename() const;
    void setAppendMode(bool append);
    bool isAppendMode() const;
};
```

#### RotatingFileAppender

```cpp
class RotatingFileAppender : public LogAppender {
public:
    RotatingFileAppender(const std::string& filename, 
                        size_t maxFileSize, 
                        int maxFiles,
                        const std::string& name = "rotating_file");
    
    void setMaxFileSize(size_t size);
    size_t getMaxFileSize() const;
    void setMaxFiles(int count);
    int getMaxFiles() const;
};
```

### Logging Macros

```cpp
#define LOG_TRACE(message) Logger::getInstance().trace(message)
#define LOG_DEBUG(message) Logger::getInstance().debug(message)
#define LOG_INFO(message) Logger::getInstance().info(message)
#define LOG_WARNING(message) Logger::getInstance().warning(message)
#define LOG_ERROR(message) Logger::getInstance().error(message)
#define LOG_CRITICAL(message) Logger::getInstance().critical(message)

#define LOG_TRACE_CAT(message, category) Logger::getInstance().trace(message, category)
#define LOG_DEBUG_CAT(message, category) Logger::getInstance().debug(message, category)
#define LOG_INFO_CAT(message, category) Logger::getInstance().info(message, category)
#define LOG_WARNING_CAT(message, category) Logger::getInstance().warning(message, category)
#define LOG_ERROR_CAT(message, category) Logger::getInstance().error(message, category)
#define LOG_CRITICAL_CAT(message, category) Logger::getInstance().critical(message, category)
```

## Configuration Management

### ConfigManager Class

```cpp
class ConfigManager {
public:
    static ConfigManager& getInstance();
    
    bool loadConfig(const std::string& filename);
    bool saveConfig(const std::string& filename = "") const;
    
    // String values
    std::string getConfigString(const std::string& key, const std::string& defaultValue = "") const;
    void setConfigString(const std::string& key, const std::string& value);
    
    // Integer values
    int getConfigInt(const std::string& key, int defaultValue = 0) const;
    void setConfigInt(const std::string& key, int value);
    
    // Double values
    double getConfigDouble(const std::string& key, double defaultValue = 0.0) const;
    void setConfigDouble(const std::string& key, double value);
    
    // Boolean values
    bool getConfigBool(const std::string& key, bool defaultValue = false) const;
    void setConfigBool(const std::string& key, bool value);
    
    // Array values
    std::vector<std::string> getConfigArray(const std::string& key) const;
    void setConfigArray(const std::string& key, const std::vector<std::string>& value);
    
    // Object values
    nlohmann::json getConfigObject(const std::string& key) const;
    void setConfigObject(const std::string& key, const nlohmann::json& value);
    
    // Utility methods
    bool hasKey(const std::string& key) const;
    void removeKey(const std::string& key);
    std::vector<std::string> getAllKeys() const;
    void clear();
    
    // Validation
    bool validateConfig(const nlohmann::json& schema) const;
    
    // Change notifications
    void setChangeCallback(std::function<void(const std::string&)> callback);
    void removeChangeCallback();
};
```

### Configuration File Formats

#### JSON Configuration

```json
{
  "application": {
    "name": "MyApplication",
    "version": "1.0.0",
    "debug": true
  },
  "logging": {
    "level": "INFO",
    "file": "app.log",
    "console": true
  },
  "network": {
    "host": "localhost",
    "port": 8080,
    "timeout": 30000
  }
}
```

#### YAML Configuration

```yaml
application:
  name: MyApplication
  version: 1.0.0
  debug: true

logging:
  level: INFO
  file: app.log
  console: true

network:
  host: localhost
  port: 8080
  timeout: 30000
```

## System Monitoring

### SystemMonitor Class

```cpp
class SystemMonitor {
public:
    struct SystemMetrics {
        double cpuUsage;           // CPU usage percentage (0-100)
        size_t memoryUsed;         // Memory used in bytes
        size_t memoryTotal;        // Total memory in bytes
        double memoryUsagePercent; // Memory usage percentage (0-100)
        size_t diskUsed;           // Disk used in bytes
        size_t diskTotal;          // Total disk space in bytes
        double diskUsagePercent;   // Disk usage percentage (0-100)
        double networkBytesIn;     // Network bytes received
        double networkBytesOut;    // Network bytes sent
        std::chrono::system_clock::time_point timestamp;
    };
    
    static SystemMonitor& getInstance();
    
    SystemMetrics getCurrentMetrics();
    double getCpuUsage();
    size_t getMemoryUsage();
    size_t getTotalMemory();
    double getMemoryUsagePercent();
    size_t getDiskUsage(const std::string& path = "/");
    size_t getTotalDiskSpace(const std::string& path = "/");
    double getDiskUsagePercent(const std::string& path = "/");
    
    void startMonitoring(std::chrono::milliseconds interval = std::chrono::milliseconds(1000));
    void stopMonitoring();
    bool isMonitoring() const;
    
    void setMetricsCallback(std::function<void(const SystemMetrics&)> callback);
    void removeMetricsCallback();
    
    std::vector<SystemMetrics> getHistoricalMetrics(size_t count = 100) const;
    void clearHistory();
};
```

### PerformanceCounter Class

```cpp
class PerformanceCounter {
public:
    explicit PerformanceCounter(const std::string& name);
    
    void start();
    void stop();
    void reset();
    
    std::chrono::nanoseconds getElapsed() const;
    double getElapsedSeconds() const;
    double getElapsedMilliseconds() const;
    
    std::string getName() const;
    bool isRunning() const;
};
```

### HealthCheck System

```cpp
class HealthCheck {
public:
    enum class Status {
        HEALTHY,
        WARNING,
        CRITICAL,
        UNKNOWN
    };
    
    struct Result {
        Status status;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        std::chrono::milliseconds duration;
    };
    
    virtual ~HealthCheck() = default;
    virtual Result check() = 0;
    virtual std::string getName() const = 0;
};

class HealthCheckManager {
public:
    static HealthCheckManager& getInstance();
    
    void registerHealthCheck(std::unique_ptr<HealthCheck> check);
    void unregisterHealthCheck(const std::string& name);
    
    HealthCheck::Result runHealthCheck(const std::string& name);
    std::map<std::string, HealthCheck::Result> runAllHealthChecks();
    
    void startPeriodicChecks(std::chrono::seconds interval = std::chrono::seconds(60));
    void stopPeriodicChecks();
    
    void setHealthCheckCallback(std::function<void(const std::string&, const HealthCheck::Result&)> callback);
};
```

## Network Management

### HttpClient Class

```cpp
class HttpClient {
public:
    enum class Method {
        GET,
        POST,
        PUT,
        DELETE,
        PATCH,
        HEAD,
        OPTIONS
    };
    
    struct Request {
        Method method = Method::GET;
        std::string url;
        std::map<std::string, std::string> headers;
        std::string body;
        std::chrono::milliseconds timeout = std::chrono::milliseconds(30000);
    };
    
    struct Response {
        int statusCode = 0;
        std::string statusMessage;
        std::map<std::string, std::string> headers;
        std::string body;
        std::chrono::milliseconds duration;
        bool success = false;
    };
    
    explicit HttpClient(const std::string& baseUrl = "");
    
    Response execute(const Request& request);
    Response get(const std::string& path, const std::map<std::string, std::string>& headers = {});
    Response post(const std::string& path, const std::string& body, const std::map<std::string, std::string>& headers = {});
    Response put(const std::string& path, const std::string& body, const std::map<std::string, std::string>& headers = {});
    Response del(const std::string& path, const std::map<std::string, std::string>& headers = {});
    
    void setBaseUrl(const std::string& baseUrl);
    std::string getBaseUrl() const;
    
    void setDefaultTimeout(std::chrono::milliseconds timeout);
    std::chrono::milliseconds getDefaultTimeout() const;
    
    void setDefaultHeaders(const std::map<std::string, std::string>& headers);
    std::map<std::string, std::string> getDefaultHeaders() const;
    
    void setUserAgent(const std::string& userAgent);
    std::string getUserAgent() const;
};
```

### WebSocketClient Class

```cpp
class WebSocketClient {
public:
    enum class State {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING
    };
    
    enum class MessageType {
        TEXT,
        BINARY,
        PING,
        PONG,
        CLOSE
    };
    
    struct Message {
        MessageType type;
        std::vector<uint8_t> data;
        std::chrono::system_clock::time_point timestamp;
    };
    
    explicit WebSocketClient(const std::string& url);
    
    bool connect(std::chrono::milliseconds timeout = std::chrono::milliseconds(10000));
    void disconnect();
    State getState() const;
    
    bool sendText(const std::string& message);
    bool sendBinary(const std::vector<uint8_t>& data);
    bool ping(const std::string& payload = "");
    
    void setOnMessage(std::function<void(const Message&)> callback);
    void setOnConnect(std::function<void()> callback);
    void setOnDisconnect(std::function<void(int code, const std::string& reason)> callback);
    void setOnError(std::function<void(const std::string& error)> callback);
    
    void setHeaders(const std::map<std::string, std::string>& headers);
    void setSubprotocols(const std::vector<std::string>& protocols);
};
```

### RestClient Class

```cpp
template<typename T>
class RestClient {
public:
    explicit RestClient(const std::string& baseUrl);
    
    // CRUD operations
    std::optional<T> get(const std::string& id);
    std::vector<T> getAll();
    std::optional<T> create(const T& item);
    std::optional<T> update(const std::string& id, const T& item);
    bool remove(const std::string& id);
    
    // Query operations
    std::vector<T> query(const std::map<std::string, std::string>& params);
    std::vector<T> search(const std::string& query);
    
    // Pagination
    struct PageResult {
        std::vector<T> items;
        size_t totalCount;
        size_t pageNumber;
        size_t pageSize;
        bool hasNext;
        bool hasPrevious;
    };
    
    PageResult getPage(size_t pageNumber, size_t pageSize = 20);
    
    // Configuration
    void setAuthToken(const std::string& token);
    void setApiKey(const std::string& key);
    void setTimeout(std::chrono::milliseconds timeout);
};
```

## Error Handling

### Exception Classes

```cpp
// Base exception class
class CoreException : public std::exception {
public:
    explicit CoreException(const std::string& message, const std::string& category = "");
    const char* what() const noexcept override;
    const std::string& getCategory() const;
    const std::string& getMessage() const;
    
private:
    std::string m_message;
    std::string m_category;
};

// Configuration exceptions
class ConfigException : public CoreException {
public:
    explicit ConfigException(const std::string& message);
};

// Network exceptions
class NetworkException : public CoreException {
public:
    explicit NetworkException(const std::string& message, int errorCode = 0);
    int getErrorCode() const;
    
private:
    int m_errorCode;
};

// File I/O exceptions
class FileException : public CoreException {
public:
    explicit FileException(const std::string& message, const std::string& filename = "");
    const std::string& getFilename() const;
    
private:
    std::string m_filename;
};

// Validation exceptions
class ValidationException : public CoreException {
public:
    explicit ValidationException(const std::string& message, const std::string& field = "");
    const std::string& getField() const;
    
private:
    std::string m_field;
};
```

### Error Codes

```cpp
enum class ErrorCode {
    SUCCESS = 0,
    
    // General errors (1000-1999)
    UNKNOWN_ERROR = 1000,
    INVALID_ARGUMENT = 1001,
    NULL_POINTER = 1002,
    OUT_OF_MEMORY = 1003,
    TIMEOUT = 1004,
    
    // Configuration errors (2000-2999)
    CONFIG_FILE_NOT_FOUND = 2000,
    CONFIG_PARSE_ERROR = 2001,
    CONFIG_VALIDATION_ERROR = 2002,
    CONFIG_KEY_NOT_FOUND = 2003,
    
    // Network errors (3000-3999)
    NETWORK_CONNECTION_FAILED = 3000,
    NETWORK_TIMEOUT = 3001,
    NETWORK_DNS_ERROR = 3002,
    NETWORK_SSL_ERROR = 3003,
    HTTP_BAD_REQUEST = 3400,
    HTTP_UNAUTHORIZED = 3401,
    HTTP_FORBIDDEN = 3403,
    HTTP_NOT_FOUND = 3404,
    HTTP_INTERNAL_ERROR = 3500,
    
    // File I/O errors (4000-4999)
    FILE_NOT_FOUND = 4000,
    FILE_ACCESS_DENIED = 4001,
    FILE_READ_ERROR = 4002,
    FILE_WRITE_ERROR = 4003,
    DIRECTORY_NOT_FOUND = 4004,
    
    // System errors (5000-5999)
    SYSTEM_RESOURCE_UNAVAILABLE = 5000,
    SYSTEM_PERMISSION_DENIED = 5001,
    SYSTEM_OPERATION_NOT_SUPPORTED = 5002
};

std::string errorCodeToString(ErrorCode code);
ErrorCode stringToErrorCode(const std::string& str);
```

## Utilities

### StringUtils

```cpp
class StringUtils {
public:
    static std::string trim(const std::string& str);
    static std::string trimLeft(const std::string& str);
    static std::string trimRight(const std::string& str);
    
    static std::string toLower(const std::string& str);
    static std::string toUpper(const std::string& str);
    
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    
    static std::string join(const std::vector<std::string>& parts, const std::string& separator);
    
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool endsWith(const std::string& str, const std::string& suffix);
    static bool contains(const std::string& str, const std::string& substring);
    
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);
    static std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);
    
    static bool isNumeric(const std::string& str);
    static bool isAlpha(const std::string& str);
    static bool isAlphaNumeric(const std::string& str);
    
    static std::string format(const std::string& format, ...);
};
```

### FileUtils

```cpp
class FileUtils {
public:
    static bool exists(const std::string& path);
    static bool isFile(const std::string& path);
    static bool isDirectory(const std::string& path);
    
    static size_t getFileSize(const std::string& path);
    static std::chrono::system_clock::time_point getLastModified(const std::string& path);
    
    static std::string readTextFile(const std::string& path);
    static std::vector<uint8_t> readBinaryFile(const std::string& path);
    
    static bool writeTextFile(const std::string& path, const std::string& content);
    static bool writeBinaryFile(const std::string& path, const std::vector<uint8_t>& data);
    
    static bool createDirectory(const std::string& path);
    static bool createDirectories(const std::string& path);
    static bool removeFile(const std::string& path);
    static bool removeDirectory(const std::string& path, bool recursive = false);
    
    static std::vector<std::string> listDirectory(const std::string& path);
    static std::vector<std::string> listFiles(const std::string& path, const std::string& extension = "");
    
    static std::string getExtension(const std::string& path);
    static std::string getFilename(const std::string& path);
    static std::string getDirectory(const std::string& path);
    static std::string getAbsolutePath(const std::string& path);
    
    static std::string joinPath(const std::string& path1, const std::string& path2);
    static std::string normalizePath(const std::string& path);
};
```

### TimeUtils

```cpp
class TimeUtils {
public:
    static std::string formatTimestamp(const std::chrono::system_clock::time_point& time, 
                                     const std::string& format = "%Y-%m-%d %H:%M:%S");
    
    static std::chrono::system_clock::time_point parseTimestamp(const std::string& str, 
                                                               const std::string& format = "%Y-%m-%d %H:%M:%S");
    
    static std::string formatDuration(const std::chrono::nanoseconds& duration);
    static std::chrono::nanoseconds parseDuration(const std::string& str);
    
    static std::chrono::system_clock::time_point now();
    static std::time_t toTimeT(const std::chrono::system_clock::time_point& time);
    static std::chrono::system_clock::time_point fromTimeT(std::time_t time);
    
    static std::string getTimezone();
    static void setTimezone(const std::string& timezone);
};
```

### JsonUtils

```cpp
class JsonUtils {
public:
    static nlohmann::json parseString(const std::string& jsonStr);
    static nlohmann::json parseFile(const std::string& filename);
    
    static std::string toString(const nlohmann::json& json, int indent = -1);
    static bool toFile(const nlohmann::json& json, const std::string& filename, int indent = 2);
    
    static bool validate(const nlohmann::json& json, const nlohmann::json& schema);
    
    static nlohmann::json merge(const nlohmann::json& base, const nlohmann::json& overlay);
    static nlohmann::json flatten(const nlohmann::json& json, const std::string& separator = ".");
    static nlohmann::json unflatten(const nlohmann::json& json, const std::string& separator = ".");
    
    static std::vector<std::string> getKeys(const nlohmann::json& json);
    static bool hasKey(const nlohmann::json& json, const std::string& key);
    static nlohmann::json getValue(const nlohmann::json& json, const std::string& key, const nlohmann::json& defaultValue = nlohmann::json{});
};
```

---

## Version Information

**Current Version:** 1.0.0  
**API Version:** 1.0  
**Last Updated:** 2024

## Support

For questions about the API or to report issues:

- **Documentation:** [docs/](../index.md)
- **Examples:** [examples/](../examples/)
- **Issues:** GitHub Issues
- **Discussions:** GitHub Discussions

---

*This API reference is automatically generated from the source code. For the most up-to-date information, please refer to the header files in the source code.*