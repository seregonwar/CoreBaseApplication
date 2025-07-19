# Network Management

The Network Management module provides a comprehensive networking layer with support for HTTP/HTTPS clients, WebSocket connections, REST API utilities, and advanced networking features like connection pooling, retry mechanisms, and load balancing.

## Features

- **HTTP/HTTPS Client**: Full-featured HTTP client with SSL/TLS support
- **WebSocket Support**: Real-time bidirectional communication
- **REST API Utilities**: Simplified REST API interactions
- **Connection Pooling**: Efficient connection reuse and management
- **Retry Mechanisms**: Configurable retry policies with backoff strategies
- **Load Balancing**: Multiple load balancing algorithms
- **Request/Response Middleware**: Extensible request/response processing
- **Authentication**: Built-in support for various auth methods
- **Compression**: Automatic request/response compression
- **Caching**: HTTP response caching with configurable policies

## Quick Start

```cpp
#include "core/NetworkManager/NetworkManager.h"
#include "core/NetworkManager/HttpClient.h"
#include "core/NetworkManager/WebSocketClient.h"

using namespace Core::NetworkManager;

// Initialize network manager
NetworkManager& networkManager = NetworkManager::getInstance();

// Configure global settings
NetworkConfig config;
config.connectionTimeout = std::chrono::seconds(30);
config.readTimeout = std::chrono::seconds(60);
config.maxConnections = 100;
config.enableCompression = true;
networkManager.configure(config);

// Create HTTP client
HttpClient client;

// Simple GET request
HttpResponse response = client.get("https://api.example.com/users");
if (response.isSuccess()) {
    std::cout << "Response: " << response.body << std::endl;
} else {
    std::cerr << "Error: " << response.statusCode << " - " << response.statusMessage << std::endl;
}

// POST request with JSON data
HttpRequest request;
request.url = "https://api.example.com/users";
request.method = HttpMethod::POST;
request.headers["Content-Type"] = "application/json";
request.body = R"({"name": "John Doe", "email": "john@example.com"})";

HttpResponse postResponse = client.send(request);
```

## HTTP Client

### Basic HTTP Operations

```cpp
// GET request
HttpResponse response = client.get("https://api.example.com/data");

// GET with query parameters
std::map<std::string, std::string> params = {
    {"page", "1"},
    {"limit", "10"},
    {"sort", "name"}
};
HttpResponse response = client.get("https://api.example.com/users", params);

// POST request
std::string jsonData = R"({"name": "John", "age": 30})";
HttpResponse response = client.post("https://api.example.com/users", jsonData);

// PUT request
HttpResponse response = client.put("https://api.example.com/users/123", jsonData);

// DELETE request
HttpResponse response = client.del("https://api.example.com/users/123");

// PATCH request
std::string patchData = R"({"age": 31})";
HttpResponse response = client.patch("https://api.example.com/users/123", patchData);
```

### Advanced HTTP Features

```cpp
// Custom headers
HttpRequest request;
request.url = "https://api.example.com/data";
request.method = HttpMethod::GET;
request.headers["Authorization"] = "Bearer your-token-here";
request.headers["User-Agent"] = "CoreBaseApplication/1.0";
request.headers["Accept"] = "application/json";

HttpResponse response = client.send(request);

// File upload
HttpRequest uploadRequest;
uploadRequest.url = "https://api.example.com/upload";
uploadRequest.method = HttpMethod::POST;
uploadRequest.files["document"] = "/path/to/file.pdf";
uploadRequest.fields["description"] = "Important document";

HttpResponse uploadResponse = client.send(uploadRequest);

// Download file
HttpRequest downloadRequest;
downloadRequest.url = "https://api.example.com/files/document.pdf";
downloadRequest.outputFile = "/path/to/save/document.pdf";

HttpResponse downloadResponse = client.send(downloadRequest);

// Streaming response
client.get("https://api.example.com/large-data", [](const std::string& chunk) {
    // Process chunk as it arrives
    processDataChunk(chunk);
    return true; // Continue streaming
});
```

