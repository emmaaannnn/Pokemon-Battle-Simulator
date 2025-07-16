#include <gtest/gtest.h>
#include "test_utils.h"
#include "pokemon.h"

class PokemonTest : public TestUtils::PokemonTestFixture {
protected:
    void SetUp() override {
        TestUtils::PokemonTestFixture::SetUp();
        
        // Create test Pokemon
        testPokemon = TestUtils::createTestPokemon("testmon", 100, 80, 70, 90, 85, 75, {"normal"});
    }
    
    Pokemon testPokemon;
};

// Test Pokemon creation and basic properties
TEST_F(PokemonTest, BasicProperties) {
    EXPECT_EQ(testPokemon.name, "testmon");
    EXPECT_EQ(testPokemon.hp, 100);
    EXPECT_EQ(testPokemon.current_hp, 100);
    EXPECT_EQ(testPokemon.attack, 80);
    EXPECT_EQ(testPokemon.defense, 70);
    EXPECT_EQ(testPokemon.special_attack, 90);
    EXPECT_EQ(testPokemon.special_defense, 85);
    EXPECT_EQ(testPokemon.speed, 75);
    EXPECT_EQ(testPokemon.types.size(), 1);
    EXPECT_EQ(testPokemon.types[0], "normal");
}

// Test Pokemon health management
TEST_F(PokemonTest, HealthManagement) {
    EXPECT_TRUE(testPokemon.isAlive());
    EXPECT_FLOAT_EQ(testPokemon.getHealthPercentage(), 100.0);
    
    // Test taking damage
    testPokemon.takeDamage(30);
    EXPECT_EQ(testPokemon.current_hp, 70);
    EXPECT_FLOAT_EQ(testPokemon.getHealthPercentage(), 70.0);
    EXPECT_TRUE(testPokemon.isAlive());
    
    // Test healing
    testPokemon.heal(20);
    EXPECT_EQ(testPokemon.current_hp, 90);
    EXPECT_FLOAT_EQ(testPokemon.getHealthPercentage(), 90.0);
    
    // Test healing beyond max HP
    testPokemon.heal(50);
    EXPECT_EQ(testPokemon.current_hp, 100);
    EXPECT_FLOAT_EQ(testPokemon.getHealthPercentage(), 100.0);
    
    // Test fainting
    testPokemon.takeDamage(150);
    EXPECT_EQ(testPokemon.current_hp, 0);
    EXPECT_FLOAT_EQ(testPokemon.getHealthPercentage(), 0.0);
    EXPECT_FALSE(testPokemon.isAlive());
}

// Test status condition management
TEST_F(PokemonTest, StatusConditions) {
    EXPECT_FALSE(testPokemon.hasStatusCondition());
    EXPECT_EQ(testPokemon.status, StatusCondition::NONE);
    
    // Test applying poison
    testPokemon.applyStatusCondition(StatusCondition::POISON);
    EXPECT_TRUE(testPokemon.hasStatusCondition());
    EXPECT_EQ(testPokemon.status, StatusCondition::POISON);
    EXPECT_EQ(testPokemon.getStatusConditionName(), "Poisoned");
    
    // Test status immunity (can't apply another status when already statused)
    testPokemon.applyStatusCondition(StatusCondition::BURN);
    EXPECT_EQ(testPokemon.status, StatusCondition::POISON); // Should remain poisoned
    
    // Test clearing status
    testPokemon.clearStatusCondition();
    EXPECT_FALSE(testPokemon.hasStatusCondition());
    EXPECT_EQ(testPokemon.status, StatusCondition::NONE);
}

// Test status condition processing
TEST_F(PokemonTest, StatusConditionProcessing) {
    // Test poison processing
    testPokemon.applyStatusCondition(StatusCondition::POISON);
    int initialHp = testPokemon.current_hp;
    testPokemon.processStatusCondition();
    EXPECT_LT(testPokemon.current_hp, initialHp); // Should take damage
    
    // Test burn processing
    testPokemon.clearStatusCondition();
    testPokemon.applyStatusCondition(StatusCondition::BURN);
    initialHp = testPokemon.current_hp;
    testPokemon.processStatusCondition();
    EXPECT_LT(testPokemon.current_hp, initialHp); // Should take damage
    
    // Test paralysis processing (shouldn't damage)
    testPokemon.clearStatusCondition();
    testPokemon.applyStatusCondition(StatusCondition::PARALYSIS);
    initialHp = testPokemon.current_hp;
    testPokemon.processStatusCondition();
    EXPECT_EQ(testPokemon.current_hp, initialHp); // Should not take damage
}

// Test stat modifications
TEST_F(PokemonTest, StatModifications) {
    // Test attack modification
    int baseAttack = testPokemon.attack;
    testPokemon.modifyAttack(2);
    EXPECT_EQ(testPokemon.attack_stage, 2);
    EXPECT_GT(testPokemon.getEffectiveAttack(), baseAttack);
    
    // Test defense modification
    int baseDefense = testPokemon.defense;
    testPokemon.modifyDefense(-1);
    EXPECT_EQ(testPokemon.defense_stage, -1);
    EXPECT_LT(testPokemon.getEffectiveDefense(), baseDefense);
    
    // Test speed modification
    int baseSpeed = testPokemon.speed;
    testPokemon.modifySpeed(1);
    EXPECT_EQ(testPokemon.speed_stage, 1);
    EXPECT_GT(testPokemon.getEffectiveSpeed(), baseSpeed);
    
    // Test special attack modification
    int baseSpAttack = testPokemon.special_attack;
    testPokemon.modifySpecialAttack(3);
    EXPECT_EQ(testPokemon.special_attack_stage, 3);
    EXPECT_GT(testPokemon.getEffectiveSpecialAttack(), baseSpAttack);
    
    // Test special defense modification
    int baseSpDefense = testPokemon.special_defense;
    testPokemon.modifySpecialDefense(-2);
    EXPECT_EQ(testPokemon.special_defense_stage, -2);
    EXPECT_LT(testPokemon.getEffectiveSpecialDefense(), baseSpDefense);
}

