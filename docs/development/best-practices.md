# Development Best Practices

This guide outlines the coding standards, design patterns, and best practices for developing with CoreBaseApplication (CBA). Following these guidelines ensures code quality, maintainability, and consistency across the project.

## Code Style and Standards

### C++ Coding Standards

#### Naming Conventions

```cpp
// Classes: PascalCase
class NetworkManager {
public:
    // Public methods: camelCase
    bool initializeConnection();
    void setConnectionTimeout(int timeout);
    
    // Public members: camelCase (avoid public members when possible)
    int maxRetries;
    
private:
    // Private methods: camelCase
    void handleConnectionError();
    
    // Private members: m_ prefix + camelCase
    std::string m_serverUrl;
    int m_connectionTimeout;
    bool m_isConnected;
};

// Namespaces: PascalCase
namespace Core::NetworkManager {
    // Functions: camelCase
    bool validateUrl(const std::string& url);
    
    // Constants: UPPER_SNAKE_CASE
    const int DEFAULT_TIMEOUT = 30;
    const std::string DEFAULT_USER_AGENT = "CoreBaseApplication/1.0";
    
    // Enums: PascalCase with UPPER_SNAKE_CASE values
    enum class ConnectionState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        ERROR
    };
}

// Variables: camelCase
int connectionCount = 0;
std::string serverAddress = "localhost";
bool isInitialized = false;

// File names: PascalCase for classes, snake_case for utilities
// NetworkManager.h, NetworkManager.cpp
// string_utils.h, string_utils.cpp
```

#### Header File Structure

```cpp
// NetworkManager.h
#pragma once

// System includes first
#include <string>
#include <memory>
#include <vector>

// Third-party includes
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

// Project includes
#include "core/CoreClass/ConfigManager.h"
#include "core/Common/Logger.h"

namespace Core::NetworkManager {

/**
 * @brief Manages network connections and HTTP requests
 * 
 * The NetworkManager class provides a high-level interface for making
 * HTTP requests, managing connection pools, and handling network errors.
 * 
 * @example
 * @code
 * NetworkManager manager;
 * manager.initialize();
 * auto response = manager.get("https://api.example.com/data");
 * @endcode
 */
class NetworkManager {
public:
    /**
     * @brief Default constructor
     */
    NetworkManager();
    
    /**
     * @brief Destructor
     */
    ~NetworkManager();
    
    // Delete copy constructor and assignment operator
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;
    
    // Allow move constructor and assignment operator
    NetworkManager(NetworkManager&&) = default;
    NetworkManager& operator=(NetworkManager&&) = default;
    
    /**
     * @brief Initialize the network manager
     * @return true if initialization successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Shutdown the network manager
     */
    void shutdown();
    
private:
    class Impl; // PIMPL idiom
    std::unique_ptr<Impl> m_impl;
};

} // namespace Core::NetworkManager
```

#### Implementation File Structure

```cpp
// NetworkManager.cpp
#include "NetworkManager.h"

// System includes
#include <iostream>
#include <stdexcept>

// Project includes
#include "core/Common/Logger.h"

namespace Core::NetworkManager {

// PIMPL implementation
class NetworkManager::Impl {
public:
    Impl() : m_isInitialized(false) {}
    
    bool initialize() {
        try {
            // Initialization logic
            m_isInitialized = true;
            LOG_INFO("NetworkManager initialized successfully");
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize NetworkManager: " + std::string(e.what()));
            return false;
        }
    }
    
private:
    bool m_isInitialized;
};

// Public interface implementation
NetworkManager::NetworkManager() : m_impl(std::make_unique<Impl>()) {}

NetworkManager::~NetworkManager() = default;

bool NetworkManager::initialize() {
    return m_impl->initialize();
}

void NetworkManager::shutdown() {
    LOG_INFO("NetworkManager shutting down");
    // Cleanup logic
}

} // namespace Core::NetworkManager
```

### Error Handling

#### Exception Safety

