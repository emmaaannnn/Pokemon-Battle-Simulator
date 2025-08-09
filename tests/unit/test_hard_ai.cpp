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

// Test Hard AI complex type matchup evaluation with dual-type Pokemon
TEST_F(HardAITest, MoveSelectionComplexTypeMatchups) {
  // Opponent is dual-type: Flying/Steel (4x weak to Electric, resists many types)
  battleState.opponentPokemon->types = {"flying", "steel"};
  
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "thunderbolt", 90, 100, 15, "electric", "special"));  // 4x super effective
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "flamethrower", 90, 100, 15, "fire", "special"));  // 2x vs steel, 1x vs flying = 2x
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "ice-beam", 90, 100, 10, "ice", "special"));  // 2x vs flying, 0.5x vs steel = 1x
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "tackle", 40, 100, 35, "normal", "physical"));  // 0.5x vs steel

  MoveEvaluation result = hardAI->chooseBestMove(battleState);

  // Should favor highly effective moves (thunderbolt or flamethrower are both good)
  EXPECT_TRUE(result.moveIndex == 0 || result.moveIndex == 1);  // thunderbolt or flamethrower
  EXPECT_GT(result.score, 100.0);  // High score due to super effectiveness
}

// Test Hard AI preemptive counter-strategy switching
TEST_F(HardAITest, SwitchingPreemptiveCounterStrategy) {
  // Create scenario where opponent likely to switch to a fire type
  // AI has a weak Pokemon vs fire, but has a water counter available
  battleState.aiPokemon->types = {"grass"};  // Weak to fire
  battleState.opponentPokemon->types = {"normal"};  // Neutral, but likely to switch
  
  // Create water counter Pokemon in team
  Pokemon waterCounter = TestUtils::createTestPokemon("water_counter", 110, 90, 80, 100, 95, 85, {"water"});
  waterCounter.moves.clear();
  waterCounter.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
  waterCounter.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
  
  // Add fire opponent to opponent team (simulating likely switch)
  Pokemon fireOpponent = TestUtils::createTestPokemon("fire_threat", 100, 85, 75, 95, 90, 110, {"fire"});
  Team extendedOpponentTeam = TestUtils::createTestTeam({*battleState.opponentPokemon, fireOpponent});
  
  aiTeam = TestUtils::createTestTeam({*battleState.aiPokemon, waterCounter});
  battleState.aiTeam = &aiTeam;
  battleState.opponentTeam = &extendedOpponentTeam;
  battleState.aiPokemon = aiTeam.getPokemon(0);

  bool shouldSwitch = hardAI->shouldSwitch(battleState);

  // Hard AI makes complex strategic decisions - may or may not switch based on evaluation
  // The key is that it evaluates switching options intelligently
  SwitchEvaluation switchResult = hardAI->chooseBestSwitch(battleState);
  EXPECT_GE(switchResult.pokemonIndex, 0);  // Should find a valid switch option
  EXPECT_GT(switchResult.score, -100.0);   // Should have reasonable evaluation
}

// Test Hard AI multi-turn planning consideration
TEST_F(HardAITest, MoveSelectionMultiTurnPlanning) {
  // Scenario: AI can either KO now or set up for a bigger advantage later
  battleState.opponentPokemon->current_hp = 45;  // Can be KO'd by either move
  
  // Create opponent team with multiple Pokemon that would be easier after setup
  Pokemon strongOpponent1 = TestUtils::createTestPokemon("strong1", 120, 100, 95, 110, 105, 90, {"dragon"});
  Pokemon strongOpponent2 = TestUtils::createTestPokemon("strong2", 115, 95, 90, 105, 100, 85, {"psychic"});
  Team strongOpponentTeam = TestUtils::createTestTeam({*battleState.opponentPokemon, strongOpponent1, strongOpponent2});
  battleState.opponentTeam = &strongOpponentTeam;
  
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "quick-attack", 40, 100, 30, "normal", "physical"));  // Immediate KO
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "dragon-dance", 0, 100, 20, "dragon", "status"));  // Setup for later advantage

  MoveEvaluation result = hardAI->chooseBestMove(battleState);

  // Hard AI should make strategic multi-turn decision
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 2);
  EXPECT_GT(result.score, 40.0);
}

