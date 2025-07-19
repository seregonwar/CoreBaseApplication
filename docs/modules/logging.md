# Advanced Logging System

The Advanced Logging module provides a robust, high-performance logging system with support for multiple appenders, log rotation, compression, and flexible formatting.

## Features

- **Multiple Appenders**: Console, file, rotating file, and custom appenders
- **Log Rotation**: Automatic file rotation based on size or time
- **Compression**: Automatic compression of rotated log files
- **Thread Safety**: Concurrent logging from multiple threads
- **Configurable Levels**: Debug, Info, Warning, Error, Fatal
- **Custom Formatting**: Flexible log message formatting
- **Performance Optimized**: Minimal overhead with buffering

## Quick Start

```cpp
#include "core/AdvancedLogging/RotatingLogger.h"

using namespace Core::AdvancedLogging;

// Get logger instance
auto& logger = Logger::getInstance();

// Add console appender
logger.addAppender(LogAppenderFactory::createConsoleAppender());

// Add rotating file appender
logger.addAppender(LogAppenderFactory::createRotatingFileAppender(
    "logs/application.log",  // File path
    10 * 1024 * 1024,        // Max size (10 MB)
    5,                       // Number of backup files
    true                     // Enable compression
));

// Set log level
logger.setLevel(LogLevel::INFO);

// Log messages
logger.info("Application started");
logger.warning("Low disk space", "System");
logger.error("Database connection failed", "Database");
```

## Log Levels

```cpp
enum class LogLevel {
    DEBUG = 0,    // Detailed information for debugging
    INFO = 1,     // General information messages
    WARNING = 2,  // Warning messages
    ERROR = 3,    // Error messages
    FATAL = 4     // Fatal error messages
};
```

## Appender Types

### Console Appender

Logs messages to the console with color coding:

```cpp
// Create console appender
auto consoleAppender = LogAppenderFactory::createConsoleAppender();
logger.addAppender(consoleAppender);

// With custom format
auto consoleAppender = LogAppenderFactory::createConsoleAppender(
    "[%timestamp%] [%level%] %message%"
);
```

### File Appender

Logs messages to a single file:

```cpp
// Create file appender
auto fileAppender = LogAppenderFactory::createFileAppender(
    "logs/application.log"
);
logger.addAppender(fileAppender);

// With custom format and immediate flush
auto fileAppender = LogAppenderFactory::createFileAppender(
    "logs/application.log",
    "[%timestamp%] [%level%] [%category%] %message%",
    true  // Immediate flush
);
```

### Rotating File Appender

Automatically rotates log files based on size:

```cpp
// Create rotating file appender
auto rotatingAppender = LogAppenderFactory::createRotatingFileAppender(
    "logs/application.log",  // Base filename
    10 * 1024 * 1024,        // Max file size (10 MB)
    5,                       // Number of backup files
    true                     // Enable compression
);
logger.addAppender(rotatingAppender);
```

This creates files like:
- `application.log` (current)
- `application.log.1.gz` (most recent backup)
- `application.log.2.gz`
- `application.log.3.gz`
- `application.log.4.gz`
- `application.log.5.gz` (oldest backup)

### Time-Based Rotating Appender

Rotates log files based on time intervals:

```cpp
// Create time-based rotating appender
auto timeRotatingAppender = LogAppenderFactory::createTimeRotatingAppender(
    "logs/application",      // Base filename
    TimeRotationInterval::DAILY,  // Rotation interval
    30,                      // Keep 30 days of logs
    true                     // Enable compression
);
```

## Custom Formatting

Supported format placeholders:

- `%timestamp%` - Full timestamp
- `%date%` - Date only
- `%time%` - Time only
- `%level%` - Log level
- `%category%` - Log category
- `%message%` - Log message
- `%thread%` - Thread ID
- `%file%` - Source file name
- `%line%` - Source line number
- `%function%` - Function name

```cpp
// Custom format example
std::string customFormat = "[%timestamp%] [%thread%] [%level%] [%category%] %message% (%file%:%line%)"; 
auto appender = LogAppenderFactory::createFileAppender(
    "logs/detailed.log",
    customFormat
);
```

## Advanced Usage

### Conditional Logging

```cpp
// Log only if condition is met
if (logger.isLevelEnabled(LogLevel::DEBUG)) {
    std::string expensiveDebugInfo = generateDebugInfo();
    logger.debug(expensiveDebugInfo);
}

// Using macros for automatic condition checking
LOG_DEBUG("Debug info: " << variable << ", state: " << state);
LOG_INFO("Processing item: " << itemId);
LOG_ERROR("Failed to process: " << error.what());
```

### Structured Logging

