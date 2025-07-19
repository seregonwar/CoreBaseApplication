# Development Setup

This guide will help you set up your development environment for CoreBaseApplication (CBA) and get you started with building, testing, and contributing to the project.

## Prerequisites

### System Requirements

- **Operating System**: Windows 10/11, Linux (Ubuntu 20.04+), macOS 10.15+
- **Memory**: Minimum 8GB RAM (16GB recommended)
- **Storage**: At least 5GB free space
- **Network**: Internet connection for downloading dependencies

### Required Tools

#### Windows

```powershell
# Install Visual Studio 2022 with C++ workload
# Download from: https://visualstudio.microsoft.com/downloads/

# Install Git
winget install Git.Git

# Install CMake
winget install Kitware.CMake

# Install vcpkg (package manager)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

#### Linux (Ubuntu/Debian)

```bash
# Update package list
sudo apt update

# Install build essentials
sudo apt install -y build-essential cmake git

# Install additional dependencies
sudo apt install -y \
    libssl-dev \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    libboost-all-dev \
    python3-dev \
    openjdk-11-jdk

# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
```

#### macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install development tools
brew install cmake git openssl curl jsoncpp boost python openjdk

# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
```

### Optional Tools

- **IDE**: Visual Studio Code, CLion, or Qt Creator
- **Debugger**: GDB (Linux/macOS), Visual Studio Debugger (Windows)
- **Profiler**: Valgrind (Linux/macOS), Intel VTune, or Visual Studio Profiler
- **Static Analysis**: Clang Static Analyzer, PVS-Studio, or SonarQube

## Project Setup

### Clone the Repository

```bash
# Clone the main repository
git clone https://github.com/seregonwar/CoreBaseApplication-CBA.git
cd CoreBaseApplication-CBA

# Initialize submodules
git submodule update --init --recursive
```

### Configure Build Environment

#### Using CMake Presets (Recommended)

```bash
# List available presets
cmake --list-presets

# Configure for development (Debug build)
cmake --preset dev

# Configure for release
cmake --preset release

# Configure with specific options
cmake --preset dev -DCBA_ENABLE_TESTS=ON -DCBA_ENABLE_PYTHON_BINDINGS=ON
```

#### Manual Configuration

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCBA_ENABLE_TESTS=ON \
    -DCBA_ENABLE_PYTHON_BINDINGS=ON \
    -DCBA_ENABLE_JAVA_BINDINGS=ON \
    -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Build the Project

```bash
# Build all targets
cmake --build . --config Debug

# Build specific target
cmake --build . --target CoreAPI --config Debug

# Build with parallel jobs
cmake --build . --config Debug --parallel 8

# Build and install
cmake --build . --config Debug --target install
```

### Run Tests

```bash
# Run all tests
ctest --output-on-failure

# Run specific test suite
ctest -R "ConfigManager*" --output-on-failure

# Run tests with verbose output
ctest --verbose

# Run tests in parallel
ctest --parallel 4
```

## Development Workflow

### Branch Strategy

```bash
# Create feature branch
git checkout -b feature/new-networking-feature

# Create bugfix branch
git checkout -b bugfix/fix-memory-leak

# Create hotfix branch
git checkout -b hotfix/critical-security-fix
```

### Code Style and Formatting

#### Install Code Formatters

```bash
# Install clang-format
# Ubuntu/Debian
sudo apt install clang-format

# macOS
brew install clang-format

# Windows (via LLVM)
# Download from: https://llvm.org/builds/
```

#### Format Code

```bash
# Format all source files
find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Format specific file
clang-format -i src/core/CoreAPI.cpp

# Check formatting without modifying
clang-format --dry-run --Werror src/core/CoreAPI.cpp
```

### Pre-commit Hooks

```bash
# Install pre-commit
pip install pre-commit

# Install hooks
pre-commit install

# Run hooks manually
pre-commit run --all-files
```

### Static Analysis