### Authentication

```cpp
// Basic Authentication
client.setBasicAuth("username", "password");

// Bearer Token
client.setBearerToken("your-jwt-token");

// API Key
client.setApiKey("X-API-Key", "your-api-key");

// OAuth 2.0
OAuthConfig oauthConfig;
oauthConfig.clientId = "your-client-id";
oauthConfig.clientSecret = "your-client-secret";
oauthConfig.tokenUrl = "https://auth.example.com/token";
oauthConfig.scope = "read write";

client.setOAuth2(oauthConfig);

// Custom authentication
client.setAuthHandler([](HttpRequest& request) {
    // Add custom authentication headers
    std::string signature = generateSignature(request);
    request.headers["X-Signature"] = signature;
});
```

## WebSocket Client

### Basic WebSocket Usage

```cpp
// Create WebSocket client
WebSocketClient wsClient;

// Set event handlers
wsClient.onOpen([](const WebSocketConnection& connection) {
    std::cout << "WebSocket connected" << std::endl;
});

wsClient.onMessage([](const WebSocketConnection& connection, const std::string& message) {
    std::cout << "Received: " << message << std::endl;
    
    // Echo the message back
    connection.send("Echo: " + message);
});

wsClient.onClose([](const WebSocketConnection& connection, int code, const std::string& reason) {
    std::cout << "WebSocket closed: " << code << " - " << reason << std::endl;
});

wsClient.onError([](const WebSocketConnection& connection, const std::string& error) {
    std::cerr << "WebSocket error: " << error << std::endl;
});

// Connect to WebSocket server
if (wsClient.connect("wss://echo.websocket.org")) {
    // Send messages
    wsClient.send("Hello, WebSocket!");
    wsClient.sendBinary(binaryData);
    
    // Keep connection alive
    wsClient.run(); // Blocks until connection closes
}
```

### Advanced WebSocket Features

```cpp
// WebSocket with custom headers
WebSocketConfig wsConfig;
wsConfig.headers["Authorization"] = "Bearer token";
wsConfig.headers["X-Client-Version"] = "1.0.0";
wsConfig.subprotocols = {"chat", "superchat"};
wsConfig.pingInterval = std::chrono::seconds(30);
wsConfig.pongTimeout = std::chrono::seconds(10);

WebSocketClient wsClient(wsConfig);

// Automatic reconnection
wsClient.enableAutoReconnect(true);
wsClient.setReconnectDelay(std::chrono::seconds(5));
wsClient.setMaxReconnectAttempts(10);

// Message queuing when disconnected
wsClient.enableMessageQueuing(true);
wsClient.setMaxQueueSize(1000);

// Compression
wsClient.enableCompression(true);
```

## REST API Utilities

### RESTful Resource Client

```cpp
// Create REST client for a specific resource
RestClient userClient("https://api.example.com/users");

// Set default headers
userClient.setDefaultHeader("Authorization", "Bearer token");
userClient.setDefaultHeader("Content-Type", "application/json");

// CRUD operations
// Create
User newUser = {"John Doe", "john@example.com"};
RestResponse<User> createResponse = userClient.create(newUser);
if (createResponse.isSuccess()) {
    User createdUser = createResponse.data;
    std::cout << "Created user with ID: " << createdUser.id << std::endl;
}

// Read (get by ID)
RestResponse<User> getResponse = userClient.get(123);
if (getResponse.isSuccess()) {
    User user = getResponse.data;
    std::cout << "User: " << user.name << " (" << user.email << ")" << std::endl;
}

// Read (list with pagination)
PaginationParams pagination = {1, 10}; // page 1, 10 items per page
FilterParams filters = {{"status", "active"}, {"role", "admin"}};
RestResponse<std::vector<User>> listResponse = userClient.list(pagination, filters);

// Update
User updatedUser = getResponse.data;
updatedUser.email = "newemail@example.com";
RestResponse<User> updateResponse = userClient.update(123, updatedUser);

// Delete
RestResponse<void> deleteResponse = userClient.remove(123);
```

