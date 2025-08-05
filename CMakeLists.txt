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
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ────────────────────────────────
#  Source / header lists
# ────────────────────────────────
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

set(MAIN_SOURCES ${CORE_SOURCES} src/main.cpp)

# ────────────────────────────────
#  Executables
# ────────────────────────────────
add_executable(pokemon_battle ${MAIN_SOURCES} ${HEADERS})
add_executable(game-demo      examples/game-demo.cpp  ${CORE_SOURCES} ${HEADERS})
add_executable(debug-test     examples/debug-test.cpp ${CORE_SOURCES} ${HEADERS})

foreach(tgt IN LISTS
        pokemon_battle
        game-demo
        debug-test)
    target_include_directories(${tgt} PRIVATE src include)
    set_target_properties(${tgt}
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endforeach()

# ────────────────────────────────
#  Data-file copying
# ────────────────────────────────
add_custom_target(copy_data_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
    COMMENT "Copying data files to build directory"
)

add_dependencies(pokemon_battle copy_data_files)
add_dependencies(game-demo      copy_data_files)
add_dependencies(debug-test     copy_data_files)

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
install(TARGETS pokemon_battle game-demo debug-test
        RUNTIME DESTINATION bin)

install(DIRECTORY data/      DESTINATION share/pokemon-battle-sim/data)
install(FILES     README.md  DESTINATION share/pokemon-battle-sim)