```cpp
// Use RAII for resource management
class ResourceManager {
public:
    ResourceManager() {
        m_resource = acquireResource();
        if (!m_resource) {
            throw std::runtime_error("Failed to acquire resource");
        }
    }
    
    ~ResourceManager() {
        if (m_resource) {
            releaseResource(m_resource);
        }
    }
    
    // Strong exception safety guarantee
    void updateResource(const ResourceData& data) {
        auto newResource = createResource(data); // May throw
        
        // Only modify state after all operations that can throw
        releaseResource(m_resource);
        m_resource = newResource;
    }
    
private:
    Resource* m_resource;
};

// Use smart pointers for automatic cleanup
std::unique_ptr<NetworkConnection> createConnection(const std::string& url) {
    auto connection = std::make_unique<NetworkConnection>();
    
    if (!connection->connect(url)) {
        throw ConnectionException("Failed to connect to " + url);
    }
    
    return connection;
}

// Prefer error codes for expected failures
enum class ValidationResult {
    SUCCESS,
    INVALID_URL,
    INVALID_PORT,
    INVALID_PROTOCOL
};

ValidationResult validateUrl(const std::string& url) {
    if (url.empty()) {
        return ValidationResult::INVALID_URL;
    }
    
    // Validation logic...
    return ValidationResult::SUCCESS;
}
```

#### Error Propagation

```cpp
// Use std::optional for operations that may fail
std::optional<UserData> getUserData(int userId) {
    if (userId <= 0) {
        return std::nullopt;
    }
    
    // Database query logic...
    if (userFound) {
        return UserData{/* ... */};
    }
    
    return std::nullopt;
}

// Use std::expected (C++23) or custom Result type
template<typename T, typename E>
class Result {
public:
    static Result success(T value) {
        return Result(std::move(value));
    }
    
    static Result error(E error) {
        return Result(std::move(error));
    }
    
    bool isSuccess() const { return m_hasValue; }
    bool isError() const { return !m_hasValue; }
    
    const T& value() const {
        if (!m_hasValue) {
            throw std::runtime_error("Accessing value of error result");
        }
        return m_value;
    }
    
    const E& error() const {
        if (m_hasValue) {
            throw std::runtime_error("Accessing error of success result");
        }
        return m_error;
    }
    
private:
    explicit Result(T value) : m_value(std::move(value)), m_hasValue(true) {}
    explicit Result(E error) : m_error(std::move(error)), m_hasValue(false) {}
    
    union {
        T m_value;
        E m_error;
    };
    bool m_hasValue;
};

// Usage
Result<UserData, DatabaseError> result = fetchUserFromDatabase(userId);
if (result.isSuccess()) {
    processUser(result.value());
} else {
    handleDatabaseError(result.error());
}
```

### Memory Management

#### Smart Pointers

```cpp
// Use std::unique_ptr for exclusive ownership
class DatabaseConnection {
public:
    static std::unique_ptr<DatabaseConnection> create(const std::string& connectionString) {
        auto connection = std::make_unique<DatabaseConnection>();
        if (!connection->connect(connectionString)) {
            return nullptr;
        }
        return connection;
    }
    
private:
    DatabaseConnection() = default;
};

// Use std::shared_ptr for shared ownership
class ResourceCache {
public:
    std::shared_ptr<Resource> getResource(const std::string& key) {
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            if (auto resource = it->second.lock()) {
                return resource;
            } else {
                m_cache.erase(it); // Remove expired weak_ptr
            }
        }
        
        auto resource = std::make_shared<Resource>(key);
        m_cache[key] = resource;
        return resource;
    }
    
private:
    std::unordered_map<std::string, std::weak_ptr<Resource>> m_cache;
};

// Avoid raw pointers except for non-owning references
class EventHandler {
public:
    void setLogger(Logger* logger) { // Non-owning pointer
        m_logger = logger;
    }
    
    void logEvent(const std::string& message) {
        if (m_logger) {
            m_logger->info(message);
        }
    }
    
private:
    Logger* m_logger = nullptr; // Non-owning
};
```

## Design Patterns

### RAII (Resource Acquisition Is Initialization)

