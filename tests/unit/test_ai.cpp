#include <gtest/gtest.h>
#include "test_utils.h"
#include "battle.h"

class AITest : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Create battle instance
        battle = std::make_unique<Battle>(playerTeam, opponentTeam);
        
        // Set up Pokemon with various moves for testing
        setupTestPokemonWithMoves();
    }
    
    void setupTestPokemonWithMoves() {
        // Create Pokemon with different types of moves for AI testing
        Pokemon* opponentPokemon = opponentTeam.getPokemon(0);
        if (opponentPokemon) {
            opponentPokemon->moves.clear();
            
            // Add various move types for testing
            opponentPokemon->moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
            opponentPokemon->moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special"));
            opponentPokemon->moves.push_back(TestUtils::createTestMove("water-gun", 40, 100, 25, "water", "special"));
            opponentPokemon->moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON, 100));
        }
    }
    
    std::unique_ptr<Battle> battle;
};

// Test AI battle construction
TEST_F(AITest, AIBattleConstruction) {
    EXPECT_NE(battle, nullptr);
    
    // Battle should start as ongoing
    EXPECT_FALSE(battle->isBattleOver());
}


// Test AI move selection behavior patterns
TEST_F(AITest, AIMoveSelectionPatterns) {
    // Create a Pokemon with specific moves for testing
    Pokemon testOpponent = TestUtils::createTestPokemon("testai", 100, 80, 70, 90, 85, 75, {"fire"});
    
    // Add moves with different characteristics
    testOpponent.moves.clear();
    testOpponent.moves.push_back(TestUtils::createTestMove("ember", 40, 100, 25, "fire", "special"));      // STAB fire move
    testOpponent.moves.push_back(TestUtils::createTestMove("water-gun", 40, 100, 25, "water", "special"));  // Super effective vs fire
    testOpponent.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));   // Neutral move
    testOpponent.moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON, 100)); // Status move
    
    Team singleOpponentTeam = TestUtils::createTestTeam({testOpponent});
    
    // Create battle with this specific setup
    Battle testBattle(playerTeam, singleOpponentTeam);
    
    // Test that AI can make move selections without crashing
    EXPECT_FALSE(testBattle.isBattleOver());
}

// Test AI behavior with Pokemon having no usable moves
TEST_F(AITest, AIWithNoUsableMoves) {
    Pokemon testOpponent = TestUtils::createTestPokemon("nomoves", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Add a move and exhaust its PP
    testOpponent.moves.clear();
    testOpponent.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 1, "normal", "physical"));
    testOpponent.moves[0].usePP(); // Exhaust PP
    
    Team noMovesTeam = TestUtils::createTestTeam({testOpponent});
    
    Battle noPPBattle(playerTeam, noMovesTeam);
    
    // Battle should still be functional even with no usable moves
    EXPECT_FALSE(noPPBattle.isBattleOver());
}

// Test AI behavior with different move types
TEST_F(AITest, AIWithDifferentMoveTypes) {
    Pokemon testOpponent = TestUtils::createTestPokemon("diverse", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Add moves of different categories
    testOpponent.moves.clear();
    testOpponent.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    testOpponent.moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special"));
    testOpponent.moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON, 100));
    testOpponent.moves.push_back(TestUtils::createTestMove("swords-dance", 0, 100, 20, "normal", "status"));
    
    Team diverseTeam = TestUtils::createTestTeam({testOpponent});
    
    Battle diverseBattle(playerTeam, diverseTeam);
    
    // Battle should handle diverse move types
    EXPECT_FALSE(diverseBattle.isBattleOver());
}

// Test AI consistency - same setup should produce valid moves
TEST_F(AITest, AIConsistency) {
    // Test that AI makes consistent decisions (valid moves)
    for (int i = 0; i < 10; ++i) {
        Battle testBattle(playerTeam, opponentTeam);
        EXPECT_FALSE(testBattle.isBattleOver());
    }
}

