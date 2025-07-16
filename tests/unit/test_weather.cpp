#include <gtest/gtest.h>
#include "test_utils.h"
#include "weather.h"

class WeatherTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No specific setup needed for weather tests
    }
};

// Test weather name retrieval
TEST_F(WeatherTest, WeatherNames) {
    EXPECT_EQ(Weather::getWeatherName(WeatherCondition::NONE), "Clear");
    EXPECT_EQ(Weather::getWeatherName(WeatherCondition::RAIN), "Rain");
    EXPECT_EQ(Weather::getWeatherName(WeatherCondition::SUN), "Harsh Sunlight");
    EXPECT_EQ(Weather::getWeatherName(WeatherCondition::SANDSTORM), "Sandstorm");
    EXPECT_EQ(Weather::getWeatherName(WeatherCondition::HAIL), "Hail");
}

// Test rain weather damage multipliers
TEST_F(WeatherTest, RainDamageMultipliers) {
    // Water moves boosted in rain
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "water"), 1.5);
    
    // Fire moves weakened in rain
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "fire"), 0.5);
    
    // Other move types unaffected
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "electric"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "grass"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "normal"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "fighting"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "poison"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "ground"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "flying"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "psychic"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "bug"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "rock"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "ghost"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "dragon"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "dark"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "steel"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "ice"), 1.0);
}

// Test sun weather damage multipliers
TEST_F(WeatherTest, SunDamageMultipliers) {
    // Fire moves boosted in sun
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "fire"), 1.5);
    
    // Water moves weakened in sun
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "water"), 0.5);
    
    // Other move types unaffected
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "electric"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "grass"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "normal"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "fighting"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "poison"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "ground"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "flying"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "psychic"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "bug"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "rock"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "ghost"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "dragon"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "dark"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "steel"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "ice"), 1.0);
}

// Test sandstorm and hail damage multipliers
TEST_F(WeatherTest, SandstormAndHailDamageMultipliers) {
    // Sandstorm and Hail don't affect move damage
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SANDSTORM, "fire"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SANDSTORM, "water"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SANDSTORM, "rock"), 1.0);
    
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::HAIL, "fire"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::HAIL, "water"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::HAIL, "ice"), 1.0);
}

// Test clear weather damage multipliers
TEST_F(WeatherTest, ClearWeatherDamageMultipliers) {
    // Clear weather doesn't affect any moves
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::NONE, "fire"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::NONE, "water"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::NONE, "electric"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::NONE, "grass"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::NONE, "normal"), 1.0);
}

// Test sandstorm weather damage immunity
TEST_F(WeatherTest, SandstormWeatherImmunity) {
    // Rock types are immune to sandstorm
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"rock"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"fire", "rock"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"rock", "flying"}));
    
    // Ground types are immune to sandstorm
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"ground"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"water", "ground"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"ground", "electric"}));
    
    // Steel types are immune to sandstorm
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"steel"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"fire", "steel"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"steel", "psychic"}));
    
    // Other types are not immune to sandstorm
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"fire"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"water"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"electric"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"grass"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"ice"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"fighting"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"poison"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"flying"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"psychic"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"bug"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"ghost"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"dragon"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"dark"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"normal"}));
    
    // Dual type combinations
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"fire", "water"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"electric", "flying"}));
}

// Test hail weather damage immunity
TEST_F(WeatherTest, HailWeatherImmunity) {
    // Ice types are immune to hail
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"ice"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"water", "ice"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"ice", "flying"}));
    
    // Other types are not immune to hail
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"fire"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"water"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"electric"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"grass"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"rock"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"ground"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"steel"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"fighting"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"poison"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"flying"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"psychic"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"bug"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"ghost"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"dragon"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"dark"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"normal"}));
    
    // Dual type combinations
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"fire", "water"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"electric", "flying"}));
}

