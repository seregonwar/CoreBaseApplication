cmake_minimum_required(VERSION 3.15)
project({{project_name}} VERSION {{version}})

# Imposta lo standard C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Aggiungi i file sorgente
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Crea l'eseguibile
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Aggiungi le directory di include
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Aggiungi le dipendenze
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    nlohmann_json::nlohmann_json
) 