#include <gtest/gtest.h>
#include "test_utils.h"
#include "move.h"

class MoveTest : public TestUtils::PokemonTestFixture {
protected:
    void SetUp() override {
        TestUtils::PokemonTestFixture::SetUp();
        
        // Create test moves
        damageMove = TestUtils::createTestMove("testmove", 80, 100, 15, "normal", "physical");
        statusMove = TestUtils::createTestMove("statusmove", 0, 85, 10, "normal", "status", StatusCondition::POISON, 100);
        specialMove = TestUtils::createTestMove("specialmove", 90, 95, 10, "fire", "special");
    }
    
    Move damageMove;
    Move statusMove;
    Move specialMove;
};

// Test Move creation and basic properties
TEST_F(MoveTest, BasicProperties) {
    EXPECT_EQ(damageMove.name, "testmove");
    EXPECT_EQ(damageMove.power, 80);
    EXPECT_EQ(damageMove.accuracy, 100);
    EXPECT_EQ(damageMove.current_pp, 15);
    EXPECT_EQ(damageMove.getMaxPP(), 15);
    EXPECT_EQ(damageMove.type, "normal");
    EXPECT_EQ(damageMove.damage_class, "physical");
    EXPECT_EQ(damageMove.priority, 0);
}

// Test PP management
TEST_F(MoveTest, PPManagement) {
    EXPECT_EQ(damageMove.getRemainingPP(), 15);
    EXPECT_EQ(damageMove.getMaxPP(), 15);
    EXPECT_TRUE(damageMove.canUse());
    
    // Test using PP
    damageMove.usePP();
    EXPECT_EQ(damageMove.getRemainingPP(), 14);
    EXPECT_TRUE(damageMove.canUse());
    
    // Test using all PP
    for (int i = 0; i < 14; ++i) {
        damageMove.usePP();
    }
    EXPECT_EQ(damageMove.getRemainingPP(), 0);
    EXPECT_FALSE(damageMove.canUse());
    
    // Test using PP when none remaining
    damageMove.usePP();
    EXPECT_EQ(damageMove.getRemainingPP(), 0); // Should not go negative
    
    // Test restoring PP
    damageMove.restorePP(5);
    EXPECT_EQ(damageMove.getRemainingPP(), 5);
    EXPECT_TRUE(damageMove.canUse());
    
    // Test restoring beyond max PP
    damageMove.restorePP(20);
    EXPECT_EQ(damageMove.getRemainingPP(), 15); // Should not exceed max
}

// Test move types and categories
TEST_F(MoveTest, MoveTypes) {
    // Test physical move
    EXPECT_EQ(damageMove.damage_class, "physical");
    EXPECT_GT(damageMove.power, 0);
    
    // Test special move
    EXPECT_EQ(specialMove.damage_class, "special");
    EXPECT_GT(specialMove.power, 0);
    EXPECT_EQ(specialMove.type, "fire");
    
    // Test status move
    EXPECT_EQ(statusMove.damage_class, "status");
    EXPECT_EQ(statusMove.power, 0);
    EXPECT_GT(statusMove.ailment_chance, 0);
}

// Test status condition application
TEST_F(MoveTest, StatusConditions) {
    EXPECT_EQ(statusMove.ailment_chance, 100);
    EXPECT_EQ(statusMove.getStatusCondition(), StatusCondition::POISON);
    
    // Test move without status condition
    EXPECT_EQ(damageMove.getStatusCondition(), StatusCondition::NONE);
    EXPECT_EQ(damageMove.ailment_chance, 0);
}

// Test move effects and properties
TEST_F(MoveTest, MoveEffects) {
    // Test critical hit rate
    EXPECT_EQ(damageMove.crit_rate, 0);
    
    // Test drain/healing
    EXPECT_EQ(damageMove.drain, 0);
    EXPECT_EQ(damageMove.healing, 0);
    
    // Test flinch chance
    EXPECT_EQ(damageMove.flinch_chance, 0);
    
    // Test stat modification chance
    EXPECT_EQ(damageMove.stat_chance, 0);
}

// Test move priority
TEST_F(MoveTest, MovePriority) {
    // Test normal priority
    EXPECT_EQ(damageMove.priority, 0);
    
    // Test high priority move
    Move quickMove = TestUtils::createTestMove("quickmove", 40, 100, 30, "normal", "physical");
    quickMove.priority = 1;
    EXPECT_EQ(quickMove.priority, 1);
    
    // Test low priority move
    Move slowMove = TestUtils::createTestMove("slowmove", 100, 80, 5, "normal", "physical");
    slowMove.priority = -1;
    EXPECT_EQ(slowMove.priority, -1);
}

// Test multi-hit moves
TEST_F(MoveTest, MultiHitMoves) {
    Move multiHitMove = TestUtils::createTestMove("multihit", 25, 85, 10, "normal", "physical");
    multiHitMove.min_hits = 2;
    multiHitMove.max_hits = 5;
    
    EXPECT_EQ(multiHitMove.min_hits, 2);
    EXPECT_EQ(multiHitMove.max_hits, 5);
    EXPECT_GT(multiHitMove.max_hits, multiHitMove.min_hits);
}

// Test healing moves
TEST_F(MoveTest, HealingMoves) {
    Move healingMove = TestUtils::createTestMove("heal", 0, 100, 10, "normal", "status");
    healingMove.healing = 50; // 50% healing
    
    EXPECT_EQ(healingMove.healing, 50);
    EXPECT_EQ(healingMove.power, 0);
    EXPECT_EQ(healingMove.damage_class, "status");
}

