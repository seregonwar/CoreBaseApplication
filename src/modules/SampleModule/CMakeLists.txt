cmake_minimum_required(VERSION 3.10)

project(SampleModule)

add_library(SampleModule SHARED SampleModule.cpp)

target_include_directories(SampleModule PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(SampleModule PRIVATE core)

install(TARGETS SampleModule DESTINATION modules)