```cpp
// File handle wrapper
class FileHandle {
public:
    explicit FileHandle(const std::string& filename) {
        m_file = fopen(filename.c_str(), "r");
        if (!m_file) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
    }
    
    ~FileHandle() {
        if (m_file) {
            fclose(m_file);
        }
    }
    
    // Delete copy operations
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    
    // Allow move operations
    FileHandle(FileHandle&& other) noexcept : m_file(other.m_file) {
        other.m_file = nullptr;
    }
    
    FileHandle& operator=(FileHandle&& other) noexcept {
        if (this != &other) {
            if (m_file) {
                fclose(m_file);
            }
            m_file = other.m_file;
            other.m_file = nullptr;
        }
        return *this;
    }
    
    FILE* get() const { return m_file; }
    
private:
    FILE* m_file;
};

// Lock guard for custom mutex
template<typename Mutex>
class LockGuard {
public:
    explicit LockGuard(Mutex& mutex) : m_mutex(mutex) {
        m_mutex.lock();
    }
    
    ~LockGuard() {
        m_mutex.unlock();
    }
    
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
    
private:
    Mutex& m_mutex;
};
```

### Singleton Pattern (Thread-Safe)

```cpp
// Meyer's Singleton (C++11 thread-safe)
class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(m_mutex);
        // Logging implementation
    }
    
    // Delete copy and move operations
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    
private:
    Logger() = default;
    ~Logger() = default;
    
    mutable std::mutex m_mutex;
};

// Alternative: Dependency injection friendly singleton
class ConfigManager {
public:
    static ConfigManager& getDefault() {
        static ConfigManager instance;
        return instance;
    }
    
    // Allow creating custom instances for testing
    ConfigManager() = default;
    
private:
    // Implementation
};
```

### Factory Pattern

```cpp
// Abstract factory
class LoggerFactory {
public:
    enum class LoggerType {
        CONSOLE,
        FILE,
        SYSLOG
    };
    
    static std::unique_ptr<ILogger> createLogger(LoggerType type, const std::string& config) {
        switch (type) {
            case LoggerType::CONSOLE:
                return std::make_unique<ConsoleLogger>(config);
            case LoggerType::FILE:
                return std::make_unique<FileLogger>(config);
            case LoggerType::SYSLOG:
                return std::make_unique<SyslogLogger>(config);
            default:
                throw std::invalid_argument("Unknown logger type");
        }
    }
};

// Registration-based factory
class NetworkClientFactory {
public:
    using CreateFunction = std::function<std::unique_ptr<INetworkClient>(const std::string&)>;
    
    static void registerClient(const std::string& protocol, CreateFunction createFunc) {
        getRegistry()[protocol] = std::move(createFunc);
    }
    
    static std::unique_ptr<INetworkClient> createClient(const std::string& protocol, const std::string& config) {
        auto& registry = getRegistry();
        auto it = registry.find(protocol);
        if (it != registry.end()) {
            return it->second(config);
        }
        throw std::invalid_argument("Unknown protocol: " + protocol);
    }
    
private:
    static std::unordered_map<std::string, CreateFunction>& getRegistry() {
        static std::unordered_map<std::string, CreateFunction> registry;
        return registry;
    }
};

// Auto-registration helper
template<typename T>
class AutoRegister {
public:
    AutoRegister(const std::string& protocol) {
        NetworkClientFactory::registerClient(protocol, [](const std::string& config) {
            return std::make_unique<T>(config);
        });
    }
};

// Usage in implementation files
static AutoRegister<HttpClient> httpClientRegistration("http");
static AutoRegister<HttpsClient> httpsClientRegistration("https");
```

### Observer Pattern

```cpp
// Type-safe observer pattern
template<typename EventType>
class Observable {
public:
    using Observer = std::function<void(const EventType&)>;
    using ObserverId = size_t;
    
    ObserverId addObserver(Observer observer) {
        std::lock_guard<std::mutex> lock(m_mutex);
        ObserverId id = m_nextId++;
        m_observers[id] = std::move(observer);
        return id;
    }
    
    void removeObserver(ObserverId id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_observers.erase(id);
    }
    
    void notifyObservers(const EventType& event) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& [id, observer] : m_observers) {
            try {
                observer(event);
            } catch (const std::exception& e) {
                // Log error but continue notifying other observers
                LOG_ERROR("Observer notification failed: " + std::string(e.what()));
            }
        }
    }
    
private:
    mutable std::mutex m_mutex;
    std::unordered_map<ObserverId, Observer> m_observers;
    ObserverId m_nextId = 1;
};

// Usage
struct NetworkEvent {
    enum Type { CONNECTED, DISCONNECTED, ERROR };
    Type type;
    std::string message;
};

class NetworkManager : public Observable<NetworkEvent> {
public:
    void connect() {
        // Connection logic...
        notifyObservers({NetworkEvent::CONNECTED, "Connected to server"});
    }
};
```

