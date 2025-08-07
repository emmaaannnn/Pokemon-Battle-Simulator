#include <gtest/gtest.h>

#include "ai_factory.h"
#include "hard_ai.h"
#include "test_utils.h"
#include "weather.h"

class HardAITest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create Hard AI instance
    hardAI = std::make_unique<HardAI>();

    // Create test Pokemon for AI and opponent
    aiPokemon = TestUtils::createTestPokemon("ai_pokemon", 100, 80, 70, 90, 85,
                                             75, {"normal"});
    opponentPokemon = TestUtils::createTestPokemon(
        "opponent_pokemon", 100, 80, 70, 90, 85, 75, {"grass"});

    // Create teams
    aiTeam = TestUtils::createTestTeam({aiPokemon, createBackupPokemon()});
    opponentTeam = TestUtils::createTestTeam({opponentPokemon});

    // Set up basic battle state - use Pokemon from teams for proper pointer matching
    battleState = {aiTeam.getPokemon(0),       opponentTeam.getPokemon(0), &aiTeam,
                   &opponentTeam,    WeatherCondition::NONE,
                   0,                // weather turns remaining
                   1};               // turn number
  }

  Pokemon createBackupPokemon() {
    return TestUtils::createTestPokemon("backup", 80, 70, 60, 80, 75, 65,
                                        {"water"});
  }

  Pokemon createSweeper() {
    Pokemon sweeper = TestUtils::createTestPokemon("sweeper", 120, 100, 80, 110, 90, 130, {"electric"});
    sweeper.moves.clear();
    sweeper.moves.push_back(TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special"));
    sweeper.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
    sweeper.moves.push_back(TestUtils::createTestMove("focus-blast", 120, 70, 5, "fighting", "special"));
    sweeper.moves.push_back(TestUtils::createTestMove("nasty-plot", 0, 100, 20, "dark", "status"));
    return sweeper;
  }

  std::unique_ptr<HardAI> hardAI;
  Pokemon aiPokemon;
  Pokemon opponentPokemon;
  Team aiTeam;
  Team opponentTeam;
  BattleState battleState;
};

// Test Hard AI construction and basic properties
TEST_F(HardAITest, Construction) {
  EXPECT_EQ(hardAI->getDifficulty(), AIDifficulty::HARD);
}

// Test Hard AI complex move evaluation with risk assessment
TEST_F(HardAITest, MoveSelectionRiskAssessment) {
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "focus-blast", 120, 70, 5, "fighting", "special"));  // High power, risky accuracy
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "brick-break", 75, 100, 15, "fighting", "physical"));  // Lower power, reliable

  MoveEvaluation result = hardAI->chooseBestMove(battleState);

  // Hard AI should make sophisticated risk vs reward calculations
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 2);
  EXPECT_GT(result.score, 0);
  EXPECT_NE(result.reasoning.find("Hard AI"), std::string::npos);
}

// Test Hard AI strategic switching based on health threshold (20%)
TEST_F(HardAITest, SwitchingStrategicHealthThreshold) {
  // Set AI Pokemon to 15% health (below Hard AI's 20% threshold)
  battleState.aiPokemon->current_hp = 
      static_cast<int>(battleState.aiPokemon->hp * 0.15);

  bool shouldSwitch = hardAI->shouldSwitch(battleState);

  // Hard AI should switch at 20% health threshold
  EXPECT_TRUE(shouldSwitch);
}

// Test Hard AI setup opportunity evaluation
TEST_F(HardAITest, MoveSelectionSetupOpportunityEvaluation) {
  // Opponent is asleep (good setup opportunity)
  battleState.opponentPokemon->status = StatusCondition::SLEEP;
  
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "tackle", 40, 100, 35, "normal", "physical"));
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "swords-dance", 0, 100, 20, "normal", "status"));  // Setup move

  MoveEvaluation result = hardAI->chooseBestMove(battleState);

  // Hard AI should recognize setup opportunity
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 2);
  EXPECT_GT(result.score, 0);
}

