cmake_minimum_required(VERSION 3.15)
project({{project_name}} VERSION {{version}})

# Imposta lo standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Opzioni di build
option(BUILD_TESTS "Build test suite" ON)

# Trova le dipendenze
find_package(GTest REQUIRED)

# Aggiungi le directory di include
include_directories(${PROJECT_SOURCE_DIR}/include)

# Aggiungi i file sorgente
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_library(${PROJECT_NAME} ${SOURCES})

# Configura i test
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Imposta le proprietà del progetto
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    DESCRIPTION "{{description}}"
    AUTHOR "{{author}}"
) 