// Test draining moves
TEST_F(MoveTest, DrainingMoves) {
    Move drainingMove = TestUtils::createTestMove("drain", 75, 100, 10, "grass", "special");
    drainingMove.drain = 50; // 50% of damage dealt is recovered
    
    EXPECT_EQ(drainingMove.drain, 50);
    EXPECT_GT(drainingMove.power, 0);
}

// Test recoil moves
TEST_F(MoveTest, RecoilMoves) {
    Move recoilMove = TestUtils::createTestMove("recoil", 120, 100, 15, "normal", "physical");
    recoilMove.drain = -25; // 25% recoil damage
    
    EXPECT_EQ(recoilMove.drain, -25);
    EXPECT_GT(recoilMove.power, 0);
}

// Test move accuracy
TEST_F(MoveTest, MoveAccuracy) {
    EXPECT_EQ(damageMove.accuracy, 100);
    
    // Test low accuracy move
    Move lowAccuracyMove = TestUtils::createTestMove("lowaccuracy", 120, 70, 5, "normal", "physical");
    EXPECT_EQ(lowAccuracyMove.accuracy, 70);
    
    // Test never-miss move
    Move neverMissMove = TestUtils::createTestMove("nevermiss", 60, 0, 20, "normal", "physical");
    EXPECT_EQ(neverMissMove.accuracy, 0); // 0 accuracy means never miss
}

// Test move categories
TEST_F(MoveTest, MoveCategories) {
    // Test damage category
    EXPECT_EQ(damageMove.category, "damage");
    
    // Test ailment category
    EXPECT_EQ(statusMove.category, "ailment");
    
    // Test other categories
    Move statMove = TestUtils::createTestMove("statmove", 0, 100, 20, "normal", "status");
    statMove.category = "net-good-stats";
    EXPECT_EQ(statMove.category, "net-good-stats");
}

// Test OHKO moves
TEST_F(MoveTest, OHKOMoves) {
    Move ohkoMove = TestUtils::createTestMove("ohko", 0, 30, 5, "normal", "physical");
    ohkoMove.category = "ohko";
    
    EXPECT_EQ(ohkoMove.category, "ohko");
    EXPECT_EQ(ohkoMove.power, 0); // OHKO moves typically have 0 power
    EXPECT_LT(ohkoMove.accuracy, 100); // OHKO moves typically have low accuracy
}

// Test move validation
TEST_F(MoveTest, MoveValidation) {
    // Test that moves are created with valid data
    EXPECT_FALSE(damageMove.name.empty());
    EXPECT_FALSE(damageMove.type.empty());
    EXPECT_FALSE(damageMove.damage_class.empty());
    
    // Test that PP values are reasonable
    EXPECT_GT(damageMove.getMaxPP(), 0);
    EXPECT_GE(damageMove.current_pp, 0);
    EXPECT_LE(damageMove.current_pp, damageMove.getMaxPP());
    
    // Test that accuracy is within reasonable bounds
    EXPECT_GE(damageMove.accuracy, 0);
    EXPECT_LE(damageMove.accuracy, 100);
}

// Test move comparison and equality
TEST_F(MoveTest, MoveComparison) {
    Move move1 = TestUtils::createTestMove("testmove", 80, 100, 15, "normal", "physical");
    Move move2 = TestUtils::createTestMove("testmove", 80, 100, 15, "normal", "physical");
    
    // Test that moves with same properties are considered equal
    EXPECT_EQ(move1.name, move2.name);
    EXPECT_EQ(move1.power, move2.power);
    EXPECT_EQ(move1.accuracy, move2.accuracy);
    EXPECT_EQ(move1.pp, move2.pp);
    EXPECT_EQ(move1.type, move2.type);
    EXPECT_EQ(move1.damage_class, move2.damage_class);
}

// Test move effects with different ailments
TEST_F(MoveTest, VariousAilments) {
    // Test burn move
    Move burnMove = TestUtils::createTestMove("burnmove", 0, 85, 15, "fire", "status", StatusCondition::BURN, 100);
    EXPECT_EQ(burnMove.getStatusCondition(), StatusCondition::BURN);
    
    // Test paralysis move
    Move paralysisMove = TestUtils::createTestMove("paralysismove", 0, 100, 20, "electric", "status", StatusCondition::PARALYSIS, 100);
    EXPECT_EQ(paralysisMove.getStatusCondition(), StatusCondition::PARALYSIS);
    
    // Test sleep move
    Move sleepMove = TestUtils::createTestMove("sleepmove", 0, 75, 15, "grass", "status", StatusCondition::SLEEP, 100);
    EXPECT_EQ(sleepMove.getStatusCondition(), StatusCondition::SLEEP);
    
    // Test freeze move
    Move freezeMove = TestUtils::createTestMove("freezemove", 0, 70, 10, "ice", "status", StatusCondition::FREEZE, 100);
    EXPECT_EQ(freezeMove.getStatusCondition(), StatusCondition::FREEZE);
}

// Test move PP restoration to full
TEST_F(MoveTest, FullPPRestoration) {
    // Use up some PP
    damageMove.usePP();
    damageMove.usePP();
    damageMove.usePP();
    EXPECT_EQ(damageMove.getRemainingPP(), 12);
    
    // Restore to full
    damageMove.restorePP();
    EXPECT_EQ(damageMove.getRemainingPP(), damageMove.getMaxPP());
}