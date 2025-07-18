cmake_minimum_required(VERSION 3.20)
project(CoreBaseApplication)

# Imposta lo standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Imposta le variabili di output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Trova pacchetti richiesti da vcpkg
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Aggiunge dipendenze come sottodirectory
add_subdirectory(third_party/gtest)
add_subdirectory(third_party/pybind11)

# Trova Python prima di definire la libreria core
find_package(Python 3.12 COMPONENTS Interpreter Development REQUIRED)

# Aggiunge le directory di inclusione
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/core)

# Libreria 'core'
add_library(core STATIC
    src/core/CoreAPI.cpp
    src/core/CoreClass/ConfigManager.cpp
    src/core/CoreClass/Core.cpp
    src/core/CoreClass/ErrorHandler.cpp
    src/core/CoreClass/IPCManager.cpp
    src/core/CoreClass/ModuleManager.cpp
    src/core/CoreClass/ResourceManager.cpp
    src/core/Monitoring/SystemMonitor.cpp
    src/core/bindings/python/python_bindings.cpp
)


target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pybind11/include
    ${Python_INCLUDE_DIRS}
)
target_link_libraries(core PRIVATE nlohmann_json::nlohmann_json GTest::gtest GTest::gtest_main pybind11::headers ${Python_LIBRARIES})



# Python Bindings
pybind11_add_module(python_bindings
    src/core/bindings/python/python_bindings.cpp
)
target_link_libraries(python_bindings PRIVATE core)

# Test per i binding Python
add_executable(PythonBindingsTest
    src/tests/python_bindings_test.cpp
)
target_link_libraries(PythonBindingsTest PRIVATE core GTest::gtest GTest::gtest_main nlohmann_json::nlohmann_json pybind11::embed)

# Abilita i test
enable_testing()

# Aggiunge i test di GTest
include(GoogleTest)
gtest_discover_tests(PythonBindingsTest)

# Test per ConfigManager
add_executable(ConfigManagerTest
    src/core/tests/ConfigManager_test.cpp
)
target_link_libraries(ConfigManagerTest PRIVATE core GTest::gtest GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(ConfigManagerTest)

# Moduli
add_subdirectory(src/modules/SampleModule)

# Applicazione GUI
add_subdirectory(src/apps/cba_gui)

# Definisce USE_PYTHON_BINDINGS per abilitare i binding Python
add_definitions(-DUSE_PYTHON_BINDINGS)

# Assicura che la definizione sia applicata alla libreria core
target_compile_definitions(core PRIVATE USE_PYTHON_BINDINGS)