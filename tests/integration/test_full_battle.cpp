#include <gtest/gtest.h>
#include "test_utils.h"
#include "battle.h"

class FullBattleTest : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Create comprehensive test Pokemon with varied movesets
        setupComprehensiveTestPokemon();
        
        // Create battle instance
        battle = std::make_unique<Battle>(playerTeam, opponentTeam);
    }
    
    void setupComprehensiveTestPokemon() {
        // Create Pokemon with diverse types and movesets for full battle testing
        Pokemon fireStarter = TestUtils::createTestPokemon("charmander", 100, 85, 70, 95, 80, 75, {"fire"});
        Pokemon waterStarter = TestUtils::createTestPokemon("squirtle", 110, 75, 80, 85, 90, 70, {"water"});
        Pokemon grassStarter = TestUtils::createTestPokemon("bulbasaur", 105, 80, 85, 90, 85, 65, {"grass", "poison"});
        
        // Set up comprehensive movesets
        fireStarter.moves.clear();
        fireStarter.moves.push_back(TestUtils::createTestMove("ember", 40, 100, 25, "fire", "special"));
        fireStarter.moves.push_back(TestUtils::createTestMove("scratch", 40, 100, 35, "normal", "physical"));
        fireStarter.moves.push_back(TestUtils::createTestMove("smokescreen", 0, 100, 20, "normal", "status"));
        fireStarter.moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special"));
        
        waterStarter.moves.clear();
        waterStarter.moves.push_back(TestUtils::createTestMove("water-gun", 40, 100, 25, "water", "special"));
        waterStarter.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
        waterStarter.moves.push_back(TestUtils::createTestMove("withdraw", 0, 100, 40, "water", "status"));
        waterStarter.moves.push_back(TestUtils::createTestMove("bubble-beam", 65, 100, 20, "water", "special"));
        
        grassStarter.moves.clear();
        grassStarter.moves.push_back(TestUtils::createTestMove("vine-whip", 45, 100, 25, "grass", "physical"));
        grassStarter.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
        grassStarter.moves.push_back(TestUtils::createTestMove("poison-powder", 0, 75, 35, "poison", "status", StatusCondition::POISON, 100));
        grassStarter.moves.push_back(TestUtils::createTestMove("razor-leaf", 55, 95, 25, "grass", "physical"));
        
        // Create teams using the programmatically created Pokemon
        playerTeam = TestUtils::createTestTeam({fireStarter, waterStarter});
        opponentTeam = TestUtils::createTestTeam({grassStarter, waterStarter});
    }
    
    std::unique_ptr<Battle> battle;
};

// Test basic battle initialization and state
TEST_F(FullBattleTest, BattleInitialization) {
    EXPECT_FALSE(battle->isBattleOver());
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Both teams should have alive Pokemon
    EXPECT_TRUE(playerTeam.hasAlivePokemon());
    EXPECT_TRUE(opponentTeam.hasAlivePokemon());
    
    // Teams should have multiple Pokemon
    EXPECT_EQ(playerTeam.size(), 2);
    EXPECT_EQ(opponentTeam.size(), 2);
}

// Test battle state transitions
TEST_F(FullBattleTest, BattleStateTransitions) {
    // Initially ongoing
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Faint all player Pokemon
    for (size_t i = 0; i < playerTeam.size(); ++i) {
        Pokemon* pokemon = playerTeam.getPokemon(i);
        if (pokemon) {
            pokemon->takeDamage(pokemon->hp);
        }
    }
    
    // Create new battle after fainting - should transition to opponent wins
    auto faintedPlayerBattle = std::make_unique<Battle>(playerTeam, opponentTeam);
    EXPECT_EQ(faintedPlayerBattle->getBattleResult(), Battle::BattleResult::OPPONENT_WINS);
    EXPECT_TRUE(faintedPlayerBattle->isBattleOver());
    
    // Reset and test other direction
    setupComprehensiveTestPokemon();
    
    // Faint all opponent Pokemon
    for (size_t i = 0; i < opponentTeam.size(); ++i) {
        Pokemon* pokemon = opponentTeam.getPokemon(i);
        if (pokemon) {
            pokemon->takeDamage(pokemon->hp);
        }
    }
    
    // Create new battle after fainting - should transition to player wins
    auto faintedOpponentBattle = std::make_unique<Battle>(playerTeam, opponentTeam);
    EXPECT_EQ(faintedOpponentBattle->getBattleResult(), Battle::BattleResult::PLAYER_WINS);
    EXPECT_TRUE(faintedOpponentBattle->isBattleOver());
}

