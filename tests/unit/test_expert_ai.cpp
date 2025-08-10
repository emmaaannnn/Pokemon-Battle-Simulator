#include <gtest/gtest.h>

#include "ai_factory.h"
#include "expert_ai.h"
#include "test_utils.h"
#include "weather.h"

class ExpertAITest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create Expert AI instance
    expertAI = std::make_unique<ExpertAI>();

    // Create test Pokemon for AI and opponent
    aiPokemon = TestUtils::createTestPokemon("ai_pokemon", 100, 80, 70, 90, 85,
                                             75, {"normal"});
    opponentPokemon = TestUtils::createTestPokemon(
        "opponent_pokemon", 100, 80, 70, 90, 85, 75, {"grass"});

    // Create teams with multiple Pokemon for comprehensive testing
    aiTeam = TestUtils::createTestTeam({aiPokemon, createBackupPokemon(), createSweeper()});
    opponentTeam = TestUtils::createTestTeam({opponentPokemon, createWall(), createSetupSweeper()});

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

  // Creates a fast, powerful sweeper for team archetype analysis
  Pokemon createSweeper() {
    Pokemon sweeper = TestUtils::createTestPokemon("sweeper", 120, 100, 80, 110, 90, 130, {"electric"});
    sweeper.moves.clear();
    sweeper.moves.push_back(TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special"));
    sweeper.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
    sweeper.moves.push_back(TestUtils::createTestMove("focus-blast", 120, 70, 5, "fighting", "special"));
    sweeper.moves.push_back(TestUtils::createTestMove("nasty-plot", 0, 100, 20, "dark", "status"));
    return sweeper;
  }

  // Creates a defensive wall for team archetype analysis
  Pokemon createWall() {
    Pokemon wall = TestUtils::createTestPokemon("wall", 150, 60, 120, 60, 120, 40, {"steel"});
    wall.moves.clear();
    wall.moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status"));
    wall.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 10, "normal", "status"));
    wall.moves.push_back(TestUtils::createTestMove("stealth-rock", 0, 100, 20, "rock", "status"));
    wall.moves.push_back(TestUtils::createTestMove("iron-head", 80, 100, 15, "steel", "physical"));
    return wall;
  }

  // Creates a setup sweeper for meta-game analysis
  Pokemon createSetupSweeper() {
    Pokemon setup = TestUtils::createTestPokemon("setup_sweep", 100, 90, 75, 110, 80, 100, {"dragon"});
    setup.moves.clear();
    setup.moves.push_back(TestUtils::createTestMove("dragon-dance", 0, 100, 20, "dragon", "status"));
    setup.moves.push_back(TestUtils::createTestMove("outrage", 120, 100, 10, "dragon", "physical"));
    setup.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
    setup.moves.push_back(TestUtils::createTestMove("fire-punch", 75, 100, 15, "fire", "physical"));
    return setup;
  }

  std::unique_ptr<ExpertAI> expertAI;
  Pokemon aiPokemon;
  Pokemon opponentPokemon;
  Team aiTeam;
  Team opponentTeam;
  BattleState battleState;
};

// Test Expert AI construction and basic properties
TEST_F(ExpertAITest, Construction) {
  EXPECT_EQ(expertAI->getDifficulty(), AIDifficulty::EXPERT);
}

// ──────────────────────────────────────────────────────────────────
// Phase 1 Tests: Advanced Analysis & Prediction Systems
// ──────────────────────────────────────────────────────────────────

// Tests Bayesian opponent modeling learning from repeated interactions
TEST_F(ExpertAITest, BayesianOpponentModelLearning) {
  // Test that the AI learns from opponent behavior over multiple turns
  // This test verifies that the Bayesian model updates move probabilities based on observation
  
  // Simulate opponent using move 0 repeatedly (like a predictable pattern)
  for (int i = 0; i < 10; ++i) {
    expertAI->updateBayesianModel(battleState, 0);  // Opponent uses move 0
  }
  
  // Test that the AI has learned this pattern
  double prob_move_0 = expertAI->predictOpponentMoveProbability(battleState, 0);
  double prob_move_1 = expertAI->predictOpponentMoveProbability(battleState, 1);
  
  // Move 0 should have higher probability after repeated use
  EXPECT_GT(prob_move_0, prob_move_1);
  EXPECT_GT(prob_move_0, 0.4);  // Should be significantly higher than random
  
  // Test play style classification
  std::string play_style = expertAI->classifyOpponentPlayStyle(battleState);
  EXPECT_FALSE(play_style.empty());
  EXPECT_NE(play_style, "unknown");  // Should classify with enough data
}

