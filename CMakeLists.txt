cmake_minimum_required(VERSION 3.16)
project(PokemonBattleSimulator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags for better error detection
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Source files
set(SOURCES
    Move.cpp
    Pokemon.cpp
    Team.cpp
    Battle.cpp
    main.cpp
)

# Header files
set(HEADERS
    Move.h
    Pokemon.h
    Team.h
    Battle.h
)

# Create executable
add_executable(pokemon_battle ${SOURCES} ${HEADERS})

# Find and link required libraries if needed
# For JSON library, we're using the header-only nlohmann/json
# If you want to use system-installed nlohmann_json:
# find_package(nlohmann_json REQUIRED)
# target_link_libraries(pokemon_battle nlohmann_json::nlohmann_json)

# Set output directory
set_target_properties(pokemon_battle PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
) 