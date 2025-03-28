cmake_minimum_required(VERSION 3.15)
project(CoreBaseApplication VERSION 1.0.0)

# Imposta lo standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Aggiungi GTest
add_subdirectory(third_party/gtest)

# Aggiungi le directory di include
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/core)

# Aggiungi i file sorgente del core
file(GLOB_RECURSE CORE_SOURCES "src/core/*.cpp")
add_library(core STATIC ${CORE_SOURCES})

# Collega le librerie al core
target_link_libraries(core PRIVATE
    gtest
    gtest_main
)

# Aggiungi gli esempi
add_subdirectory(examples/test_app) 