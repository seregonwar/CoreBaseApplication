cmake_minimum_required(VERSION 3.15)
project(test_app VERSION 1.0.0)

# Imposta lo standard C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Imposta il percorso di SFML
if(WIN32)
    set(SFML_ROOT "C:/SFML")
    set(SFML_DIR "${SFML_ROOT}/lib/cmake/SFML")
    set(SFML_INCLUDE_DIR "${SFML_ROOT}/include")
    set(SFML_LIBRARY_DIR "${SFML_ROOT}/lib")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${SFML_ROOT})
endif()

# Trova le dipendenze
find_package(SFML 2.5 COMPONENTS graphics REQUIRED)

# Aggiungi le directory di include
include_directories(
    ${CMAKE_SOURCE_DIR}/../../src
    ${CMAKE_SOURCE_DIR}/../../src/core
    ${CMAKE_SOURCE_DIR}/../../src/core/CoreClass
    ${CMAKE_SOURCE_DIR}/../../src/core/bindings/bridge
    ${CMAKE_SOURCE_DIR}/../../src/core/Monitoring/GUI
    ${SFML_INCLUDE_DIR}
)

# Aggiungi i file sorgente dell'applicazione
file(GLOB_RECURSE APP_SOURCES "src/*.cpp")

# Aggiungi i file sorgente del core necessari
set(CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/../../src/core/CoreAPI.cpp
    ${CMAKE_SOURCE_DIR}/../../src/core/ProjectTemplate.cpp
    ${CMAKE_SOURCE_DIR}/../../src/core/CoreCLI.cpp
    ${CMAKE_SOURCE_DIR}/../../src/core/CoreClass/Core.cpp
    ${CMAKE_SOURCE_DIR}/../../src/core/CoreClass/ConfigManager.cpp
    ${CMAKE_SOURCE_DIR}/../../src/core/CoreClass/ResourceManager.cpp
    ${CMAKE_SOURCE_DIR}/../../src/core/CoreClass/ModuleLoader.cpp
    ${CMAKE_SOURCE_DIR}/../../src/core/CoreClass/ErrorHandler.cpp
    ${CMAKE_SOURCE_DIR}/../../src/core/CoreClass/IPCManager.cpp
    ${CMAKE_SOURCE_DIR}/../../src/core/CoreClass/ModuleManager.cpp
    ${CMAKE_SOURCE_DIR}/../../src/core/Monitoring/GUI/MonitoringDashboard.cpp
)

# Crea la libreria core minima
add_library(core STATIC ${CORE_SOURCES})

# Collega le librerie
target_link_libraries(core PRIVATE 
    sfml-graphics
)

# Crea l'eseguibile
add_executable(test_app ${APP_SOURCES})

# Collega le librerie
target_link_libraries(test_app PRIVATE
    core
    sfml-graphics
)

# Copia le DLL di SFML nella directory di output
if(WIN32)
    file(GLOB SFML_DLLS "${SFML_LIBRARY_DIR}/*.dll")
    file(COPY ${SFML_DLLS} DESTINATION ${CMAKE_BINARY_DIR})
endif() 