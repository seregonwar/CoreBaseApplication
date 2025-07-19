# System Monitoring

The System Monitoring module provides comprehensive system resource monitoring, performance metrics collection, and health checking capabilities for applications built with CoreBaseApplication.

## Features

- **Real-time Metrics**: CPU, memory, disk, and network monitoring
- **Performance Counters**: Application-specific performance tracking
- **Health Checks**: Automated system health verification
- **Alerting System**: Configurable alerts and notifications
- **Metrics Export**: Prometheus, InfluxDB, and custom exporters
- **Historical Data**: Time-series data storage and analysis
- **Dashboard Integration**: Built-in web dashboard
- **Custom Metrics**: User-defined metrics and collectors

## Quick Start

```cpp
#include "core/SystemMonitor/SystemMonitor.h"
#include "core/SystemMonitor/MetricsCollector.h"

using namespace Core::SystemMonitor;

// Initialize system monitor
SystemMonitor& monitor = SystemMonitor::getInstance();

// Start monitoring
monitor.start();

// Get current system metrics
SystemMetrics metrics = monitor.getCurrentMetrics();
std::cout << "CPU Usage: " << metrics.cpuUsage << "%" << std::endl;
std::cout << "Memory Usage: " << metrics.memoryUsage << "%" << std::endl;
std::cout << "Disk Usage: " << metrics.diskUsage << "%" << std::endl;

// Register custom metric
monitor.registerMetric("requests_per_second", MetricType::COUNTER);
monitor.incrementCounter("requests_per_second");

// Add health check
monitor.addHealthCheck("database", []() {
    return checkDatabaseConnection() ? HealthStatus::HEALTHY : HealthStatus::UNHEALTHY;
});
```

## System Metrics

### CPU Monitoring

```cpp
// Get CPU usage information
CpuMetrics cpu = monitor.getCpuMetrics();

std::cout << "Overall CPU Usage: " << cpu.overallUsage << "%" << std::endl;
std::cout << "User CPU Time: " << cpu.userTime << "%" << std::endl;
std::cout << "System CPU Time: " << cpu.systemTime << "%" << std::endl;
std::cout << "Idle Time: " << cpu.idleTime << "%" << std::endl;

// Per-core CPU usage
for (size_t i = 0; i < cpu.coreUsage.size(); ++i) {
    std::cout << "Core " << i << " Usage: " << cpu.coreUsage[i] << "%" << std::endl;
}

// CPU load averages (Linux/macOS)
LoadAverage load = monitor.getLoadAverage();
std::cout << "Load Average (1m): " << load.oneMinute << std::endl;
std::cout << "Load Average (5m): " << load.fiveMinute << std::endl;
std::cout << "Load Average (15m): " << load.fifteenMinute << std::endl;
```

### Memory Monitoring

```cpp
// Get memory usage information
MemoryMetrics memory = monitor.getMemoryMetrics();

std::cout << "Total Memory: " << memory.totalMemory / (1024*1024) << " MB" << std::endl;
std::cout << "Used Memory: " << memory.usedMemory / (1024*1024) << " MB" << std::endl;
std::cout << "Free Memory: " << memory.freeMemory / (1024*1024) << " MB" << std::endl;
std::cout << "Memory Usage: " << memory.usagePercentage << "%" << std::endl;

// Process-specific memory
ProcessMemory processMemory = monitor.getProcessMemory();
std::cout << "Process RSS: " << processMemory.rss / (1024*1024) << " MB" << std::endl;
std::cout << "Process VMS: " << processMemory.vms / (1024*1024) << " MB" << std::endl;
std::cout << "Process Shared: " << processMemory.shared / (1024*1024) << " MB" << std::endl;
```

### Disk Monitoring

```cpp
// Get disk usage information
std::vector<DiskMetrics> disks = monitor.getDiskMetrics();

for (const auto& disk : disks) {
    std::cout << "Disk: " << disk.mountPoint << std::endl;
    std::cout << "  Total: " << disk.totalSpace / (1024*1024*1024) << " GB" << std::endl;
    std::cout << "  Used: " << disk.usedSpace / (1024*1024*1024) << " GB" << std::endl;
    std::cout << "  Free: " << disk.freeSpace / (1024*1024*1024) << " GB" << std::endl;
    std::cout << "  Usage: " << disk.usagePercentage << "%" << std::endl;
}

// Disk I/O statistics
DiskIOMetrics diskIO = monitor.getDiskIOMetrics();
std::cout << "Read Operations: " << diskIO.readOperations << std::endl;
std::cout << "Write Operations: " << diskIO.writeOperations << std::endl;
std::cout << "Bytes Read: " << diskIO.bytesRead << std::endl;
std::cout << "Bytes Written: " << diskIO.bytesWritten << std::endl;
```

