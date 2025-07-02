cmake_minimum_required(VERSION 3.16)
project(PokemonBattleSimulator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags for better error detection
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Main executable source files
set(MAIN_SOURCES
    src/move.cpp
    src/pokemon.cpp
    src/team.cpp
    src/battle.cpp
    src/type_effectiveness.cpp
    src/move_type_mapping.cpp
    src/weather.cpp
    src/main.cpp
)

# Core library source files (without main.cpp for reuse in examples)
set(CORE_SOURCES
    src/move.cpp
    src/pokemon.cpp
    src/team.cpp
    src/battle.cpp
    src/type_effectiveness.cpp
    src/move_type_mapping.cpp
    src/weather.cpp
)

# Header files
set(HEADERS
    src/move.h
    src/pokemon.h
    src/team.h
    src/battle.h
    src/type_effectiveness.h
    src/move_type_mapping.h
    src/weather.h
)

# Create main executable
add_executable(pokemon_battle ${MAIN_SOURCES} ${HEADERS})

# Create example executables
add_executable(game-demo examples/game-demo.cpp ${CORE_SOURCES} ${HEADERS})
add_executable(debug-test examples/debug-test.cpp ${CORE_SOURCES} ${HEADERS})

# Set include directories for all targets
target_include_directories(pokemon_battle PRIVATE src include)
target_include_directories(game-demo PRIVATE src include)
target_include_directories(debug-test PRIVATE src include)

# Find and link required libraries if needed
# For JSON library, we're using the header-only nlohmann/json
# If you want to use system-installed nlohmann_json:
# find_package(nlohmann_json REQUIRED)
# target_link_libraries(pokemon_battle nlohmann_json::nlohmann_json)

# Set output directory for all executables
set_target_properties(pokemon_battle game-demo debug-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Create a custom target to copy data files to build directory
add_custom_target(copy_data_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
    COMMENT "Copying data files to build directory"
)

# Make executables depend on data files
add_dependencies(pokemon_battle copy_data_files)
add_dependencies(game-demo copy_data_files)
add_dependencies(debug-test copy_data_files)

# Enable testing
enable_testing()

# Add test directory if it exists and contains tests
if(EXISTS ${CMAKE_SOURCE_DIR}/tests AND EXISTS ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt)
    add_subdirectory(tests)
endif()

# Installation rules
install(TARGETS pokemon_battle game-demo debug-test
    RUNTIME DESTINATION bin
)

install(DIRECTORY data/
    DESTINATION share/pokemon-battle-sim/data
)

install(FILES README.md
    DESTINATION share/pokemon-battle-sim
) 