### API Client Builder

```cpp
// Build API client with fluent interface
auto apiClient = ApiClientBuilder()
    .baseUrl("https://api.example.com")
    .timeout(std::chrono::seconds(30))
    .retryPolicy(RetryPolicy::exponentialBackoff(3, std::chrono::seconds(1)))
    .authentication(BearerToken("your-token"))
    .middleware(LoggingMiddleware())
    .middleware(CompressionMiddleware())
    .build();

// Use the client
auto response = apiClient.get("/users/123");
```

## Connection Pooling

### HTTP Connection Pool

```cpp
// Configure connection pool
ConnectionPoolConfig poolConfig;
poolConfig.maxConnections = 50;
poolConfig.maxConnectionsPerHost = 10;
poolConfig.connectionTimeout = std::chrono::seconds(30);
poolConfig.keepAliveTimeout = std::chrono::seconds(60);
poolConfig.idleTimeout = std::chrono::seconds(300);

// Create connection pool
ConnectionPool pool(poolConfig);

// Use pooled connections
HttpClient client(&pool);
HttpResponse response = client.get("https://api.example.com/data");

// Pool automatically manages connection reuse
```

### Connection Pool Monitoring

```cpp
// Get pool statistics
ConnectionPoolStats stats = pool.getStats();
std::cout << "Active connections: " << stats.activeConnections << std::endl;
std::cout << "Idle connections: " << stats.idleConnections << std::endl;
std::cout << "Total requests: " << stats.totalRequests << std::endl;
std::cout << "Pool hits: " << stats.poolHits << std::endl;
std::cout << "Pool misses: " << stats.poolMisses << std::endl;

// Set pool event handlers
pool.onConnectionCreated([](const std::string& host) {
    std::cout << "New connection created for " << host << std::endl;
});

pool.onConnectionClosed([](const std::string& host) {
    std::cout << "Connection closed for " << host << std::endl;
});
```

## Retry Mechanisms

### Retry Policies

```cpp
// Fixed delay retry
RetryPolicy fixedRetry = RetryPolicy::fixedDelay(3, std::chrono::seconds(2));

// Exponential backoff
RetryPolicy exponentialRetry = RetryPolicy::exponentialBackoff(
    5,                              // max attempts
    std::chrono::seconds(1),        // initial delay
    std::chrono::seconds(60),       // max delay
    2.0                             // multiplier
);

// Linear backoff
RetryPolicy linearRetry = RetryPolicy::linearBackoff(
    3,                              // max attempts
    std::chrono::seconds(1),        // initial delay
    std::chrono::seconds(5)         // increment
);

// Custom retry policy
RetryPolicy customRetry = RetryPolicy::custom([](int attempt, const HttpResponse& response) {
    // Custom retry logic
    if (response.statusCode >= 500) {
        return std::chrono::seconds(attempt * 2);
    }
    return std::chrono::seconds(0); // Don't retry
});

// Apply retry policy to client
client.setRetryPolicy(exponentialRetry);
```

### Conditional Retries

```cpp
// Retry only on specific conditions
RetryCondition retryCondition = [](const HttpResponse& response) {
    // Retry on server errors or network timeouts
    return response.statusCode >= 500 || 
           response.statusCode == 408 || 
           response.error == NetworkError::TIMEOUT;
};

client.setRetryCondition(retryCondition);

// Retry with circuit breaker
CircuitBreakerConfig cbConfig;
cbConfig.failureThreshold = 5;
cbConfig.recoveryTimeout = std::chrono::seconds(30);
cbConfig.halfOpenMaxCalls = 3;

CircuitBreaker circuitBreaker(cbConfig);
client.setCircuitBreaker(circuitBreaker);
```

