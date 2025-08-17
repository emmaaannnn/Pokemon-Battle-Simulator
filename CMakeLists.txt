cmake_minimum_required(VERSION 3.16)
project(PokemonBattleSimulator)

# ────────────────────────────────
#  Build settings
# ────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif (MSVC)
    add_compile_options(/W4)
endif()

# ────────────────────────────────
#  Include paths
# ────────────────────────────────
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ai
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ────────────────────────────────
#  Source / header lists (organized)
# ────────────────────────────────
set(CORE_SOURCES
    src/core/move.cpp
    src/core/pokemon.cpp
    src/core/team.cpp
    src/core/battle.cpp
    src/core/weather.cpp
    src/core/battle_events.cpp
)

set(AI_SOURCES
    src/ai/ai_strategy.cpp
    src/ai/ai_factory.cpp
    src/ai/easy_ai.cpp
    src/ai/medium_ai.cpp
    src/ai/hard_ai.cpp
    src/ai/expert_ai.cpp
)

set(UTILS_SOURCES
    src/utils/type_effectiveness.cpp
    src/utils/move_type_mapping.cpp
    src/utils/input_validator.cpp
    src/utils/health_bar_animator.cpp
    src/utils/health_bar_event_listener.cpp
)

set(ALL_SOURCES ${CORE_SOURCES} ${AI_SOURCES} ${UTILS_SOURCES})

set(CORE_HEADERS
    include/core/move.h
    include/core/pokemon.h
    include/core/team.h
    include/core/battle.h
    include/core/weather.h
    include/core/battle_events.h
)

set(AI_HEADERS
    include/ai/ai_strategy.h
    include/ai/ai_factory.h
    include/ai/easy_ai.h
    include/ai/medium_ai.h
    include/ai/hard_ai.h
    include/ai/expert_ai.h
)

set(UTILS_HEADERS
    include/utils/type_effectiveness.h
    include/utils/move_type_mapping.h
    include/utils/input_validator.h
    include/utils/health_bar_animator.h
    include/utils/health_bar_event_listener.h
    include/utils/input_validator_templates.hpp
    include/utils/json.hpp
)

set(ALL_HEADERS ${CORE_HEADERS} ${AI_HEADERS} ${UTILS_HEADERS})

set(MAIN_SOURCES ${ALL_SOURCES} src/main.cpp)

# ────────────────────────────────
#  Executables
# ────────────────────────────────
add_executable(pokemon_battle ${MAIN_SOURCES} ${ALL_HEADERS})

target_include_directories(pokemon_battle PRIVATE 
    include/core include/ai include/utils src)
set_target_properties(pokemon_battle
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ────────────────────────────────
#  Data-file copying
# ────────────────────────────────
add_custom_target(copy_data_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
    COMMENT "Copying data files to build directory"
)

add_dependencies(pokemon_battle copy_data_files)

# ────────────────────────────────
#  Testing (GoogleTest + subdir)
# ────────────────────────────────
enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # Windows MSVC nicety
FetchContent_MakeAvailable(googletest)

if (EXISTS ${CMAKE_SOURCE_DIR}/tests AND EXISTS ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt)
    add_subdirectory(tests)
endif()

# ────────────────────────────────
#  Install rules
# ────────────────────────────────
install(TARGETS pokemon_battle
        RUNTIME DESTINATION bin)

install(DIRECTORY data/      DESTINATION share/pokemon-battle-sim/data)
install(FILES     README.md  DESTINATION share/pokemon-battle-sim)
