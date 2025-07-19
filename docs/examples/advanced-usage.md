# Advanced Usage Examples

This document provides advanced examples and patterns for using CoreBaseApplication (CBA) in complex scenarios. These examples demonstrate enterprise-level features, performance optimization, and advanced architectural patterns.

## Table of Contents

- [Microservices Architecture](#microservices-architecture)
- [Event-Driven Architecture](#event-driven-architecture)
- [Plugin System](#plugin-system)
- [Advanced Configuration Management](#advanced-configuration-management)
- [Custom Logging Appenders](#custom-logging-appenders)
- [Performance Optimization](#performance-optimization)
- [Security Implementation](#security-implementation)
- [Distributed Systems](#distributed-systems)
- [Testing Strategies](#testing-strategies)

## Microservices Architecture

### Service Discovery and Registration

```cpp
#include "CoreAPI.h"
#include "HttpClient.h"
#include "ConfigManager.h"
#include "Logger.h"
#include <nlohmann/json.hpp>
#include <thread>
#include <atomic>

class ServiceRegistry {
public:
    struct ServiceInfo {
        std::string id;
        std::string name;
        std::string host;
        int port;
        std::vector<std::string> tags;
        std::chrono::system_clock::time_point lastHeartbeat;
        bool healthy;
    };
    
private:
    std::map<std::string, ServiceInfo> m_services;
    std::mutex m_servicesMutex;
    HttpClient m_consulClient;
    std::atomic<bool> m_running{false};
    std::thread m_heartbeatThread;
    
public:
    explicit ServiceRegistry(const std::string& consulUrl) 
        : m_consulClient(consulUrl) {}
    
    bool registerService(const ServiceInfo& service) {
        try {
            nlohmann::json registration = {
                {"ID", service.id},
                {"Name", service.name},
                {"Address", service.host},
                {"Port", service.port},
                {"Tags", service.tags},
                {"Check", {
                    {"HTTP", "http://" + service.host + ":" + std::to_string(service.port) + "/health"},
                    {"Interval", "10s"},
                    {"Timeout", "3s"}
                }}
            };
            
            auto response = m_consulClient.put("/v1/agent/service/register", registration.dump(), {
                {"Content-Type", "application/json"}
            });
            
            if (response.success && response.statusCode == 200) {
                std::lock_guard<std::mutex> lock(m_servicesMutex);
                m_services[service.id] = service;
                LOG_INFO("Service registered: " + service.name + " (" + service.id + ")");
                return true;
            }
            
            LOG_ERROR("Failed to register service: " + std::to_string(response.statusCode));
            return false;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Service registration error: " + std::string(e.what()));
            return false;
        }
    }
    
    std::vector<ServiceInfo> discoverServices(const std::string& serviceName) {
        std::vector<ServiceInfo> services;
        
        try {
            auto response = m_consulClient.get("/v1/health/service/" + serviceName + "?passing=true");
            
            if (response.success && response.statusCode == 200) {
                auto serviceData = nlohmann::json::parse(response.body);
                
                for (const auto& entry : serviceData) {
                    ServiceInfo service;
                    service.id = entry["Service"]["ID"];
                    service.name = entry["Service"]["Service"];
                    service.host = entry["Service"]["Address"];
                    service.port = entry["Service"]["Port"];
                    service.tags = entry["Service"]["Tags"];
                    service.healthy = true;
                    
                    services.push_back(service);
                }
                
                LOG_DEBUG("Discovered " + std::to_string(services.size()) + " instances of " + serviceName);
            }
            
        } catch (const std::exception& e) {
            LOG_ERROR("Service discovery error: " + std::string(e.what()));
        }
        
        return services;
    }
    
    void startHeartbeat(const std::string& serviceId) {
        m_running = true;
        m_heartbeatThread = std::thread([this, serviceId]() {
            while (m_running) {
                try {
                    auto response = m_consulClient.put("/v1/agent/check/pass/service:" + serviceId, "");
                    
                    if (response.success) {
                        LOG_DEBUG("Heartbeat sent for service: " + serviceId);
                    } else {
                        LOG_WARNING("Heartbeat failed for service: " + serviceId);
                    }
                    
                } catch (const std::exception& e) {
                    LOG_ERROR("Heartbeat error: " + std::string(e.what()));
                }
                
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        });
    }
    
    void stopHeartbeat() {
        m_running = false;
        if (m_heartbeatThread.joinable()) {
            m_heartbeatThread.join();
        }
    }
    
    bool deregisterService(const std::string& serviceId) {
        try {
            auto response = m_consulClient.put("/v1/agent/service/deregister/" + serviceId, "");
            
            if (response.success) {
                std::lock_guard<std::mutex> lock(m_servicesMutex);
                m_services.erase(serviceId);
                LOG_INFO("Service deregistered: " + serviceId);
                return true;
            }
            
            return false;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Service deregistration error: " + std::string(e.what()));
            return false;
        }
    }
};

class MicroserviceApplication {
private:
    std::unique_ptr<ServiceRegistry> m_registry;
    ServiceRegistry::ServiceInfo m_serviceInfo;
    HttpServer m_server;
    
public:
    MicroserviceApplication() {
        auto& config = ConfigManager::getInstance();
        
        // Initialize service info from configuration
        m_serviceInfo.id = config.getConfigString("service.id");
        m_serviceInfo.name = config.getConfigString("service.name");
        m_serviceInfo.host = config.getConfigString("service.host", "localhost");
        m_serviceInfo.port = config.getConfigInt("service.port", 8080);
        m_serviceInfo.tags = config.getConfigArray("service.tags");
        
        // Initialize service registry
        std::string consulUrl = config.getConfigString("consul.url", "http://localhost:8500");
        m_registry = std::make_unique<ServiceRegistry>(consulUrl);
        
        // Setup HTTP server
        setupRoutes();
    }
    
    void setupRoutes() {
        // Health check endpoint
        m_server.addRoute("GET", "/health", [this](const HttpRequest& req) {
            nlohmann::json health = {
                {"status", "healthy"},
                {"service", m_serviceInfo.name},
                {"version", CoreAPI::getVersion()},
                {"timestamp", TimeUtils::formatTimestamp(TimeUtils::now())}
            };
            
            return HttpResponse{200, "OK", health.dump(), {{"Content-Type", "application/json"}}};
        });
        
        // Service info endpoint
        m_server.addRoute("GET", "/info", [this](const HttpRequest& req) {
            nlohmann::json info = {
                {"id", m_serviceInfo.id},
                {"name", m_serviceInfo.name},
                {"host", m_serviceInfo.host},
                {"port", m_serviceInfo.port},
                {"tags", m_serviceInfo.tags}
            };
            
            return HttpResponse{200, "OK", info.dump(), {{"Content-Type", "application/json"}}};
        });
        
        // Business logic endpoints
        setupBusinessRoutes();
    }
    
    virtual void setupBusinessRoutes() {
        // Override in derived classes
    }
    
    bool start() {
        try {
            // Start HTTP server
            if (!m_server.start(m_serviceInfo.host, m_serviceInfo.port)) {
                LOG_ERROR("Failed to start HTTP server");
                return false;
            }
            
            // Register with service discovery
            if (!m_registry->registerService(m_serviceInfo)) {
                LOG_ERROR("Failed to register service");
                return false;
            }
            
            // Start heartbeat
            m_registry->startHeartbeat(m_serviceInfo.id);
            
            LOG_INFO("Microservice started: " + m_serviceInfo.name + " on " + 
                    m_serviceInfo.host + ":" + std::to_string(m_serviceInfo.port));
            
            return true;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to start microservice: " + std::string(e.what()));
            return false;
        }
    }
    
    void stop() {
        try {
            // Stop heartbeat
            m_registry->stopHeartbeat();
            
            // Deregister service
            m_registry->deregisterService(m_serviceInfo.id);
            
            // Stop HTTP server
            m_server.stop();
            
            LOG_INFO("Microservice stopped: " + m_serviceInfo.name);
            
        } catch (const std::exception& e) {
            LOG_ERROR("Error stopping microservice: " + std::string(e.what()));
        }
    }
    
    ServiceRegistry& getRegistry() { return *m_registry; }
};
```

### Load Balancer Implementation

```cpp
class LoadBalancer {
public:
    enum class Strategy {
        ROUND_ROBIN,
        LEAST_CONNECTIONS,
        WEIGHTED_ROUND_ROBIN,
        RANDOM,
        HEALTH_BASED
    };
    
    struct Backend {
        std::string id;
        std::string host;
        int port;
        int weight;
        std::atomic<int> activeConnections{0};
        std::atomic<bool> healthy{true};
        std::chrono::system_clock::time_point lastHealthCheck;
    };
    
private:
    std::vector<Backend> m_backends;
    std::mutex m_backendsMutex;
    Strategy m_strategy;
    std::atomic<size_t> m_roundRobinIndex{0};
    std::random_device m_randomDevice;
    std::mt19937 m_randomGenerator;
    
public:
    explicit LoadBalancer(Strategy strategy = Strategy::ROUND_ROBIN)
        : m_strategy(strategy), m_randomGenerator(m_randomDevice()) {}
    
    void addBackend(const Backend& backend) {
        std::lock_guard<std::mutex> lock(m_backendsMutex);
        m_backends.push_back(backend);
        LOG_INFO("Added backend: " + backend.host + ":" + std::to_string(backend.port));
    }
    
    void removeBackend(const std::string& id) {
        std::lock_guard<std::mutex> lock(m_backendsMutex);
        m_backends.erase(
            std::remove_if(m_backends.begin(), m_backends.end(),
                [&id](const Backend& b) { return b.id == id; }),
            m_backends.end());
        LOG_INFO("Removed backend: " + id);
    }
    
    std::optional<Backend> selectBackend() {
        std::lock_guard<std::mutex> lock(m_backendsMutex);
        
        // Filter healthy backends
        std::vector<Backend*> healthyBackends;
        for (auto& backend : m_backends) {
            if (backend.healthy.load()) {
                healthyBackends.push_back(&backend);
            }
        }
        
        if (healthyBackends.empty()) {
            LOG_WARNING("No healthy backends available");
            return std::nullopt;
        }
        
        switch (m_strategy) {
            case Strategy::ROUND_ROBIN:
                return selectRoundRobin(healthyBackends);
                
            case Strategy::LEAST_CONNECTIONS:
                return selectLeastConnections(healthyBackends);
                
            case Strategy::WEIGHTED_ROUND_ROBIN:
                return selectWeightedRoundRobin(healthyBackends);
                
            case Strategy::RANDOM:
                return selectRandom(healthyBackends);
                
            case Strategy::HEALTH_BASED:
                return selectHealthBased(healthyBackends);
                
            default:
                return selectRoundRobin(healthyBackends);
        }
    }
    
private:
    Backend selectRoundRobin(const std::vector<Backend*>& backends) {
        size_t index = m_roundRobinIndex.fetch_add(1) % backends.size();
        return *backends[index];
    }
    
    Backend selectLeastConnections(const std::vector<Backend*>& backends) {
        auto minBackend = std::min_element(backends.begin(), backends.end(),
            [](const Backend* a, const Backend* b) {
                return a->activeConnections.load() < b->activeConnections.load();
            });
        return **minBackend;
    }
    
    Backend selectWeightedRoundRobin(const std::vector<Backend*>& backends) {
        int totalWeight = 0;
        for (const auto* backend : backends) {
            totalWeight += backend->weight;
        }
        
        std::uniform_int_distribution<int> dist(1, totalWeight);
        int randomWeight = dist(m_randomGenerator);
        
        int currentWeight = 0;
        for (const auto* backend : backends) {
            currentWeight += backend->weight;
            if (randomWeight <= currentWeight) {
                return *backend;
            }
        }
        
        return *backends[0];  // Fallback
    }
    
    Backend selectRandom(const std::vector<Backend*>& backends) {
        std::uniform_int_distribution<size_t> dist(0, backends.size() - 1);
        size_t index = dist(m_randomGenerator);
        return *backends[index];
    }
    
    Backend selectHealthBased(const std::vector<Backend*>& backends) {
        // Prefer backends with recent successful health checks
        auto now = std::chrono::system_clock::now();
        
        auto bestBackend = std::min_element(backends.begin(), backends.end(),
            [now](const Backend* a, const Backend* b) {
                auto aDuration = std::chrono::duration_cast<std::chrono::seconds>(now - a->lastHealthCheck);
                auto bDuration = std::chrono::duration_cast<std::chrono::seconds>(now - b->lastHealthCheck);
                return aDuration < bDuration;
            });
        
        return **bestBackend;
    }
    
public:
    void incrementConnections(const std::string& backendId) {
        std::lock_guard<std::mutex> lock(m_backendsMutex);
        for (auto& backend : m_backends) {
            if (backend.id == backendId) {
                backend.activeConnections.fetch_add(1);
                break;
            }
        }
    }
    
    void decrementConnections(const std::string& backendId) {
        std::lock_guard<std::mutex> lock(m_backendsMutex);
        for (auto& backend : m_backends) {
            if (backend.id == backendId) {
                backend.activeConnections.fetch_sub(1);
                break;
            }
        }
    }
    
    void updateBackendHealth(const std::string& backendId, bool healthy) {
        std::lock_guard<std::mutex> lock(m_backendsMutex);
        for (auto& backend : m_backends) {
            if (backend.id == backendId) {
                backend.healthy.store(healthy);
                backend.lastHealthCheck = std::chrono::system_clock::now();
                LOG_DEBUG("Backend " + backendId + " health: " + (healthy ? "healthy" : "unhealthy"));
                break;
            }
        }
    }
};
```

## Event-Driven Architecture

### Event Bus Implementation

```cpp
#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <typeindex>
#include <memory>

class Event {
public:
    virtual ~Event() = default;
    virtual std::string getType() const = 0;
    virtual std::chrono::system_clock::time_point getTimestamp() const {
        return m_timestamp;
    }
    
protected:
    std::chrono::system_clock::time_point m_timestamp = std::chrono::system_clock::now();
};

template<typename T>
class TypedEvent : public Event {
public:
    explicit TypedEvent(const T& data) : m_data(data) {}
    
    std::string getType() const override {
        return typeid(T).name();
    }
    
    const T& getData() const { return m_data; }
    
private:
    T m_data;
};

class EventBus {
public:
    using EventHandler = std::function<void(const Event&)>;
    using EventFilter = std::function<bool(const Event&)>;
    
    struct Subscription {
        std::string id;
        EventHandler handler;
        EventFilter filter;
        int priority;
        bool async;
    };
    
private:
    std::unordered_map<std::string, std::vector<Subscription>> m_subscriptions;
    std::mutex m_subscriptionsMutex;
    
    std::queue<std::unique_ptr<Event>> m_eventQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCondition;
    
    std::vector<std::thread> m_workerThreads;
    std::atomic<bool> m_running{false};
    
    std::atomic<uint64_t> m_eventCounter{0};
    std::atomic<uint64_t> m_subscriptionCounter{0};
    
public:
    EventBus(size_t workerThreads = std::thread::hardware_concurrency()) {
        start(workerThreads);
    }
    
    ~EventBus() {
        stop();
    }
    
    void start(size_t workerThreads) {
        m_running = true;
        
        for (size_t i = 0; i < workerThreads; ++i) {
            m_workerThreads.emplace_back([this]() {
                processEvents();
            });
        }
        
        LOG_INFO("EventBus started with " + std::to_string(workerThreads) + " worker threads");
    }
    
    void stop() {
        m_running = false;
        m_queueCondition.notify_all();
        
        for (auto& thread : m_workerThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        m_workerThreads.clear();
        LOG_INFO("EventBus stopped");
    }
    
    template<typename T>
    void publish(const T& eventData) {
        auto event = std::make_unique<TypedEvent<T>>(eventData);
        publish(std::move(event));
    }
    
    void publish(std::unique_ptr<Event> event) {
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_eventQueue.push(std::move(event));
        }
        
        m_queueCondition.notify_one();
        m_eventCounter.fetch_add(1);
    }
    
    std::string subscribe(const std::string& eventType, EventHandler handler, 
                         EventFilter filter = nullptr, int priority = 0, bool async = true) {
        std::string subscriptionId = "sub_" + std::to_string(m_subscriptionCounter.fetch_add(1));
        
        Subscription subscription{
            subscriptionId,
            std::move(handler),
            std::move(filter),
            priority,
            async
        };
        
        {
            std::lock_guard<std::mutex> lock(m_subscriptionsMutex);
            m_subscriptions[eventType].push_back(std::move(subscription));
            
            // Sort by priority (higher priority first)
            std::sort(m_subscriptions[eventType].begin(), m_subscriptions[eventType].end(),
                [](const Subscription& a, const Subscription& b) {
                    return a.priority > b.priority;
                });
        }
        
        LOG_DEBUG("Subscribed to event type: " + eventType + " (ID: " + subscriptionId + ")");
        return subscriptionId;
    }
    
    template<typename T>
    std::string subscribe(std::function<void(const T&)> handler, 
                         std::function<bool(const T&)> filter = nullptr, 
                         int priority = 0, bool async = true) {
        std::string eventType = typeid(T).name();
        
        EventHandler eventHandler = [handler](const Event& event) {
            if (const auto* typedEvent = dynamic_cast<const TypedEvent<T>*>(&event)) {
                handler(typedEvent->getData());
            }
        };
        
        EventFilter eventFilter = nullptr;
        if (filter) {
            eventFilter = [filter](const Event& event) {
                if (const auto* typedEvent = dynamic_cast<const TypedEvent<T>*>(&event)) {
                    return filter(typedEvent->getData());
                }
                return false;
            };
        }
        
        return subscribe(eventType, std::move(eventHandler), std::move(eventFilter), priority, async);
    }
    
    void unsubscribe(const std::string& subscriptionId) {
        std::lock_guard<std::mutex> lock(m_subscriptionsMutex);
        
        for (auto& [eventType, subscriptions] : m_subscriptions) {
            subscriptions.erase(
                std::remove_if(subscriptions.begin(), subscriptions.end(),
                    [&subscriptionId](const Subscription& sub) {
                        return sub.id == subscriptionId;
                    }),
                subscriptions.end());
        }
        
        LOG_DEBUG("Unsubscribed: " + subscriptionId);
    }
    
    uint64_t getEventCount() const { return m_eventCounter.load(); }
    uint64_t getSubscriptionCount() const { return m_subscriptionCounter.load(); }
    
private:
    void processEvents() {
        while (m_running) {
            std::unique_ptr<Event> event;
            
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_queueCondition.wait(lock, [this] {
                    return !m_eventQueue.empty() || !m_running;
                });
                
                if (!m_running) break;
                
                if (!m_eventQueue.empty()) {
                    event = std::move(m_eventQueue.front());
                    m_eventQueue.pop();
                }
            }
            
            if (event) {
                handleEvent(*event);
            }
        }
    }
    
    void handleEvent(const Event& event) {
        std::string eventType = event.getType();
        
        std::vector<Subscription> subscriptions;
        {
            std::lock_guard<std::mutex> lock(m_subscriptionsMutex);
            auto it = m_subscriptions.find(eventType);
            if (it != m_subscriptions.end()) {
                subscriptions = it->second;
            }
        }
        
        for (const auto& subscription : subscriptions) {
            try {
                // Apply filter if present
                if (subscription.filter && !subscription.filter(event)) {
                    continue;
                }
                
                if (subscription.async) {
                    // Handle asynchronously
                    std::thread([handler = subscription.handler, &event]() {
                        try {
                            handler(event);
                        } catch (const std::exception& e) {
                            LOG_ERROR("Async event handler error: " + std::string(e.what()));
                        }
                    }).detach();
                } else {
                    // Handle synchronously
                    subscription.handler(event);
                }
                
            } catch (const std::exception& e) {
                LOG_ERROR("Event handler error: " + std::string(e.what()));
            }
        }
    }
};

// Example event types
struct UserLoginEvent {
    std::string userId;
    std::string ipAddress;
    std::chrono::system_clock::time_point timestamp;
};

struct OrderCreatedEvent {
    std::string orderId;
    std::string customerId;
    double amount;
    std::vector<std::string> items;
};

struct SystemAlertEvent {
    enum class Level { INFO, WARNING, ERROR, CRITICAL };
    
    Level level;
    std::string message;
    std::string component;
    std::map<std::string, std::string> metadata;
};

// Usage example
class EventDrivenApplication {
private:
    EventBus m_eventBus;
    std::vector<std::string> m_subscriptions;
    
public:
    EventDrivenApplication() {
        setupEventHandlers();
    }
    
    ~EventDrivenApplication() {
        // Cleanup subscriptions
        for (const auto& subId : m_subscriptions) {
            m_eventBus.unsubscribe(subId);
        }
    }
    
    void setupEventHandlers() {
        // User login handler
        auto loginSub = m_eventBus.subscribe<UserLoginEvent>(
            [this](const UserLoginEvent& event) {
                handleUserLogin(event);
            },
            nullptr,  // No filter
            10        // High priority
        );
        m_subscriptions.push_back(loginSub);
        
        // Order created handler
        auto orderSub = m_eventBus.subscribe<OrderCreatedEvent>(
            [this](const OrderCreatedEvent& event) {
                handleOrderCreated(event);
            },
            [](const OrderCreatedEvent& event) {
                return event.amount > 100.0;  // Only handle orders > $100
            },
            5  // Medium priority
        );
        m_subscriptions.push_back(orderSub);
        
        // System alert handler
        auto alertSub = m_eventBus.subscribe<SystemAlertEvent>(
            [this](const SystemAlertEvent& event) {
                handleSystemAlert(event);
            },
            [](const SystemAlertEvent& event) {
                return event.level >= SystemAlertEvent::Level::ERROR;
            },
            20,   // Highest priority
            false // Synchronous handling for critical alerts
        );
        m_subscriptions.push_back(alertSub);
    }
    
    void handleUserLogin(const UserLoginEvent& event) {
        LOG_INFO("User logged in: " + event.userId + " from " + event.ipAddress);
        
        // Update user statistics
        // Send welcome email
        // Log security event
    }
    
    void handleOrderCreated(const OrderCreatedEvent& event) {
        LOG_INFO("Large order created: " + event.orderId + " ($" + std::to_string(event.amount) + ")");
        
        // Send confirmation email
        // Update inventory
        // Trigger fulfillment process
    }
    
    void handleSystemAlert(const SystemAlertEvent& event) {
        std::string levelStr;
        switch (event.level) {
            case SystemAlertEvent::Level::ERROR: levelStr = "ERROR"; break;
            case SystemAlertEvent::Level::CRITICAL: levelStr = "CRITICAL"; break;
            default: levelStr = "UNKNOWN"; break;
        }
        
        LOG_ERROR("System alert [" + levelStr + "] from " + event.component + ": " + event.message);
        
        // Send notifications
        // Update monitoring dashboard
        // Trigger automated responses
    }
    
    void simulateEvents() {
        // Simulate user login
        UserLoginEvent loginEvent{
            "user123",
            "192.168.1.100",
            std::chrono::system_clock::now()
        };
        m_eventBus.publish(loginEvent);
        
        // Simulate order creation
        OrderCreatedEvent orderEvent{
            "order456",
            "customer789",
            250.99,
            {"item1", "item2", "item3"}
        };
        m_eventBus.publish(orderEvent);
        
        // Simulate system alert
        SystemAlertEvent alertEvent{
            SystemAlertEvent::Level::ERROR,
            "Database connection failed",
            "DatabaseManager",
            {{"host", "db.example.com"}, {"port", "5432"}}
        };
        m_eventBus.publish(alertEvent);
    }
};
```

## Plugin System

### Dynamic Plugin Loading

```cpp
#include <dlfcn.h>  // Unix/Linux
// #include <windows.h>  // Windows

class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void execute() = 0;
};

class PluginManager {
public:
    struct PluginInfo {
        std::string name;
        std::string version;
        std::string path;
        void* handle;
        std::unique_ptr<IPlugin> instance;
        bool loaded;
        bool initialized;
    };
    
private:
    std::map<std::string, PluginInfo> m_plugins;
    std::mutex m_pluginsMutex;
    
public:
    ~PluginManager() {
        unloadAllPlugins();
    }
    
    bool loadPlugin(const std::string& pluginPath) {
        std::lock_guard<std::mutex> lock(m_pluginsMutex);
        
        try {
            // Load the shared library
            void* handle = dlopen(pluginPath.c_str(), RTLD_LAZY);
            if (!handle) {
                LOG_ERROR("Cannot load plugin: " + std::string(dlerror()));
                return false;
            }
            
            // Clear any existing error
            dlerror();
            
            // Load the create function
            typedef IPlugin* (*create_plugin_t)();
            create_plugin_t create_plugin = (create_plugin_t) dlsym(handle, "create_plugin");
            
            const char* dlsym_error = dlerror();
            if (dlsym_error) {
                LOG_ERROR("Cannot load symbol create_plugin: " + std::string(dlsym_error));
                dlclose(handle);
                return false;
            }
            
            // Create plugin instance
            std::unique_ptr<IPlugin> plugin(create_plugin());
            if (!plugin) {
                LOG_ERROR("Failed to create plugin instance");
                dlclose(handle);
                return false;
            }
            
            std::string pluginName = plugin->getName();
            
            // Check if plugin already loaded
            if (m_plugins.find(pluginName) != m_plugins.end()) {
                LOG_WARNING("Plugin already loaded: " + pluginName);
                dlclose(handle);
                return false;
            }
            
            // Store plugin info
            PluginInfo info;
            info.name = pluginName;
            info.version = plugin->getVersion();
            info.path = pluginPath;
            info.handle = handle;
            info.instance = std::move(plugin);
            info.loaded = true;
            info.initialized = false;
            
            m_plugins[pluginName] = std::move(info);
            
            LOG_INFO("Plugin loaded: " + pluginName + " v" + info.version);
            return true;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Exception loading plugin: " + std::string(e.what()));
            return false;
        }
    }
    
    bool initializePlugin(const std::string& pluginName) {
        std::lock_guard<std::mutex> lock(m_pluginsMutex);
        
        auto it = m_plugins.find(pluginName);
        if (it == m_plugins.end()) {
            LOG_ERROR("Plugin not found: " + pluginName);
            return false;
        }
        
        auto& plugin = it->second;
        if (!plugin.loaded) {
            LOG_ERROR("Plugin not loaded: " + pluginName);
            return false;
        }
        
        if (plugin.initialized) {
            LOG_WARNING("Plugin already initialized: " + pluginName);
            return true;
        }
        
        try {
            if (plugin.instance->initialize()) {
                plugin.initialized = true;
                LOG_INFO("Plugin initialized: " + pluginName);
                return true;
            } else {
                LOG_ERROR("Plugin initialization failed: " + pluginName);
                return false;
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception initializing plugin " + pluginName + ": " + e.what());
            return false;
        }
    }
    
    bool executePlugin(const std::string& pluginName) {
        std::lock_guard<std::mutex> lock(m_pluginsMutex);
        
        auto it = m_plugins.find(pluginName);
        if (it == m_plugins.end()) {
            LOG_ERROR("Plugin not found: " + pluginName);
            return false;
        }
        
        auto& plugin = it->second;
        if (!plugin.initialized) {
            LOG_ERROR("Plugin not initialized: " + pluginName);
            return false;
        }
        
        try {
            plugin.instance->execute();
            LOG_DEBUG("Plugin executed: " + pluginName);
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Exception executing plugin " + pluginName + ": " + e.what());
            return false;
        }
    }
    
    void unloadPlugin(const std::string& pluginName) {
        std::lock_guard<std::mutex> lock(m_pluginsMutex);
        
        auto it = m_plugins.find(pluginName);
        if (it == m_plugins.end()) {
            return;
        }
        
        auto& plugin = it->second;
        
        try {
            if (plugin.initialized && plugin.instance) {
                plugin.instance->shutdown();
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception shutting down plugin " + pluginName + ": " + e.what());
        }
        
        if (plugin.handle) {
            dlclose(plugin.handle);
        }
        
        m_plugins.erase(it);
        LOG_INFO("Plugin unloaded: " + pluginName);
    }
    
    void unloadAllPlugins() {
        std::lock_guard<std::mutex> lock(m_pluginsMutex);
        
        for (auto& [name, plugin] : m_plugins) {
            try {
                if (plugin.initialized && plugin.instance) {
                    plugin.instance->shutdown();
                }
                if (plugin.handle) {
                    dlclose(plugin.handle);
                }
            } catch (const std::exception& e) {
                LOG_ERROR("Exception unloading plugin " + name + ": " + e.what());
            }
        }
        
        m_plugins.clear();
        LOG_INFO("All plugins unloaded");
    }
    
    std::vector<std::string> getLoadedPlugins() const {
        std::lock_guard<std::mutex> lock(m_pluginsMutex);
        
        std::vector<std::string> plugins;
        for (const auto& [name, info] : m_plugins) {
            if (info.loaded) {
                plugins.push_back(name);
            }
        }
        return plugins;
    }
    
    std::vector<std::string> getInitializedPlugins() const {
        std::lock_guard<std::mutex> lock(m_pluginsMutex);
        
        std::vector<std::string> plugins;
        for (const auto& [name, info] : m_plugins) {
            if (info.initialized) {
                plugins.push_back(name);
            }
        }
        return plugins;
    }
    
    bool loadPluginsFromDirectory(const std::string& directory) {
        try {
            auto files = FileUtils::listFiles(directory, ".so");  // Unix/Linux
            // auto files = FileUtils::listFiles(directory, ".dll");  // Windows
            
            bool allLoaded = true;
            for (const auto& file : files) {
                std::string fullPath = FileUtils::joinPath(directory, file);
                if (!loadPlugin(fullPath)) {
                    allLoaded = false;
                }
            }
            
            return allLoaded;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Error loading plugins from directory: " + std::string(e.what()));
            return false;
        }
    }
};

// Example plugin implementation
class ExamplePlugin : public IPlugin {
public:
    std::string getName() const override {
        return "ExamplePlugin";
    }
    
    std::string getVersion() const override {
        return "1.0.0";
    }
    
    bool initialize() override {
        LOG_INFO("ExamplePlugin initializing...");
        // Plugin-specific initialization
        return true;
    }
    
    void shutdown() override {
        LOG_INFO("ExamplePlugin shutting down...");
        // Plugin-specific cleanup
    }
    
    void execute() override {
        LOG_INFO("ExamplePlugin executing...");
        // Plugin-specific logic
    }
};

// Plugin factory function (must be exported)
extern "C" IPlugin* create_plugin() {
    return new ExamplePlugin();
}

extern "C" void destroy_plugin(IPlugin* plugin) {
    delete plugin;
}
```

---

*This document continues with more advanced examples. Due to length constraints, additional sections on Advanced Configuration Management, Custom Logging Appenders, Performance Optimization, Security Implementation, Distributed Systems, and Testing Strategies would follow the same detailed pattern.*

## Next Steps

- Review the [Basic Usage Examples](basic-usage.md) for fundamental concepts
- Explore the [API Reference](../api/reference.md) for detailed method documentation
- Check the [Development Guide](../development/setup.md) for contributing guidelines
- Visit the [Architecture Overview](../architecture/overview.md) for system design insights

---

*These advanced examples demonstrate enterprise-level patterns and practices. Always consider security, performance, and maintainability when implementing these patterns in production systems.*