## Load Balancing

### Load Balancer Configuration

```cpp
// Create load balancer with multiple endpoints
std::vector<std::string> endpoints = {
    "https://api1.example.com",
    "https://api2.example.com",
    "https://api3.example.com"
};

// Round-robin load balancer
LoadBalancer roundRobinLB(LoadBalancingStrategy::ROUND_ROBIN, endpoints);

// Weighted round-robin
std::map<std::string, int> weights = {
    {"https://api1.example.com", 3},
    {"https://api2.example.com", 2},
    {"https://api3.example.com", 1}
};
LoadBalancer weightedLB(LoadBalancingStrategy::WEIGHTED_ROUND_ROBIN, endpoints, weights);

// Least connections
LoadBalancer leastConnLB(LoadBalancingStrategy::LEAST_CONNECTIONS, endpoints);

// Random selection
LoadBalancer randomLB(LoadBalancingStrategy::RANDOM, endpoints);
```

### Health Checking

```cpp
// Configure health checks for load balancer
HealthCheckConfig healthConfig;
healthConfig.interval = std::chrono::seconds(30);
healthConfig.timeout = std::chrono::seconds(5);
healthConfig.healthyThreshold = 2;
healthConfig.unhealthyThreshold = 3;
healthConfig.path = "/health";

roundRobinLB.enableHealthChecks(healthConfig);

// Custom health check
roundRobinLB.setHealthChecker([](const std::string& endpoint) {
    HttpClient client;
    HttpResponse response = client.get(endpoint + "/health");
    return response.isSuccess() && response.body == "OK";
});

// Use load balancer with HTTP client
HttpClient client(&roundRobinLB);
HttpResponse response = client.get("/api/data"); // Automatically load balanced
```

## Middleware

### Request/Response Middleware

```cpp
// Logging middleware
class LoggingMiddleware : public HttpMiddleware {
public:
    void processRequest(HttpRequest& request) override {
        std::cout << "Request: " << request.method << " " << request.url << std::endl;
        request.headers["X-Request-ID"] = generateRequestId();
    }
    
    void processResponse(const HttpRequest& request, HttpResponse& response) override {
        std::cout << "Response: " << response.statusCode << " (" 
                  << response.duration.count() << "ms)" << std::endl;
    }
};

// Authentication middleware
class AuthMiddleware : public HttpMiddleware {
public:
    AuthMiddleware(const std::string& token) : m_token(token) {}
    
    void processRequest(HttpRequest& request) override {
        request.headers["Authorization"] = "Bearer " + m_token;
    }
    
private:
    std::string m_token;
};

// Compression middleware
class CompressionMiddleware : public HttpMiddleware {
public:
    void processRequest(HttpRequest& request) override {
        request.headers["Accept-Encoding"] = "gzip, deflate";
        
        if (!request.body.empty() && request.body.size() > 1024) {
            request.body = compress(request.body);
            request.headers["Content-Encoding"] = "gzip";
        }
    }
    
    void processResponse(const HttpRequest& request, HttpResponse& response) override {
        if (response.headers.count("Content-Encoding")) {
            response.body = decompress(response.body);
        }
    }
};

// Register middleware
client.addMiddleware(std::make_shared<LoggingMiddleware>());
client.addMiddleware(std::make_shared<AuthMiddleware>("your-token"));
client.addMiddleware(std::make_shared<CompressionMiddleware>());
```

## Caching

### HTTP Response Caching