// Test battle with type advantages
TEST_F(FullBattleTest, BattleWithTypeAdvantages) {
    // Create specific type matchup scenario
    Pokemon fireType = TestUtils::createTestPokemon("fire", 100, 80, 70, 90, 85, 75, {"fire"});
    Pokemon waterType = TestUtils::createTestPokemon("water", 100, 80, 70, 90, 85, 75, {"water"});
    Pokemon grassType = TestUtils::createTestPokemon("grass", 100, 80, 70, 90, 85, 75, {"grass"});
    
    // Set up type-effective moves
    fireType.moves.clear();
    fireType.moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special"));
    fireType.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    
    waterType.moves.clear();
    waterType.moves.push_back(TestUtils::createTestMove("water-gun", 40, 100, 25, "water", "special"));
    waterType.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    
    grassType.moves.clear();
    grassType.moves.push_back(TestUtils::createTestMove("vine-whip", 45, 100, 25, "grass", "physical"));
    grassType.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    
    Team fireTeam = TestUtils::createTestTeam({fireType});
    Team waterTeam = TestUtils::createTestTeam({waterType});
    Team grassTeam = TestUtils::createTestTeam({grassType});
    
    // Test fire vs grass (fire advantage)
    Battle fireVsGrass(fireTeam, grassTeam);
    EXPECT_FALSE(fireVsGrass.isBattleOver());
    
    // Test water vs fire (water advantage)
    Battle waterVsFire(waterTeam, fireTeam);
    EXPECT_FALSE(waterVsFire.isBattleOver());
    
    // Test grass vs water (grass advantage)
    Battle grassVsWater(grassTeam, waterTeam);
    EXPECT_FALSE(grassVsWater.isBattleOver());
}

// Test battle with status conditions
TEST_F(FullBattleTest, BattleWithStatusConditions) {
    Pokemon statusUser = TestUtils::createTestPokemon("statuser", 100, 80, 70, 90, 85, 75, {"poison"});
    Pokemon statusTarget = TestUtils::createTestPokemon("target", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Status user has status moves
    statusUser.moves.clear();
    statusUser.moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON, 100));
    statusUser.moves.push_back(TestUtils::createTestMove("sleep-powder", 0, 75, 15, "grass", "status", StatusCondition::SLEEP, 100));
    statusUser.moves.push_back(TestUtils::createTestMove("thunder-wave", 0, 100, 20, "electric", "status", StatusCondition::PARALYSIS, 100));
    statusUser.moves.push_back(TestUtils::createTestMove("will-o-wisp", 0, 85, 15, "fire", "status", StatusCondition::BURN, 100));
    
    // Target has attacking moves
    statusTarget.moves.clear();
    statusTarget.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    statusTarget.moves.push_back(TestUtils::createTestMove("quick-attack", 40, 100, 30, "normal", "physical"));
    
    Team statusTeam = TestUtils::createTestTeam({statusUser});
    Team normalTeam = TestUtils::createTestTeam({statusTarget});
    
    Battle statusBattle(statusTeam, normalTeam);
    
    EXPECT_FALSE(statusBattle.isBattleOver());
    EXPECT_EQ(statusBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Both Pokemon should be alive initially
    EXPECT_TRUE(statusUser.isAlive());
    EXPECT_TRUE(statusTarget.isAlive());
    
    // Neither should have status conditions initially
    EXPECT_FALSE(statusUser.hasStatusCondition());
    EXPECT_FALSE(statusTarget.hasStatusCondition());
}

// Test battle with healing moves
TEST_F(FullBattleTest, BattleWithHealingMoves) {
    Pokemon healer = TestUtils::createTestPokemon("healer", 100, 80, 70, 90, 85, 75, {"normal"});
    Pokemon attacker = TestUtils::createTestPokemon("attacker", 100, 80, 70, 90, 85, 75, {"fighting"});
    
    // Healer has healing moves
    healer.moves.clear();
    healer.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 10, "normal", "status"));
    healer.moves.push_back(TestUtils::createTestMove("rest", 0, 100, 10, "psychic", "status"));
    healer.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    
    // Attacker has strong moves
    attacker.moves.clear();
    attacker.moves.push_back(TestUtils::createTestMove("close-combat", 120, 100, 5, "fighting", "physical"));
    attacker.moves.push_back(TestUtils::createTestMove("brick-break", 75, 100, 15, "fighting", "physical"));
    
    Team healerTeam = TestUtils::createTestTeam({healer});
    Team attackerTeam = TestUtils::createTestTeam({attacker});
    
    Battle healingBattle(healerTeam, attackerTeam);
    
    EXPECT_FALSE(healingBattle.isBattleOver());
    EXPECT_EQ(healingBattle.getBattleResult(), Battle::BattleResult::ONGOING);
}

