cmake_minimum_required(VERSION 3.16)
project(PokemonBattleSimulator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags for better error detection
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
elseif (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

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
    src/ai_strategy.cpp
    src/ai_factory.cpp
    src/easy_ai.cpp
    src/medium_ai.cpp
    src/hard_ai.cpp
    src/expert_ai.cpp
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
    src/ai_strategy.cpp
    src/ai_factory.cpp
    src/easy_ai.cpp
    src/medium_ai.cpp
    src/hard_ai.cpp
    src/expert_ai.cpp
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
    src/ai_strategy.h
    src/ai_factory.h
    src/easy_ai.h
    src/medium_ai.h
    src/hard_ai.h
    src/expert_ai.h
)

# Create main executable
add_executable(pokemon_battle ${MAIN_SOURCES} ${HEADERS})

# Example executables removed for streamlined experience

# Set include directories for all targets
target_include_directories(pokemon_battle PRIVATE src include)

# Find and link required libraries if needed
# For JSON library, we're using the header-only nlohmann/json
# If you want to use system-installed nlohmann_json:
# find_package(nlohmann_json REQUIRED)
# target_link_libraries(pokemon_battle nlohmann_json::nlohmann_json)

# Set output directory for all executables
set_target_properties(pokemon_battle PROPERTIES
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

# Enable testing
enable_testing()

# Add Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add test directory if it exists and contains tests
if(EXISTS ${CMAKE_SOURCE_DIR}/tests AND EXISTS ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt)
    add_subdirectory(tests)
endif()

# Installation rules
install(TARGETS pokemon_battle
    RUNTIME DESTINATION bin
)

install(DIRECTORY data/
    DESTINATION share/pokemon-battle-sim/data
)

install(FILES README.md
    DESTINATION share/pokemon-battle-sim
) 