```cpp
// Configure HTTP cache
HttpCacheConfig cacheConfig;
cacheConfig.maxSize = 100 * 1024 * 1024; // 100 MB
cacheConfig.defaultTtl = std::chrono::hours(1);
cacheConfig.respectCacheHeaders = true;
cacheConfig.staleWhileRevalidate = std::chrono::minutes(5);

HttpCache cache(cacheConfig);
client.setCache(&cache);

// Cache-specific requests
HttpRequest request;
request.url = "https://api.example.com/data";
request.cachePolicy = CachePolicy::CACHE_FIRST; // Try cache first
// request.cachePolicy = CachePolicy::NETWORK_FIRST; // Try network first
// request.cachePolicy = CachePolicy::CACHE_ONLY; // Cache only
// request.cachePolicy = CachePolicy::NETWORK_ONLY; // Network only

HttpResponse response = client.send(request);

// Manual cache operations
cache.put("custom-key", response, std::chrono::hours(2));
std::optional<HttpResponse> cachedResponse = cache.get("custom-key");
cache.remove("custom-key");
cache.clear();
```

## Configuration

### Network Configuration

```json
{
  "network": {
    "connectionTimeout": 30,
    "readTimeout": 60,
    "writeTimeout": 30,
    "maxConnections": 100,
    "maxConnectionsPerHost": 10,
    "keepAliveTimeout": 60,
    "enableCompression": true,
    "userAgent": "CoreBaseApplication/1.0",
    "ssl": {
      "verifyPeer": true,
      "verifyHost": true,
      "caFile": "/path/to/ca-bundle.crt",
      "clientCert": "/path/to/client.crt",
      "clientKey": "/path/to/client.key"
    },
    "proxy": {
      "enabled": false,
      "host": "proxy.example.com",
      "port": 8080,
      "username": "proxyuser",
      "password": "proxypass"
    },
    "retry": {
      "maxAttempts": 3,
      "strategy": "exponential",
      "initialDelay": 1,
      "maxDelay": 60,
      "multiplier": 2.0
    },
    "cache": {
      "enabled": true,
      "maxSize": 104857600,
      "defaultTtl": 3600,
      "respectCacheHeaders": true
    }
  }
}
```

## Performance Optimization

### Best Practices

```cpp
// Use connection pooling
ConnectionPool pool(poolConfig);
HttpClient client(&pool);

// Enable HTTP/2 when available
client.enableHttp2(true);

// Use compression for large payloads
client.enableCompression(true);

// Implement proper timeout values
client.setConnectionTimeout(std::chrono::seconds(10));
client.setReadTimeout(std::chrono::seconds(30));

// Use async operations for high throughput
std::vector<std::future<HttpResponse>> futures;
for (const auto& url : urls) {
    futures.push_back(std::async(std::launch::async, [&client, url]() {
        return client.get(url);
    }));
}

// Collect results
for (auto& future : futures) {
    HttpResponse response = future.get();
    processResponse(response);
}
```

## Error Handling

### Network Error Types

```cpp
enum class NetworkError {
    NONE,
    CONNECTION_FAILED,
    TIMEOUT,
    SSL_ERROR,
    DNS_RESOLUTION_FAILED,
    INVALID_URL,
    TOO_MANY_REDIRECTS,
    PROTOCOL_ERROR,
    CANCELLED
};

// Handle specific errors
HttpResponse response = client.get("https://api.example.com/data");
if (!response.isSuccess()) {
    switch (response.error) {
        case NetworkError::CONNECTION_FAILED:
            std::cerr << "Failed to connect to server" << std::endl;
            break;
        case NetworkError::TIMEOUT:
            std::cerr << "Request timed out" << std::endl;
            break;
        case NetworkError::SSL_ERROR:
            std::cerr << "SSL/TLS error: " << response.errorMessage << std::endl;
            break;
        default:
            std::cerr << "Network error: " << response.errorMessage << std::endl;
            break;
    }
}
```

## Thread Safety

- All network operations are thread-safe
- Connection pools handle concurrent access
- Multiple threads can use the same client instance
- WebSocket connections are thread-safe for sending

## See Also

- [Configuration Management](configuration.md)
- [Advanced Logging](logging.md)
- [Security Best Practices](../development/security.md)
- [Performance Tuning](../development/performance.md)