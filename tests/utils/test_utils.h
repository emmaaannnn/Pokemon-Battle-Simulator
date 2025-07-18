#pragma once

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>

#include "pokemon.h"
#include "move.h"
#include "team.h"
#include "battle.h"
#include "type_effectiveness.h"
#include "weather.h"

namespace TestUtils {

/**
 * Creates a test Pokemon with specified stats and moves
 */
Pokemon createTestPokemon(const std::string& name, int hp = 100, int attack = 80, int defense = 70,
                         int special_attack = 90, int special_defense = 85, int speed = 75,
                         const std::vector<std::string>& types = {"normal"},
                         const std::vector<std::string>& moveNames = {"testmove"});

/**
 * Creates a test Move with specified properties
 */
Move createTestMove(const std::string& name, int power = 80, int accuracy = 100, int pp = 15,
                   const std::string& type = "normal", const std::string& damageClass = "physical",
                   StatusCondition ailment = StatusCondition::NONE, int ailmentChance = 0);

/**
 * Creates a test Team with the given Pokemon
 */
Team createTestTeam(const std::vector<Pokemon>& pokemon);

/**
 * Writes a JSON file with the given content (for test data)
 */
void writeTestJsonFile(const std::string& filename, const std::string& content);

/**
 * Creates test Pokemon JSON data
 */
std::string createTestPokemonJson(const std::string& name, int hp, int attack, int defense,
                                 int special_attack, int special_defense, int speed,
                                 const std::vector<std::string>& types);

/**
 * Creates test Move JSON data
 */
std::string createTestMoveJson(const std::string& name, int power, int accuracy, int pp,
                              const std::string& type, const std::string& damageClass,
                              const std::string& ailment = "none", int ailmentChance = 0);

/**
 * Compares two floating point numbers with epsilon tolerance
 */
bool compareFloats(double a, double b, double epsilon = 1e-6);

/**
 * Sets up test environment with basic test data files
 */
void setupTestEnvironment();

// Test assertion macros
#define EXPECT_FLOAT_EQ_EPSILON(expected, actual, epsilon) \
    EXPECT_TRUE(TestUtils::compareFloats(expected, actual, epsilon)) \
    << "Expected: " << expected << ", Actual: " << actual << ", Epsilon: " << epsilon

#define ASSERT_FLOAT_EQ_EPSILON(expected, actual, epsilon) \
    ASSERT_TRUE(TestUtils::compareFloats(expected, actual, epsilon)) \
    << "Expected: " << expected << ", Actual: " << actual << ", Epsilon: " << epsilon

// Custom test fixture for Pokemon tests
class PokemonTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Skip setupTestEnvironment() to avoid overwriting our fixed JSON files
        // TestUtils::setupTestEnvironment();
    }
    
    void TearDown() override {
        // Clean up test data if needed
    }
};

// Custom test fixture for Battle tests
class BattleTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Skip setupTestEnvironment() to avoid overwriting our fixed JSON files
        // TestUtils::setupTestEnvironment();
        
        // Create test Pokemon
        testPokemon1 = TestUtils::createTestPokemon("testmon1", 100, 80, 70, 90, 85, 75, {"normal"});
        testPokemon2 = TestUtils::createTestPokemon("testmon2", 90, 85, 65, 95, 80, 85, {"fire"});
        
        // Create test teams
        playerTeam = TestUtils::createTestTeam({testPokemon1});
        opponentTeam = TestUtils::createTestTeam({testPokemon2});
    }
    
    Pokemon testPokemon1;
    Pokemon testPokemon2;
    Team playerTeam;
    Team opponentTeam;
};

} // namespace TestUtils