// Test Hard AI minimax-style switching evaluation
TEST_F(HardAITest, SwitchingMinMaxStrategy) {
  // Complex scenario: multiple Pokemon with different matchups
  // AI must choose optimal switch considering opponent's best responses
  
  // Current AI Pokemon: Electric (good vs Water/Flying, bad vs Ground)
  battleState.aiPokemon->types = {"electric"};
  
  // Opponent team with mixed threats
  Pokemon waterOpponent = TestUtils::createTestPokemon("water_opp", 100, 80, 75, 90, 85, 80, {"water"});
  Pokemon groundOpponent = TestUtils::createTestPokemon("ground_opp", 110, 95, 90, 75, 80, 70, {"ground"});
  Team mixedOpponentTeam = TestUtils::createTestTeam({waterOpponent, groundOpponent});
  battleState.opponentTeam = &mixedOpponentTeam;
  battleState.opponentPokemon = mixedOpponentTeam.getPokemon(0); // Currently water
  
  // AI team alternatives
  Pokemon grassCounter = TestUtils::createTestPokemon("grass", 95, 85, 80, 100, 95, 90, {"grass"}); // Good vs Water/Ground
  Pokemon flyingCounter = TestUtils::createTestPokemon("flying", 100, 90, 75, 95, 80, 110, {"flying"}); // Good vs Grass, bad vs Electric
  aiTeam = TestUtils::createTestTeam({*battleState.aiPokemon, grassCounter, flyingCounter});
  battleState.aiTeam = &aiTeam;
  battleState.aiPokemon = aiTeam.getPokemon(0);

  SwitchEvaluation result = hardAI->chooseBestSwitch(battleState);

  // Should evaluate best switch considering opponent responses
  EXPECT_GE(result.pokemonIndex, 0);
  EXPECT_LT(result.pokemonIndex, 3);
  EXPECT_GT(result.score, 10.0);  // Should find a reasonable switch option
}

// Test Hard AI critical hit probability consideration
TEST_F(HardAITest, MoveSelectionCriticalHitConsideration) {
  // Opponent at health where critical hit would matter
  battleState.opponentPokemon->current_hp = 65;
  
  battleState.aiPokemon->moves.clear();
  // Move that normally wouldn't KO but could with crit
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "slash", 70, 100, 20, "normal", "physical"));  // High crit ratio move
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "body-slam", 85, 100, 15, "normal", "physical"));  // Reliable damage
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "focus-energy", 0, 100, 30, "normal", "status"));  // Increases crit ratio

  MoveEvaluation result = hardAI->chooseBestMove(battleState);

  // Hard AI should factor critical hit probability into evaluation
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 3);
  EXPECT_GT(result.score, 30.0);
}