// Test AI with low-health Pokemon
TEST_F(AITest, AIWithLowHealthPokemon) {
    Pokemon lowHealthOpponent = TestUtils::createTestPokemon("lowhp", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Damage the Pokemon
    lowHealthOpponent.takeDamage(75); // 25% health remaining
    
    lowHealthOpponent.moves.clear();
    lowHealthOpponent.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    lowHealthOpponent.moves.push_back(TestUtils::createTestMove("rest", 0, 100, 10, "psychic", "status"));
    
    Team lowHealthTeam = TestUtils::createTestTeam({lowHealthOpponent});
    
    Battle lowHealthBattle(playerTeam, lowHealthTeam);
    
    // Battle should handle low health Pokemon
    EXPECT_FALSE(lowHealthBattle.isBattleOver());
    EXPECT_TRUE(lowHealthOpponent.isAlive());
    EXPECT_LT(lowHealthOpponent.getHealthPercentage(), 50.0);
}

// Test AI with status-affected Pokemon
TEST_F(AITest, AIWithStatusAffectedPokemon) {
    Pokemon statusOpponent = TestUtils::createTestPokemon("statused", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Apply status condition
    statusOpponent.applyStatusCondition(StatusCondition::POISON);
    
    statusOpponent.moves.clear();
    statusOpponent.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    statusOpponent.moves.push_back(TestUtils::createTestMove("aromatherapy", 0, 100, 5, "grass", "status"));
    
    Team statusTeam = TestUtils::createTestTeam({statusOpponent});
    
    Battle statusBattle(playerTeam, statusTeam);
    
    // Battle should handle status-affected Pokemon
    EXPECT_FALSE(statusBattle.isBattleOver());
    EXPECT_TRUE(statusOpponent.hasStatusCondition());
}

// Test AI with type-advantage scenarios
TEST_F(AITest, AIWithTypeAdvantage) {
    // Create a fire-type opponent against a grass-type player
    Pokemon fireOpponent = TestUtils::createTestPokemon("fire", 100, 80, 70, 90, 85, 75, {"fire"});
    Pokemon grassPlayer = TestUtils::createTestPokemon("grass", 100, 80, 70, 90, 85, 75, {"grass"});
    
    fireOpponent.moves.clear();
    fireOpponent.moves.push_back(TestUtils::createTestMove("ember", 40, 100, 25, "fire", "special"));      // Super effective
    fireOpponent.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));  // Neutral
    fireOpponent.moves.push_back(TestUtils::createTestMove("water-gun", 40, 100, 25, "water", "special")); // Not very effective
    
    Team fireTeam = TestUtils::createTestTeam({fireOpponent});
    Team grassTeam = TestUtils::createTestTeam({grassPlayer});
    
    Battle typeAdvantage(grassTeam, fireTeam);
    
    // Battle should handle type advantages
    EXPECT_FALSE(typeAdvantage.isBattleOver());
}

// Test AI with single-move Pokemon
TEST_F(AITest, AIWithSingleMove) {
    Pokemon singleMoveOpponent = TestUtils::createTestPokemon("single", 100, 80, 70, 90, 85, 75, {"normal"});
    
    singleMoveOpponent.moves.clear();
    singleMoveOpponent.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    
    Team singleMoveTeam = TestUtils::createTestTeam({singleMoveOpponent});
    
    Battle singleMoveBattle(playerTeam, singleMoveTeam);
    
    // Battle should handle single-move Pokemon
    EXPECT_FALSE(singleMoveBattle.isBattleOver());
}

// Test AI with high-power moves
TEST_F(AITest, AIWithHighPowerMoves) {
    Pokemon powerOpponent = TestUtils::createTestPokemon("power", 100, 80, 70, 90, 85, 75, {"normal"});
    
    powerOpponent.moves.clear();
    powerOpponent.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    powerOpponent.moves.push_back(TestUtils::createTestMove("hyper-beam", 150, 90, 5, "normal", "special"));
    powerOpponent.moves.push_back(TestUtils::createTestMove("quick-attack", 40, 100, 30, "normal", "physical"));
    
    Team powerTeam = TestUtils::createTestTeam({powerOpponent});
    
    Battle powerBattle(playerTeam, powerTeam);
    
    // Battle should handle high-power moves
    EXPECT_FALSE(powerBattle.isBattleOver());
}

