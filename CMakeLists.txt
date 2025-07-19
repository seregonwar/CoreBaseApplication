cmake_minimum_required(VERSION 3.20)
project(CoreBaseApplication)

# Imposta lo standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Opzioni di build per target platform
option(BUILD_FOR_WINDOWS "Build for Windows" OFF)
option(BUILD_FOR_MACOS "Build for macOS" OFF)
option(BUILD_FOR_LINUX "Build for Linux" OFF)
option(BUILD_FOR_IOS "Build for iOS" OFF)
option(BUILD_FOR_ANDROID "Build for Android" OFF)

# Auto-detect platform se nessuna flag è specificata
if(NOT BUILD_FOR_WINDOWS AND NOT BUILD_FOR_MACOS AND NOT BUILD_FOR_LINUX AND NOT BUILD_FOR_IOS AND NOT BUILD_FOR_ANDROID)
    if(WIN32)
        set(BUILD_FOR_WINDOWS ON)
    elseif(APPLE)
        if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
            set(BUILD_FOR_IOS ON)
        else()
            set(BUILD_FOR_MACOS ON)
        endif()
    elseif(UNIX AND NOT APPLE)
        if(CMAKE_SYSTEM_NAME STREQUAL "Android")
            set(BUILD_FOR_ANDROID ON)
        else()
            set(BUILD_FOR_LINUX ON)
        endif()
    endif()
endif()

# Configurazioni specifiche per piattaforma
if(BUILD_FOR_WINDOWS)
    message(STATUS "Building for Windows")
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(BUILD_FOR_MACOS)
    message(STATUS "Building for macOS")
    add_compile_definitions(PLATFORM_MACOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment version")
elseif(BUILD_FOR_LINUX)
    message(STATUS "Building for Linux")
    add_compile_definitions(PLATFORM_LINUX)
elseif(BUILD_FOR_IOS)
    message(STATUS "Building for iOS")
    add_compile_definitions(PLATFORM_IOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum iOS deployment version")
elseif(BUILD_FOR_ANDROID)
    message(STATUS "Building for Android")
    add_compile_definitions(PLATFORM_ANDROID)
endif()

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

# Trova JNI per i Java bindings (opzionale)
find_package(JNI QUIET)
if(JNI_FOUND)
    message(STATUS "JNI found, enabling Java bindings")
    set(ENABLE_JAVA_BINDINGS ON)
else()
    message(STATUS "JNI not found, Java bindings disabled")
    set(ENABLE_JAVA_BINDINGS OFF)
endif()

# Aggiunge le directory di inclusione
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/core)

# Libreria 'core'
set(CORE_SOURCES
    src/core/CoreAPI.cpp
    src/core/CoreClass/ConfigManager.cpp
    src/core/CoreClass/Core.cpp
    src/core/CoreClass/ErrorHandler.cpp
    src/core/CoreClass/IPCManager.cpp
    src/core/CoreClass/ModuleManager.cpp
    src/core/CoreClass/ResourceManager.cpp
    src/core/Monitoring/SystemMonitor.cpp
    src/core/NetworkManager.cpp
    src/core/bindings/python/python_bindings.cpp
)

# Aggiungi Java bindings se disponibili
if(ENABLE_JAVA_BINDINGS)
    list(APPEND CORE_SOURCES src/core/bindings/java/JavaBindings.cpp)
endif()

add_library(core STATIC ${CORE_SOURCES})


target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pybind11/include
    ${Python_INCLUDE_DIRS}
)

# Aggiungi JNI include directories se disponibili
if(ENABLE_JAVA_BINDINGS)
    target_include_directories(core PUBLIC ${JNI_INCLUDE_DIRS})
    target_compile_definitions(core PRIVATE ENABLE_JAVA_BINDINGS)
endif()

target_link_libraries(core PRIVATE nlohmann_json::nlohmann_json GTest::gtest GTest::gtest_main pybind11::headers ${Python_LIBRARIES})

# Aggiungi librerie di sistema specifiche per piattaforma
if(BUILD_FOR_WINDOWS OR WIN32)
    target_link_libraries(core PRIVATE ws2_32 psapi pdh)
