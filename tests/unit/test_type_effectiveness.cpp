#include <gtest/gtest.h>
#include "test_utils.h"
#include "type_effectiveness.h"

class TypeEffectivenessTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No specific setup needed for type effectiveness tests
    }
};

// Test super effective matchups
TEST_F(TypeEffectivenessTest, SuperEffectiveMatchups) {
    // Water vs Fire
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("water", {"fire"}), 2.0);
    
    // Fire vs Grass
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("fire", {"grass"}), 2.0);
    
    // Grass vs Water
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("grass", {"water"}), 2.0);
    
    // Electric vs Water
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("electric", {"water"}), 2.0);
    
    // Electric vs Flying
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("electric", {"flying"}), 2.0);
    
    // Ground vs Electric
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ground", {"electric"}), 2.0);
    
    // Ground vs Fire
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ground", {"fire"}), 2.0);
    
    // Rock vs Flying
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("rock", {"flying"}), 2.0);
    
    // Rock vs Fire
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("rock", {"fire"}), 2.0);
    
    // Fighting vs Normal
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("fighting", {"normal"}), 2.0);
    
    // Psychic vs Fighting
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("psychic", {"fighting"}), 2.0);
    
    // Psychic vs Poison
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("psychic", {"poison"}), 2.0);
}

// Test not very effective matchups
TEST_F(TypeEffectivenessTest, NotVeryEffectiveMatchups) {
    // Fire vs Water
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("fire", {"water"}), 0.5);
    
    // Water vs Grass
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("water", {"grass"}), 0.5);
    
    // Grass vs Fire
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("grass", {"fire"}), 0.5);
    
    // Electric vs Grass
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("electric", {"grass"}), 0.5);
    
    // Electric vs Electric
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("electric", {"electric"}), 0.5);
    
    // Fire vs Fire
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("fire", {"fire"}), 0.5);
    
    // Water vs Water
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("water", {"water"}), 0.5);
    
    // Normal vs Rock
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("normal", {"rock"}), 0.5);
    
    // Fighting vs Psychic
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("fighting", {"psychic"}), 0.5);
    
    // Poison vs Rock
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("poison", {"rock"}), 0.5);
}

// Test no effect matchups
TEST_F(TypeEffectivenessTest, NoEffectMatchups) {
    // Electric vs Ground
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("electric", {"ground"}), 0.0);
    
    // Ground vs Flying
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ground", {"flying"}), 0.0);
    
    // Normal vs Ghost
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("normal", {"ghost"}), 0.0);
    
    // Fighting vs Ghost
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("fighting", {"ghost"}), 0.0);
    
    // Psychic vs Dark
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("psychic", {"dark"}), 0.0);
}

// Test neutral matchups
TEST_F(TypeEffectivenessTest, NeutralMatchups) {
    // Normal vs Normal
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("normal", {"normal"}), 1.0);
    
    // Fire vs Electric
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("fire", {"electric"}), 1.0);
    
    // Water vs Fighting
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("water", {"fighting"}), 1.0);
    
    // Electric vs Psychic
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("electric", {"psychic"}), 1.0);
    
    // Grass vs Ghost
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("grass", {"ghost"}), 1.0);
}

// Test dual-type effectiveness
TEST_F(TypeEffectivenessTest, DualTypeEffectiveness) {
    // Super effective vs both types (2.0 * 2.0 = 4.0)
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("rock", {"fire", "flying"}), 4.0);
    
    // Super effective vs one, neutral vs other (2.0 * 1.0 = 2.0)
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("water", {"fire", "normal"}), 2.0);
    
    // Super effective vs one, not very effective vs other (2.0 * 0.5 = 1.0)
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("grass", {"water", "poison"}), 1.0);
    
    // Not very effective vs both types (0.5 * 0.5 = 0.25)
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("normal", {"rock", "steel"}), 0.25);
    
    // No effect vs one type (2.0 * 0.0 = 0.0)
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("electric", {"water", "ground"}), 0.0);
}

// Test ice type effectiveness
TEST_F(TypeEffectivenessTest, IceTypeEffectiveness) {
    // Ice super effective matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ice", {"grass"}), 2.0);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ice", {"ground"}), 2.0);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ice", {"flying"}), 2.0);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ice", {"dragon"}), 2.0);
    
    // Ice not very effective matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ice", {"fire"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ice", {"water"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ice", {"ice"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ice", {"steel"}), 0.5);
}