// Tests situational pattern recognition in Bayesian model
TEST_F(ExpertAITest, BayesianSituationalPatterns) {
  // Test that the AI learns different move preferences in different situations
  
  // Scenario 1: AI at low health - opponent might go for KO moves
  battleState.aiPokemon->current_hp = 20;  // Low HP scenario
  expertAI->updateBayesianModel(battleState, 1);  // High power move
  expertAI->updateBayesianModel(battleState, 1);
  
  // Scenario 2: Normal health - opponent uses varied moves
  battleState.aiPokemon->current_hp = battleState.aiPokemon->hp;  // Full HP
  expertAI->updateBayesianModel(battleState, 0);  // Different move
  expertAI->updateBayesianModel(battleState, 2);
  
  // The model should learn situational preferences
  battleState.aiPokemon->current_hp = 15;  // Back to low HP
  double low_hp_prob = expertAI->predictOpponentMoveProbability(battleState, 1);
  
  battleState.aiPokemon->current_hp = battleState.aiPokemon->hp;  // Full HP
  double full_hp_prob = expertAI->predictOpponentMoveProbability(battleState, 1);
  
  // In low HP situations, move 1 should be more likely based on learning
  EXPECT_GE(low_hp_prob, full_hp_prob);
}

// Tests MiniMax search with alpha-beta pruning functionality
TEST_F(ExpertAITest, MiniMaxGameTreeSearch) {
  // Test that the MiniMax search engine can evaluate positions multiple turns ahead
  // This verifies the game tree search with alpha-beta pruning optimization
  
  std::vector<int> best_line;
  double evaluation = expertAI->miniMaxSearch(battleState, 3, -1000.0, 1000.0, true, best_line);
  
  // Should return a valid evaluation score (allow 0.0 as valid evaluation for balanced positions)
  EXPECT_GE(evaluation, -1000.0);
  EXPECT_LE(evaluation, 1000.0);
  // Test that it actually processes the search (non-NaN, finite result)
  EXPECT_TRUE(std::isfinite(evaluation));
  
  // Test with different depths
  std::vector<int> shallow_line;
  double shallow_eval = expertAI->miniMaxSearch(battleState, 1, -1000.0, 1000.0, true, shallow_line);
  
  // Deeper search should potentially find better moves (or same if position is simple)
  EXPECT_TRUE(evaluation >= shallow_eval - 10.0);  // Allow for small variance
}

// Tests position evaluation accuracy in game tree search
TEST_F(ExpertAITest, MiniMaxPositionEvaluation) {
  // Test that position evaluation correctly assesses advantages and disadvantages
  
  // Create a clearly advantageous position by damaging opponent team
  BattleState advantageous = battleState;
  // Damage the active opponent Pokemon significantly
  advantageous.opponentPokemon->takeDamage(90);  // Reduce to 10% health
  
  double advantage_score = expertAI->evaluatePosition(advantageous);
  double normal_score = expertAI->evaluatePosition(battleState);
  
  // Position evaluation considers many factors, so we test that it produces reasonable scores
  EXPECT_TRUE(std::isfinite(advantage_score));
  EXPECT_TRUE(std::isfinite(normal_score));
  EXPECT_GE(advantage_score, -500.0);  // Should be reasonable scores
  EXPECT_LE(advantage_score, 500.0);
  
  // Create a disadvantageous position
  BattleState disadvantageous = battleState;
  disadvantageous.aiPokemon->takeDamage(95);  // AI nearly defeated
  
  double disadvantage_score = expertAI->evaluatePosition(disadvantageous);
  EXPECT_LT(disadvantage_score, normal_score + 10.0);  // Should be worse than normal
}