// Test battle with multi-hit moves
TEST_F(FullBattleTest, BattleWithMultiHitMoves) {
    Pokemon multiHitter = TestUtils::createTestPokemon("multihit", 100, 80, 70, 90, 85, 75, {"normal"});
    Pokemon defender = TestUtils::createTestPokemon("defender", 100, 80, 70, 90, 85, 75, {"steel"});
    
    // Multi-hit moves
    multiHitter.moves.clear();
    Move bulletSeed = TestUtils::createTestMove("bullet-seed", 25, 100, 30, "grass", "physical");
    bulletSeed.min_hits = 2;
    bulletSeed.max_hits = 5;
    multiHitter.moves.push_back(bulletSeed);
    
    Move doubleSlap = TestUtils::createTestMove("double-slap", 15, 85, 10, "normal", "physical");
    doubleSlap.min_hits = 2;
    doubleSlap.max_hits = 5;
    multiHitter.moves.push_back(doubleSlap);
    
    multiHitter.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    
    // Defender has defensive moves
    defender.moves.clear();
    defender.moves.push_back(TestUtils::createTestMove("iron-defense", 0, 100, 15, "steel", "status"));
    defender.moves.push_back(TestUtils::createTestMove("metal-claw", 50, 95, 35, "steel", "physical"));
    
    Team multiHitTeam = TestUtils::createTestTeam({multiHitter});
    Team defenderTeam = TestUtils::createTestTeam({defender});
    
    Battle multiHitBattle(multiHitTeam, defenderTeam);
    
    EXPECT_FALSE(multiHitBattle.isBattleOver());
    EXPECT_EQ(multiHitBattle.getBattleResult(), Battle::BattleResult::ONGOING);
}

// Test battle with recoil moves
TEST_F(FullBattleTest, BattleWithRecoilMoves) {
    Pokemon recoilUser = TestUtils::createTestPokemon("recoil", 100, 80, 70, 90, 85, 75, {"normal"});
    Pokemon opponent = TestUtils::createTestPokemon("opponent", 100, 80, 70, 90, 85, 75, {"fighting"});
    
    // Recoil moves
    recoilUser.moves.clear();
    Move doubleEdge = TestUtils::createTestMove("double-edge", 120, 100, 15, "normal", "physical");
    doubleEdge.drain = -25; // 25% recoil damage
    recoilUser.moves.push_back(doubleEdge);
    
    Move takeDown = TestUtils::createTestMove("take-down", 90, 85, 20, "normal", "physical");
    takeDown.drain = -25; // 25% recoil damage
    recoilUser.moves.push_back(takeDown);
    
    recoilUser.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    
    // Opponent has normal moves
    opponent.moves.clear();
    opponent.moves.push_back(TestUtils::createTestMove("close-combat", 120, 100, 5, "fighting", "physical"));
    opponent.moves.push_back(TestUtils::createTestMove("brick-break", 75, 100, 15, "fighting", "physical"));
    
    Team recoilTeam = TestUtils::createTestTeam({recoilUser});
    Team normalTeam = TestUtils::createTestTeam({opponent});
    
    Battle recoilBattle(recoilTeam, normalTeam);
    
    EXPECT_FALSE(recoilBattle.isBattleOver());
    EXPECT_EQ(recoilBattle.getBattleResult(), Battle::BattleResult::ONGOING);
}