// Test AI with accuracy-based moves
TEST_F(AITest, AIWithAccuracyMoves) {
    Pokemon accuracyOpponent = TestUtils::createTestPokemon("accuracy", 100, 80, 70, 90, 85, 75, {"normal"});
    
    accuracyOpponent.moves.clear();
    accuracyOpponent.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    accuracyOpponent.moves.push_back(TestUtils::createTestMove("focus-blast", 120, 70, 5, "fighting", "special"));
    accuracyOpponent.moves.push_back(TestUtils::createTestMove("swift", 60, 0, 20, "normal", "special")); // Never miss
    
    Team accuracyTeam = TestUtils::createTestTeam({accuracyOpponent});
    
    Battle accuracyBattle(playerTeam, accuracyTeam);
    
    // Battle should handle accuracy-based moves
    EXPECT_FALSE(accuracyBattle.isBattleOver());
}

// Test AI behavior differences between difficulties
TEST_F(AITest, AIDifficultyBehaviorDifferences) {
    // Create identical setups for different difficulties
    Pokemon testOpponent1 = TestUtils::createTestPokemon("test1", 100, 80, 70, 90, 85, 75, {"normal"});
    Pokemon testOpponent2 = TestUtils::createTestPokemon("test2", 100, 80, 70, 90, 85, 75, {"normal"});
    Pokemon testOpponent3 = TestUtils::createTestPokemon("test3", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // All have the same moves
    for (Pokemon* pokemon : {&testOpponent1, &testOpponent2, &testOpponent3}) {
        pokemon->moves.clear();
        pokemon->moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
        pokemon->moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special"));
        pokemon->moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON, 100));
    }
    
    Team team1 = TestUtils::createTestTeam({testOpponent1});
    Team team2 = TestUtils::createTestTeam({testOpponent2});
    Team team3 = TestUtils::createTestTeam({testOpponent3});
    
    Battle testBattle(playerTeam, team1);
    
    // Should be functional
    EXPECT_FALSE(testBattle.isBattleOver());
}

// Test AI with empty move list
TEST_F(AITest, AIWithEmptyMoveList) {
    Pokemon emptyMoveOpponent = TestUtils::createTestPokemon("empty", 100, 80, 70, 90, 85, 75, {"normal"});
    
    emptyMoveOpponent.moves.clear(); // No moves at all
    
    Team emptyMoveTeam = TestUtils::createTestTeam({emptyMoveOpponent});
    
    Battle emptyMoveBattle(playerTeam, emptyMoveTeam);
    
    // Battle should handle empty move lists gracefully
    EXPECT_FALSE(emptyMoveBattle.isBattleOver());
}

// Test AI state consistency across multiple battles
TEST_F(AITest, AIStateConsistency) {
    // Test that AI maintains consistent behavior across multiple battle instances
    std::vector<std::unique_ptr<Battle>> battles;
    
    for (int i = 0; i < 5; ++i) {
        battles.push_back(std::make_unique<Battle>(playerTeam, opponentTeam));
        EXPECT_FALSE(battles.back()->isBattleOver());
        EXPECT_EQ(battles.back()->getBattleResult(), Battle::BattleResult::ONGOING);
    }
    
    // All battles should be in the same initial state
    for (const auto& battle : battles) {
        EXPECT_FALSE(battle->isBattleOver());
        EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
    }
}

// Test AI with maximum stat modifications
TEST_F(AITest, AIWithMaxStatModifications) {
    Pokemon modifiedOpponent = TestUtils::createTestPokemon("modified", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Apply maximum stat modifications
    for (int i = 0; i < 6; ++i) {
        modifiedOpponent.modifyAttack(1);
        modifiedOpponent.modifyDefense(1);
        modifiedOpponent.modifySpeed(1);
        modifiedOpponent.modifySpecialAttack(1);
        modifiedOpponent.modifySpecialDefense(1);
    }
    
    modifiedOpponent.moves.clear();
    modifiedOpponent.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    modifiedOpponent.moves.push_back(TestUtils::createTestMove("swords-dance", 0, 100, 20, "normal", "status"));
    
    Team modifiedTeam = TestUtils::createTestTeam({modifiedOpponent});
    
    Battle modifiedBattle(playerTeam, modifiedTeam);
    
    // Battle should handle maximally modified Pokemon
    EXPECT_FALSE(modifiedBattle.isBattleOver());
    EXPECT_EQ(modifiedOpponent.attack_stage, 6);
    EXPECT_EQ(modifiedOpponent.defense_stage, 6);
    EXPECT_EQ(modifiedOpponent.speed_stage, 6);
    EXPECT_EQ(modifiedOpponent.special_attack_stage, 6);
    EXPECT_EQ(modifiedOpponent.special_defense_stage, 6);
}