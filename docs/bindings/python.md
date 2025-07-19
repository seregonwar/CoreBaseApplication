# Python Bindings

CoreBaseApplication (CBA) provides comprehensive Python bindings that allow you to leverage the full power of the C++ framework from Python applications. This document covers installation, usage, and advanced integration patterns.

## Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [Core API](#core-api)
- [Configuration Management](#configuration-management)
- [Logging System](#logging-system)
- [HTTP Client](#http-client)
- [System Monitoring](#system-monitoring)
- [Event System](#event-system)
- [Advanced Usage](#advanced-usage)
- [Performance Considerations](#performance-considerations)
- [Troubleshooting](#troubleshooting)

## Installation

### Prerequisites

- Python 3.8 or higher
- pip package manager
- Compatible C++ runtime (automatically handled)

### Install from PyPI

```bash
pip install corebaseapplication
```

### Install from Source

```bash
git clone https://github.com/seregonwar/CoreBaseApplication-CBA.git
cd CoreBaseApplication-CBA/python
pip install -e .
```

### Verify Installation

```python
import cba
print(f"CBA Version: {cba.get_version()}")
print(f"Build Info: {cba.get_build_info()}")
```

## Quick Start

### Basic Application

```python
import cba
import asyncio

class MyApplication:
    def __init__(self):
        # Initialize CBA
        cba.initialize()
        
        # Setup logging
        self.logger = cba.Logger("MyApp")
        self.logger.set_level(cba.LogLevel.INFO)
        
        # Load configuration
        self.config = cba.ConfigManager()
        self.config.load_from_file("config.json")
        
    def run(self):
        self.logger.info("Application starting...")
        
        # Your application logic here
        app_name = self.config.get_string("app.name", "DefaultApp")
        self.logger.info(f"Running {app_name}")
        
        # Cleanup
        cba.shutdown()
        self.logger.info("Application stopped.")

if __name__ == "__main__":
    app = MyApplication()
    app.run()
```

### Async Application

```python
import cba
import asyncio

class AsyncApplication:
    def __init__(self):
        cba.initialize()
        self.logger = cba.Logger("AsyncApp")
        self.config = cba.ConfigManager()
        self.http_client = cba.HttpClient()
        
    async def fetch_data(self, url):
        """Fetch data from a URL asynchronously."""
        try:
            response = await self.http_client.get_async(url)
            if response.success:
                self.logger.info(f"Fetched {len(response.body)} bytes from {url}")
                return response.body
            else:
                self.logger.error(f"Failed to fetch {url}: {response.status_code}")
                return None
        except Exception as e:
            self.logger.error(f"Exception fetching {url}: {str(e)}")
            return None
    
    async def run(self):
        self.logger.info("Async application starting...")
        
        # Fetch multiple URLs concurrently
        urls = [
            "https://api.github.com/users/octocat",
            "https://httpbin.org/json",
            "https://jsonplaceholder.typicode.com/posts/1"
        ]
        
        tasks = [self.fetch_data(url) for url in urls]
        results = await asyncio.gather(*tasks)
        
        for i, result in enumerate(results):
            if result:
                self.logger.info(f"URL {i+1} returned {len(result)} bytes")
        
        cba.shutdown()
        self.logger.info("Async application stopped.")

async def main():
    app = AsyncApplication()
    await app.run()

if __name__ == "__main__":
    asyncio.run(main())
```

## Core API

### Initialization and Shutdown

```python
import cba

# Initialize CBA with default settings
cba.initialize()

# Initialize with custom settings
settings = cba.InitializationSettings()
settings.log_level = cba.LogLevel.DEBUG
settings.thread_pool_size = 8
settings.enable_metrics = True
cba.initialize(settings)

# Get version and build information
print(f"Version: {cba.get_version()}")
print(f"Build: {cba.get_build_info()}")
print(f"Platform: {cba.get_platform_info()}")

# Shutdown (automatically called at exit)
cba.shutdown()
```

### Error Handling

```python
import cba

try:
    # CBA operations
    config = cba.ConfigManager()
    config.load_from_file("nonexistent.json")
except cba.ConfigException as e:
    print(f"Configuration error: {e.message}")
    print(f"Error code: {e.code}")
except cba.CBAException as e:
    print(f"CBA error: {e.message}")
except Exception as e:
    print(f"Unexpected error: {str(e)}")
```

## Configuration Management

### Basic Configuration

```python
import cba
import json

# Create configuration manager
config = cba.ConfigManager()

# Load from JSON file
config.load_from_file("config.json")

# Load from JSON string
config_data = {
    "database": {
        "host": "localhost",
        "port": 5432,
        "name": "mydb"
    },
    "api": {
        "timeout": 30,
        "retries": 3,
        "endpoints": ["api1.example.com", "api2.example.com"]
    }
}
config.load_from_string(json.dumps(config_data))

# Get configuration values
db_host = config.get_string("database.host")
db_port = config.get_int("database.port")
api_timeout = config.get_int("api.timeout", 60)  # Default value
endpoints = config.get_array("api.endpoints")

# Set configuration values
config.set_string("app.version", "1.0.0")
config.set_int("app.max_connections", 100)
config.set_bool("app.debug_mode", True)

# Save configuration
config.save_to_file("updated_config.json")
```

### Advanced Configuration

```python
import cba
import os

class ConfigurationManager:
    def __init__(self, config_file="config.json"):
        self.config = cba.ConfigManager()
        self.config_file = config_file
        self.load_configuration()
        
    def load_configuration(self):
        """Load configuration with environment variable substitution."""
        try:
            # Load base configuration
            self.config.load_from_file(self.config_file)
            
            # Override with environment variables
            self._apply_env_overrides()
            
            # Validate configuration
            self._validate_config()
            
        except Exception as e:
            print(f"Failed to load configuration: {str(e)}")
            raise
    
    def _apply_env_overrides(self):
        """Apply environment variable overrides."""
        env_mappings = {
            "DATABASE_HOST": "database.host",
            "DATABASE_PORT": "database.port",
            "DATABASE_NAME": "database.name",
            "API_KEY": "api.key",
            "LOG_LEVEL": "logging.level"
        }
        
        for env_var, config_key in env_mappings.items():
            value = os.getenv(env_var)
            if value:
                if config_key.endswith(".port"):
                    self.config.set_int(config_key, int(value))
                else:
                    self.config.set_string(config_key, value)
    
    def _validate_config(self):
        """Validate required configuration values."""
        required_keys = [
            "database.host",
            "database.port",
            "database.name"
        ]
        
        for key in required_keys:
            if not self.config.has_key(key):
                raise ValueError(f"Required configuration key missing: {key}")
    
    def get_database_config(self):
        """Get database configuration as a dictionary."""
        return {
            "host": self.config.get_string("database.host"),
            "port": self.config.get_int("database.port"),
            "name": self.config.get_string("database.name"),
            "username": self.config.get_string("database.username", ""),
            "password": self.config.get_string("database.password", "")
        }
    
    def get_api_config(self):
        """Get API configuration as a dictionary."""
        return {
            "key": self.config.get_string("api.key"),
            "timeout": self.config.get_int("api.timeout", 30),
            "retries": self.config.get_int("api.retries", 3),
            "base_url": self.config.get_string("api.base_url")
        }

# Usage
config_manager = ConfigurationManager("app_config.json")
db_config = config_manager.get_database_config()
api_config = config_manager.get_api_config()
```

## Logging System

### Basic Logging

```python
import cba

# Create logger
logger = cba.Logger("MyApp")

# Set log level
logger.set_level(cba.LogLevel.INFO)

# Log messages
logger.debug("Debug message")
logger.info("Info message")
logger.warning("Warning message")
logger.error("Error message")
logger.critical("Critical message")

# Log with formatting
user_id = 12345
logger.info(f"User {user_id} logged in")

# Log exceptions
try:
    result = 10 / 0
except Exception as e:
    logger.error(f"Division error: {str(e)}")
    logger.exception("Full exception details:")  # Includes stack trace
```

### Advanced Logging

```python
import cba
import json
from datetime import datetime

class StructuredLogger:
    def __init__(self, name, enable_structured=True):
        self.logger = cba.Logger(name)
        self.enable_structured = enable_structured
        
        # Configure file appender
        file_appender = cba.FileAppender("logs/app.log")
        file_appender.set_max_size(10 * 1024 * 1024)  # 10MB
        file_appender.set_max_files(5)
        self.logger.add_appender(file_appender)
        
        # Configure console appender
        console_appender = cba.ConsoleAppender()
        console_appender.set_colored_output(True)
        self.logger.add_appender(console_appender)
    
    def log_structured(self, level, message, **kwargs):
        """Log structured data as JSON."""
        if self.enable_structured:
            log_data = {
                "timestamp": datetime.utcnow().isoformat(),
                "level": level.name,
                "message": message,
                **kwargs
            }
            log_message = json.dumps(log_data)
        else:
            log_message = message
            
        if level == cba.LogLevel.DEBUG:
            self.logger.debug(log_message)
        elif level == cba.LogLevel.INFO:
            self.logger.info(log_message)
        elif level == cba.LogLevel.WARNING:
            self.logger.warning(log_message)
        elif level == cba.LogLevel.ERROR:
            self.logger.error(log_message)
        elif level == cba.LogLevel.CRITICAL:
            self.logger.critical(log_message)
    
    def log_request(self, method, url, status_code, duration_ms, user_id=None):
        """Log HTTP request."""
        self.log_structured(
            cba.LogLevel.INFO,
            "HTTP request processed",
            method=method,
            url=url,
            status_code=status_code,
            duration_ms=duration_ms,
            user_id=user_id,
            request_type="http"
        )
    
    def log_database_query(self, query, duration_ms, rows_affected=None):
        """Log database query."""
        self.log_structured(
            cba.LogLevel.DEBUG,
            "Database query executed",
            query=query[:100] + "..." if len(query) > 100 else query,
            duration_ms=duration_ms,
            rows_affected=rows_affected,
            query_type="database"
        )
    
    def log_business_event(self, event_type, entity_id, details=None):
        """Log business event."""
        self.log_structured(
            cba.LogLevel.INFO,
            f"Business event: {event_type}",
            event_type=event_type,
            entity_id=entity_id,
            details=details or {},
            category="business"
        )

# Usage
logger = StructuredLogger("WebApp")

# Log HTTP request
logger.log_request("GET", "/api/users/123", 200, 45, user_id="user123")

# Log database query
logger.log_database_query("SELECT * FROM users WHERE id = ?", 12, rows_affected=1)

# Log business event
logger.log_business_event("user_registration", "user123", {
    "email": "user@example.com",
    "source": "web"
})
```

## HTTP Client

### Basic HTTP Operations

```python
import cba
import json

# Create HTTP client
client = cba.HttpClient()

# Configure client
client.set_timeout(30)  # 30 seconds
client.set_user_agent("MyApp/1.0")
client.set_max_redirects(5)

# GET request
response = client.get("https://api.github.com/users/octocat")
if response.success:
    user_data = json.loads(response.body)
    print(f"User: {user_data['name']}")
else:
    print(f"Error: {response.status_code} - {response.error_message}")

# POST request with JSON data
post_data = {
    "name": "John Doe",
    "email": "john@example.com"
}

headers = {"Content-Type": "application/json"}
response = client.post(
    "https://httpbin.org/post",
    json.dumps(post_data),
    headers
)

if response.success:
    result = json.loads(response.body)
    print(f"Posted data: {result['json']}")

# PUT request
update_data = {"name": "Jane Doe"}
response = client.put(
    "https://httpbin.org/put",
    json.dumps(update_data),
    headers
)

# DELETE request
response = client.delete("https://httpbin.org/delete")

# Custom headers
custom_headers = {
    "Authorization": "Bearer your-token",
    "X-Custom-Header": "custom-value"
}
response = client.get("https://api.example.com/data", custom_headers)
```

### Advanced HTTP Client

```python
import cba
import json
import asyncio
from typing import Optional, Dict, Any

class APIClient:
    def __init__(self, base_url: str, api_key: Optional[str] = None):
        self.base_url = base_url.rstrip('/')
        self.api_key = api_key
        self.client = cba.HttpClient()
        self.logger = cba.Logger("APIClient")
        
        # Configure client
        self.client.set_timeout(30)
        self.client.set_user_agent("CBA-Python-Client/1.0")
        
    def _get_headers(self, additional_headers: Optional[Dict[str, str]] = None) -> Dict[str, str]:
        """Get default headers with optional additional headers."""
        headers = {
            "Content-Type": "application/json",
            "Accept": "application/json"
        }
        
        if self.api_key:
            headers["Authorization"] = f"Bearer {self.api_key}"
        
        if additional_headers:
            headers.update(additional_headers)
            
        return headers
    
    def _make_url(self, endpoint: str) -> str:
        """Construct full URL from endpoint."""
        return f"{self.base_url}/{endpoint.lstrip('/')}"
    
    def _handle_response(self, response: cba.HttpResponse) -> Dict[str, Any]:
        """Handle HTTP response and parse JSON."""
        if not response.success:
            error_msg = f"HTTP {response.status_code}: {response.error_message}"
            self.logger.error(error_msg)
            raise cba.HttpException(error_msg)
        
        try:
            return json.loads(response.body) if response.body else {}
        except json.JSONDecodeError as e:
            self.logger.error(f"Failed to parse JSON response: {str(e)}")
            raise ValueError(f"Invalid JSON response: {str(e)}")
    
    def get(self, endpoint: str, params: Optional[Dict[str, Any]] = None, 
           headers: Optional[Dict[str, str]] = None) -> Dict[str, Any]:
        """Make GET request."""
        url = self._make_url(endpoint)
        
        if params:
            # Convert params to query string
            query_params = "&".join([f"{k}={v}" for k, v in params.items()])
            url += f"?{query_params}"
        
        self.logger.debug(f"GET {url}")
        response = self.client.get(url, self._get_headers(headers))
        return self._handle_response(response)
    
    def post(self, endpoint: str, data: Optional[Dict[str, Any]] = None,
            headers: Optional[Dict[str, str]] = None) -> Dict[str, Any]:
        """Make POST request."""
        url = self._make_url(endpoint)
        body = json.dumps(data) if data else ""
        
        self.logger.debug(f"POST {url}")
        response = self.client.post(url, body, self._get_headers(headers))
        return self._handle_response(response)
    
    def put(self, endpoint: str, data: Optional[Dict[str, Any]] = None,
           headers: Optional[Dict[str, str]] = None) -> Dict[str, Any]:
        """Make PUT request."""
        url = self._make_url(endpoint)
        body = json.dumps(data) if data else ""
        
        self.logger.debug(f"PUT {url}")
        response = self.client.put(url, body, self._get_headers(headers))
        return self._handle_response(response)
    
    def delete(self, endpoint: str, headers: Optional[Dict[str, str]] = None) -> Dict[str, Any]:
        """Make DELETE request."""
        url = self._make_url(endpoint)
        
        self.logger.debug(f"DELETE {url}")
        response = self.client.delete(url, self._get_headers(headers))
        return self._handle_response(response)
    
    async def get_async(self, endpoint: str, params: Optional[Dict[str, Any]] = None,
                       headers: Optional[Dict[str, str]] = None) -> Dict[str, Any]:
        """Make asynchronous GET request."""
        url = self._make_url(endpoint)
        
        if params:
            query_params = "&".join([f"{k}={v}" for k, v in params.items()])
            url += f"?{query_params}"
        
        self.logger.debug(f"GET (async) {url}")
        response = await self.client.get_async(url, self._get_headers(headers))
        return self._handle_response(response)
    
    async def post_async(self, endpoint: str, data: Optional[Dict[str, Any]] = None,
                        headers: Optional[Dict[str, str]] = None) -> Dict[str, Any]:
        """Make asynchronous POST request."""
        url = self._make_url(endpoint)
        body = json.dumps(data) if data else ""
        
        self.logger.debug(f"POST (async) {url}")
        response = await self.client.post_async(url, body, self._get_headers(headers))
        return self._handle_response(response)

# Usage
api_client = APIClient("https://api.example.com", "your-api-key")

# Synchronous requests
users = api_client.get("users", params={"page": 1, "limit": 10})
print(f"Found {len(users)} users")

new_user = api_client.post("users", {
    "name": "John Doe",
    "email": "john@example.com"
})
print(f"Created user with ID: {new_user['id']}")

# Asynchronous requests
async def fetch_multiple_resources():
    tasks = [
        api_client.get_async("users/1"),
        api_client.get_async("users/2"),
        api_client.get_async("users/3")
    ]
    
    results = await asyncio.gather(*tasks)
    for i, user in enumerate(results):
        print(f"User {i+1}: {user['name']}")

# Run async example
# asyncio.run(fetch_multiple_resources())
```

## System Monitoring

### Basic Monitoring

```python
import cba
import time

# Create system monitor
monitor = cba.SystemMonitor()

# Get system metrics
cpu_usage = monitor.get_cpu_usage()
memory_info = monitor.get_memory_info()
disk_info = monitor.get_disk_info()
network_info = monitor.get_network_info()

print(f"CPU Usage: {cpu_usage:.1f}%")
print(f"Memory Usage: {memory_info.used_mb:.1f}MB / {memory_info.total_mb:.1f}MB")
print(f"Disk Usage: {disk_info.used_gb:.1f}GB / {disk_info.total_gb:.1f}GB")
print(f"Network: RX {network_info.bytes_received}, TX {network_info.bytes_sent}")

# Performance counters
counter = cba.PerformanceCounter("requests_processed")
counter.increment()
counter.add(5)
print(f"Requests processed: {counter.get_value()}")

# Timing operations
timer = cba.Timer("database_query")
timer.start()
time.sleep(0.1)  # Simulate database query
timer.stop()
print(f"Query took: {timer.get_duration_ms():.2f}ms")
```

### Advanced Monitoring

```python
import cba
import time
import threading
from typing import Dict, Any
from dataclasses import dataclass
from datetime import datetime, timedelta

@dataclass
class MetricSnapshot:
    timestamp: datetime
    cpu_usage: float
    memory_usage_mb: float
    disk_usage_gb: float
    active_connections: int
    requests_per_second: float

class ApplicationMonitor:
    def __init__(self, collection_interval: int = 60):
        self.monitor = cba.SystemMonitor()
        self.logger = cba.Logger("AppMonitor")
        self.collection_interval = collection_interval
        
        # Performance counters
        self.request_counter = cba.PerformanceCounter("total_requests")
        self.error_counter = cba.PerformanceCounter("total_errors")
        self.connection_gauge = cba.Gauge("active_connections")
        
        # Metrics storage
        self.metrics_history = []
        self.max_history_size = 1440  # 24 hours at 1-minute intervals
        
        # Monitoring thread
        self.monitoring_thread = None
        self.stop_monitoring = threading.Event()
        
    def start_monitoring(self):
        """Start background monitoring."""
        if self.monitoring_thread and self.monitoring_thread.is_alive():
            self.logger.warning("Monitoring already started")
            return
        
        self.stop_monitoring.clear()
        self.monitoring_thread = threading.Thread(target=self._monitoring_loop)
        self.monitoring_thread.daemon = True
        self.monitoring_thread.start()
        
        self.logger.info(f"Started monitoring with {self.collection_interval}s interval")
    
    def stop_monitoring(self):
        """Stop background monitoring."""
        if self.monitoring_thread and self.monitoring_thread.is_alive():
            self.stop_monitoring.set()
            self.monitoring_thread.join(timeout=5)
            self.logger.info("Stopped monitoring")
    
    def _monitoring_loop(self):
        """Background monitoring loop."""
        while not self.stop_monitoring.wait(self.collection_interval):
            try:
                snapshot = self._collect_metrics()
                self._store_snapshot(snapshot)
                self._check_alerts(snapshot)
            except Exception as e:
                self.logger.error(f"Error in monitoring loop: {str(e)}")
    
    def _collect_metrics(self) -> MetricSnapshot:
        """Collect current system metrics."""
        cpu_usage = self.monitor.get_cpu_usage()
        memory_info = self.monitor.get_memory_info()
        disk_info = self.monitor.get_disk_info()
        
        # Calculate requests per second
        current_requests = self.request_counter.get_value()
        if len(self.metrics_history) > 0:
            last_snapshot = self.metrics_history[-1]
            time_diff = (datetime.now() - last_snapshot.timestamp).total_seconds()
            request_diff = current_requests - (last_snapshot.requests_per_second * time_diff)
            requests_per_second = request_diff / self.collection_interval if time_diff > 0 else 0
        else:
            requests_per_second = 0
        
        return MetricSnapshot(
            timestamp=datetime.now(),
            cpu_usage=cpu_usage,
            memory_usage_mb=memory_info.used_mb,
            disk_usage_gb=disk_info.used_gb,
            active_connections=self.connection_gauge.get_value(),
            requests_per_second=requests_per_second
        )
    
    def _store_snapshot(self, snapshot: MetricSnapshot):
        """Store metrics snapshot."""
        self.metrics_history.append(snapshot)
        
        # Limit history size
        if len(self.metrics_history) > self.max_history_size:
            self.metrics_history.pop(0)
    
    def _check_alerts(self, snapshot: MetricSnapshot):
        """Check for alert conditions."""
        # CPU usage alert
        if snapshot.cpu_usage > 80:
            self.logger.warning(f"High CPU usage: {snapshot.cpu_usage:.1f}%")
        
        # Memory usage alert
        if snapshot.memory_usage_mb > 1024:  # 1GB
            self.logger.warning(f"High memory usage: {snapshot.memory_usage_mb:.1f}MB")
        
        # Error rate alert
        total_requests = self.request_counter.get_value()
        total_errors = self.error_counter.get_value()
        if total_requests > 0:
            error_rate = (total_errors / total_requests) * 100
            if error_rate > 5:  # 5% error rate
                self.logger.error(f"High error rate: {error_rate:.1f}%")
    
    def record_request(self, success: bool = True):
        """Record a request."""
        self.request_counter.increment()
        if not success:
            self.error_counter.increment()
    
    def record_connection(self, connected: bool):
        """Record connection change."""
        if connected:
            self.connection_gauge.increment()
        else:
            self.connection_gauge.decrement()
    
    def get_current_metrics(self) -> Dict[str, Any]:
        """Get current metrics summary."""
        if not self.metrics_history:
            return {}
        
        latest = self.metrics_history[-1]
        total_requests = self.request_counter.get_value()
        total_errors = self.error_counter.get_value()
        error_rate = (total_errors / total_requests * 100) if total_requests > 0 else 0
        
        return {
            "timestamp": latest.timestamp.isoformat(),
            "cpu_usage_percent": latest.cpu_usage,
            "memory_usage_mb": latest.memory_usage_mb,
            "disk_usage_gb": latest.disk_usage_gb,
            "active_connections": latest.active_connections,
            "requests_per_second": latest.requests_per_second,
            "total_requests": total_requests,
            "total_errors": total_errors,
            "error_rate_percent": error_rate
        }
    
    def get_metrics_history(self, hours: int = 1) -> list[MetricSnapshot]:
        """Get metrics history for the specified number of hours."""
        cutoff_time = datetime.now() - timedelta(hours=hours)
        return [m for m in self.metrics_history if m.timestamp >= cutoff_time]
    
    def export_metrics_prometheus(self) -> str:
        """Export metrics in Prometheus format."""
        if not self.metrics_history:
            return ""
        
        latest = self.metrics_history[-1]
        timestamp = int(latest.timestamp.timestamp() * 1000)
        
        metrics = [
            f"# HELP cpu_usage_percent Current CPU usage percentage",
            f"# TYPE cpu_usage_percent gauge",
            f"cpu_usage_percent {latest.cpu_usage} {timestamp}",
            f"",
            f"# HELP memory_usage_mb Current memory usage in MB",
            f"# TYPE memory_usage_mb gauge",
            f"memory_usage_mb {latest.memory_usage_mb} {timestamp}",
            f"",
            f"# HELP active_connections Current number of active connections",
            f"# TYPE active_connections gauge",
            f"active_connections {latest.active_connections} {timestamp}",
            f"",
            f"# HELP requests_per_second Current requests per second",
            f"# TYPE requests_per_second gauge",
            f"requests_per_second {latest.requests_per_second} {timestamp}",
            f"",
            f"# HELP total_requests_total Total number of requests processed",
            f"# TYPE total_requests_total counter",
            f"total_requests_total {self.request_counter.get_value()} {timestamp}",
            f"",
            f"# HELP total_errors_total Total number of errors",
            f"# TYPE total_errors_total counter",
            f"total_errors_total {self.error_counter.get_value()} {timestamp}"
        ]
        
        return "\n".join(metrics)

# Usage
app_monitor = ApplicationMonitor(collection_interval=30)  # 30 seconds
app_monitor.start_monitoring()

# Simulate application activity
for i in range(10):
    app_monitor.record_request(success=(i % 10 != 0))  # 10% error rate
    app_monitor.record_connection(True)
    time.sleep(1)

# Get current metrics
current_metrics = app_monitor.get_current_metrics()
print(f"Current metrics: {current_metrics}")

# Export for Prometheus
prometheus_metrics = app_monitor.export_metrics_prometheus()
print(f"Prometheus metrics:\n{prometheus_metrics}")

# Stop monitoring
app_monitor.stop_monitoring()
```

## Event System

### Basic Event Handling

```python
import cba
from typing import Any, Dict

# Create event bus
event_bus = cba.EventBus()

# Define event handler
def user_login_handler(event_data: Dict[str, Any]):
    user_id = event_data.get('user_id')
    ip_address = event_data.get('ip_address')
    print(f"User {user_id} logged in from {ip_address}")

# Subscribe to events
subscription_id = event_bus.subscribe("user.login", user_login_handler)

# Publish event
event_bus.publish("user.login", {
    "user_id": "user123",
    "ip_address": "192.168.1.100",
    "timestamp": "2024-01-15T10:30:00Z"
})

# Unsubscribe
event_bus.unsubscribe(subscription_id)
```

### Advanced Event System

```python
import cba
import json
import asyncio
from typing import Any, Dict, Callable, Optional
from dataclasses import dataclass
from datetime import datetime
from enum import Enum

class EventPriority(Enum):
    LOW = 1
    NORMAL = 5
    HIGH = 10
    CRITICAL = 20

@dataclass
class Event:
    type: str
    data: Dict[str, Any]
    timestamp: datetime
    priority: EventPriority = EventPriority.NORMAL
    source: Optional[str] = None
    correlation_id: Optional[str] = None

class EventManager:
    def __init__(self):
        self.event_bus = cba.EventBus()
        self.logger = cba.Logger("EventManager")
        self.subscriptions = {}
        self.event_history = []
        self.max_history = 1000
        
    def subscribe(self, event_type: str, handler: Callable[[Event], None], 
                 priority: int = 5, async_handler: bool = True) -> str:
        """Subscribe to events with advanced options."""
        
        def wrapper(event_data: Dict[str, Any]):
            try:
                event = Event(
                    type=event_type,
                    data=event_data.get('data', {}),
                    timestamp=datetime.fromisoformat(event_data.get('timestamp', datetime.now().isoformat())),
                    priority=EventPriority(event_data.get('priority', EventPriority.NORMAL.value)),
                    source=event_data.get('source'),
                    correlation_id=event_data.get('correlation_id')
                )
                
                # Store in history
                self._store_event(event)
                
                # Call handler
                if async_handler:
                    asyncio.create_task(self._async_handler_wrapper(handler, event))
                else:
                    handler(event)
                    
            except Exception as e:
                self.logger.error(f"Error processing event {event_type}: {str(e)}")
        
        subscription_id = self.event_bus.subscribe(event_type, wrapper, priority=priority)
        self.subscriptions[subscription_id] = {
            'event_type': event_type,
            'handler': handler,
            'priority': priority,
            'async': async_handler
        }
        
        self.logger.debug(f"Subscribed to {event_type} with priority {priority}")
        return subscription_id
    
    async def _async_handler_wrapper(self, handler: Callable[[Event], None], event: Event):
        """Wrapper for async event handlers."""
        try:
            if asyncio.iscoroutinefunction(handler):
                await handler(event)
            else:
                handler(event)
        except Exception as e:
            self.logger.error(f"Error in async event handler: {str(e)}")
    
    def publish(self, event_type: str, data: Dict[str, Any], 
               priority: EventPriority = EventPriority.NORMAL,
               source: Optional[str] = None, correlation_id: Optional[str] = None):
        """Publish event with metadata."""
        
        event_data = {
            'data': data,
            'timestamp': datetime.now().isoformat(),
            'priority': priority.value,
            'source': source,
            'correlation_id': correlation_id
        }
        
        self.event_bus.publish(event_type, event_data)
        self.logger.debug(f"Published event {event_type} with priority {priority.name}")
    
    def _store_event(self, event: Event):
        """Store event in history."""
        self.event_history.append(event)
        
        # Limit history size
        if len(self.event_history) > self.max_history:
            self.event_history.pop(0)
    
    def get_event_history(self, event_type: Optional[str] = None, 
                         limit: int = 100) -> list[Event]:
        """Get event history with optional filtering."""
        events = self.event_history
        
        if event_type:
            events = [e for e in events if e.type == event_type]
        
        return events[-limit:] if limit else events
    
    def get_subscription_info(self) -> Dict[str, Any]:
        """Get information about current subscriptions."""
        return {
            'total_subscriptions': len(self.subscriptions),
            'subscriptions': {
                sub_id: {
                    'event_type': info['event_type'],
                    'priority': info['priority'],
                    'async': info['async']
                }
                for sub_id, info in self.subscriptions.items()
            }
        }
    
    def unsubscribe(self, subscription_id: str):
        """Unsubscribe from events."""
        if subscription_id in self.subscriptions:
            event_type = self.subscriptions[subscription_id]['event_type']
            self.event_bus.unsubscribe(subscription_id)
            del self.subscriptions[subscription_id]
            self.logger.debug(f"Unsubscribed from {event_type}")
        else:
            self.logger.warning(f"Subscription not found: {subscription_id}")

# Example usage
class UserService:
    def __init__(self, event_manager: EventManager):
        self.event_manager = event_manager
        self.logger = cba.Logger("UserService")
        self.setup_event_handlers()
    
    def setup_event_handlers(self):
        """Setup event handlers for user-related events."""
        
        # High priority handler for user login
        self.event_manager.subscribe(
            "user.login",
            self.handle_user_login,
            priority=EventPriority.HIGH.value
        )
        
        # Normal priority handler for user registration
        self.event_manager.subscribe(
            "user.register",
            self.handle_user_registration,
            priority=EventPriority.NORMAL.value
        )
        
        # Critical priority handler for security events
        self.event_manager.subscribe(
            "security.breach",
            self.handle_security_breach,
            priority=EventPriority.CRITICAL.value,
            async_handler=False  # Synchronous for critical events
        )
    
    def handle_user_login(self, event: Event):
        """Handle user login event."""
        user_id = event.data.get('user_id')
        ip_address = event.data.get('ip_address')
        
        self.logger.info(f"User {user_id} logged in from {ip_address}")
        
        # Update last login time
        # Send welcome notification
        # Log security event
        
        # Publish follow-up event
        self.event_manager.publish(
            "user.activity",
            {
                'user_id': user_id,
                'activity': 'login',
                'ip_address': ip_address
            },
            source="UserService",
            correlation_id=event.correlation_id
        )
    
    def handle_user_registration(self, event: Event):
        """Handle user registration event."""
        user_data = event.data
        
        self.logger.info(f"New user registered: {user_data.get('email')}")
        
        # Send welcome email
        # Create user profile
        # Setup default preferences
        
        # Publish welcome event
        self.event_manager.publish(
            "user.welcome",
            {
                'user_id': user_data.get('user_id'),
                'email': user_data.get('email')
            },
            source="UserService",
            correlation_id=event.correlation_id
        )
    
    def handle_security_breach(self, event: Event):
        """Handle security breach event (synchronous, critical)."""
        breach_data = event.data
        
        self.logger.critical(f"Security breach detected: {breach_data.get('type')}")
        
        # Immediate actions:
        # - Lock affected accounts
        # - Send alerts to security team
        # - Log detailed information
        # - Trigger automated response

# Usage example
event_manager = EventManager()
user_service = UserService(event_manager)

# Simulate events
event_manager.publish(
    "user.login",
    {
        'user_id': 'user123',
        'ip_address': '192.168.1.100',
        'user_agent': 'Mozilla/5.0...'
    },
    priority=EventPriority.HIGH,
    source="AuthService",
    correlation_id="req-12345"
)

event_manager.publish(
    "user.register",
    {
        'user_id': 'user456',
        'email': 'newuser@example.com',
        'registration_source': 'web'
    },
    source="RegistrationService",
    correlation_id="req-12346"
)

# Get event history
recent_events = event_manager.get_event_history(limit=10)
for event in recent_events:
    print(f"{event.timestamp}: {event.type} - {event.data}")

# Get subscription info
sub_info = event_manager.get_subscription_info()
print(f"Active subscriptions: {sub_info['total_subscriptions']}")
```

## Performance Considerations

### Memory Management

```python
import cba
import gc
import psutil
import os

class MemoryManager:
    def __init__(self):
        self.logger = cba.Logger("MemoryManager")
        self.process = psutil.Process(os.getpid())
        
    def get_memory_usage(self) -> dict:
        """Get current memory usage statistics."""
        memory_info = self.process.memory_info()
        return {
            'rss_mb': memory_info.rss / 1024 / 1024,  # Resident Set Size
            'vms_mb': memory_info.vms / 1024 / 1024,  # Virtual Memory Size
            'percent': self.process.memory_percent()
        }
    
    def force_garbage_collection(self):
        """Force garbage collection and log results."""
        before = self.get_memory_usage()
        
        # Force garbage collection
        collected = gc.collect()
        
        after = self.get_memory_usage()
        freed_mb = before['rss_mb'] - after['rss_mb']
        
        self.logger.info(f"GC collected {collected} objects, freed {freed_mb:.2f}MB")
        
    def monitor_memory_threshold(self, threshold_mb: float = 500):
        """Monitor memory usage and trigger GC if threshold exceeded."""
        usage = self.get_memory_usage()
        
        if usage['rss_mb'] > threshold_mb:
            self.logger.warning(f"Memory usage high: {usage['rss_mb']:.2f}MB")
            self.force_garbage_collection()

# Usage
memory_manager = MemoryManager()

# Monitor memory during operations
for i in range(1000):
    # Simulate memory-intensive operations
    large_data = [j for j in range(10000)]
    
    if i % 100 == 0:
        memory_manager.monitor_memory_threshold(100)  # 100MB threshold
        usage = memory_manager.get_memory_usage()
        print(f"Iteration {i}: Memory usage {usage['rss_mb']:.2f}MB")
```

### Connection Pooling

```python
import cba
import threading
from queue import Queue, Empty
from contextlib import contextmanager

class ConnectionPool:
    def __init__(self, create_connection_func, max_connections: int = 10):
        self.create_connection = create_connection_func
        self.max_connections = max_connections
        self.pool = Queue(maxsize=max_connections)
        self.active_connections = 0
        self.lock = threading.Lock()
        self.logger = cba.Logger("ConnectionPool")
        
        # Pre-populate pool
        for _ in range(max_connections // 2):
            try:
                conn = self.create_connection()
                self.pool.put(conn)
            except Exception as e:
                self.logger.error(f"Failed to create initial connection: {str(e)}")
    
    @contextmanager
    def get_connection(self, timeout: float = 5.0):
        """Get connection from pool with context manager."""
        connection = None
        try:
            # Try to get existing connection
            try:
                connection = self.pool.get(timeout=timeout)
            except Empty:
                # Create new connection if pool is empty and under limit
                with self.lock:
                    if self.active_connections < self.max_connections:
                        connection = self.create_connection()
                        self.active_connections += 1
                    else:
                        raise Exception("Connection pool exhausted")
            
            yield connection
            
        except Exception as e:
            self.logger.error(f"Connection error: {str(e)}")
            raise
        finally:
            # Return connection to pool
            if connection:
                try:
                    self.pool.put(connection, timeout=1.0)
                except:
                    # Pool is full, close connection
                    if hasattr(connection, 'close'):
                        connection.close()
                    with self.lock:
                        self.active_connections -= 1

# Example with HTTP client pool
def create_http_client():
    client = cba.HttpClient()
    client.set_timeout(30)
    return client

http_pool = ConnectionPool(create_http_client, max_connections=5)

# Usage
with http_pool.get_connection() as client:
    response = client.get("https://api.example.com/data")
    print(f"Response: {response.status_code}")
```

## Troubleshooting

### Common Issues

1. **Import Errors**
   ```python
   # If you get import errors, check installation
   import sys
   print(sys.path)
   
   # Reinstall if necessary
   # pip uninstall corebaseapplication
   # pip install corebaseapplication
   ```

2. **Memory Leaks**
   ```python
   # Always call shutdown to clean up resources
   import atexit
   atexit.register(cba.shutdown)
   
   # Or use context manager
   with cba.CBAContext():
       # Your application code
       pass
   ```

3. **Thread Safety**
   ```python
   # CBA objects are thread-safe, but use locks for shared state
   import threading
   
   lock = threading.Lock()
   shared_counter = 0
   
   def thread_safe_increment():
       global shared_counter
       with lock:
           shared_counter += 1
   ```

4. **Performance Issues**
   ```python
   # Enable debug logging to identify bottlenecks
   logger = cba.Logger("Debug")
   logger.set_level(cba.LogLevel.DEBUG)
   
   # Use async operations for I/O bound tasks
   import asyncio
   
   async def async_operations():
       tasks = []
       for url in urls:
           task = client.get_async(url)
           tasks.append(task)
       
       results = await asyncio.gather(*tasks)
       return results
   ```

### Debug Mode

```python
import cba
import os

# Enable debug mode
os.environ['CBA_DEBUG'] = '1'
cba.initialize()

# Debug logging
logger = cba.Logger("Debug")
logger.set_level(cba.LogLevel.DEBUG)

# Enable verbose HTTP logging
client = cba.HttpClient()
client.set_debug_mode(True)

# Monitor performance
monitor = cba.SystemMonitor()
monitor.enable_detailed_logging(True)
```

### Getting Help

- **Documentation**: [https://cba-docs.example.com](https://cba-docs.example.com)
- **GitHub Issues**: [https://github.com/seregonwar/CoreBaseApplication-CBA/issues](https://github.com/seregonwar/CoreBaseApplication-CBA/issues)
- **Community Forum**: work-in-progress
- **Email Support**: seregonwar@gmail.com

---

*This documentation covers the essential aspects of using CBA Python bindings. For more advanced topics and examples, refer to the [Advanced Usage Examples](../examples/advanced-usage.md) and [API Reference](../api/reference.md).*