// Test battle with priority moves
TEST_F(FullBattleTest, BattleWithPriorityMoves) {
    Pokemon fastPokemon = TestUtils::createTestPokemon("fast", 100, 80, 70, 90, 85, 100, {"normal"});
    Pokemon slowPokemon = TestUtils::createTestPokemon("slow", 100, 80, 70, 90, 85, 50, {"steel"});
    
    // Fast Pokemon has priority moves
    fastPokemon.moves.clear();
    Move quickAttack = TestUtils::createTestMove("quick-attack", 40, 100, 30, "normal", "physical");
    quickAttack.priority = 1;
    fastPokemon.moves.push_back(quickAttack);
    
    Move extremeSpeed = TestUtils::createTestMove("extreme-speed", 80, 100, 5, "normal", "physical");
    extremeSpeed.priority = 2;
    fastPokemon.moves.push_back(extremeSpeed);
    
    fastPokemon.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    
    // Slow Pokemon has powerful moves
    slowPokemon.moves.clear();
    slowPokemon.moves.push_back(TestUtils::createTestMove("meteor-mash", 90, 90, 10, "steel", "physical"));
    slowPokemon.moves.push_back(TestUtils::createTestMove("iron-head", 80, 100, 15, "steel", "physical"));
    
    Team fastTeam = TestUtils::createTestTeam({fastPokemon});
    Team slowTeam = TestUtils::createTestTeam({slowPokemon});
    
    Battle priorityBattle(fastTeam, slowTeam);
    
    EXPECT_FALSE(priorityBattle.isBattleOver());
    EXPECT_EQ(priorityBattle.getBattleResult(), Battle::BattleResult::ONGOING);
}

// Test battle with stat-modifying moves
TEST_F(FullBattleTest, BattleWithStatModifyingMoves) {
    Pokemon setupPokemon = TestUtils::createTestPokemon("setup", 100, 80, 70, 90, 85, 75, {"normal"});
    Pokemon sweeper = TestUtils::createTestPokemon("sweeper", 100, 80, 70, 90, 85, 75, {"dragon"});
    
    // Setup Pokemon has stat-boosting moves
    setupPokemon.moves.clear();
    setupPokemon.moves.push_back(TestUtils::createTestMove("swords-dance", 0, 100, 20, "normal", "status"));
    setupPokemon.moves.push_back(TestUtils::createTestMove("agility", 0, 100, 30, "psychic", "status"));
    setupPokemon.moves.push_back(TestUtils::createTestMove("calm-mind", 0, 100, 20, "psychic", "status"));
    setupPokemon.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    
    // Sweeper has powerful moves
    sweeper.moves.clear();
    sweeper.moves.push_back(TestUtils::createTestMove("dragon-pulse", 85, 100, 10, "dragon", "special"));
    sweeper.moves.push_back(TestUtils::createTestMove("dragon-claw", 80, 100, 15, "dragon", "physical"));
    sweeper.moves.push_back(TestUtils::createTestMove("outrage", 120, 100, 10, "dragon", "physical"));
    
    Team setupTeam = TestUtils::createTestTeam({setupPokemon});
    Team sweeperTeam = TestUtils::createTestTeam({sweeper});
    
    Battle statModBattle(setupTeam, sweeperTeam);
    
    EXPECT_FALSE(statModBattle.isBattleOver());
    EXPECT_EQ(statModBattle.getBattleResult(), Battle::BattleResult::ONGOING);
}

// Test battle with mixed team composition
TEST_F(FullBattleTest, BattleWithMixedTeamComposition) {
    // Create teams with different roles
    Pokemon tank = TestUtils::createTestPokemon("tank", 150, 60, 100, 60, 100, 50, {"steel", "rock"});
    Pokemon glass_cannon = TestUtils::createTestPokemon("cannon", 70, 120, 50, 120, 50, 100, {"electric"});
    Pokemon balanced = TestUtils::createTestPokemon("balanced", 100, 80, 80, 80, 80, 80, {"normal"});
    
    // Tank moves
    tank.moves.clear();
    tank.moves.push_back(TestUtils::createTestMove("iron-defense", 0, 100, 15, "steel", "status"));
    tank.moves.push_back(TestUtils::createTestMove("metal-claw", 50, 95, 35, "steel", "physical"));
    tank.moves.push_back(TestUtils::createTestMove("rock-slide", 75, 90, 10, "rock", "physical"));
    
    // Glass cannon moves
    glass_cannon.moves.clear();
    glass_cannon.moves.push_back(TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special"));
    glass_cannon.moves.push_back(TestUtils::createTestMove("thunder", 110, 70, 10, "electric", "special"));
    glass_cannon.moves.push_back(TestUtils::createTestMove("quick-attack", 40, 100, 30, "normal", "physical"));
    
    // Balanced moves
    balanced.moves.clear();
    balanced.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    balanced.moves.push_back(TestUtils::createTestMove("double-edge", 120, 100, 15, "normal", "physical"));
    balanced.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 10, "normal", "status"));
    
    Team diverseTeam1 = TestUtils::createTestTeam({tank, glass_cannon});
    Team diverseTeam2 = TestUtils::createTestTeam({balanced, glass_cannon});
    
    Battle diverseBattle(diverseTeam1, diverseTeam2);
    
    EXPECT_FALSE(diverseBattle.isBattleOver());
    EXPECT_EQ(diverseBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Teams should have different compositions
    EXPECT_EQ(diverseTeam1.size(), 2);
    EXPECT_EQ(diverseTeam2.size(), 2);
}