```bash
# Run clang-tidy
clang-tidy src/core/*.cpp -- -Isrc/include

# Run with CMake integration
cmake -DCMAKE_CXX_CLANG_TIDY=clang-tidy ..

# Run cppcheck
cppcheck --enable=all --std=c++17 src/
```

## IDE Configuration

### Visual Studio Code

#### Required Extensions

```json
{
  "recommendations": [
    "ms-vscode.cpptools",
    "ms-vscode.cmake-tools",
    "ms-vscode.cpptools-extension-pack",
    "llvm-vs-code-extensions.vscode-clangd",
    "ms-python.python",
    "redhat.java"
  ]
}
```

#### Settings Configuration

```json
{
  "cmake.configureOnOpen": true,
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
  "C_Cpp.default.cppStandard": "c++17",
  "files.associations": {
    "*.h": "cpp",
    "*.cpp": "cpp"
  },
  "editor.formatOnSave": true,
  "C_Cpp.clang_format_style": "file"
}
```

### CLion

#### Project Configuration

1. Open CLion and select "Open or Import"
2. Navigate to the project directory and select `CMakeLists.txt`
3. Configure CMake settings:
   - Build Type: Debug/Release
   - CMake options: `-DCBA_ENABLE_TESTS=ON`
   - Toolchain: System default or custom

#### Code Style

1. Go to Settings → Editor → Code Style → C/C++
2. Import the `.clang-format` file from the project root
3. Enable "Reformat code" in commit options

### Visual Studio (Windows)

#### Project Setup

1. Open Visual Studio 2022
2. Select "Open a local folder"
3. Navigate to the project directory
4. Visual Studio will automatically detect CMake configuration

#### Configuration

```json
// CMakeSettings.json
{
  "configurations": [
    {
      "name": "x64-Debug",
      "generator": "Ninja",
      "configurationType": "Debug",
      "buildRoot": "${projectDir}\\build\\${name}",
      "installRoot": "${projectDir}\\install\\${name}",
      "cmakeCommandArgs": "-DCBA_ENABLE_TESTS=ON",
      "buildCommandArgs": "",
      "ctestCommandArgs": "",
      "inheritEnvironments": [ "msvc_x64_x64" ]
    }
  ]
}
```

## Debugging

### GDB (Linux/macOS)

```bash
# Build with debug symbols
cmake --build . --config Debug

# Run with GDB
gdb ./build/bin/cba_gui

# Common GDB commands
(gdb) break main
(gdb) run
(gdb) step
(gdb) next
(gdb) print variable_name
(gdb) backtrace
(gdb) continue
```

### Visual Studio Debugger (Windows)

1. Set breakpoints by clicking in the left margin
2. Press F5 to start debugging
3. Use F10 (step over) and F11 (step into)
4. View variables in the Locals/Autos windows
5. Use the Call Stack window to navigate frames

### Memory Debugging

#### Valgrind (Linux/macOS)

```bash
# Install Valgrind
sudo apt install valgrind  # Ubuntu/Debian
brew install valgrind      # macOS

# Run memory check
valgrind --tool=memcheck --leak-check=full ./build/bin/cba_gui

# Run with suppressions
valgrind --tool=memcheck --suppressions=valgrind.supp ./build/bin/cba_gui
```

#### AddressSanitizer

```bash
# Build with AddressSanitizer
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
cmake --build .

# Run the application
./build/bin/cba_gui
```

## Testing

### Unit Tests

```bash
# Run all unit tests
ctest --output-on-failure

# Run specific test
ctest -R "ConfigManagerTest" --verbose

# Run tests with coverage
cmake -DCBA_ENABLE_COVERAGE=ON ..
cmake --build .
ctest
gcov src/core/*.cpp
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

### Integration Tests

```bash
# Run integration tests
ctest -L integration

# Run performance tests
ctest -L performance
```

### Writing Tests

```cpp
// Example unit test
#include <gtest/gtest.h>
#include "core/CoreAPI.h"

class CoreAPITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }
    
    void TearDown() override {
        // Cleanup code
    }
};

TEST_F(CoreAPITest, InitializeReturnsTrue) {
    Core::CoreAPI api;
    EXPECT_TRUE(api.initialize());
}

