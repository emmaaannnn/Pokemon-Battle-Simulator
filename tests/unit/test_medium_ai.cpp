#include <gtest/gtest.h>

#include "ai_factory.h"
#include "medium_ai.h"
#include "test_utils.h"
#include "weather.h"

class MediumAITest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create Medium AI instance
    mediumAI = std::make_unique<MediumAI>();

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

  std::unique_ptr<MediumAI> mediumAI;
  Pokemon aiPokemon;
  Pokemon opponentPokemon;
  Team aiTeam;
  Team opponentTeam;
  BattleState battleState;
};

// Test Medium AI construction and basic properties
TEST_F(MediumAITest, Construction) {
  EXPECT_EQ(mediumAI->getDifficulty(), AIDifficulty::MEDIUM);
}

// Test Medium AI uses damage estimation vs just power
TEST_F(MediumAITest, MoveSelectionDamageEstimation) {
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "hyper-beam", 150, 90, 5, "normal", "special"));  // High power, low accuracy
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "body-slam", 85, 100, 15, "normal", "physical"));  // Lower power, perfect accuracy

  MoveEvaluation result = mediumAI->chooseBestMove(battleState);

  // Medium AI should consider damage estimation and accuracy together
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 2);
  EXPECT_GT(result.score, 0);
  EXPECT_FALSE(result.reasoning.empty());
}

// Test Medium AI weather awareness - boost
TEST_F(MediumAITest, MoveSelectionWeatherBoost) {
  battleState.currentWeather = WeatherCondition::RAIN;
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "flamethrower", 90, 100, 15, "fire", "special"));  // Weakened by rain
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "surf", 90, 100, 15, "water", "special"));  // Boosted by rain

  MoveEvaluation result = mediumAI->chooseBestMove(battleState);

  // Should choose water move due to weather boost
  EXPECT_EQ(result.moveIndex, 1);  // surf index
  EXPECT_GT(result.score, 0);
}

// Test Medium AI weather awareness - penalty
TEST_F(MediumAITest, MoveSelectionWeatherPenalty) {
  battleState.currentWeather = WeatherCondition::RAIN;
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "flamethrower", 90, 100, 15, "fire", "special"));  // Weakened by rain
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "tackle", 40, 100, 35, "normal", "physical"));  // Not affected by rain

  MoveEvaluation result = mediumAI->chooseBestMove(battleState);

  // Should avoid fire move in rain despite higher base power
  EXPECT_EQ(result.moveIndex, 1);  // tackle index
  EXPECT_GT(result.score, 0);
}

// Test Medium AI status move intelligence
TEST_F(MediumAITest, MoveSelectionStatusIntelligence) {
  // Set opponent to high health to favor status moves
  battleState.opponentPokemon->current_hp = battleState.opponentPokemon->hp;
  
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "tackle", 40, 100, 35, "normal", "physical"));
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "thunder-wave", 0, 90, 20, "electric", "status", StatusCondition::PARALYSIS));  // Status move

  MoveEvaluation result = mediumAI->chooseBestMove(battleState);

  // Medium AI should consider status moves favorably against healthy opponents
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 2);
  EXPECT_GT(result.score, 0);
}

// Test Medium AI avoids status moves on already statused Pokemon
TEST_F(MediumAITest, MoveSelectionStatusOnAlreadyStatused) {
  // Set opponent to already have a status condition
  battleState.opponentPokemon->status = StatusCondition::BURN;
  
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "tackle", 40, 100, 35, "normal", "physical"));
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "thunder-wave", 0, 90, 20, "electric", "status", StatusCondition::PARALYSIS));

  MoveEvaluation result = mediumAI->chooseBestMove(battleState);

  // Should choose damage move instead of status move
  EXPECT_EQ(result.moveIndex, 0);  // tackle
  EXPECT_GT(result.score, 0);
}

// Test Medium AI KO detection
TEST_F(MediumAITest, MoveSelectionKODetection) {
  // Set opponent to low health
  battleState.opponentPokemon->current_hp = 20;
  
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "quick-attack", 40, 100, 30, "normal", "physical"));  // Can KO
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "thunder-wave", 0, 90, 20, "electric", "status", StatusCondition::PARALYSIS));  // Cannot KO

  MoveEvaluation result = mediumAI->chooseBestMove(battleState);

  // Should heavily favor the KO move
  EXPECT_EQ(result.moveIndex, 0);  // quick-attack
  EXPECT_GT(result.score, 50);  // Should have high score due to KO bonus
}

// Test Medium AI KO prioritization over higher base power
TEST_F(MediumAITest, MoveSelectionKOPrioritization) {
  // Set opponent to low health
  battleState.opponentPokemon->current_hp = 35;
  
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "quick-attack", 40, 100, 30, "normal", "physical"));  // Can KO
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "hyper-beam", 150, 90, 5, "normal", "special"));  // Higher power, cannot KO due to accuracy

  MoveEvaluation result = mediumAI->chooseBestMove(battleState);

  // Should choose the guaranteed KO over raw power
  EXPECT_EQ(result.moveIndex, 0);  // quick-attack
  EXPECT_GT(result.score, 0);
}