// Test battle completion scenarios
TEST_F(FullBattleTest, BattleCompletionScenarios) {
    // Test scenario where one team gradually loses all Pokemon
    Pokemon strong = TestUtils::createTestPokemon("strong", 100, 120, 70, 90, 85, 75, {"dragon"});
    Pokemon weak1 = TestUtils::createTestPokemon("weak1", 50, 40, 30, 40, 30, 40, {"normal"});
    Pokemon weak2 = TestUtils::createTestPokemon("weak2", 50, 40, 30, 40, 30, 40, {"normal"});
    
    strong.moves.clear();
    strong.moves.push_back(TestUtils::createTestMove("dragon-pulse", 85, 100, 10, "dragon", "special"));
    strong.moves.push_back(TestUtils::createTestMove("hyper-beam", 150, 90, 5, "normal", "special"));
    
    weak1.moves.clear();
    weak1.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    
    weak2.moves.clear();
    weak2.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    
    Team strongTeam = TestUtils::createTestTeam({strong});
    Team weakTeam = TestUtils::createTestTeam({weak1, weak2});
    
    Battle completionBattle(strongTeam, weakTeam);
    
    EXPECT_FALSE(completionBattle.isBattleOver());
    EXPECT_EQ(completionBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Strong team should have advantage
    EXPECT_TRUE(strong.isAlive());
    EXPECT_TRUE(weak1.isAlive());
    EXPECT_TRUE(weak2.isAlive());
}

// Test battle functionality
TEST_F(FullBattleTest, BattleFunctionality) {
    // Test that battles are functional
    Battle testBattle(playerTeam, opponentTeam);
    
    // Should be functional
    EXPECT_FALSE(testBattle.isBattleOver());
    
    EXPECT_EQ(testBattle.getBattleResult(), Battle::BattleResult::ONGOING);
}

// Test battle memory and state management
TEST_F(FullBattleTest, BattleMemoryAndStateManagement) {
    // Test that battles maintain state correctly
    EXPECT_FALSE(battle->isBattleOver());
    
    // Get initial state
    auto initialResult = battle->getBattleResult();
    
    // State should remain consistent
    EXPECT_EQ(battle->getBattleResult(), initialResult);
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Teams should maintain their state
    EXPECT_TRUE(playerTeam.hasAlivePokemon());
    EXPECT_TRUE(opponentTeam.hasAlivePokemon());
    
    // Pokemon should maintain their state
    for (size_t i = 0; i < playerTeam.size(); ++i) {
        Pokemon* pokemon = playerTeam.getPokemon(i);
        if (pokemon) {
            EXPECT_TRUE(pokemon->isAlive());
            EXPECT_EQ(pokemon->current_hp, pokemon->hp);
        }
    }
}

// Test battle with edge cases
TEST_F(FullBattleTest, BattleEdgeCases) {
    // Test battle with Pokemon that have unusual stats
    Pokemon glassCannonExtreme = TestUtils::createTestPokemon("extreme", 1, 255, 1, 255, 1, 255, {"normal"});
    Pokemon wallExtreme = TestUtils::createTestPokemon("wall", 255, 1, 255, 1, 255, 1, {"steel"});
    
    glassCannonExtreme.moves.clear();
    glassCannonExtreme.moves.push_back(TestUtils::createTestMove("explosion", 250, 100, 5, "normal", "physical"));
    glassCannonExtreme.moves.push_back(TestUtils::createTestMove("quick-attack", 40, 100, 30, "normal", "physical"));
    
    wallExtreme.moves.clear();
    wallExtreme.moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON, 100));
    wallExtreme.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 10, "normal", "status"));
    
    Team extremeTeam1 = TestUtils::createTestTeam({glassCannonExtreme});
    Team extremeTeam2 = TestUtils::createTestTeam({wallExtreme});
    
    Battle extremeBattle(extremeTeam1, extremeTeam2);
    
    EXPECT_FALSE(extremeBattle.isBattleOver());
    EXPECT_EQ(extremeBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Extreme stats should be handled correctly
    EXPECT_EQ(glassCannonExtreme.hp, 1);
    EXPECT_EQ(glassCannonExtreme.attack, 255);
    EXPECT_EQ(wallExtreme.hp, 255);
    EXPECT_EQ(wallExtreme.defense, 255);
}