### Network Monitoring

```cpp
// Get network interface information
std::vector<NetworkMetrics> interfaces = monitor.getNetworkMetrics();

for (const auto& iface : interfaces) {
    std::cout << "Interface: " << iface.interfaceName << std::endl;
    std::cout << "  Bytes Received: " << iface.bytesReceived << std::endl;
    std::cout << "  Bytes Sent: " << iface.bytesSent << std::endl;
    std::cout << "  Packets Received: " << iface.packetsReceived << std::endl;
    std::cout << "  Packets Sent: " << iface.packetsSent << std::endl;
    std::cout << "  Errors: " << iface.errors << std::endl;
    std::cout << "  Drops: " << iface.drops << std::endl;
}

// Network connection statistics
NetworkConnectionMetrics connections = monitor.getConnectionMetrics();
std::cout << "Active Connections: " << connections.activeConnections << std::endl;
std::cout << "Listening Ports: " << connections.listeningPorts << std::endl;
std::cout << "TCP Connections: " << connections.tcpConnections << std::endl;
std::cout << "UDP Connections: " << connections.udpConnections << std::endl;
```

## Custom Metrics

### Metric Types

```cpp
enum class MetricType {
    COUNTER,    // Monotonically increasing value
    GAUGE,      // Current value that can go up or down
    HISTOGRAM,  // Distribution of values
    TIMER       // Time-based measurements
};
```

### Counter Metrics

```cpp
// Register counter metric
monitor.registerMetric("http_requests_total", MetricType::COUNTER);
monitor.registerMetric("database_queries_total", MetricType::COUNTER);

// Increment counters
monitor.incrementCounter("http_requests_total");
monitor.incrementCounter("database_queries_total", 5); // Increment by 5

// Get counter value
uint64_t requestCount = monitor.getCounterValue("http_requests_total");
```

### Gauge Metrics

```cpp
// Register gauge metric
monitor.registerMetric("active_connections", MetricType::GAUGE);
monitor.registerMetric("queue_size", MetricType::GAUGE);

// Set gauge values
monitor.setGaugeValue("active_connections", 42);
monitor.setGaugeValue("queue_size", 128);

// Increment/decrement gauges
monitor.incrementGauge("active_connections");
monitor.decrementGauge("queue_size", 10);

// Get gauge value
double connections = monitor.getGaugeValue("active_connections");
```

### Histogram Metrics

```cpp
// Register histogram metric
monitor.registerMetric("request_duration_seconds", MetricType::HISTOGRAM);

// Record values
monitor.recordHistogram("request_duration_seconds", 0.125);
monitor.recordHistogram("request_duration_seconds", 0.250);
monitor.recordHistogram("request_duration_seconds", 0.089);

// Get histogram statistics
HistogramStats stats = monitor.getHistogramStats("request_duration_seconds");
std::cout << "Count: " << stats.count << std::endl;
std::cout << "Sum: " << stats.sum << std::endl;
std::cout << "Average: " << stats.average << std::endl;
std::cout << "Min: " << stats.min << std::endl;
std::cout << "Max: " << stats.max << std::endl;
std::cout << "95th Percentile: " << stats.percentile95 << std::endl;
std::cout << "99th Percentile: " << stats.percentile99 << std::endl;
```

### Timer Metrics

```cpp
// Register timer metric
monitor.registerMetric("database_query_duration", MetricType::TIMER);

// Manual timing
auto start = std::chrono::high_resolution_clock::now();
// ... perform operation ...
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
monitor.recordTimer("database_query_duration", duration.count());

// Automatic timing with RAII
{
    TimerScope timer(monitor, "database_query_duration");
    // ... perform operation ...
    // Timer automatically recorded when scope exits
}

// Lambda-based timing
monitor.timeOperation("database_query_duration", []() {
    // ... perform operation ...
    return performDatabaseQuery();
});
```

## Health Checks

### Basic Health Checks

```cpp
// Add simple health check
monitor.addHealthCheck("database", []() {
    try {
        return testDatabaseConnection() ? HealthStatus::HEALTHY : HealthStatus::UNHEALTHY;
    } catch (const std::exception& e) {
        return HealthStatus::UNHEALTHY;
    }
});

// Add health check with details
monitor.addHealthCheck("external_api", []() {
    HealthCheckResult result;
    try {
        auto response = callExternalAPI();
        if (response.statusCode == 200) {
            result.status = HealthStatus::HEALTHY;
            result.message = "API responding normally";
            result.responseTime = response.duration;
        } else {
            result.status = HealthStatus::UNHEALTHY;
            result.message = "API returned status " + std::to_string(response.statusCode);
        }
    } catch (const std::exception& e) {
        result.status = HealthStatus::UNHEALTHY;
        result.message = "API call failed: " + std::string(e.what());
    }
    return result;
});
```