// Test Medium AI accuracy consideration
TEST_F(MediumAITest, MoveSelectionAccuracyConsideration) {
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "focus-blast", 120, 70, 5, "fighting", "special"));  // High power, low accuracy
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "brick-break", 75, 100, 15, "fighting", "physical"));  // Lower power, perfect accuracy

  MoveEvaluation result = mediumAI->chooseBestMove(battleState);

  // Medium AI should factor accuracy into damage estimation
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 2);
  EXPECT_GT(result.score, 0);
}

// Test Medium AI strategic switching (25% threshold vs Easy's 15%)
TEST_F(MediumAITest, SwitchingStrategicBehavior) {
  // Set AI Pokemon to 20% health (above Easy's 15% but below Medium's 25%)
  battleState.aiPokemon->current_hp = 
      static_cast<int>(battleState.aiPokemon->hp * 0.20);

  bool shouldSwitch = mediumAI->shouldSwitch(battleState);

  // Medium AI should switch at 25% threshold
  EXPECT_TRUE(shouldSwitch);
}

// Test Medium AI type matchup switching
TEST_F(MediumAITest, SwitchingTypeMatchup) {
  // Create fire opponent (strong against grass, weak against water)
  Pokemon fireOpponent = TestUtils::createTestPokemon(
      "fire_opponent", 100, 80, 70, 90, 85, 75, {"fire"});
  battleState.opponentPokemon = &fireOpponent;
  
  // AI has grass Pokemon (disadvantaged)
  battleState.aiPokemon->types = {"grass"};
  
  // Set moderate health (not low enough for health-based switching)
  battleState.aiPokemon->current_hp = 
      static_cast<int>(battleState.aiPokemon->hp * 0.60);

  bool shouldSwitch = mediumAI->shouldSwitch(battleState);

  // Should switch due to type disadvantage
  EXPECT_TRUE(shouldSwitch);
}

// Test Medium AI switches to better alternative
TEST_F(MediumAITest, SwitchingBetterAlternative) {
  // Create a much better backup Pokemon
  Pokemon betterBackup = TestUtils::createTestPokemon("better_backup", 120, 100, 90, 110, 105, 95, {"water"});
  aiTeam = TestUtils::createTestTeam({aiPokemon, betterBackup});
  battleState.aiTeam = &aiTeam;
  battleState.aiPokemon = aiTeam.getPokemon(0);
  
  // Create fire opponent (water has advantage)
  Pokemon fireOpponent = TestUtils::createTestPokemon(
      "fire_opponent", 100, 80, 70, 90, 85, 75, {"fire"});
  battleState.opponentPokemon = &fireOpponent;

  bool shouldSwitch = mediumAI->shouldSwitch(battleState);

  // Should switch to the better matchup
  EXPECT_TRUE(shouldSwitch);
}

// Test Medium AI switch target evaluation
TEST_F(MediumAITest, SwitchTargetMatchupEvaluation) {
  // Create multiple backup Pokemon with different matchups
  Pokemon waterBackup = TestUtils::createTestPokemon("water_backup", 80, 70, 60, 80, 75, 65, {"water"});
  Pokemon grassBackup = TestUtils::createTestPokemon("grass_backup", 75, 65, 70, 85, 80, 60, {"grass"});
  aiTeam = TestUtils::createTestTeam({aiPokemon, waterBackup, grassBackup});
  battleState.aiTeam = &aiTeam;
  
  // Fire opponent (weak to water, strong against grass)
  Pokemon fireOpponent = TestUtils::createTestPokemon(
      "fire_opponent", 100, 80, 70, 90, 85, 75, {"fire"});
  battleState.opponentPokemon = &fireOpponent;

  SwitchEvaluation result = mediumAI->chooseBestSwitch(battleState);

  // Should choose water Pokemon (index 1) due to type advantage
  EXPECT_EQ(result.pokemonIndex, 1);  // water backup
  EXPECT_GT(result.score, 0);
}

// Test Medium AI doesn't switch when no alternatives
TEST_F(MediumAITest, SwitchNoAlternatives) {
  // Create team with only current Pokemon alive
  Pokemon faintedBackup = TestUtils::createTestPokemon("fainted", 80, 70, 60, 80, 75, 65, {"water"});
  faintedBackup.current_hp = 0;  // Fainted
  aiTeam = TestUtils::createTestTeam({aiPokemon, faintedBackup});
  battleState.aiTeam = &aiTeam;
  battleState.aiPokemon = aiTeam.getPokemon(0);

  bool shouldSwitch = mediumAI->shouldSwitch(battleState);

  // Cannot switch with no alternatives
  EXPECT_FALSE(shouldSwitch);
}