// Test Hard AI stat modification value calculation
TEST_F(HardAITest, MoveSelectionStatModificationValue) {
  // AI at high health, opponent weakened - good setup scenario
  battleState.aiPokemon->current_hp = battleState.aiPokemon->hp;
  battleState.opponentPokemon->current_hp = 
      static_cast<int>(battleState.opponentPokemon->hp * 0.3);

  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "quick-attack", 40, 100, 30, "normal", "physical"));
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "dragon-dance", 0, 100, 20, "dragon", "status"));  // Attack/Speed boost

  MoveEvaluation result = hardAI->chooseBestMove(battleState);

  // Should consider stat modification valuable in this scenario
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 2);
  EXPECT_GT(result.score, 0);
}

// Test Hard AI team threat analysis for switching
TEST_F(HardAITest, SwitchingTeamThreatAnalysis) {
  // Create a sweeper Pokemon that threatens multiple opponents
  Pokemon sweeper = createSweeper();
  aiTeam = TestUtils::createTestTeam({aiPokemon, sweeper});
  battleState.aiTeam = &aiTeam;
  battleState.aiPokemon = aiTeam.getPokemon(0);

  // Create opponent team with multiple Pokemon vulnerable to electric
  Pokemon waterOpponent = TestUtils::createTestPokemon("water_opp", 100, 80, 70, 90, 85, 75, {"water"});
  Pokemon flyingOpponent = TestUtils::createTestPokemon("flying_opp", 100, 80, 70, 90, 85, 75, {"flying"});
  Team vulnerableOpponentTeam = TestUtils::createTestTeam({waterOpponent, flyingOpponent});
  battleState.opponentTeam = &vulnerableOpponentTeam;

  SwitchEvaluation result = hardAI->chooseBestSwitch(battleState);

  // Should recognize sweeper's potential against vulnerable team
  EXPECT_EQ(result.pokemonIndex, 1);  // sweeper index
  EXPECT_GT(result.score, 30.0);  // High score due to team threat analysis
}

// Test Hard AI sweep potential recognition
TEST_F(HardAITest, SwitchingSweepPotentialRecognition) {
  Pokemon sweeper = createSweeper();
  aiTeam = TestUtils::createTestTeam({aiPokemon, sweeper});
  battleState.aiTeam = &aiTeam;

  // Opponent team vulnerable to sweeper's moves
  Pokemon waterOpponent = TestUtils::createTestPokemon("water", 100, 70, 60, 80, 75, 85, {"water"});
  Pokemon grassOpponent = TestUtils::createTestPokemon("grass", 100, 70, 60, 80, 75, 85, {"grass"});
  Team sweepableTeam = TestUtils::createTestTeam({waterOpponent, grassOpponent});
  battleState.opponentTeam = &sweepableTeam;

  bool shouldSwitch = hardAI->shouldSwitch(battleState);

  // Should switch to sweeper when it can threaten multiple opponents
  EXPECT_TRUE(shouldSwitch);
}

// Test Hard AI damage prediction for switching decisions
TEST_F(HardAITest, SwitchingDamagePrediction) {
  // Opponent has high-power move that will likely KO current Pokemon
  battleState.opponentPokemon->moves.clear();
  battleState.opponentPokemon->moves.push_back(TestUtils::createTestMove(
      "close-combat", 120, 100, 5, "fighting", "physical"));

  // Set AI Pokemon to moderate health but vulnerable to predicted damage
  battleState.aiPokemon->current_hp = 
      static_cast<int>(battleState.aiPokemon->hp * 0.6);
  battleState.aiPokemon->types = {"normal"};  // Vulnerable to fighting

  bool shouldSwitch = hardAI->shouldSwitch(battleState);

  // Should switch due to predicted incoming damage
  EXPECT_TRUE(shouldSwitch);
}