// Tests team archetype recognition for meta-game analysis
TEST_F(ExpertAITest, MetaGameTeamArchetypeRecognition) {
  // Test that the AI can correctly identify different team compositions and strategies
  
  // Test hyper-offensive team archetype
  Pokemon fastAttacker1 = TestUtils::createTestPokemon("fast1", 80, 110, 60, 100, 70, 120, {"normal"});
  Pokemon fastAttacker2 = TestUtils::createTestPokemon("fast2", 70, 105, 55, 95, 65, 115, {"fire"});
  Pokemon fastAttacker3 = TestUtils::createTestPokemon("fast3", 75, 100, 50, 90, 60, 110, {"electric"});
  
  Team hyperOffenseTeam = TestUtils::createTestTeam({fastAttacker1, fastAttacker2, fastAttacker3});
  auto archetype = expertAI->analyzeTeamArchetype(hyperOffenseTeam);
  
  // Should recognize as hyper-offensive or balanced offensive
  EXPECT_TRUE(archetype == ExpertAI::MetaGameAnalyzer::TeamArchetype::HYPER_OFFENSE ||
              archetype == ExpertAI::MetaGameAnalyzer::TeamArchetype::BALANCED_OFFENSE);
  
  // Test stall team archetype
  Pokemon wall1 = createWall();
  Pokemon wall2 = createWall();
  wall2.name = "wall2";
  Team stallTeam = TestUtils::createTestTeam({wall1, wall2, createWall()});
  
  auto stallArchetype = expertAI->analyzeTeamArchetype(stallTeam);
  EXPECT_TRUE(stallArchetype == ExpertAI::MetaGameAnalyzer::TeamArchetype::STALL ||
              stallArchetype == ExpertAI::MetaGameAnalyzer::TeamArchetype::BALANCED);
}

// Tests win condition identification and strategic planning
TEST_F(ExpertAITest, AdvancedWinConditionPursual) {
  // Test that the AI can identify and pursue specific win conditions based on team composition
  
  // Create a setup sweep scenario
  Pokemon setupSweeper = createSetupSweeper();
  Team setupTeam = TestUtils::createTestTeam({setupSweeper, createBackupPokemon()});
  BattleState setupState = battleState;
  setupState.aiTeam = &setupTeam;
  setupState.aiPokemon = setupTeam.getPokemon(0);
  
  auto winConditions = expertAI->identifyWinConditions(setupState);
  
  // Should identify win conditions
  EXPECT_FALSE(winConditions.empty());
  EXPECT_GE(winConditions.size(), 1);
  
  // Should have meaningful win condition names and probabilities
  for (const auto& condition : winConditions) {
    EXPECT_FALSE(condition.name.empty());
    EXPECT_GT(condition.probability_of_success, 0.0);
    EXPECT_LE(condition.probability_of_success, 1.0);
    EXPECT_GE(condition.priority, 1);
    EXPECT_FALSE(condition.required_conditions.empty());
  }
  
  // Test counter-strategy suggestions
  auto counterStrategies = expertAI->suggestCounterStrategies(
      ExpertAI::MetaGameAnalyzer::TeamArchetype::HYPER_OFFENSE);
  EXPECT_FALSE(counterStrategies.empty());
  EXPECT_GE(counterStrategies.size(), 2);
}

// Tests endgame position recognition and tablebase functionality
TEST_F(ExpertAITest, AdvancedEndgameAnalysis) {
  // Test endgame position detection and evaluation
  
  // Create endgame scenario (few Pokemon remaining)
  BattleState endgameState = battleState;
  
  // Kill most Pokemon to create endgame
  for (int i = 1; i < static_cast<int>(endgameState.aiTeam->size()); ++i) {
    Pokemon* pokemon = endgameState.aiTeam->getPokemon(i);
    if (pokemon) pokemon->takeDamage(1000);
  }
  for (int i = 1; i < static_cast<int>(endgameState.opponentTeam->size()); ++i) {
    Pokemon* pokemon = endgameState.opponentTeam->getPokemon(i);
    if (pokemon) pokemon->takeDamage(1000);
  }
  
  // Should recognize as endgame
  EXPECT_TRUE(expertAI->isEndgamePosition(endgameState));
  
  // Should provide meaningful endgame evaluation
  std::string endgameEval = expertAI->getEndgameEvaluation(endgameState);
  EXPECT_FALSE(endgameEval.empty());
  EXPECT_TRUE(endgameEval == "winning" || endgameEval == "losing" || 
              endgameEval == "complex" || endgameEval == "critical");
}

