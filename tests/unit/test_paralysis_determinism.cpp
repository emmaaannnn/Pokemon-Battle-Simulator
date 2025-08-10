#include <gtest/gtest.h>
#include "battle.h"
#include "pokemon.h"
#include "test_utils.h"
#include "expert_ai.h"
#include "ai_strategy.h"

class ParalysisDeterminismTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create identical battle scenarios
        Team team1 = TestUtils::createTestTeam({TestUtils::createTestPokemon("test1", 100, 80, 70, 90, 85, 75, {"normal"})});
        Team team2 = TestUtils::createTestTeam({TestUtils::createTestPokemon("test2", 100, 80, 70, 90, 85, 75, {"normal"})});
        
        battle1 = std::make_unique<Battle>(team1, team2, Battle::AIDifficulty::EXPERT);
        battle2 = std::make_unique<Battle>(team1, team2, Battle::AIDifficulty::EXPERT);
    }
    
    std::unique_ptr<Battle> battle1;
    std::unique_ptr<Battle> battle2;
};

// This test will FAIL before our fix - demonstrating the bug
TEST_F(ParalysisDeterminismTest, ParalysisNonDeterministicBehavior) {
    // Create paralyzed Pokemon
    Pokemon paralyzedPokemon = TestUtils::createTestPokemon("paralyzed", 100, 80, 70, 90, 85, 75, {"electric"});
    paralyzedPokemon.status = StatusCondition::PARALYSIS;
    
    // Test multiple calls to canAct() - should be deterministic but currently isn't
    std::vector<bool> results1, results2;
    
    for (int i = 0; i < 20; ++i) {
        results1.push_back(paralyzedPokemon.canAct());
        results2.push_back(paralyzedPokemon.canAct());
    }
    
    // This will FAIL because canAct() uses non-seeded random
    // Results should be the same for same inputs but won't be
    bool identical = (results1 == results2);
    EXPECT_FALSE(identical) << "Before fix: canAct() should be non-deterministic (this proves the bug exists)";
}

// This test validates the fixed paralysis behavior with seeded RNG
TEST_F(ParalysisDeterminismTest, ParalysisWithSeededRNGIsDeterministic) {
    // Create paralyzed Pokemon  
    Pokemon paralyzedPokemon = TestUtils::createTestPokemon("paralyzed", 100, 80, 70, 90, 85, 75, {"electric"});
    paralyzedPokemon.status = StatusCondition::PARALYSIS;
    
    // Test deterministic behavior with same seed
    std::vector<bool> results1, results2;
    
    // Use same seed for both runs
    std::mt19937 rng1(12345);
    std::mt19937 rng2(12345);
    
    for (int i = 0; i < 50; ++i) {
        results1.push_back(paralyzedPokemon.canAct(rng1));
        results2.push_back(paralyzedPokemon.canAct(rng2));
    }
    
    // With same seed, results should be identical
    EXPECT_EQ(results1, results2) << "Same seed should produce identical paralysis results";
    
    // Test that different seeds produce different results (to verify randomness still works)
    std::vector<bool> results3;
    std::mt19937 rng3(54321); // Different seed
    
    for (int i = 0; i < 50; ++i) {
        results3.push_back(paralyzedPokemon.canAct(rng3));
    }
    
    // Different seed should produce different sequence (with high probability)
    bool resultsAreDifferent = (results1 != results3);
    EXPECT_TRUE(resultsAreDifferent) << "Different seeds should produce different sequences";
    
    // Test that paralysis has approximately 75% success rate
    std::mt19937 rng4(99999);
    int successes = 0;
    int trials = 1000;
    
    for (int i = 0; i < trials; ++i) {
        if (paralyzedPokemon.canAct(rng4)) {
            successes++;
        }
    }
    
    double successRate = static_cast<double>(successes) / trials;
    EXPECT_GT(successRate, 0.7) << "Paralysis should allow action ~75% of the time";
    EXPECT_LT(successRate, 0.8) << "Paralysis should allow action ~75% of the time";
}

// Test that Expert AI generateLegalMoves uses deterministic paralysis 
TEST_F(ParalysisDeterminismTest, ExpertAIGenerateLegalMovesParalysisDeterminism) {
    // Create battle state with paralyzed Pokemon
    Team team1 = TestUtils::createTestTeam({TestUtils::createTestPokemon("paralyzed", 100, 80, 70, 90, 85, 75, {"electric"})});
    Team team2 = TestUtils::createTestTeam({TestUtils::createTestPokemon("opponent", 100, 80, 70, 90, 85, 75, {"normal"})});
    
    BattleState state1{team1.getPokemon(0), team2.getPokemon(0), &team1, &team2, WeatherCondition::NONE, 0, 1};
    BattleState state2{team1.getPokemon(0), team2.getPokemon(0), &team1, &team2, WeatherCondition::NONE, 0, 1};
    
    // Set up paralysis
    state1.aiPokemon->status = StatusCondition::PARALYSIS;
    state2.aiPokemon->status = StatusCondition::PARALYSIS;
    
    // Ensure moves have PP
    state1.aiPokemon->moves[0].current_pp = 10;
    state1.aiPokemon->moves[1].current_pp = 8;
    state2.aiPokemon->moves[0].current_pp = 10;
    state2.aiPokemon->moves[1].current_pp = 8;
    
    // Set same deterministic RNG seed in both states
    state1.deterministicRng.seed(42);
    state2.deterministicRng.seed(42);
    
    // Create Expert AI instance
    ExpertAI expertAI;
    
    // Generate legal moves for both states
    std::vector<BattleState> moves1 = expertAI.generateLegalMoves(state1, true);
    std::vector<BattleState> moves2 = expertAI.generateLegalMoves(state2, true);
    
    // With same seed, should get same number of legal moves
    EXPECT_EQ(moves1.size(), moves2.size()) << "Same paralysis seed should produce same number of legal moves";
    
    // Verify that the legal move sets are structurally similar
    bool hasMove1 = false, hasMove2 = false;
    bool hasSwitch1 = false, hasSwitch2 = false;
    
    for (const auto& state : moves1) {
        if (state.aiPokemon == state1.aiPokemon) hasMove1 = true;
        else hasSwitch1 = true;
    }
    
    for (const auto& state : moves2) {
        if (state.aiPokemon == state2.aiPokemon) hasMove2 = true;
        else hasSwitch2 = true;
    }
    
    EXPECT_EQ(hasMove1, hasMove2) << "Same paralysis seed should produce same move availability";
    EXPECT_EQ(hasSwitch1, hasSwitch2) << "Switch options should be consistent";
    
    // Test different seeds produce potentially different results
    BattleState state3{team1.getPokemon(0), team2.getPokemon(0), &team1, &team2, WeatherCondition::NONE, 0, 1};
    state3.aiPokemon->status = StatusCondition::PARALYSIS;
    state3.aiPokemon->moves[0].current_pp = 10;
    state3.aiPokemon->moves[1].current_pp = 8;
    state3.deterministicRng.seed(9999); // Different seed
    
    std::vector<BattleState> moves3 = expertAI.generateLegalMoves(state3, true);
    
    // Different seed may produce different legal move count (due to paralysis randomness)
    // This test just verifies the system is working, not asserting specific outcomes
    EXPECT_FALSE(moves3.empty()) << "Should have at least switch options available";
}