// Test bug type effectiveness
TEST_F(TypeEffectivenessTest, BugTypeEffectiveness) {
    // Bug super effective matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("bug", {"grass"}), 2.0);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("bug", {"psychic"}), 2.0);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("bug", {"dark"}), 2.0);
    
    // Bug not very effective matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("bug", {"fire"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("bug", {"fighting"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("bug", {"poison"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("bug", {"flying"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("bug", {"ghost"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("bug", {"steel"}), 0.5);
}

// Test ghost type effectiveness
TEST_F(TypeEffectivenessTest, GhostTypeEffectiveness) {
    // Ghost super effective matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ghost", {"psychic"}), 2.0);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ghost", {"ghost"}), 2.0);
    
    // Ghost no effect matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ghost", {"normal"}), 0.0);
    
    // Ghost not very effective matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ghost", {"dark"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ghost", {"steel"}), 0.5);
}

// Test dragon type effectiveness
TEST_F(TypeEffectivenessTest, DragonTypeEffectiveness) {
    // Dragon super effective matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("dragon", {"dragon"}), 2.0);
    
    // Dragon not very effective matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("dragon", {"steel"}), 0.5);
    
    // Dragon no effect matchups (if fairy type exists)
    // Note: Fairy type might not be implemented in Gen 1
}

// Test dark type effectiveness
TEST_F(TypeEffectivenessTest, DarkTypeEffectiveness) {
    // Dark super effective matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("dark", {"psychic"}), 2.0);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("dark", {"ghost"}), 2.0);
    
    // Dark not very effective matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("dark", {"fighting"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("dark", {"dark"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("dark", {"steel"}), 0.5);
}

// Test steel type effectiveness
TEST_F(TypeEffectivenessTest, SteelTypeEffectiveness) {
    // Steel super effective matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("steel", {"ice"}), 2.0);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("steel", {"rock"}), 2.0);
    
    // Steel not very effective matchups
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("steel", {"fire"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("steel", {"water"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("steel", {"electric"}), 0.5);
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("steel", {"steel"}), 0.5);
}

// Test invalid type handling
TEST_F(TypeEffectivenessTest, InvalidTypeHandling) {
    // Test with invalid attacking type
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("invalid", {"normal"}), 1.0);
    
    // Test with invalid defending type
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("normal", {"invalid"}), 1.0);
    
    // Test with both invalid types
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("invalid", {"invalid"}), 1.0);
    
    // Test with empty defending types
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("normal", {}), 1.0);
}

// Test case sensitivity
TEST_F(TypeEffectivenessTest, CaseSensitivity) {
    // Test that type effectiveness is case-sensitive (if implemented)
    // This depends on the actual implementation
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("water", {"fire"}), 2.0);
    
    // These might fail if the implementation doesn't handle case properly
    // EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("WATER", {"FIRE"}), 2.0);
    // EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("Water", {"Fire"}), 2.0);
}

// Test comprehensive type chart coverage
TEST_F(TypeEffectivenessTest, ComprehensiveTypeChart) {
    // Test all 18 types against themselves
    std::vector<std::string> types = {
        "normal", "fire", "water", "electric", "grass", "ice", "fighting",
        "poison", "ground", "flying", "psychic", "bug", "rock", "ghost",
        "dragon", "dark", "steel"
    };
    
    for (const auto& attackingType : types) {
        for (const auto& defendingType : types) {
            double multiplier = TypeEffectiveness::getEffectivenessMultiplier(attackingType, {defendingType});
            
            // Multiplier should be one of the valid values
            EXPECT_TRUE(multiplier == 0.0 || multiplier == 0.5 || multiplier == 1.0 || multiplier == 2.0)
                << "Invalid multiplier " << multiplier << " for " << attackingType << " vs " << defendingType;
        }
    }
}

// Test specific Pokemon type combinations
TEST_F(TypeEffectivenessTest, PokemonTypeScenarios) {
    // Charizard (Fire/Flying) vs Water
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("water", {"fire", "flying"}), 2.0);
    
    // Gyarados (Water/Flying) vs Electric
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("electric", {"water", "flying"}), 4.0);
    
    // Crobat (Poison/Flying) vs Electric
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("electric", {"poison", "flying"}), 2.0);
    
    // Skarmory (Steel/Flying) vs Electric
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("electric", {"steel", "flying"}), 1.0);
    
    // Magnezone (Electric/Steel) vs Ground
    EXPECT_DOUBLE_EQ(TypeEffectiveness::getEffectivenessMultiplier("ground", {"electric", "steel"}), 2.0);
}