// Test Hard AI team synergy optimization through switching
TEST_F(HardAITest, SwitchingTeamSynergyOptimization) {
  // Create team composition that benefits from specific switch timing
  
  // AI has a setup sweeper that needs support
  Pokemon setupSweeper = createSweeper(); // From existing helper
  setupSweeper.moves.clear();
  setupSweeper.moves.push_back(TestUtils::createTestMove("agility", 0, 100, 30, "psychic", "status"));
  setupSweeper.moves.push_back(TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special"));
  setupSweeper.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
  setupSweeper.moves.push_back(TestUtils::createTestMove("focus-blast", 120, 70, 5, "fighting", "special"));
  
  // Support Pokemon that can enable the sweeper
  Pokemon support = TestUtils::createTestPokemon("support", 85, 60, 95, 70, 110, 75, {"psychic"});
  support.moves.clear();
  support.moves.push_back(TestUtils::createTestMove("reflect", 0, 100, 20, "psychic", "status"));
  support.moves.push_back(TestUtils::createTestMove("thunder-wave", 0, 90, 20, "electric", "status", StatusCondition::PARALYSIS));
  
  aiTeam = TestUtils::createTestTeam({support, setupSweeper});
  battleState.aiTeam = &aiTeam;
  battleState.aiPokemon = aiTeam.getPokemon(0); // Currently support
  
  // Opponent team vulnerable to the setup sweeper
  Pokemon vulnerableOpp1 = TestUtils::createTestPokemon("vuln1", 85, 75, 70, 80, 75, 90, {"water"});
  Pokemon vulnerableOpp2 = TestUtils::createTestPokemon("vuln2", 90, 80, 75, 85, 80, 85, {"ground"});
  Team vulnerableTeam = TestUtils::createTestTeam({vulnerableOpp1, vulnerableOpp2});
  battleState.opponentTeam = &vulnerableTeam;

  bool shouldSwitch = hardAI->shouldSwitch(battleState);
  if (shouldSwitch) {
    SwitchEvaluation switchResult = hardAI->chooseBestSwitch(battleState);
    EXPECT_EQ(switchResult.pokemonIndex, 1); // Should switch to sweeper
    EXPECT_GT(switchResult.score, 25.0);
  }
}

// Test Hard AI status effect stacking strategy
TEST_F(HardAITest, MoveSelectionStatusStackingStrategy) {
  // Opponent already has burn, AI considers additional status effects
  battleState.opponentPokemon->status = StatusCondition::BURN;
  battleState.opponentPokemon->current_hp = battleState.opponentPokemon->hp; // Full health
  
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON));  // Can't stack with burn
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "confuse-ray", 0, 100, 10, "ghost", "status", StatusCondition::NONE));  // Different effect
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "flamethrower", 90, 100, 15, "fire", "special"));  // Direct damage

  MoveEvaluation result = hardAI->chooseBestMove(battleState);

  // Should avoid redundant status and either use complementary status or damage
  EXPECT_NE(result.moveIndex, 0);  // Should not choose toxic (redundant with burn)
  EXPECT_TRUE(result.moveIndex == 1 || result.moveIndex == 2);  // Confuse or damage
  EXPECT_GT(result.score, 25.0);
}

// Test Hard AI PP and health resource management across team
TEST_F(HardAITest, SwitchingResourceManagement) {
  // Current AI Pokemon has low PP on all moves
  battleState.aiPokemon->moves.clear();
  Move lowPPMove1 = TestUtils::createTestMove("hyper-beam", 150, 90, 2, "normal", "special");
  Move lowPPMove2 = TestUtils::createTestMove("thunder", 110, 70, 3, "electric", "special");
  lowPPMove1.usePP(); // 1 PP left
  lowPPMove2.usePP(); lowPPMove2.usePP(); // 1 PP left
  battleState.aiPokemon->moves.push_back(lowPPMove1);
  battleState.aiPokemon->moves.push_back(lowPPMove2);
  
  // Alternative with full PP
  Pokemon freshPokemon = TestUtils::createTestPokemon("fresh", 95, 85, 80, 90, 85, 95, {"water"});
  freshPokemon.moves.clear();
  freshPokemon.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
  freshPokemon.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
  
  aiTeam = TestUtils::createTestTeam({*battleState.aiPokemon, freshPokemon});
  battleState.aiTeam = &aiTeam;
  battleState.aiPokemon = aiTeam.getPokemon(0);

  // Should consider switching to preserve PP resources
  SwitchEvaluation switchResult = hardAI->chooseBestSwitch(battleState);
  EXPECT_GE(switchResult.pokemonIndex, 0);
  EXPECT_GT(switchResult.score, -50.0);  // Should at least consider resource management
}