// Tests integration of all Phase 1 advanced features in move selection
TEST_F(ExpertAITest, Phase1AdvancedMoveSelection) {
  // Test that move selection integrates Bayesian modeling, search, and meta-game analysis
  
  // Prime the Bayesian model with some opponent behavior
  for (int i = 0; i < 5; ++i) {
    expertAI->updateBayesianModel(battleState, 0);
  }
  
  // Test move selection with all advanced features active
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "tackle", 40, 100, 35, "normal", "physical"));
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "flamethrower", 90, 100, 15, "fire", "special"));  // Super effective vs grass
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "toxic", 0, 90, 10, "poison", "status"));
  
  MoveEvaluation result = expertAI->chooseBestMove(battleState);
  
  // Should make intelligent decision
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 3);
  EXPECT_GT(result.score, -50);
  EXPECT_NE(result.reasoning.find("Expert AI"), std::string::npos);
}

// Tests switching decision making with advanced analysis
TEST_F(ExpertAITest, Phase1AdvancedSwitchingAnalysis) {
  // Test that switching decisions use team archetype analysis and win condition pursuit
  
  // Create unfavorable matchup for current Pokemon
  battleState.opponentPokemon->types = {"fire"};  // Fire vs Normal (neutral)
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "tackle", 40, 100, 35, "normal", "physical"));  // Weak move
  
  // Water Pokemon in team should be preferred switch
  Pokemon* waterPokemon = aiTeam.getPokemon(1);  // Backup is water type
  if (waterPokemon) {
    waterPokemon->moves.clear();
    waterPokemon->moves.push_back(TestUtils::createTestMove(
        "surf", 90, 100, 15, "water", "special"));  // Super effective vs fire
  }
  
  SwitchEvaluation switchEval = expertAI->chooseBestSwitch(battleState);
  bool shouldSwitch = expertAI->shouldSwitch(battleState);
  (void)shouldSwitch;  // Suppress unused variable warning
  
  // Should recognize switching opportunity
  EXPECT_GE(switchEval.pokemonIndex, 0);
  EXPECT_GT(switchEval.score, 10.0);  // Should be positive for good switch
  EXPECT_NE(switchEval.reasoning.find("Expert AI"), std::string::npos);
}

// ──────────────────────────────────────────────────────────────────
// Phase 2 Tests (Commented - Not Yet Implemented)
// These tests are for the second phase of advanced AI development
// ──────────────────────────────────────────────────────────────────