## Thread Safety

### Synchronization Primitives

```cpp
// Use std::mutex for basic synchronization
class ThreadSafeCounter {
public:
    void increment() {
        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_count;
    }
    
    int getValue() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_count;
    }
    
private:
    mutable std::mutex m_mutex;
    int m_count = 0;
};

// Use std::shared_mutex for reader-writer scenarios
class ThreadSafeCache {
public:
    std::optional<std::string> get(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_cache.find(key);
        return (it != m_cache.end()) ? std::make_optional(it->second) : std::nullopt;
    }
    
    void put(const std::string& key, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_cache[key] = value;
    }
    
    void clear() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_cache.clear();
    }
    
private:
    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, std::string> m_cache;
};

// Use std::atomic for simple operations
class AtomicCounter {
public:
    void increment() {
        m_count.fetch_add(1, std::memory_order_relaxed);
    }
    
    int getValue() const {
        return m_count.load(std::memory_order_relaxed);
    }
    
private:
    std::atomic<int> m_count{0};
};
```

### Lock-Free Programming

```cpp
// Lock-free queue (simplified)
template<typename T>
class LockFreeQueue {
public:
    void push(T item) {
        auto newNode = new Node{std::move(item), nullptr};
        Node* prevTail = m_tail.exchange(newNode, std::memory_order_acq_rel);
        prevTail->next.store(newNode, std::memory_order_release);
    }
    
    bool pop(T& result) {
        Node* head = m_head.load(std::memory_order_acquire);
        Node* next = head->next.load(std::memory_order_acquire);
        
        if (next == nullptr) {
            return false; // Queue is empty
        }
        
        result = std::move(next->data);
        m_head.store(next, std::memory_order_release);
        delete head;
        return true;
    }
    
private:
    struct Node {
        T data;
        std::atomic<Node*> next;
    };
    
    std::atomic<Node*> m_head;
    std::atomic<Node*> m_tail;
};
```

## Performance Best Practices

### Memory Optimization

```cpp
// Use object pools for frequently allocated objects
template<typename T>
class ObjectPool {
public:
    template<typename... Args>
    std::unique_ptr<T, std::function<void(T*)>> acquire(Args&&... args) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_pool.empty()) {
            return {new T(std::forward<Args>(args)...), [this](T* obj) {
                this->release(obj);
            }};
        }
        
        auto obj = std::move(m_pool.back());
        m_pool.pop_back();
        
        // Reinitialize object
        *obj = T(std::forward<Args>(args)...);
        
        return {obj.release(), [this](T* obj) {
            this->release(obj);
        }};
    }
    
private:
    void release(T* obj) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_pool.size() < m_maxSize) {
            m_pool.emplace_back(obj);
        } else {
            delete obj;
        }
    }
    
    std::mutex m_mutex;
    std::vector<std::unique_ptr<T>> m_pool;
    size_t m_maxSize = 100;
};

// Use small string optimization
class SmallString {
public:
    SmallString(const char* str) {
        size_t len = strlen(str);
        if (len <= SMALL_SIZE) {
            m_isSmall = true;
            strcpy(m_small, str);
        } else {
            m_isSmall = false;
            m_large = new char[len + 1];
            strcpy(m_large, str);
        }
    }
    
    ~SmallString() {
        if (!m_isSmall) {
            delete[] m_large;
        }
    }
    
    const char* c_str() const {
        return m_isSmall ? m_small : m_large;
    }
    
private:
    static constexpr size_t SMALL_SIZE = 15;
    bool m_isSmall;
    union {
        char m_small[SMALL_SIZE + 1];
        char* m_large;
    };
};
```

### Algorithm Optimization

