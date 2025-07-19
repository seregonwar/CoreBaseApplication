# Aggiungi tutti i file sorgente dei test
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# Crea l'eseguibile dei test
add_executable(test_app_test ${TEST_SOURCES})

# Collega le librerie
target_link_libraries(test_app_test PRIVATE
    core
    gtest
    gtest_main
)

# Aggiungi le directory di include
target_include_directories(test_app_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
)

# Registra il test
add_test(NAME test_app_test COMMAND test_app_test) 