// Test stat modification limits
TEST_F(PokemonTest, StatModificationLimits) {
    // Test maximum stat boost (+6)
    for (int i = 0; i < 10; ++i) {
        testPokemon.modifyAttack(1);
    }
    EXPECT_EQ(testPokemon.attack_stage, 6); // Should cap at +6
    
    // Test minimum stat reduction (-6)
    for (int i = 0; i < 15; ++i) {
        testPokemon.modifyDefense(-1);
    }
    EXPECT_EQ(testPokemon.defense_stage, -6); // Should cap at -6
}

// Test effective stat calculations with status effects
TEST_F(PokemonTest, EffectiveStatsWithStatus) {
    int baseAttack = testPokemon.attack;
    int baseSpeed = testPokemon.speed;
    
    // Test burn effect on attack
    testPokemon.applyStatusCondition(StatusCondition::BURN);
    EXPECT_LT(testPokemon.getEffectiveAttack(), baseAttack); // Attack should be halved
    
    // Test paralysis effect on speed
    testPokemon.clearStatusCondition();
    testPokemon.applyStatusCondition(StatusCondition::PARALYSIS);
    EXPECT_LT(testPokemon.getEffectiveSpeed(), baseSpeed); // Speed should be halved
}

// Test canAct method
TEST_F(PokemonTest, CanAct) {
    // Normal state - should be able to act
    EXPECT_TRUE(testPokemon.canAct());
    
    // Poisoned - should be able to act
    testPokemon.applyStatusCondition(StatusCondition::POISON);
    EXPECT_TRUE(testPokemon.canAct());
    
    // Burned - should be able to act
    testPokemon.clearStatusCondition();
    testPokemon.applyStatusCondition(StatusCondition::BURN);
    EXPECT_TRUE(testPokemon.canAct());
    
    // Paralyzed - might not be able to act (25% chance)
    testPokemon.clearStatusCondition();
    testPokemon.applyStatusCondition(StatusCondition::PARALYSIS);
    // Note: This test is probabilistic, so we'll test multiple times
    bool canActAtLeastOnce = false;
    bool cannotActAtLeastOnce = false;
    for (int i = 0; i < 100; ++i) {
        if (testPokemon.canAct()) {
            canActAtLeastOnce = true;
        } else {
            cannotActAtLeastOnce = true;
        }
    }
    EXPECT_TRUE(canActAtLeastOnce);
    EXPECT_TRUE(cannotActAtLeastOnce);
    
    // Sleeping - should not be able to act initially
    testPokemon.clearStatusCondition();
    testPokemon.applyStatusCondition(StatusCondition::SLEEP);
    // Sleep has turns remaining, so initially can't act
    // This might need adjustment based on implementation
    
    // Frozen - should not be able to act initially
    testPokemon.clearStatusCondition();
    testPokemon.applyStatusCondition(StatusCondition::FREEZE);
    // Similar to sleep, initially can't act
}

// Test dual-type Pokemon
TEST_F(PokemonTest, DualTypes) {
    Pokemon dualTypePokemon = TestUtils::createTestPokemon("dualmon", 100, 80, 70, 90, 85, 75, {"fire", "flying"});
    
    EXPECT_EQ(dualTypePokemon.types.size(), 2);
    EXPECT_EQ(dualTypePokemon.types[0], "fire");
    EXPECT_EQ(dualTypePokemon.types[1], "flying");
}

// Test Pokemon with moves
TEST_F(PokemonTest, MovesManagement) {
    EXPECT_GT(testPokemon.moves.size(), 0);
    
    // Test move access
    if (!testPokemon.moves.empty()) {
        Move& firstMove = testPokemon.moves[0];
        EXPECT_FALSE(firstMove.name.empty());
        EXPECT_TRUE(firstMove.canUse());
        
        // Test PP usage
        int initialPP = firstMove.getRemainingPP();
        firstMove.usePP();
        EXPECT_EQ(firstMove.getRemainingPP(), initialPP - 1);
    }
}

// Test Pokemon initialization from JSON (if loadFromFile method exists)
TEST_F(PokemonTest, JsonDataLoading) {
    // This test depends on the actual implementation
    // We'll create a basic test for data consistency
    Pokemon pokemon;
    
    // Test that Pokemon can be created with valid data
    EXPECT_NO_THROW({
        pokemon = TestUtils::createTestPokemon("testmon", 100, 80, 70, 90, 85, 75, {"normal"});
    });
    
    // Verify the data is correctly set
    EXPECT_EQ(pokemon.name, "testmon");
    EXPECT_EQ(pokemon.hp, 100);
    EXPECT_EQ(pokemon.types[0], "normal");
}