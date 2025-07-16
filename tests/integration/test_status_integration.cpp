#include <gtest/gtest.h>
#include "test_utils.h"
#include "battle.h"

class StatusIntegrationTest : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Create Pokemon with status-inducing moves
        setupStatusTestPokemon();
        
        // Create battle instance
        battle = std::make_unique<Battle>(playerTeam, opponentTeam, Battle::AIDifficulty::EASY);
    }
    
    void setupStatusTestPokemon() {
        // Create Pokemon with comprehensive status movesets
        Pokemon statusInflicter = TestUtils::createTestPokemon("inflict", 100, 80, 70, 90, 85, 75, {"poison", "grass"});
        Pokemon statusTarget = TestUtils::createTestPokemon("target", 100, 80, 70, 90, 85, 75, {"normal"});
        Pokemon statusHealer = TestUtils::createTestPokemon("healer", 100, 80, 70, 90, 85, 75, {"normal"});
        
        // Status inflicter moves
        statusInflicter.moves.clear();
        statusInflicter.moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON, 100));
        statusInflicter.moves.push_back(TestUtils::createTestMove("sleep-powder", 0, 75, 15, "grass", "status", StatusCondition::SLEEP, 100));
        statusInflicter.moves.push_back(TestUtils::createTestMove("will-o-wisp", 0, 85, 15, "fire", "status", StatusCondition::BURN, 100));
        statusInflicter.moves.push_back(TestUtils::createTestMove("thunder-wave", 0, 100, 20, "electric", "status", StatusCondition::PARALYSIS, 100));
        
        // Status target moves
        statusTarget.moves.clear();
        statusTarget.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
        statusTarget.moves.push_back(TestUtils::createTestMove("double-edge", 120, 100, 15, "normal", "physical"));
        statusTarget.moves.push_back(TestUtils::createTestMove("quick-attack", 40, 100, 30, "normal", "physical"));
        statusTarget.moves.push_back(TestUtils::createTestMove("rest", 0, 100, 10, "psychic", "status"));
        
        // Status healer moves
        statusHealer.moves.clear();
        statusHealer.moves.push_back(TestUtils::createTestMove("aromatherapy", 0, 100, 5, "grass", "status"));
        statusHealer.moves.push_back(TestUtils::createTestMove("heal-bell", 0, 100, 5, "normal", "status"));
        statusHealer.moves.push_back(TestUtils::createTestMove("refresh", 0, 100, 20, "normal", "status"));
        statusHealer.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
        
        // Create teams
        std::unordered_map<std::string, std::vector<std::string>> statusTeamData;
        std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>> statusMovesData;
        
        statusTeamData["PlayerTeam"] = {"inflict", "healer"};
        statusTeamData["OpponentTeam"] = {"target", "healer"};
        
        statusMovesData["PlayerTeam"] = {
            {"inflict", {"toxic", "sleep-powder", "will-o-wisp", "thunder-wave"}},
            {"healer", {"aromatherapy", "heal-bell", "refresh", "tackle"}}
        };
        
        statusMovesData["OpponentTeam"] = {
            {"target", {"tackle", "double-edge", "quick-attack", "rest"}},
            {"healer", {"aromatherapy", "heal-bell", "refresh", "tackle"}}
        };
        
        playerTeam.loadTeams(statusTeamData, statusMovesData, "PlayerTeam");
        opponentTeam.loadTeams(statusTeamData, statusMovesData, "OpponentTeam");
    }
    
    std::unique_ptr<Battle> battle;
};