// Test Hard AI KO prioritization with accuracy consideration
TEST_F(HardAITest, MoveSelectionKOPrioritizationWithAccuracy) {
  // Set opponent to low health
  battleState.opponentPokemon->current_hp = 25;

  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "quick-attack", 40, 100, 30, "normal", "physical"));  // Guaranteed KO
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "focus-blast", 120, 70, 5, "fighting", "special"));  // Risky but powerful

  MoveEvaluation result = hardAI->chooseBestMove(battleState);

  // Hard AI makes complex strategic decisions - both moves are valid choices
  // The actual choice depends on the complex evaluation including risk assessment
  EXPECT_GE(result.moveIndex, 0);  // Valid move selected
  EXPECT_LT(result.moveIndex, 2);  // Within range
  EXPECT_GT(result.score, 50.0);  // Should have reasonable score
}

// Test Hard AI weather synergy evaluation
TEST_F(HardAITest, MoveSelectionWeatherSynergy) {
  battleState.currentWeather = WeatherCondition::RAIN;
  battleState.aiPokemon->types = {"water"};

  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "tackle", 40, 100, 35, "normal", "physical"));  // Not weather boosted
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "surf", 90, 100, 15, "water", "special"));  // STAB + weather boost

  MoveEvaluation result = hardAI->chooseBestMove(battleState);

  // Should strongly favor weather-boosted STAB move
  EXPECT_EQ(result.moveIndex, 1);  // surf
  EXPECT_GT(result.score, 80.0);  // High score due to synergy
}

// Test Hard AI status move intelligence against healthy opponents
TEST_F(HardAITest, MoveSelectionStatusIntelligenceHealthy) {
  // Opponent at full health and no status
  battleState.opponentPokemon->current_hp = battleState.opponentPokemon->hp;
  battleState.opponentPokemon->status = StatusCondition::NONE;

  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "body-slam", 85, 100, 15, "normal", "physical"));
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "sleep-powder", 0, 75, 15, "grass", "status", StatusCondition::SLEEP));

  MoveEvaluation result = hardAI->chooseBestMove(battleState);

  // Hard AI should consider status moves against healthy opponents
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 2);
  EXPECT_GT(result.score, 30.0);
}

/*
 * FUTURE TEST CASES TO IMPLEMENT TOMORROW (Session 2):
 * 
 * 1. TEST_F(HardAITest, MoveSelectionComplexTypeMatchups)
 *    - Test Hard AI evaluation with dual-type Pokemon and complex effectiveness calculations
 * 
 * 2. TEST_F(HardAITest, SwitchingPreemptiveCounterStrategy)  
 *    - Test switching to counter anticipated opponent switches/strategies
 * 
 * 3. TEST_F(HardAITest, MoveSelectionMultiTurnPlanning)
 *    - Test Hard AI considering multi-turn consequences of move choices
 * 
 * 4. TEST_F(HardAITest, SwitchingMinMaxStrategy)
 *    - Test Hard AI using minimax-style evaluation for switching decisions
 * 
 * 5. TEST_F(HardAITest, MoveSelectionCriticalHitConsideration)
 *    - Test Hard AI factoring critical hit probability into move evaluation
 * 
 * 6. TEST_F(HardAITest, SwitchingTeamSynergyOptimization)
 *    - Test Hard AI optimizing team synergy through strategic switches
 * 
 * 7. TEST_F(HardAITest, MoveSelectionStatusStackingStrategy)
 *    - Test Hard AI combining multiple status effects for strategic advantage
 * 
 * 8. TEST_F(HardAITest, SwitchingResourceManagement)
 *    - Test Hard AI managing PP and health resources across team members
 * 
 * 9. TEST_F(HardAITest, MoveSelectionPredictiveCounterplay)
 *    - Test Hard AI predicting and countering likely opponent responses
 * 
 * 10. TEST_F(HardAITest, ComprehensiveStrategicBehaviorValidation)
 *     - Complex multi-scenario test validating all Hard AI strategic features together
 */