```cpp
// Use appropriate containers
// std::vector for sequential access
// std::unordered_map for fast lookups
// std::set for sorted unique elements
// std::deque for front/back insertion

// Reserve capacity when size is known
void processItems(const std::vector<Item>& items) {
    std::vector<ProcessedItem> results;
    results.reserve(items.size()); // Avoid reallocations
    
    for (const auto& item : items) {
        results.emplace_back(processItem(item));
    }
}

// Use move semantics
class DataProcessor {
public:
    void addData(std::vector<Data> data) { // Pass by value for move
        m_data = std::move(data); // Move assignment
    }
    
    std::vector<Data> extractData() {
        return std::move(m_data); // Move return
    }
    
private:
    std::vector<Data> m_data;
};

// Use string_view for read-only string parameters
void processString(std::string_view str) {
    // No copying, works with std::string, const char*, etc.
    if (str.starts_with("prefix")) {
        // Process...
    }
}
```

## Testing Best Practices

### Unit Testing

```cpp
// Use Google Test framework
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Test fixture for related tests
class NetworkManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_manager = std::make_unique<NetworkManager>();
        m_manager->initialize();
    }
    
    void TearDown() override {
        m_manager->shutdown();
        m_manager.reset();
    }
    
    std::unique_ptr<NetworkManager> m_manager;
};

// Test naming: MethodName_StateUnderTest_ExpectedBehavior
TEST_F(NetworkManagerTest, Connect_ValidUrl_ReturnsTrue) {
    EXPECT_TRUE(m_manager->connect("https://example.com"));
}

TEST_F(NetworkManagerTest, Connect_InvalidUrl_ReturnsFalse) {
    EXPECT_FALSE(m_manager->connect("invalid-url"));
}

TEST_F(NetworkManagerTest, Connect_EmptyUrl_ThrowsException) {
    EXPECT_THROW(m_manager->connect(""), std::invalid_argument);
}

// Parameterized tests
class UrlValidationTest : public ::testing::TestWithParam<std::pair<std::string, bool>> {};

TEST_P(UrlValidationTest, ValidateUrl) {
    auto [url, expected] = GetParam();
    EXPECT_EQ(validateUrl(url), expected);
}

INSTANTIATE_TEST_SUITE_P(
    UrlValidationTests,
    UrlValidationTest,
    ::testing::Values(
        std::make_pair("https://example.com", true),
        std::make_pair("http://localhost:8080", true),
        std::make_pair("invalid-url", false),
        std::make_pair("", false)
    )
);
```

### Mocking

```cpp
// Mock interface
class MockHttpClient : public IHttpClient {
public:
    MOCK_METHOD(HttpResponse, get, (const std::string& url), (override));
    MOCK_METHOD(HttpResponse, post, (const std::string& url, const std::string& data), (override));
};

// Test with mock
TEST(ApiClientTest, GetUser_ValidId_ReturnsUser) {
    MockHttpClient mockClient;
    ApiClient apiClient(&mockClient);
    
    // Setup expectations
    HttpResponse mockResponse{200, R"({"id": 123, "name": "John"})"};
    EXPECT_CALL(mockClient, get("https://api.example.com/users/123"))
        .WillOnce(::testing::Return(mockResponse));
    
    // Execute test
    auto user = apiClient.getUser(123);
    
    // Verify results
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->id, 123);
    EXPECT_EQ(user->name, "John");
}
```

## Documentation

### Code Documentation