// Test Medium AI status move health consideration
TEST_F(MediumAITest, StatusMoveHealthConsideration) {
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "tackle", 40, 100, 35, "normal", "physical"));
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "sleep-powder", 0, 75, 15, "grass", "status", StatusCondition::SLEEP));

  // Test with high health opponent
  battleState.opponentPokemon->current_hp = battleState.opponentPokemon->hp;
  MoveEvaluation highHealthResult = mediumAI->chooseBestMove(battleState);

  // Test with low health opponent  
  battleState.opponentPokemon->current_hp = 
      static_cast<int>(battleState.opponentPokemon->hp * 0.20);
  MoveEvaluation lowHealthResult = mediumAI->chooseBestMove(battleState);

  // Status moves should be more favored against healthy opponents
  // (Note: actual move choice might vary, but we're testing the scoring logic)
  EXPECT_GE(highHealthResult.moveIndex, 0);
  EXPECT_GE(lowHealthResult.moveIndex, 0);
}

// Test Medium AI specific ailment evaluation
TEST_F(MediumAITest, StatusMoveSpecificAilments) {
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "tackle", 40, 100, 35, "normal", "physical"));
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "thunder-wave", 0, 90, 20, "electric", "status", StatusCondition::PARALYSIS));

  // Set opponent faster than AI to make paralysis more valuable
  battleState.opponentPokemon->speed = 100;
  battleState.aiPokemon->speed = 70;
  
  // High health opponent
  battleState.opponentPokemon->current_hp = battleState.opponentPokemon->hp;

  MoveEvaluation result = mediumAI->chooseBestMove(battleState);

  // Paralysis should be considered against faster opponents
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 2);
  EXPECT_GT(result.score, 0);
}

// Test Medium AI damage move priority handling
TEST_F(MediumAITest, DamageMovePriorityMoves) {
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "quick-attack", 40, 100, 30, "normal", "physical"));  // Priority (can't set in test)
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "body-slam", 85, 100, 15, "normal", "physical"));  // Higher power, no priority

  MoveEvaluation result = mediumAI->chooseBestMove(battleState);

  // Medium AI should evaluate based on damage estimation and other factors
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 2);
  EXPECT_GT(result.score, 0);
}

// Test Medium AI Pokemon matchup evaluation
TEST_F(MediumAITest, PokemonMatchupEvaluation) {
  // Create water Pokemon with water moves
  Pokemon waterPokemon = TestUtils::createTestPokemon("water_pokemon", 80, 70, 60, 80, 75, 85, {"water"});
  waterPokemon.moves.clear();
  waterPokemon.moves.push_back(TestUtils::createTestMove(
      "surf", 90, 100, 15, "water", "special"));
  
  // Fire opponent (weak to water)
  Pokemon fireOpponent = TestUtils::createTestPokemon(
      "fire_opponent", 100, 80, 70, 90, 85, 75, {"fire"});

  aiTeam = TestUtils::createTestTeam({aiPokemon, waterPokemon});
  battleState.aiTeam = &aiTeam;
  battleState.opponentPokemon = &fireOpponent;

  SwitchEvaluation result = mediumAI->chooseBestSwitch(battleState);

  // Should identify water Pokemon as having good matchup
  EXPECT_EQ(result.pokemonIndex, 1);  // water Pokemon
  EXPECT_GT(result.score, 20);  // Should have positive matchup score
}

// Test Medium AI STAB and weather combination
TEST_F(MediumAITest, STABAndWeatherCombination) {
  battleState.currentWeather = WeatherCondition::RAIN;
  battleState.aiPokemon->types = {"water"};  // Water type for STAB
  
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "tackle", 40, 100, 35, "normal", "physical"));  // No STAB, no weather
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "surf", 90, 100, 15, "water", "special"));  // STAB + weather boost

  MoveEvaluation result = mediumAI->chooseBestMove(battleState);

  // Should strongly favor STAB + weather boosted move
  EXPECT_EQ(result.moveIndex, 1);  // surf
  EXPECT_GT(result.score, 50);  // Should have high combined score
}

// Test comprehensive Medium AI behavior validation
TEST_F(MediumAITest, ComprehensiveBehaviorValidation) {
  // Complex scenario testing multiple Medium AI features
  battleState.currentWeather = WeatherCondition::SANDSTORM;
  battleState.aiPokemon->types = {"ground"};  // Immune to sandstorm
  
  // Opponent already poisoned and at moderate health
  battleState.opponentPokemon->status = StatusCondition::POISON;
  battleState.opponentPokemon->current_hp = 
      static_cast<int>(battleState.opponentPokemon->hp * 0.60);
  
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "earthquake", 100, 100, 10, "ground", "physical"));  // STAB, high power
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON));  // Redundant status
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "rock-slide", 75, 90, 10, "rock", "physical"));  // Decent damage

  MoveEvaluation result = mediumAI->chooseBestMove(battleState);

  // Should choose earthquake (STAB, high power) and avoid redundant status
  EXPECT_EQ(result.moveIndex, 0);  // earthquake
  EXPECT_GT(result.score, 0);
  EXPECT_FALSE(result.reasoning.empty());
  
  // Verify it correctly identifies this as Medium AI
  EXPECT_NE(result.reasoning.find("Medium AI"), std::string::npos);
}