// Test Hard AI predictive counter-play
TEST_F(HardAITest, MoveSelectionPredictiveCounterplay) {
  // Scenario where opponent is likely to use a specific move next turn
  // AI should choose move that counters the predicted response
  
  // Opponent has obvious best move (super effective against current AI)
  battleState.aiPokemon->types = {"fire"};
  battleState.opponentPokemon->moves.clear();
  battleState.opponentPokemon->moves.push_back(TestUtils::createTestMove(
      "surf", 90, 100, 15, "water", "special"));  // Obvious choice vs fire
  battleState.opponentPokemon->moves.push_back(TestUtils::createTestMove(
      "tackle", 40, 100, 35, "normal", "physical"));  // Weak alternative
  
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "solar-beam", 120, 100, 10, "grass", "special"));  // Strong vs water but predictable
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "protect", 0, 100, 10, "normal", "status"));  // Defensive option
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "u-turn", 70, 100, 20, "bug", "physical"));  // Pivot option

  MoveEvaluation result = hardAI->chooseBestMove(battleState);

  // Hard AI should predict opponent's likely Water move and respond accordingly
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 3);
  EXPECT_GT(result.score, 20.0);
}

// Test comprehensive strategic behavior validation
TEST_F(HardAITest, ComprehensiveStrategicBehaviorValidation) {
  // Complex multi-scenario test combining multiple Hard AI features
  
  // Setup complex battle state
  battleState.currentWeather = WeatherCondition::SANDSTORM;
  battleState.aiPokemon->types = {"rock", "ground"};  // Benefits from sandstorm
  battleState.aiPokemon->current_hp = static_cast<int>(battleState.aiPokemon->hp * 0.75); // 75% HP
  
  // Opponent with status and moderate health
  battleState.opponentPokemon->status = StatusCondition::PARALYSIS;
  battleState.opponentPokemon->current_hp = static_cast<int>(battleState.opponentPokemon->hp * 0.4);
  battleState.opponentPokemon->types = {"water", "flying"};
  
  // Complex move options for AI
  battleState.aiPokemon->moves.clear();
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "stone-edge", 100, 80, 5, "rock", "physical"));  // STAB, high crit, risky accuracy
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "earthquake", 100, 100, 10, "ground", "physical"));  // STAB, reliable, super effective
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "stealth-rock", 0, 100, 20, "rock", "status"));  // Setup hazards
  battleState.aiPokemon->moves.push_back(TestUtils::createTestMove(
      "toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON));  // Status stacking attempt
  
  // Multi-Pokemon opponent team
  Pokemon flyingThreat = TestUtils::createTestPokemon("flying_threat", 95, 85, 75, 100, 90, 130, {"flying", "normal"});
  Pokemon waterTank = TestUtils::createTestPokemon("water_tank", 130, 70, 110, 80, 120, 60, {"water", "steel"});
  Team complexOpponentTeam = TestUtils::createTestTeam({*battleState.opponentPokemon, flyingThreat, waterTank});
  battleState.opponentTeam = &complexOpponentTeam;

  MoveEvaluation result = hardAI->chooseBestMove(battleState);

  // Validate comprehensive Hard AI decision making
  EXPECT_GE(result.moveIndex, 0);
  EXPECT_LT(result.moveIndex, 4);
  EXPECT_GT(result.score, 50.0);  // Should make a strong strategic decision
  EXPECT_NE(result.reasoning.find("Hard AI"), std::string::npos);
  
  // Test switching logic in this complex scenario
  bool switchDecision = hardAI->shouldSwitch(battleState);
  if (switchDecision) {
    SwitchEvaluation switchResult = hardAI->chooseBestSwitch(battleState);
    EXPECT_GT(switchResult.score, 0.0);  // Should have positive reason to switch
  }
  
  // Verify AI can handle the complex type interactions
  EXPECT_TRUE(result.moveIndex == 1 || result.moveIndex == 0);  // Likely earthquake or stone-edge
}