```cpp
/**
 * @brief Manages HTTP connections and requests
 * 
 * The HttpClient class provides a high-level interface for making HTTP requests
 * with support for connection pooling, retries, and various authentication methods.
 * 
 * @example Basic usage:
 * @code
 * HttpClient client;
 * client.setTimeout(std::chrono::seconds(30));
 * 
 * HttpResponse response = client.get("https://api.example.com/data");
 * if (response.isSuccess()) {
 *     processData(response.body);
 * }
 * @endcode
 * 
 * @note This class is thread-safe for concurrent requests
 * @warning SSL certificate verification is enabled by default
 */
class HttpClient {
public:
    /**
     * @brief Constructs an HttpClient with default settings
     * 
     * Creates a new HttpClient instance with the following defaults:
     * - Connection timeout: 30 seconds
     * - Read timeout: 60 seconds
     * - SSL verification: enabled
     * - User agent: "CoreBaseApplication/1.0"
     */
    HttpClient();
    
    /**
     * @brief Performs an HTTP GET request
     * 
     * @param url The URL to request (must be a valid HTTP/HTTPS URL)
     * @param headers Optional HTTP headers to include in the request
     * @return HttpResponse containing the server response
     * 
     * @throws std::invalid_argument if the URL is malformed
     * @throws NetworkException if the request fails
     * 
     * @example
     * @code
     * HttpClient client;
     * std::map<std::string, std::string> headers = {
     *     {"Authorization", "Bearer token123"},
     *     {"Accept", "application/json"}
     * };
     * HttpResponse response = client.get("https://api.example.com/users", headers);
     * @endcode
     */
    HttpResponse get(const std::string& url, 
                    const std::map<std::string, std::string>& headers = {});
    
    /**
     * @brief Sets the connection timeout
     * 
     * @param timeout Maximum time to wait for connection establishment
     * @pre timeout must be positive
     * @post Future requests will use the new timeout value
     */
    void setConnectionTimeout(std::chrono::seconds timeout);
    
private:
    /**
     * @brief Validates a URL format
     * 
     * @param url The URL to validate
     * @return true if the URL is valid, false otherwise
     * 
     * @internal This is an internal helper method
     */
    bool validateUrl(const std::string& url) const;
};
```

## Security Best Practices

### Input Validation

```cpp
// Always validate input parameters
class UserManager {
public:
    enum class ValidationError {
        INVALID_EMAIL,
        PASSWORD_TOO_SHORT,
        USERNAME_INVALID
    };
    
    Result<User, ValidationError> createUser(const std::string& username, 
                                           const std::string& email, 
                                           const std::string& password) {
        // Validate username
        if (!isValidUsername(username)) {
            return Result<User, ValidationError>::error(ValidationError::USERNAME_INVALID);
        }
        
        // Validate email
        if (!isValidEmail(email)) {
            return Result<User, ValidationError>::error(ValidationError::INVALID_EMAIL);
        }
        
        // Validate password
        if (password.length() < MIN_PASSWORD_LENGTH) {
            return Result<User, ValidationError>::error(ValidationError::PASSWORD_TOO_SHORT);
        }
        
        // Create user after validation
        User user{username, email, hashPassword(password)};
        return Result<User, ValidationError>::success(std::move(user));
    }
    
private:
    static constexpr size_t MIN_PASSWORD_LENGTH = 8;
    
    bool isValidUsername(const std::string& username) const {
        // Username validation logic
        return !username.empty() && 
               username.length() <= 50 && 
               std::all_of(username.begin(), username.end(), [](char c) {
                   return std::isalnum(c) || c == '_' || c == '-';
               });
    }
    
    bool isValidEmail(const std::string& email) const {
        // Email validation logic (simplified)
        return email.find('@') != std::string::npos && 
               email.length() <= 254;
    }
};
```

### Secure String Handling

```cpp
// Use secure string for sensitive data
class SecureString {
public:
    explicit SecureString(const std::string& data) {
        m_data.resize(data.size());
        std::copy(data.begin(), data.end(), m_data.begin());
    }
    
    ~SecureString() {
        // Zero out memory before deallocation
        if (!m_data.empty()) {
            std::fill(m_data.begin(), m_data.end(), 0);
        }
    }
    
    // Prevent copying
    SecureString(const SecureString&) = delete;
    SecureString& operator=(const SecureString&) = delete;
    
    // Allow moving
    SecureString(SecureString&& other) noexcept : m_data(std::move(other.m_data)) {}
    SecureString& operator=(SecureString&& other) noexcept {
        if (this != &other) {
            clear();
            m_data = std::move(other.m_data);
        }
        return *this;
    }
    
    const char* data() const { return m_data.data(); }
    size_t size() const { return m_data.size(); }
    
    void clear() {
        if (!m_data.empty()) {
            std::fill(m_data.begin(), m_data.end(), 0);
            m_data.clear();
        }
    }
    
private:
    std::vector<char> m_data;
};
```

## See Also

- [Development Setup](setup.md)
- [Performance Tuning](performance.md)
- [Security Guidelines](security.md)
- [Architecture Overview](../architecture/overview.md)
- [API Reference](../api/core-api.md)