TEST_F(CoreAPITest, GetVersionReturnsValidString) {
    Core::CoreAPI api;
    std::string version = api.getVersion();
    EXPECT_FALSE(version.empty());
    EXPECT_NE(version.find('.'), std::string::npos);
}
```

## Performance Profiling

### CPU Profiling

#### perf (Linux)

```bash
# Install perf
sudo apt install linux-tools-common linux-tools-generic

# Profile application
perf record -g ./build/bin/cba_gui
perf report

# Generate flame graph
perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg
```

#### Instruments (macOS)

```bash
# Profile with Instruments
instruments -t "Time Profiler" ./build/bin/cba_gui
```

### Memory Profiling

```bash
# Massif (Valgrind)
valgrind --tool=massif ./build/bin/cba_gui
ms_print massif.out.* > memory_profile.txt

# Heaptrack (Linux)
heaptrack ./build/bin/cba_gui
heaptrack_gui heaptrack.cba_gui.*
```

## Continuous Integration

### GitHub Actions

```yaml
# .github/workflows/ci.yml
name: CI

on: [push, pull_request]

jobs:
  build:
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
        build_type: [Debug, Release]
    
    runs-on: {% raw %}${{ matrix.os }}{% endraw %}
    
    steps:
    - uses: actions/checkout@v3
      with:
        submodules: recursive
    
    - name: Install dependencies (Ubuntu)
      if: matrix.os == 'ubuntu-latest'
      run: |
        sudo apt update
        sudo apt install -y build-essential cmake libssl-dev
    
    - name: Configure CMake
      run: |
        cmake -B build -DCMAKE_BUILD_TYPE={% raw %}${{ matrix.build_type }}{% endraw %} \
              -DCBA_ENABLE_TESTS=ON
    
    - name: Build
      run: cmake --build build --config {% raw %}${{ matrix.build_type }}{% endraw %}
    
    - name: Test
      run: |
        cd build
        ctest --output-on-failure --build-config {% raw %}${{ matrix.build_type }}{% endraw %}
```

## Troubleshooting

### Common Build Issues

#### Missing Dependencies

```bash
# Error: Could not find OpenSSL
# Solution: Install OpenSSL development packages
sudo apt install libssl-dev  # Ubuntu/Debian
brew install openssl         # macOS

# Error: Could not find Boost
# Solution: Install Boost development packages
sudo apt install libboost-all-dev  # Ubuntu/Debian
brew install boost                 # macOS
```

#### CMake Configuration Issues

```bash
# Clear CMake cache
rm -rf build/CMakeCache.txt build/CMakeFiles/

# Reconfigure with verbose output
cmake -B build --debug-output

# Check CMake variables
cmake -B build -LAH
```

#### Compilation Errors

```bash
# Build with verbose output
cmake --build build --verbose

# Build single target for faster iteration
cmake --build build --target CoreAPI

# Check compiler version
g++ --version
clang++ --version
```

### Performance Issues

```bash
# Build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..

# Enable link-time optimization
cmake -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON ..

# Use faster linker (Linux)
cmake -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" ..
```

### Test Failures

```bash
# Run tests with detailed output
ctest --verbose --output-on-failure

# Run specific failing test
ctest -R "FailingTestName" --verbose

# Run tests under debugger
gdb --args ./build/tests/ConfigManagerTest
```

## Contributing Guidelines

### Code Review Process

1. Create feature branch from `develop`
2. Implement changes with tests
3. Ensure all tests pass
4. Run static analysis tools
5. Create pull request
6. Address review feedback
7. Merge after approval

### Commit Message Format

```
type(scope): brief description

Detailed description of the change.

Fixes #123
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

### Documentation

- Update relevant documentation
- Add code comments for complex logic
- Update API documentation
- Add examples for new features

## See Also

- [Architecture Overview](../architecture/overview.md)
- [Best Practices](best-practices.md)
- [Performance Tuning](performance.md)
- [Security Guidelines](security.md)
- [API Reference](../api/core-api.md)