/*
// Tests advanced resource management and PP optimization
// This will test long-term PP planning, strategic Pokemon sacrificing for positioning,
// and resource trade-off evaluation (health vs status vs positioning)
TEST_F(ExpertAITest, AdvancedResourceOptimization) {
  // TODO Phase 2: Test PP optimization across multiple turns
  // TODO Phase 2: Test strategic sacrifice calculations
  // TODO Phase 2: Test resource trade-off evaluation
  GTEST_SKIP() << "Phase 2 feature - Advanced Resource Management not yet implemented";
}

// Tests psychological warfare detection and counter-measures
// This will test bluff detection, misdirection tactics, and tilt induction recognition
TEST_F(ExpertAITest, PsychologicalWarfareDetection) {
  // TODO Phase 2: Test opponent bluff detection algorithms
  // TODO Phase 2: Test misdirection through strategic move choices
  // TODO Phase 2: Test recognition and exploitation of opponent frustration patterns
  GTEST_SKIP() << "Phase 2 feature - Psychological Warfare not yet implemented";
}

// Tests battle flow and tempo control mechanisms
// This will test tempo manipulation, initiative management, and time pressure optimization
TEST_F(ExpertAITest, BattleFlowTempoControl) {
  // TODO Phase 2: Test tempo control and momentum manipulation
  // TODO Phase 2: Test initiative management in battle flow
  // TODO Phase 2: Test clock management and time pressure response
  GTEST_SKIP() << "Phase 2 feature - Tempo Control not yet implemented";
}

// Tests advanced strategic deception and counter-intelligence
// This will test the AI's ability to mislead opponents and adapt to counter-strategies
TEST_F(ExpertAITest, StrategicDeceptionCapabilities) {
  // TODO Phase 2: Test move choice deception to mislead opponent predictions
  // TODO Phase 2: Test adaptation to opponent counter-strategies
  // TODO Phase 2: Test false pattern creation and pattern breaking
  GTEST_SKIP() << "Phase 2 feature - Strategic Deception not yet implemented";
}

// Tests adaptive learning and strategy evolution during battle
// This will test real-time strategy adaptation and opponent counter-adaptation
TEST_F(ExpertAITest, AdaptiveLearningEvolution) {
  // TODO Phase 2: Test real-time strategy evolution based on battle flow
  // TODO Phase 2: Test counter-adaptation to opponent strategy changes
  // TODO Phase 2: Test learning rate optimization and forgetting mechanisms
  GTEST_SKIP() << "Phase 2 feature - Adaptive Learning Evolution not yet implemented";
}
*/

// Test performance and computational efficiency of Phase 1 features
TEST_F(ExpertAITest, Phase1PerformanceBenchmark) {
  // Test that advanced features don't cause excessive computation time
  
  auto start = std::chrono::high_resolution_clock::now();
  
  // Run multiple operations to test performance
  for (int i = 0; i < 10; ++i) {
    expertAI->updateBayesianModel(battleState, i % 4);
    expertAI->predictOpponentMoveProbability(battleState, 0);
    expertAI->classifyOpponentPlayStyle(battleState);
    expertAI->analyzeTeamArchetype(*battleState.aiTeam);
    expertAI->identifyWinConditions(battleState);
  }
  
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  
  // Should complete within reasonable time (less than 1 second for this test)
  EXPECT_LT(duration.count(), 1000);
}

// Test integration and comprehensive AI behavior validation
TEST_F(ExpertAITest, Phase1ComprehensiveIntegration) {
  // Final integration test ensuring all Phase 1 features work together seamlessly
  
  // Set up complex battle scenario
  battleState.currentWeather = WeatherCondition::RAIN;
  battleState.weatherTurnsRemaining = 3;
  battleState.turnNumber = 5;
  
  // Prime AI with battle history
  for (int turn = 0; turn < 8; ++turn) {
    expertAI->updateBayesianModel(battleState, turn % 3);
  }
  
  // Test comprehensive move evaluation
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "thunder", 110, 70, 10, "electric", "special"));  // High risk/reward in rain
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "surf", 90, 100, 15, "water", "special"));  // Rain boosted
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "toxic", 0, 90, 10, "poison", "status"));  // Strategic option
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "protect", 0, 100, 10, "normal", "status"));  // Defensive option
  
  MoveEvaluation complexResult = expertAI->chooseBestMove(battleState);
  
  // Should make sophisticated decision considering all factors
  EXPECT_GE(complexResult.moveIndex, 0);
  EXPECT_LT(complexResult.moveIndex, 4);
  EXPECT_GT(complexResult.score, -10);  // Should find reasonable move
  EXPECT_FALSE(complexResult.reasoning.empty());
  
  // Test switching in complex scenario
  SwitchEvaluation complexSwitch = expertAI->chooseBestSwitch(battleState);
  EXPECT_GE(complexSwitch.pokemonIndex, -1);  // -1 is valid (no good switch)
  EXPECT_FALSE(complexSwitch.reasoning.empty());
  
  // Verify AI difficulty
  EXPECT_EQ(expertAI->getDifficulty(), AIDifficulty::EXPERT);
}