elseif(BUILD_FOR_MACOS OR (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS"))
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(CORE_SERVICES CoreServices)
    find_library(SYSTEM_CONFIGURATION SystemConfiguration)
    find_library(IOKIT IOKit)
    target_link_libraries(core PRIVATE ${CORE_FOUNDATION} ${CORE_SERVICES} ${SYSTEM_CONFIGURATION} ${IOKIT})
elseif(BUILD_FOR_LINUX OR (UNIX AND NOT APPLE))
    target_link_libraries(core PRIVATE pthread dl)
    # Aggiungi librerie Linux specifiche se necessarie
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SYSTEMD QUIET libsystemd)
        if(SYSTEMD_FOUND)
            target_link_libraries(core PRIVATE ${SYSTEMD_LIBRARIES})
            target_include_directories(core PRIVATE ${SYSTEMD_INCLUDE_DIRS})
        endif()
    endif()
elseif(BUILD_FOR_IOS)
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(FOUNDATION Foundation)
    find_library(UIKIT UIKit)
    target_link_libraries(core PRIVATE ${CORE_FOUNDATION} ${FOUNDATION} ${UIKIT})
elseif(BUILD_FOR_ANDROID)
    target_link_libraries(core PRIVATE log android)
endif()



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

# Test per SystemMonitor
add_executable(SystemMonitorTest
    src/core/tests/SystemMonitor_test.cpp
)
target_link_libraries(SystemMonitorTest PRIVATE core GTest::gtest GTest::gtest_main)
gtest_discover_tests(SystemMonitorTest)

# Test per NetworkManager
add_executable(NetworkManagerTest
    src/core/tests/NetworkManager_test.cpp
)
target_link_libraries(NetworkManagerTest PRIVATE core GTest::gtest GTest::gtest_main)
if(BUILD_FOR_WINDOWS OR WIN32)
    target_link_libraries(NetworkManagerTest PRIVATE ws2_32)
endif()
gtest_discover_tests(NetworkManagerTest)

# Test per JavaBindings (se abilitati)
if(ENABLE_JAVA_BINDINGS)
    add_executable(JavaBindingsTest
        src/core/tests/JavaBindings_test.cpp
    )
    target_link_libraries(JavaBindingsTest PRIVATE core GTest::gtest GTest::gtest_main nlohmann_json::nlohmann_json ${JNI_LIBRARIES})
    if(BUILD_FOR_WINDOWS OR WIN32)
        target_link_libraries(JavaBindingsTest PRIVATE ws2_32)
    endif()
    gtest_discover_tests(JavaBindingsTest)
endif()

# Node.js bindings (opzionale)
option(ENABLE_NODEJS_BINDINGS "Enable Node.js bindings" OFF)
if(ENABLE_NODEJS_BINDINGS)
    find_program(NODE_EXECUTABLE node)
    find_program(NPM_EXECUTABLE npm)
    
    if(NODE_EXECUTABLE AND NPM_EXECUTABLE)
        message(STATUS "Node.js found, enabling Node.js bindings")
        
        # Aggiungi target personalizzato per compilare i bindings Node.js
        add_custom_target(nodejs_bindings
            COMMAND ${NPM_EXECUTABLE} install
            COMMAND ${NPM_EXECUTABLE} run build
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/core/bindings/nodejs
            COMMENT "Building Node.js bindings"
        )
        
        # Test per Node.js bindings
        add_custom_target(nodejs_test
            COMMAND ${NPM_EXECUTABLE} test
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/core/bindings/nodejs
            DEPENDS nodejs_bindings
            COMMENT "Testing Node.js bindings"
        )
    else()
        message(STATUS "Node.js or npm not found, Node.js bindings disabled")
    endif()
endif()

# Moduli
add_subdirectory(src/modules/SampleModule)

# Applicazione GUI
add_subdirectory(src/apps/cba_gui)

# Definisce USE_PYTHON_BINDINGS per abilitare i binding Python
add_definitions(-DUSE_PYTHON_BINDINGS)

# Assicura che la definizione sia applicata alla libreria core
target_compile_definitions(core PRIVATE USE_PYTHON_BINDINGS)