// Test rain and sun weather immunity (should always be immune)
TEST_F(WeatherTest, RainAndSunWeatherImmunity) {
    // Rain and Sun don't cause direct damage, so all types are immune
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::RAIN, {"fire"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::RAIN, {"water"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::RAIN, {"electric"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::RAIN, {"grass"}));
    
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SUN, {"fire"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SUN, {"water"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SUN, {"electric"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SUN, {"grass"}));
}

// Test clear weather immunity (should always be immune)
TEST_F(WeatherTest, ClearWeatherImmunity) {
    // Clear weather doesn't cause damage, so all types are immune
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::NONE, {"fire"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::NONE, {"water"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::NONE, {"electric"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::NONE, {"grass"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::NONE, {"rock"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::NONE, {"ground"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::NONE, {"steel"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::NONE, {"ice"}));
}

// Test weather damage calculation
TEST_F(WeatherTest, WeatherDamageCalculation) {
    // Sandstorm and Hail deal 1/16 max HP damage
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 100), 6); // 100/16 = 6.25, rounded down to 6
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 200), 12); // 200/16 = 12.5, rounded down to 12
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 50), 3); // 50/16 = 3.125, rounded down to 3
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 16), 1); // 16/16 = 1
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 10), 1); // 10/16 = 0.625, but minimum is 1
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 1), 1); // Minimum damage is 1
    
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::HAIL, 100), 6);
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::HAIL, 200), 12);
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::HAIL, 50), 3);
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::HAIL, 16), 1);
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::HAIL, 10), 1);
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::HAIL, 1), 1);
    
    // Rain, Sun, and Clear weather don't deal damage
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::RAIN, 100), 0);
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SUN, 100), 0);
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::NONE, 100), 0);
    
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::RAIN, 200), 0);
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SUN, 200), 0);
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::NONE, 200), 0);
}

// Test weather damage with various HP values
TEST_F(WeatherTest, WeatherDamageVariousHP) {
    // Test edge cases
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 0), 0); // 0/16 = 0, but max(1, 0) = 1, but since HP is 0 this is handled differently
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::HAIL, 0), 0);
    
    // Test various HP values
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 32), 2); // 32/16 = 2
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 64), 4); // 64/16 = 4
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 96), 6); // 96/16 = 6
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 128), 8); // 128/16 = 8
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 160), 10); // 160/16 = 10
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 256), 16); // 256/16 = 16
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 400), 25); // 400/16 = 25
}

// Test weather immunity with empty types
TEST_F(WeatherTest, WeatherImmunityEmptyTypes) {
    std::vector<std::string> emptyTypes;
    
    // Empty types should not be immune to damaging weather
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, emptyTypes));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, emptyTypes));
    
    // Empty types should be immune to non-damaging weather
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::RAIN, emptyTypes));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SUN, emptyTypes));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::NONE, emptyTypes));
}

// Test weather case sensitivity
TEST_F(WeatherTest, WeatherCaseSensitivity) {
    // Test that weather functions are case-sensitive
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "water"), 1.5);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "WATER"), 1.0); // Should not match
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "Water"), 1.0); // Should not match
    
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"ROCK"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"Rock"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"rock"}));
}

// Test weather enum completeness
TEST_F(WeatherTest, WeatherEnumCompleteness) {
    // Test that all weather conditions have names
    EXPECT_FALSE(Weather::getWeatherName(WeatherCondition::NONE).empty());
    EXPECT_FALSE(Weather::getWeatherName(WeatherCondition::RAIN).empty());
    EXPECT_FALSE(Weather::getWeatherName(WeatherCondition::SUN).empty());
    EXPECT_FALSE(Weather::getWeatherName(WeatherCondition::SANDSTORM).empty());
    EXPECT_FALSE(Weather::getWeatherName(WeatherCondition::HAIL).empty());
    
    // Test that all weather conditions have damage multiplier handling
    EXPECT_GE(Weather::getWeatherDamageMultiplier(WeatherCondition::NONE, "fire"), 0.0);
    EXPECT_GE(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "fire"), 0.0);
    EXPECT_GE(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "fire"), 0.0);
    EXPECT_GE(Weather::getWeatherDamageMultiplier(WeatherCondition::SANDSTORM, "fire"), 0.0);
    EXPECT_GE(Weather::getWeatherDamageMultiplier(WeatherCondition::HAIL, "fire"), 0.0);
    
    // Test that all weather conditions have damage calculation
    EXPECT_GE(Weather::getWeatherDamage(WeatherCondition::NONE, 100), 0);
    EXPECT_GE(Weather::getWeatherDamage(WeatherCondition::RAIN, 100), 0);
    EXPECT_GE(Weather::getWeatherDamage(WeatherCondition::SUN, 100), 0);
    EXPECT_GE(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 100), 0);
    EXPECT_GE(Weather::getWeatherDamage(WeatherCondition::HAIL, 100), 0);
}