### Health Check Results

```cpp
// Get overall health status
OverallHealth health = monitor.getOverallHealth();
std::cout << "Overall Status: " << toString(health.status) << std::endl;
std::cout << "Healthy Checks: " << health.healthyCount << std::endl;
std::cout << "Unhealthy Checks: " << health.unhealthyCount << std::endl;

// Get individual health check results
std::map<std::string, HealthCheckResult> results = monitor.getHealthCheckResults();
for (const auto& [name, result] : results) {
    std::cout << "Health Check: " << name << std::endl;
    std::cout << "  Status: " << toString(result.status) << std::endl;
    std::cout << "  Message: " << result.message << std::endl;
    std::cout << "  Last Check: " << result.lastCheck << std::endl;
    if (result.responseTime > 0) {
        std::cout << "  Response Time: " << result.responseTime << "ms" << std::endl;
    }
}
```

## Alerting System

### Alert Configuration

```cpp
// Configure CPU usage alert
AlertRule cpuAlert;
cpuAlert.name = "high_cpu_usage";
cpuAlert.condition = "cpu_usage > 80";
cpuAlert.severity = AlertSeverity::WARNING;
cpuAlert.duration = std::chrono::minutes(5); // Alert after 5 minutes
cpuAlert.message = "CPU usage is above 80% for 5 minutes";
monitor.addAlertRule(cpuAlert);

// Configure memory usage alert
AlertRule memoryAlert;
memoryAlert.name = "high_memory_usage";
memoryAlert.condition = "memory_usage > 90";
memoryAlert.severity = AlertSeverity::CRITICAL;
memoryAlert.duration = std::chrono::minutes(2);
memoryAlert.message = "Memory usage is critically high";
monitor.addAlertRule(memoryAlert);

// Configure custom metric alert
AlertRule requestAlert;
requestAlert.name = "high_error_rate";
requestAlert.condition = "error_rate > 5";
requestAlert.severity = AlertSeverity::ERROR;
requestAlert.duration = std::chrono::minutes(1);
requestAlert.message = "Error rate is above 5%";
monitor.addAlertRule(requestAlert);
```

### Alert Handlers

```cpp
// Register alert handlers
monitor.addAlertHandler(AlertSeverity::WARNING, [](const Alert& alert) {
    std::cout << "WARNING: " << alert.message << std::endl;
    // Log to file, send to monitoring system, etc.
});

monitor.addAlertHandler(AlertSeverity::CRITICAL, [](const Alert& alert) {
    std::cerr << "CRITICAL ALERT: " << alert.message << std::endl;
    // Send email, SMS, push notification, etc.
    sendCriticalAlert(alert);
});

// Email alert handler
monitor.addAlertHandler(AlertSeverity::ERROR, [](const Alert& alert) {
    EmailAlert email;
    email.to = "admin@example.com";
    email.subject = "System Alert: " + alert.name;
    email.body = alert.message + "\n\nTime: " + alert.timestamp;
    sendEmail(email);
});
```

## Metrics Export

### Prometheus Export

```cpp
// Enable Prometheus metrics export
PrometheusExporter prometheusExporter(8080); // Port 8080
monitor.addExporter(std::make_shared<PrometheusExporter>(prometheusExporter));

// Custom Prometheus labels
prometheusExporter.addGlobalLabel("application", "corebaseapp");
prometheusExporter.addGlobalLabel("version", "1.0.0");
prometheusExporter.addGlobalLabel("environment", "production");

// Metrics available at http://localhost:8080/metrics
```

### InfluxDB Export

```cpp
// Configure InfluxDB exporter
InfluxDBConfig influxConfig;
influxConfig.url = "http://localhost:8086";
influxConfig.database = "monitoring";
influxConfig.username = "admin";
influxConfig.password = "password";
influxConfig.batchSize = 100;
influxConfig.flushInterval = std::chrono::seconds(10);

InfluxDBExporter influxExporter(influxConfig);
monitor.addExporter(std::make_shared<InfluxDBExporter>(influxExporter));
```

### Custom Export

