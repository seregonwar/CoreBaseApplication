# Quick Start Guide

Get up and running with CoreBaseApplication in minutes.

## Prerequisites

- C++17 or later
- CMake 3.15+
- vcpkg (for dependency management)
- Visual Studio 2019+ or GCC 9+

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/your-username/CoreBaseApplication-CBA.git
cd CoreBaseApplication-CBA
```

### 2. Install Dependencies

```bash
# Using vcpkg
vcpkg install nlohmann-json gtest
```

### 3. Build the Project

```bash
# Windows
build.bat

# Linux/macOS
./build.sh
```

## Your First Application

Create a simple application using CoreBaseApplication:

```cpp
#include "core/CoreAPI.h"
#include <iostream>

int main() {
    // Initialize the framework
    Core::CoreAPI::ConfigParams params;
    params.configFilePath = "config.json";
    params.applicationName = "MyFirstApp";
    params.applicationVersion = "1.0.0";
    
    if (!Core::CoreAPI::initialize(params)) {
        std::cerr << "Failed to initialize CoreAPI" << std::endl;
        return 1;
    }
    
    // Get the core instance
    auto& core = Core::CoreAPI::getInstance();
    
    // Your application logic here
    std::cout << "Hello from CoreBaseApplication!" << std::endl;
    
    // Shutdown gracefully
    Core::CoreAPI::shutdown();
    
    return 0;
}
```

## Basic Configuration

Create a `config.json` file:

```json
{
  "application": {
    "name": "MyFirstApp",
    "version": "1.0.0",
    "logLevel": "INFO"
  },
  "logging": {
    "console": true,
    "file": {
      "enabled": true,
      "path": "logs/app.log"
    }
  }
}
```

## Next Steps

- [Learn about the architecture](../architecture/overview.md)
- [Explore core modules](../modules/core-api.md)
- [See more examples](../examples/basic-app.md)
- [Read best practices](../development/best-practices.md)