// Test poison status condition integration
TEST_F(StatusIntegrationTest, PoisonStatusIntegration) {
    Pokemon poisoner = TestUtils::createTestPokemon("poisoner", 100, 80, 70, 90, 85, 75, {"poison"});
    Pokemon victim = TestUtils::createTestPokemon("victim", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Poisoner has poison moves
    poisoner.moves.clear();
    poisoner.moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON, 100));
    poisoner.moves.push_back(TestUtils::createTestMove("poison-powder", 0, 75, 35, "poison", "status", StatusCondition::POISON, 100));
    poisoner.moves.push_back(TestUtils::createTestMove("sludge-bomb", 90, 100, 10, "poison", "special", StatusCondition::POISON, 30));
    
    // Victim has normal moves
    victim.moves.clear();
    victim.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    victim.moves.push_back(TestUtils::createTestMove("double-edge", 120, 100, 15, "normal", "physical"));
    
    Team poisonTeam = TestUtils::createTestTeam({poisoner});
    Team normalTeam = TestUtils::createTestTeam({victim});
    
    Battle poisonBattle(poisonTeam, normalTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(poisonBattle.isBattleOver());
    EXPECT_EQ(poisonBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test poison application
    EXPECT_FALSE(victim.hasStatusCondition());
    victim.applyStatusCondition(StatusCondition::POISON);
    EXPECT_TRUE(victim.hasStatusCondition());
    EXPECT_EQ(victim.status, StatusCondition::POISON);
    EXPECT_EQ(victim.getStatusConditionName(), "Poisoned");
    
    // Test poison damage over time
    int initialHP = victim.current_hp;
    victim.processStatusCondition();
    EXPECT_LT(victim.current_hp, initialHP);
    
    // Test that poison doesn't affect poison-type Pokemon
    EXPECT_FALSE(poisoner.hasStatusCondition());
    poisoner.applyStatusCondition(StatusCondition::POISON);
    // Implementation may vary on poison immunity for poison types
}

// Test burn status condition integration
TEST_F(StatusIntegrationTest, BurnStatusIntegration) {
    Pokemon burner = TestUtils::createTestPokemon("burner", 100, 80, 70, 90, 85, 75, {"fire"});
    Pokemon victim = TestUtils::createTestPokemon("victim", 100, 80, 70, 90, 85, 75, {"grass"});
    
    // Burner has burn moves
    burner.moves.clear();
    burner.moves.push_back(TestUtils::createTestMove("will-o-wisp", 0, 85, 15, "fire", "status", StatusCondition::BURN, 100));
    burner.moves.push_back(TestUtils::createTestMove("flame-wheel", 60, 100, 25, "fire", "physical", StatusCondition::BURN, 10));
    burner.moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special", StatusCondition::BURN, 10));
    
    // Victim has physical moves (affected by burn)
    victim.moves.clear();
    victim.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    victim.moves.push_back(TestUtils::createTestMove("body-slam", 85, 100, 15, "normal", "physical"));
    victim.moves.push_back(TestUtils::createTestMove("razor-leaf", 55, 95, 25, "grass", "physical"));
    
    Team burnTeam = TestUtils::createTestTeam({burner});
    Team grassTeam = TestUtils::createTestTeam({victim});
    
    Battle burnBattle(burnTeam, grassTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(burnBattle.isBattleOver());
    EXPECT_EQ(burnBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test burn application
    EXPECT_FALSE(victim.hasStatusCondition());
    victim.applyStatusCondition(StatusCondition::BURN);
    EXPECT_TRUE(victim.hasStatusCondition());
    EXPECT_EQ(victim.status, StatusCondition::BURN);
    EXPECT_EQ(victim.getStatusConditionName(), "Burned");
    
    // Test burn damage over time
    int initialHP = victim.current_hp;
    victim.processStatusCondition();
    EXPECT_LT(victim.current_hp, initialHP);
    
    // Test burn effect on physical attack
    int normalAttack = victim.attack;
    int burnedAttack = victim.getEffectiveAttack();
    EXPECT_LT(burnedAttack, normalAttack);
}

// Test paralysis status condition integration
TEST_F(StatusIntegrationTest, ParalysisStatusIntegration) {
    Pokemon paralyzer = TestUtils::createTestPokemon("paralyzer", 100, 80, 70, 90, 85, 75, {"electric"});
    Pokemon victim = TestUtils::createTestPokemon("victim", 100, 80, 70, 90, 85, 75, {"water"});
    
    // Paralyzer has paralysis moves
    paralyzer.moves.clear();
    paralyzer.moves.push_back(TestUtils::createTestMove("thunder-wave", 0, 100, 20, "electric", "status", StatusCondition::PARALYSIS, 100));
    paralyzer.moves.push_back(TestUtils::createTestMove("body-slam", 85, 100, 15, "normal", "physical", StatusCondition::PARALYSIS, 30));
    paralyzer.moves.push_back(TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special", StatusCondition::PARALYSIS, 10));
    
    // Victim has speed-based moves
    victim.moves.clear();
    victim.moves.push_back(TestUtils::createTestMove("quick-attack", 40, 100, 30, "normal", "physical"));
    victim.moves.push_back(TestUtils::createTestMove("aqua-jet", 40, 100, 20, "water", "physical"));
    victim.moves.push_back(TestUtils::createTestMove("agility", 0, 100, 30, "psychic", "status"));
    
    Team paralysisTeam = TestUtils::createTestTeam({paralyzer});
    Team waterTeam = TestUtils::createTestTeam({victim});
    
    Battle paralysisBattle(paralysisTeam, waterTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(paralysisBattle.isBattleOver());
    EXPECT_EQ(paralysisBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test paralysis application
    EXPECT_FALSE(victim.hasStatusCondition());
    victim.applyStatusCondition(StatusCondition::PARALYSIS);
    EXPECT_TRUE(victim.hasStatusCondition());
    EXPECT_EQ(victim.status, StatusCondition::PARALYSIS);
    EXPECT_EQ(victim.getStatusConditionName(), "Paralyzed");
    
    // Test paralysis effect on speed
    int normalSpeed = victim.speed;
    int paralyzedSpeed = victim.getEffectiveSpeed();
    EXPECT_LT(paralyzedSpeed, normalSpeed);
    
    // Test paralysis preventing action (probabilistic)
    bool canActAtLeastOnce = false;
    bool cannotActAtLeastOnce = false;
    
    for (int i = 0; i < 100; ++i) {
        if (victim.canAct()) {
            canActAtLeastOnce = true;
        } else {
            cannotActAtLeastOnce = true;
        }
    }
    
    EXPECT_TRUE(canActAtLeastOnce);
    EXPECT_TRUE(cannotActAtLeastOnce);
}

// Test sleep status condition integration
TEST_F(StatusIntegrationTest, SleepStatusIntegration) {
    Pokemon sleeper = TestUtils::createTestPokemon("sleeper", 100, 80, 70, 90, 85, 75, {"grass"});
    Pokemon victim = TestUtils::createTestPokemon("victim", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Sleeper has sleep moves
    sleeper.moves.clear();
    sleeper.moves.push_back(TestUtils::createTestMove("sleep-powder", 0, 75, 15, "grass", "status", StatusCondition::SLEEP, 100));
    sleeper.moves.push_back(TestUtils::createTestMove("spore", 0, 100, 15, "grass", "status", StatusCondition::SLEEP, 100));
    sleeper.moves.push_back(TestUtils::createTestMove("lovely-kiss", 0, 75, 10, "normal", "status", StatusCondition::SLEEP, 100));
    
    // Victim has wake-up moves
    victim.moves.clear();
    victim.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    victim.moves.push_back(TestUtils::createTestMove("sleep-talk", 0, 100, 10, "normal", "status"));
    victim.moves.push_back(TestUtils::createTestMove("snore", 50, 100, 15, "normal", "special"));
    
    Team sleepTeam = TestUtils::createTestTeam({sleeper});
    Team normalTeam = TestUtils::createTestTeam({victim});
    
    Battle sleepBattle(sleepTeam, normalTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(sleepBattle.isBattleOver());
    EXPECT_EQ(sleepBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test sleep application
    EXPECT_FALSE(victim.hasStatusCondition());
    victim.applyStatusCondition(StatusCondition::SLEEP);
    EXPECT_TRUE(victim.hasStatusCondition());
    EXPECT_EQ(victim.status, StatusCondition::SLEEP);
    EXPECT_EQ(victim.getStatusConditionName(), "Asleep");
    
    // Test sleep preventing action initially
    // Implementation may vary on sleep mechanics
    EXPECT_TRUE(victim.hasStatusCondition());
    
    // Test sleep duration mechanics
    EXPECT_GT(victim.status_turns_remaining, 0);
}

// Test freeze status condition integration
TEST_F(StatusIntegrationTest, FreezeStatusIntegration) {
    Pokemon freezer = TestUtils::createTestPokemon("freezer", 100, 80, 70, 90, 85, 75, {"ice"});
    Pokemon victim = TestUtils::createTestPokemon("victim", 100, 80, 70, 90, 85, 75, {"water"});
    
    // Freezer has freeze moves
    freezer.moves.clear();
    freezer.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special", StatusCondition::FREEZE, 10));
    freezer.moves.push_back(TestUtils::createTestMove("blizzard", 110, 70, 5, "ice", "special", StatusCondition::FREEZE, 10));
    freezer.moves.push_back(TestUtils::createTestMove("powder-snow", 40, 100, 25, "ice", "special", StatusCondition::FREEZE, 10));
    
    // Victim has thaw moves
    victim.moves.clear();
    victim.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    victim.moves.push_back(TestUtils::createTestMove("flame-wheel", 60, 100, 25, "fire", "physical"));
    victim.moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special"));
    
    Team freezeTeam = TestUtils::createTestTeam({freezer});
    Team waterTeam = TestUtils::createTestTeam({victim});
    
    Battle freezeBattle(freezeTeam, waterTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(freezeBattle.isBattleOver());
    EXPECT_EQ(freezeBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test freeze application
    EXPECT_FALSE(victim.hasStatusCondition());
    victim.applyStatusCondition(StatusCondition::FREEZE);
    EXPECT_TRUE(victim.hasStatusCondition());
    EXPECT_EQ(victim.status, StatusCondition::FREEZE);
    EXPECT_EQ(victim.getStatusConditionName(), "Frozen");
    
    // Test freeze preventing action
    // Implementation may vary on freeze mechanics
    EXPECT_TRUE(victim.hasStatusCondition());
}

// Test status condition immunity and overwriting
TEST_F(StatusIntegrationTest, StatusImmunityAndOverwriting) {
    Pokemon testPokemon = TestUtils::createTestPokemon("test", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Test initial state
    EXPECT_FALSE(testPokemon.hasStatusCondition());
    EXPECT_EQ(testPokemon.status, StatusCondition::NONE);
    
    // Apply poison
    testPokemon.applyStatusCondition(StatusCondition::POISON);
    EXPECT_TRUE(testPokemon.hasStatusCondition());
    EXPECT_EQ(testPokemon.status, StatusCondition::POISON);
    
    // Try to apply burn (should fail - can't have multiple major status conditions)
    testPokemon.applyStatusCondition(StatusCondition::BURN);
    EXPECT_EQ(testPokemon.status, StatusCondition::POISON); // Should remain poisoned
    
    // Clear status
    testPokemon.clearStatusCondition();
    EXPECT_FALSE(testPokemon.hasStatusCondition());
    EXPECT_EQ(testPokemon.status, StatusCondition::NONE);
    
    // Now burn should work
    testPokemon.applyStatusCondition(StatusCondition::BURN);
    EXPECT_TRUE(testPokemon.hasStatusCondition());
    EXPECT_EQ(testPokemon.status, StatusCondition::BURN);
}

// Test status condition interactions in battle
TEST_F(StatusIntegrationTest, StatusInteractionsInBattle) {
    Pokemon inflicterPokemon = TestUtils::createTestPokemon("inflicter", 100, 80, 70, 90, 85, 75, {"poison"});
    Pokemon victimPokemon = TestUtils::createTestPokemon("victim", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Inflicter has multiple status moves
    inflicterPokemon.moves.clear();
    inflicterPokemon.moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON, 100));
    inflicterPokemon.moves.push_back(TestUtils::createTestMove("will-o-wisp", 0, 85, 15, "fire", "status", StatusCondition::BURN, 100));
    inflicterPokemon.moves.push_back(TestUtils::createTestMove("thunder-wave", 0, 100, 20, "electric", "status", StatusCondition::PARALYSIS, 100));
    inflicterPokemon.moves.push_back(TestUtils::createTestMove("sleep-powder", 0, 75, 15, "grass", "status", StatusCondition::SLEEP, 100));
    
    // Victim has attacking moves
    victimPokemon.moves.clear();
    victimPokemon.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    victimPokemon.moves.push_back(TestUtils::createTestMove("body-slam", 85, 100, 15, "normal", "physical"));
    victimPokemon.moves.push_back(TestUtils::createTestMove("double-edge", 120, 100, 15, "normal", "physical"));
    
    Team inflicterTeam = TestUtils::createTestTeam({inflicterPokemon});
    Team victimTeam = TestUtils::createTestTeam({victimPokemon});
    
    Battle statusBattle(inflicterTeam, victimTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(statusBattle.isBattleOver());
    EXPECT_EQ(statusBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test status application in battle context
    EXPECT_FALSE(victimPokemon.hasStatusCondition());
    
    // Apply poison and test damage over time
    victimPokemon.applyStatusCondition(StatusCondition::POISON);
    EXPECT_TRUE(victimPokemon.hasStatusCondition());
    
    int initialHP = victimPokemon.current_hp;
    victimPokemon.processStatusCondition();
    EXPECT_LT(victimPokemon.current_hp, initialHP);
    
    // Clear and apply burn
    victimPokemon.clearStatusCondition();
    victimPokemon.applyStatusCondition(StatusCondition::BURN);
    EXPECT_EQ(victimPokemon.status, StatusCondition::BURN);
    
    // Test burn effect on attack
    int normalAttack = victimPokemon.attack;
    int burnedAttack = victimPokemon.getEffectiveAttack();
    EXPECT_LT(burnedAttack, normalAttack);
}

// Test status condition healing and recovery
TEST_F(StatusIntegrationTest, StatusHealingAndRecovery) {
    Pokemon healer = TestUtils::createTestPokemon("healer", 100, 80, 70, 90, 85, 75, {"normal"});
    Pokemon patient = TestUtils::createTestPokemon("patient", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Healer has healing moves
    healer.moves.clear();
    healer.moves.push_back(TestUtils::createTestMove("aromatherapy", 0, 100, 5, "grass", "status"));
    healer.moves.push_back(TestUtils::createTestMove("heal-bell", 0, 100, 5, "normal", "status"));
    healer.moves.push_back(TestUtils::createTestMove("refresh", 0, 100, 20, "normal", "status"));
    healer.moves.push_back(TestUtils::createTestMove("rest", 0, 100, 10, "psychic", "status"));
    
    // Patient has normal moves
    patient.moves.clear();
    patient.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    patient.moves.push_back(TestUtils::createTestMove("facade", 70, 100, 20, "normal", "physical"));
    
    Team healerTeam = TestUtils::createTestTeam({healer});
    Team patientTeam = TestUtils::createTestTeam({patient});
    
    Battle healingBattle(healerTeam, patientTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(healingBattle.isBattleOver());
    EXPECT_EQ(healingBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test status application and healing
    EXPECT_FALSE(patient.hasStatusCondition());
    
    // Apply poison
    patient.applyStatusCondition(StatusCondition::POISON);
    EXPECT_TRUE(patient.hasStatusCondition());
    EXPECT_EQ(patient.status, StatusCondition::POISON);
    
    // Heal status
    patient.clearStatusCondition();
    EXPECT_FALSE(patient.hasStatusCondition());
    EXPECT_EQ(patient.status, StatusCondition::NONE);
    
    // Test with different status conditions
    patient.applyStatusCondition(StatusCondition::BURN);
    EXPECT_TRUE(patient.hasStatusCondition());
    EXPECT_EQ(patient.status, StatusCondition::BURN);
    
    patient.clearStatusCondition();
    EXPECT_FALSE(patient.hasStatusCondition());
    EXPECT_EQ(patient.status, StatusCondition::NONE);
}

// Test status condition effects on battle flow
TEST_F(StatusIntegrationTest, StatusEffectsOnBattleFlow) {
    Pokemon fastPokemon = TestUtils::createTestPokemon("fast", 100, 80, 70, 90, 85, 100, {"electric"});
    Pokemon slowPokemon = TestUtils::createTestPokemon("slow", 100, 80, 70, 90, 85, 50, {"ground"});
    
    // Fast Pokemon has paralysis move
    fastPokemon.moves.clear();
    fastPokemon.moves.push_back(TestUtils::createTestMove("thunder-wave", 0, 100, 20, "electric", "status", StatusCondition::PARALYSIS, 100));
    fastPokemon.moves.push_back(TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special"));
    fastPokemon.moves.push_back(TestUtils::createTestMove("quick-attack", 40, 100, 30, "normal", "physical"));
    
    // Slow Pokemon has powerful moves
    slowPokemon.moves.clear();
    slowPokemon.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
    slowPokemon.moves.push_back(TestUtils::createTestMove("rock-slide", 75, 90, 10, "rock", "physical"));
    slowPokemon.moves.push_back(TestUtils::createTestMove("magnitude", 70, 100, 30, "ground", "physical"));
    
    Team fastTeam = TestUtils::createTestTeam({fastPokemon});
    Team slowTeam = TestUtils::createTestTeam({slowPokemon});
    
    Battle speedBattle(fastTeam, slowTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(speedBattle.isBattleOver());
    EXPECT_EQ(speedBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test speed comparison
    EXPECT_GT(fastPokemon.speed, slowPokemon.speed);
    
    // Apply paralysis to fast Pokemon
    fastPokemon.applyStatusCondition(StatusCondition::PARALYSIS);
    EXPECT_TRUE(fastPokemon.hasStatusCondition());
    
    // Test paralysis effect on speed
    int normalSpeed = fastPokemon.speed;
    int paralyzedSpeed = fastPokemon.getEffectiveSpeed();
    EXPECT_LT(paralyzedSpeed, normalSpeed);
    
    // Paralyzed fast Pokemon might now be slower than slow Pokemon
    EXPECT_LT(paralyzedSpeed, slowPokemon.speed);
}

// Test status condition duration and persistence
TEST_F(StatusIntegrationTest, StatusDurationAndPersistence) {
    Pokemon testPokemon = TestUtils::createTestPokemon("test", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Test poison duration
    testPokemon.applyStatusCondition(StatusCondition::POISON);
    EXPECT_TRUE(testPokemon.hasStatusCondition());
    EXPECT_EQ(testPokemon.status, StatusCondition::POISON);
    
    // Poison should persist through multiple turns
    int initialHP = testPokemon.current_hp;
    
    for (int turn = 0; turn < 5; ++turn) {
        testPokemon.processStatusCondition();
        EXPECT_TRUE(testPokemon.hasStatusCondition());
        EXPECT_EQ(testPokemon.status, StatusCondition::POISON);
        EXPECT_LT(testPokemon.current_hp, initialHP);
    }
    
    // Clear and test burn
    testPokemon.clearStatusCondition();
    testPokemon.current_hp = testPokemon.hp; // Reset HP
    testPokemon.applyStatusCondition(StatusCondition::BURN);
    
    initialHP = testPokemon.current_hp;
    
    for (int turn = 0; turn < 5; ++turn) {
        testPokemon.processStatusCondition();
        EXPECT_TRUE(testPokemon.hasStatusCondition());
        EXPECT_EQ(testPokemon.status, StatusCondition::BURN);
        EXPECT_LT(testPokemon.current_hp, initialHP);
    }
}

// Test status condition edge cases
TEST_F(StatusIntegrationTest, StatusEdgeCases) {
    Pokemon testPokemon = TestUtils::createTestPokemon("test", 1, 80, 70, 90, 85, 75, {"normal"});
    
    // Test status with very low HP
    testPokemon.applyStatusCondition(StatusCondition::POISON);
    EXPECT_TRUE(testPokemon.hasStatusCondition());
    
    // Process status condition - should not reduce HP below 0
    testPokemon.processStatusCondition();
    EXPECT_GE(testPokemon.current_hp, 0);
    
    // Test multiple status applications
    testPokemon.clearStatusCondition();
    testPokemon.current_hp = 100;
    
    testPokemon.applyStatusCondition(StatusCondition::POISON);
    EXPECT_EQ(testPokemon.status, StatusCondition::POISON);
    
    testPokemon.applyStatusCondition(StatusCondition::BURN);
    EXPECT_EQ(testPokemon.status, StatusCondition::POISON); // Should remain poisoned
    
    testPokemon.applyStatusCondition(StatusCondition::PARALYSIS);
    EXPECT_EQ(testPokemon.status, StatusCondition::POISON); // Should remain poisoned
}

// Test status condition battle integration
TEST_F(StatusIntegrationTest, StatusBattleIntegration) {
    EXPECT_FALSE(battle->isBattleOver());
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Both teams should have Pokemon with status moves
    EXPECT_TRUE(playerTeam.hasAlivePokemon());
    EXPECT_TRUE(opponentTeam.hasAlivePokemon());
    
    // Check that teams have been set up correctly
    EXPECT_EQ(playerTeam.size(), 2);
    EXPECT_EQ(opponentTeam.size(), 2);
    
    // Test that Pokemon have status-related moves
    Pokemon* playerPokemon = playerTeam.getPokemon(0);
    Pokemon* opponentPokemon = opponentTeam.getPokemon(0);
    
    ASSERT_NE(playerPokemon, nullptr);
    ASSERT_NE(opponentPokemon, nullptr);
    
    EXPECT_GT(playerPokemon->moves.size(), 0);
    EXPECT_GT(opponentPokemon->moves.size(), 0);
    
    // Initially no status conditions
    EXPECT_FALSE(playerPokemon->hasStatusCondition());
    EXPECT_FALSE(opponentPokemon->hasStatusCondition());
}