```cpp
// Create custom exporter
class CustomExporter : public IMetricsExporter {
public:
    void exportMetrics(const MetricsSnapshot& snapshot) override {
        // Export to custom monitoring system
        for (const auto& [name, value] : snapshot.counters) {
            sendToCustomSystem("counter", name, value);
        }
        
        for (const auto& [name, value] : snapshot.gauges) {
            sendToCustomSystem("gauge", name, value);
        }
        
        for (const auto& [name, stats] : snapshot.histograms) {
            sendHistogramToCustomSystem(name, stats);
        }
    }
    
private:
    void sendToCustomSystem(const std::string& type, const std::string& name, double value) {
        // Implementation specific to your monitoring system
    }
};

// Register custom exporter
monitor.addExporter(std::make_shared<CustomExporter>());
```

## Web Dashboard

### Enable Built-in Dashboard

```cpp
// Start web dashboard on port 8081
monitor.enableWebDashboard(8081);

// Configure dashboard settings
DashboardConfig dashboardConfig;
dashboardConfig.title = "CoreBaseApplication Monitoring";
dashboardConfig.refreshInterval = std::chrono::seconds(5);
dashboardConfig.enableAuthentication = true;
dashboardConfig.username = "admin";
dashboardConfig.password = "secure_password";

monitor.configureDashboard(dashboardConfig);

// Dashboard available at http://localhost:8081
```

### Custom Dashboard Widgets

```cpp
// Add custom dashboard widget
DashboardWidget cpuWidget;
cpuWidget.type = WidgetType::LINE_CHART;
cpuWidget.title = "CPU Usage";
cpuWidget.metrics = {"cpu_usage"};
cpuWidget.timeRange = std::chrono::hours(1);
cpuWidget.position = {0, 0, 6, 4}; // x, y, width, height

monitor.addDashboardWidget(cpuWidget);

// Add gauge widget
DashboardWidget memoryWidget;
memoryWidget.type = WidgetType::GAUGE;
memoryWidget.title = "Memory Usage";
memoryWidget.metrics = {"memory_usage"};
memoryWidget.thresholds = {{70, "yellow"}, {90, "red"}};
memoryWidget.position = {6, 0, 3, 4};

monitor.addDashboardWidget(memoryWidget);
```

## Configuration

### JSON Configuration

```json
{
  "monitoring": {
    "enabled": true,
    "interval": 5,
    "retention": {
      "duration": "7d",
      "maxPoints": 10000
    },
    "system": {
      "cpu": true,
      "memory": true,
      "disk": true,
      "network": true
    },
    "healthChecks": {
      "enabled": true,
      "interval": 30,
      "timeout": 10
    },
    "alerts": {
      "enabled": true,
      "rules": [
        {
          "name": "high_cpu",
          "condition": "cpu_usage > 80",
          "severity": "warning",
          "duration": "5m"
        },
        {
          "name": "high_memory",
          "condition": "memory_usage > 90",
          "severity": "critical",
          "duration": "2m"
        }
      ]
    },
    "exporters": {
      "prometheus": {
        "enabled": true,
        "port": 8080,
        "path": "/metrics"
      },
      "influxdb": {
        "enabled": false,
        "url": "http://localhost:8086",
        "database": "monitoring"
      }
    },
    "dashboard": {
      "enabled": true,
      "port": 8081,
      "authentication": false
    }
  }
}
```

## Performance Considerations

- Monitoring overhead is typically < 1% CPU usage
- Metrics collection uses lock-free data structures
- Historical data is automatically pruned based on retention settings
- Export operations are performed asynchronously
- Dashboard updates use WebSocket for real-time data

## Best Practices

1. **Monitor Key Metrics**: Focus on metrics that matter for your application
2. **Set Appropriate Thresholds**: Configure alerts based on your system's normal behavior
3. **Use Labels Wisely**: Add meaningful labels to metrics for better filtering
4. **Regular Health Checks**: Implement comprehensive health checks for all dependencies
5. **Dashboard Organization**: Group related metrics together in dashboard widgets
6. **Alert Fatigue**: Avoid too many alerts; focus on actionable ones

## Troubleshooting

### Common Issues

**High monitoring overhead:**
- Reduce collection interval
- Disable unnecessary system metrics
- Optimize custom metric collection

**Missing metrics:**
- Check if monitoring is enabled
- Verify metric registration
- Check export configuration

**Dashboard not loading:**
- Verify web server is running
- Check port availability
- Review authentication settings

## See Also

- [Configuration Management](configuration.md)
- [Advanced Logging](logging.md)
- [Performance Tuning](../development/performance.md)
- [Deployment Guide](../deployment/production.md)