```cpp
// Log with additional context
LogContext context;
context.add("userId", "12345");
context.add("sessionId", "abc-def-ghi");
context.add("requestId", "req-789");

logger.info("User login successful", "Authentication", context);

// JSON output: {"timestamp":"...", "level":"INFO", "category":"Authentication", 
//               "message":"User login successful", "userId":"12345", 
//               "sessionId":"abc-def-ghi", "requestId":"req-789"}
```

### Performance Monitoring

```cpp
// Log execution time
{
    LogTimer timer("DatabaseQuery", LogLevel::DEBUG);
    // Database operation here
    // Automatically logs execution time when timer goes out of scope
}

// Manual timing
auto start = std::chrono::high_resolution_clock::now();
// Operation here
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
logger.info("Operation completed in " + std::to_string(duration.count()) + "ms");
```

### Custom Appenders

Create custom appenders for specific needs:

```cpp
class DatabaseAppender : public ILogAppender {
public:
    DatabaseAppender(const std::string& connectionString) 
        : m_connectionString(connectionString) {}
    
    bool initialize() override {
        // Initialize database connection
        return connectToDatabase();
    }
    
    void append(const LogEntry& entry) override {
        // Insert log entry into database
        insertLogEntry(entry);
    }
    
    void flush() override {
        // Flush any pending entries
    }
    
    void shutdown() override {
        // Close database connection
        disconnectFromDatabase();
    }
    
private:
    std::string m_connectionString;
    // Database connection members
};

// Register custom appender
logger.addAppender(std::make_shared<DatabaseAppender>("connection_string"));
```

## Configuration

Configure logging through JSON:

```json
{
  "logging": {
    "level": "INFO",
    "appenders": [
      {
        "type": "console",
        "format": "[%timestamp%] [%level%] %message%",
        "colorEnabled": true
      },
      {
        "type": "rotatingFile",
        "path": "logs/application.log",
        "maxSize": 10485760,
        "maxFiles": 5,
        "compress": true,
        "format": "[%timestamp%] [%level%] [%category%] %message%"
      },
      {
        "type": "timeRotating",
        "path": "logs/daily",
        "interval": "daily",
        "maxFiles": 30,
        "compress": true
      }
    ],
    "categories": {
      "Database": "DEBUG",
      "Network": "WARNING",
      "Security": "INFO"
    }
  }
}
```

## Macros

Convenient macros for common logging operations:

```cpp
// Basic logging macros
LOG_DEBUG("Debug message");
LOG_INFO("Info message");
LOG_WARNING("Warning message");
LOG_ERROR("Error message");
LOG_FATAL("Fatal message");

// Logging with category
LOG_DEBUG_CAT("Database", "Query executed successfully");
LOG_ERROR_CAT("Network", "Connection timeout");

// Conditional logging
LOG_IF(condition, LogLevel::INFO, "Condition met");

// Function entry/exit logging
void myFunction() {
    LOG_FUNCTION_ENTRY();
    // Function logic
    LOG_FUNCTION_EXIT();
}
```

## Performance Considerations

- **Buffering**: Appenders use internal buffering to minimize I/O operations
- **Async Logging**: Optional asynchronous logging for high-throughput scenarios
- **Level Checking**: Log level checks are optimized for minimal overhead
- **String Formatting**: Lazy string formatting to avoid unnecessary work

```cpp
// Enable async logging for high performance
logger.setAsyncMode(true, 1000);  // Buffer up to 1000 entries

// Disable logging in release builds
#ifdef NDEBUG
logger.setLevel(LogLevel::WARNING);
#endif
```

## Thread Safety

- All logging operations are thread-safe
- Multiple threads can log concurrently without synchronization
- Appenders handle concurrent access internally
- No external locking required

## Best Practices

1. **Use appropriate log levels** - Don't log everything at INFO level
2. **Include context** - Add relevant information to help with debugging
3. **Avoid sensitive data** - Never log passwords, tokens, or personal information
4. **Use structured logging** - Include key-value pairs for better searchability
5. **Monitor log file sizes** - Configure rotation to prevent disk space issues
6. **Test logging configuration** - Verify logs are written correctly in all environments

## Troubleshooting

### Common Issues

**Logs not appearing:**
- Check log level configuration
- Verify appender initialization
- Check file permissions for file appenders

**Performance issues:**
- Enable async logging for high-throughput scenarios
- Reduce log level in production
- Use appropriate buffer sizes

**File rotation not working:**
- Check file permissions
- Verify disk space availability
- Check rotation configuration parameters

## See Also

- [Configuration Management](configuration.md)
- [Monitoring System](monitoring.md)
- [Best Practices](../development/best-practices.md)
- [Performance Tuning](../development/performance.md)