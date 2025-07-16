# Google Test configuration for Pokemon Battle Simulator
cmake_minimum_required(VERSION 3.16)

# Include Google Test
include(GoogleTest)

# Common test configuration
set(TEST_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# Core library sources (reusable for tests)
set(CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/move.cpp
    ${CMAKE_SOURCE_DIR}/src/pokemon.cpp
    ${CMAKE_SOURCE_DIR}/src/team.cpp
    ${CMAKE_SOURCE_DIR}/src/battle.cpp
    ${CMAKE_SOURCE_DIR}/src/type_effectiveness.cpp
    ${CMAKE_SOURCE_DIR}/src/move_type_mapping.cpp
    ${CMAKE_SOURCE_DIR}/src/weather.cpp
)

# Test utilities
set(TEST_UTILS_SOURCES
    utils/test_utils.cpp
)

# Create test utilities library
add_library(test_utils ${TEST_UTILS_SOURCES})
target_include_directories(test_utils PUBLIC ${TEST_INCLUDE_DIRS})
target_link_libraries(test_utils gtest)

# Function to create a test executable
function(create_test test_name test_source)
    add_executable(${test_name} ${test_source} ${CORE_SOURCES})
    target_include_directories(${test_name} PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(${test_name} gtest gtest_main test_utils)
    
    # Copy test data files
    add_custom_command(TARGET ${test_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data $<TARGET_FILE_DIR:${test_name}>/test_data
        COMMENT "Copying test data files"
    )
    
    # Register test with CTest
    gtest_discover_tests(${test_name})
endfunction()

# Unit Tests
create_test(test_pokemon unit/test_pokemon.cpp)
create_test(test_move unit/test_move.cpp)
create_test(test_type_effectiveness unit/test_type_effectiveness.cpp)
create_test(test_team unit/test_team.cpp)
create_test(test_battle unit/test_battle.cpp)
create_test(test_weather unit/test_weather.cpp)
create_test(test_ai unit/test_ai.cpp)

# Integration Tests
create_test(test_full_battle integration/test_full_battle.cpp)
create_test(test_status_integration integration/test_status_integration.cpp)
create_test(test_weather_integration integration/test_weather_integration.cpp)

# Placeholder for future tests
# create_test(test_switching_integration integration/test_switching_integration.cpp)
# create_test(test_ai_integration integration/test_ai_integration.cpp)

# Create a target to run all tests
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_pokemon test_move test_type_effectiveness test_team test_battle test_weather test_ai test_full_battle test_status_integration test_weather_integration
    COMMENT "Running all tests"
)