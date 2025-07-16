#include <gtest/gtest.h>
#include "test_utils.h"
#include "battle.h"
#include "weather.h"

class WeatherIntegrationTest : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Create Pokemon with weather-related moves
        setupWeatherTestPokemon();
        
        // Create battle instance
        battle = std::make_unique<Battle>(playerTeam, opponentTeam, Battle::AIDifficulty::EASY);
    }
    
    void setupWeatherTestPokemon() {
        // Create Pokemon with weather-setting moves
        Pokemon rainSetter = TestUtils::createTestPokemon("rain", 100, 80, 70, 90, 85, 75, {"water"});
        Pokemon sunSetter = TestUtils::createTestPokemon("sun", 100, 80, 70, 90, 85, 75, {"fire"});
        Pokemon sandSetter = TestUtils::createTestPokemon("sand", 100, 80, 70, 90, 85, 75, {"ground", "rock"});
        Pokemon hailSetter = TestUtils::createTestPokemon("hail", 100, 80, 70, 90, 85, 75, {"ice"});
        
        // Rain setter moves
        rainSetter.moves.clear();
        rainSetter.moves.push_back(TestUtils::createTestMove("rain-dance", 0, 100, 5, "water", "status"));
        rainSetter.moves.push_back(TestUtils::createTestMove("hydro-pump", 110, 80, 5, "water", "special"));
        rainSetter.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
        rainSetter.moves.push_back(TestUtils::createTestMove("thunder", 110, 70, 10, "electric", "special"));
        
        // Sun setter moves
        sunSetter.moves.clear();
        sunSetter.moves.push_back(TestUtils::createTestMove("sunny-day", 0, 100, 5, "fire", "status"));
        sunSetter.moves.push_back(TestUtils::createTestMove("fire-blast", 110, 85, 5, "fire", "special"));
        sunSetter.moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special"));
        sunSetter.moves.push_back(TestUtils::createTestMove("solar-beam", 120, 100, 10, "grass", "special"));
        
        // Sand setter moves
        sandSetter.moves.clear();
        sandSetter.moves.push_back(TestUtils::createTestMove("sandstorm", 0, 100, 10, "rock", "status"));
        sandSetter.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
        sandSetter.moves.push_back(TestUtils::createTestMove("rock-slide", 75, 90, 10, "rock", "physical"));
        sandSetter.moves.push_back(TestUtils::createTestMove("stone-edge", 100, 80, 5, "rock", "physical"));
        
        // Hail setter moves
        hailSetter.moves.clear();
        hailSetter.moves.push_back(TestUtils::createTestMove("hail", 0, 100, 10, "ice", "status"));
        hailSetter.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
        hailSetter.moves.push_back(TestUtils::createTestMove("blizzard", 110, 70, 5, "ice", "special"));
        hailSetter.moves.push_back(TestUtils::createTestMove("icicle-spear", 25, 100, 30, "ice", "physical"));
        
        // Create teams
        std::unordered_map<std::string, std::vector<std::string>> weatherTeamData;
        std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>> weatherMovesData;
        
        weatherTeamData["PlayerTeam"] = {"rain", "sun"};
        weatherTeamData["OpponentTeam"] = {"sand", "hail"};
        
        weatherMovesData["PlayerTeam"] = {
            {"rain", {"rain-dance", "hydro-pump", "surf", "thunder"}},
            {"sun", {"sunny-day", "fire-blast", "flamethrower", "solar-beam"}}
        };
        
        weatherMovesData["OpponentTeam"] = {
            {"sand", {"sandstorm", "earthquake", "rock-slide", "stone-edge"}},
            {"hail", {"hail", "ice-beam", "blizzard", "icicle-spear"}}
        };
        
        playerTeam.loadTeams(weatherTeamData, weatherMovesData, "PlayerTeam");
        opponentTeam.loadTeams(weatherTeamData, weatherMovesData, "OpponentTeam");
    }
    
    std::unique_ptr<Battle> battle;
};

// Test rain weather integration
TEST_F(WeatherIntegrationTest, RainWeatherIntegration) {
    Pokemon rainPokemon = TestUtils::createTestPokemon("rain", 100, 80, 70, 90, 85, 75, {"water"});
    Pokemon firePokemon = TestUtils::createTestPokemon("fire", 100, 80, 70, 90, 85, 75, {"fire"});
    
    // Rain Pokemon has water moves
    rainPokemon.moves.clear();
    rainPokemon.moves.push_back(TestUtils::createTestMove("rain-dance", 0, 100, 5, "water", "status"));
    rainPokemon.moves.push_back(TestUtils::createTestMove("hydro-pump", 110, 80, 5, "water", "special"));
    rainPokemon.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
    rainPokemon.moves.push_back(TestUtils::createTestMove("thunder", 110, 70, 10, "electric", "special"));
    
    // Fire Pokemon has fire moves
    firePokemon.moves.clear();
    firePokemon.moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special"));
    firePokemon.moves.push_back(TestUtils::createTestMove("fire-blast", 110, 85, 5, "fire", "special"));
    firePokemon.moves.push_back(TestUtils::createTestMove("ember", 40, 100, 25, "fire", "special"));
    
    Team rainTeam = TestUtils::createTestTeam({rainPokemon});
    Team fireTeam = TestUtils::createTestTeam({firePokemon});
    
    Battle rainBattle(rainTeam, fireTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(rainBattle.isBattleOver());
    EXPECT_EQ(rainBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test rain weather effects
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "water"), 1.5);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "fire"), 0.5);
    
    // Test rain immunity (no direct damage)
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::RAIN, {"water"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::RAIN, {"fire"}));
    
    // Test rain damage calculation (no damage)
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::RAIN, 100), 0);
}

// Test sun weather integration
TEST_F(WeatherIntegrationTest, SunWeatherIntegration) {
    Pokemon sunPokemon = TestUtils::createTestPokemon("sun", 100, 80, 70, 90, 85, 75, {"fire"});
    Pokemon waterPokemon = TestUtils::createTestPokemon("water", 100, 80, 70, 90, 85, 75, {"water"});
    
    // Sun Pokemon has fire moves
    sunPokemon.moves.clear();
    sunPokemon.moves.push_back(TestUtils::createTestMove("sunny-day", 0, 100, 5, "fire", "status"));
    sunPokemon.moves.push_back(TestUtils::createTestMove("fire-blast", 110, 85, 5, "fire", "special"));
    sunPokemon.moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special"));
    sunPokemon.moves.push_back(TestUtils::createTestMove("solar-beam", 120, 100, 10, "grass", "special"));
    
    // Water Pokemon has water moves
    waterPokemon.moves.clear();
    waterPokemon.moves.push_back(TestUtils::createTestMove("hydro-pump", 110, 80, 5, "water", "special"));
    waterPokemon.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
    waterPokemon.moves.push_back(TestUtils::createTestMove("water-gun", 40, 100, 25, "water", "special"));
    
    Team sunTeam = TestUtils::createTestTeam({sunPokemon});
    Team waterTeam = TestUtils::createTestTeam({waterPokemon});
    
    Battle sunBattle(sunTeam, waterTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(sunBattle.isBattleOver());
    EXPECT_EQ(sunBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test sun weather effects
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "fire"), 1.5);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "water"), 0.5);
    
    // Test sun immunity (no direct damage)
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SUN, {"fire"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SUN, {"water"}));
    
    // Test sun damage calculation (no damage)
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SUN, 100), 0);
}

// Test sandstorm weather integration
TEST_F(WeatherIntegrationTest, SandstormWeatherIntegration) {
    Pokemon sandPokemon = TestUtils::createTestPokemon("sand", 100, 80, 70, 90, 85, 75, {"ground", "rock"});
    Pokemon normalPokemon = TestUtils::createTestPokemon("normal", 100, 80, 70, 90, 85, 75, {"normal"});
    Pokemon steelPokemon = TestUtils::createTestPokemon("steel", 100, 80, 70, 90, 85, 75, {"steel"});
    
    // Sand Pokemon has ground/rock moves
    sandPokemon.moves.clear();
    sandPokemon.moves.push_back(TestUtils::createTestMove("sandstorm", 0, 100, 10, "rock", "status"));
    sandPokemon.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
    sandPokemon.moves.push_back(TestUtils::createTestMove("rock-slide", 75, 90, 10, "rock", "physical"));
    sandPokemon.moves.push_back(TestUtils::createTestMove("stone-edge", 100, 80, 5, "rock", "physical"));
    
    // Normal Pokemon has normal moves
    normalPokemon.moves.clear();
    normalPokemon.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    normalPokemon.moves.push_back(TestUtils::createTestMove("body-slam", 85, 100, 15, "normal", "physical"));
    normalPokemon.moves.push_back(TestUtils::createTestMove("double-edge", 120, 100, 15, "normal", "physical"));
    
    // Steel Pokemon has steel moves
    steelPokemon.moves.clear();
    steelPokemon.moves.push_back(TestUtils::createTestMove("metal-claw", 50, 95, 35, "steel", "physical"));
    steelPokemon.moves.push_back(TestUtils::createTestMove("iron-head", 80, 100, 15, "steel", "physical"));
    steelPokemon.moves.push_back(TestUtils::createTestMove("meteor-mash", 90, 90, 10, "steel", "physical"));
    
    Team sandTeam = TestUtils::createTestTeam({sandPokemon});
    Team normalTeam = TestUtils::createTestTeam({normalPokemon});
    Team steelTeam = TestUtils::createTestTeam({steelPokemon});
    
    Battle sandBattle(sandTeam, normalTeam, Battle::AIDifficulty::EASY);
    Battle sandVsSteel(sandTeam, steelTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(sandBattle.isBattleOver());
    EXPECT_FALSE(sandVsSteel.isBattleOver());
    
    // Test sandstorm weather effects
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SANDSTORM, "rock"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SANDSTORM, "ground"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SANDSTORM, "normal"), 1.0);
    
    // Test sandstorm immunity
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"rock"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"ground"}));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"steel"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, {"normal"}));
    
    // Test sandstorm damage calculation
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 100), 6); // 100/16 = 6.25, rounded down
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 200), 12); // 200/16 = 12.5, rounded down
}

// Test hail weather integration
TEST_F(WeatherIntegrationTest, HailWeatherIntegration) {
    Pokemon hailPokemon = TestUtils::createTestPokemon("hail", 100, 80, 70, 90, 85, 75, {"ice"});
    Pokemon normalPokemon = TestUtils::createTestPokemon("normal", 100, 80, 70, 90, 85, 75, {"normal"});
    Pokemon waterPokemon = TestUtils::createTestPokemon("water", 100, 80, 70, 90, 85, 75, {"water"});
    
    // Hail Pokemon has ice moves
    hailPokemon.moves.clear();
    hailPokemon.moves.push_back(TestUtils::createTestMove("hail", 0, 100, 10, "ice", "status"));
    hailPokemon.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
    hailPokemon.moves.push_back(TestUtils::createTestMove("blizzard", 110, 70, 5, "ice", "special"));
    hailPokemon.moves.push_back(TestUtils::createTestMove("icicle-spear", 25, 100, 30, "ice", "physical"));
    
    // Normal Pokemon has normal moves
    normalPokemon.moves.clear();
    normalPokemon.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    normalPokemon.moves.push_back(TestUtils::createTestMove("body-slam", 85, 100, 15, "normal", "physical"));
    
    // Water Pokemon has water moves
    waterPokemon.moves.clear();
    waterPokemon.moves.push_back(TestUtils::createTestMove("hydro-pump", 110, 80, 5, "water", "special"));
    waterPokemon.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
    
    Team hailTeam = TestUtils::createTestTeam({hailPokemon});
    Team normalTeam = TestUtils::createTestTeam({normalPokemon});
    Team waterTeam = TestUtils::createTestTeam({waterPokemon});
    
    Battle hailBattle(hailTeam, normalTeam, Battle::AIDifficulty::EASY);
    Battle hailVsWater(hailTeam, waterTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(hailBattle.isBattleOver());
    EXPECT_FALSE(hailVsWater.isBattleOver());
    
    // Test hail weather effects
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::HAIL, "ice"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::HAIL, "water"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::HAIL, "normal"), 1.0);
    
    // Test hail immunity
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"ice"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"water"}));
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, {"normal"}));
    
    // Test hail damage calculation
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::HAIL, 100), 6); // 100/16 = 6.25, rounded down
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::HAIL, 200), 12); // 200/16 = 12.5, rounded down
}

// Test weather transitions and interactions
TEST_F(WeatherIntegrationTest, WeatherTransitionsAndInteractions) {
    Pokemon weatherChanger = TestUtils::createTestPokemon("changer", 100, 80, 70, 90, 85, 75, {"normal"});
    Pokemon opponent = TestUtils::createTestPokemon("opponent", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Weather changer has all weather moves
    weatherChanger.moves.clear();
    weatherChanger.moves.push_back(TestUtils::createTestMove("rain-dance", 0, 100, 5, "water", "status"));
    weatherChanger.moves.push_back(TestUtils::createTestMove("sunny-day", 0, 100, 5, "fire", "status"));
    weatherChanger.moves.push_back(TestUtils::createTestMove("sandstorm", 0, 100, 10, "rock", "status"));
    weatherChanger.moves.push_back(TestUtils::createTestMove("hail", 0, 100, 10, "ice", "status"));
    
    // Opponent has normal moves
    opponent.moves.clear();
    opponent.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    opponent.moves.push_back(TestUtils::createTestMove("body-slam", 85, 100, 15, "normal", "physical"));
    
    Team weatherTeam = TestUtils::createTestTeam({weatherChanger});
    Team normalTeam = TestUtils::createTestTeam({opponent});
    
    Battle weatherBattle(weatherTeam, normalTeam, Battle::AIDifficulty::EASY);
    
    EXPECT_FALSE(weatherBattle.isBattleOver());
    EXPECT_EQ(weatherBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test that weather moves are available
    EXPECT_GT(weatherChanger.moves.size(), 0);
    EXPECT_GT(opponent.moves.size(), 0);
    
    // Test weather enum values
    EXPECT_NE(WeatherCondition::NONE, WeatherCondition::RAIN);
    EXPECT_NE(WeatherCondition::RAIN, WeatherCondition::SUN);
    EXPECT_NE(WeatherCondition::SUN, WeatherCondition::SANDSTORM);
    EXPECT_NE(WeatherCondition::SANDSTORM, WeatherCondition::HAIL);
}

// Test weather effects on move damage
TEST_F(WeatherIntegrationTest, WeatherEffectsOnMoveDamage) {
    // Test rain boosting water moves and weakening fire moves
    double rainWaterMultiplier = Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "water");
    double rainFireMultiplier = Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "fire");
    
    EXPECT_DOUBLE_EQ(rainWaterMultiplier, 1.5);
    EXPECT_DOUBLE_EQ(rainFireMultiplier, 0.5);
    
    // Test sun boosting fire moves and weakening water moves
    double sunFireMultiplier = Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "fire");
    double sunWaterMultiplier = Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "water");
    
    EXPECT_DOUBLE_EQ(sunFireMultiplier, 1.5);
    EXPECT_DOUBLE_EQ(sunWaterMultiplier, 0.5);
    
    // Test that other weather doesn't affect move damage
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SANDSTORM, "fire"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::HAIL, "water"), 1.0);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::NONE, "fire"), 1.0);
}

// Test weather damage over time
TEST_F(WeatherIntegrationTest, WeatherDamageOverTime) {
    Pokemon sandstormVictim = TestUtils::createTestPokemon("victim", 100, 80, 70, 90, 85, 75, {"normal"});
    Pokemon hailVictim = TestUtils::createTestPokemon("victim", 100, 80, 70, 90, 85, 75, {"fire"});
    Pokemon immunePokemon = TestUtils::createTestPokemon("immune", 100, 80, 70, 90, 85, 75, {"rock"});
    
    // Test sandstorm damage
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, sandstormVictim.types));
    int sandstormDamage = Weather::getWeatherDamage(WeatherCondition::SANDSTORM, sandstormVictim.hp);
    EXPECT_GT(sandstormDamage, 0);
    EXPECT_EQ(sandstormDamage, 6); // 100/16 = 6.25, rounded down
    
    // Test hail damage
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, hailVictim.types));
    int hailDamage = Weather::getWeatherDamage(WeatherCondition::HAIL, hailVictim.hp);
    EXPECT_GT(hailDamage, 0);
    EXPECT_EQ(hailDamage, 6); // 100/16 = 6.25, rounded down
    
    // Test immunity
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, immunePokemon.types));
    
    // Test no damage from rain/sun
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::RAIN, 100), 0);
    EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SUN, 100), 0);
}

// Test weather with dual-type Pokemon
TEST_F(WeatherIntegrationTest, WeatherWithDualTypes) {
    Pokemon dualType1 = TestUtils::createTestPokemon("dual1", 100, 80, 70, 90, 85, 75, {"rock", "steel"});
    Pokemon dualType2 = TestUtils::createTestPokemon("dual2", 100, 80, 70, 90, 85, 75, {"water", "ice"});
    Pokemon dualType3 = TestUtils::createTestPokemon("dual3", 100, 80, 70, 90, 85, 75, {"normal", "flying"});
    
    // Test sandstorm immunity for dual-type with immune type
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, dualType1.types)); // Rock/Steel
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, dualType2.types)); // Water/Ice
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::SANDSTORM, dualType3.types)); // Normal/Flying
    
    // Test hail immunity for dual-type with immune type
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, dualType1.types)); // Rock/Steel
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, dualType2.types)); // Water/Ice
    EXPECT_FALSE(Weather::isImmuneToWeatherDamage(WeatherCondition::HAIL, dualType3.types)); // Normal/Flying
    
    // Test that rain/sun don't cause damage regardless of type
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::RAIN, dualType1.types));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::SUN, dualType2.types));
    EXPECT_TRUE(Weather::isImmuneToWeatherDamage(WeatherCondition::NONE, dualType3.types));
}

// Test weather battle scenarios
TEST_F(WeatherIntegrationTest, WeatherBattleScenarios) {
    Pokemon rainDancer = TestUtils::createTestPokemon("dancer", 100, 80, 70, 90, 85, 75, {"water"});
    Pokemon fireBreather = TestUtils::createTestPokemon("breather", 100, 80, 70, 90, 85, 75, {"fire"});
    
    // Rain dancer has rain setup
    rainDancer.moves.clear();
    rainDancer.moves.push_back(TestUtils::createTestMove("rain-dance", 0, 100, 5, "water", "status"));
    rainDancer.moves.push_back(TestUtils::createTestMove("hydro-pump", 110, 80, 5, "water", "special"));
    rainDancer.moves.push_back(TestUtils::createTestMove("thunder", 110, 70, 10, "electric", "special"));
    
    // Fire breather has fire moves
    fireBreather.moves.clear();
    fireBreather.moves.push_back(TestUtils::createTestMove("sunny-day", 0, 100, 5, "fire", "status"));
    fireBreather.moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special"));
    fireBreather.moves.push_back(TestUtils::createTestMove("solar-beam", 120, 100, 10, "grass", "special"));
    
    Team rainTeam = TestUtils::createTestTeam({rainDancer});
    Team fireTeam = TestUtils::createTestTeam({fireBreather});
    
    Battle weatherWarBattle(rainTeam, fireTeam, Battle::AIDifficulty::MEDIUM);
    
    EXPECT_FALSE(weatherWarBattle.isBattleOver());
    EXPECT_EQ(weatherWarBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test that both teams have weather-setting moves
    EXPECT_GT(rainDancer.moves.size(), 0);
    EXPECT_GT(fireBreather.moves.size(), 0);
    
    // Test that weather effects are correctly calculated
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "water"), 1.5);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "fire"), 1.5);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "fire"), 0.5);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "water"), 0.5);
}

// Test weather persistence and duration
TEST_F(WeatherIntegrationTest, WeatherPersistenceAndDuration) {
    // Test that weather effects persist across multiple calculations
    WeatherCondition currentWeather = WeatherCondition::RAIN;
    
    // Multiple calls should return consistent results
    for (int i = 0; i < 10; ++i) {
        EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(currentWeather, "water"), 1.5);
        EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(currentWeather, "fire"), 0.5);
    }
    
    // Change weather and test again
    currentWeather = WeatherCondition::SUN;
    
    for (int i = 0; i < 10; ++i) {
        EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(currentWeather, "fire"), 1.5);
        EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(currentWeather, "water"), 0.5);
    }
}

// Test weather integration with battle AI
TEST_F(WeatherIntegrationTest, WeatherWithBattleAI) {
    Pokemon weatherUser = TestUtils::createTestPokemon("user", 100, 80, 70, 90, 85, 75, {"normal"});
    Pokemon weatherTarget = TestUtils::createTestPokemon("target", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Weather user has all weather moves
    weatherUser.moves.clear();
    weatherUser.moves.push_back(TestUtils::createTestMove("rain-dance", 0, 100, 5, "water", "status"));
    weatherUser.moves.push_back(TestUtils::createTestMove("sunny-day", 0, 100, 5, "fire", "status"));
    weatherUser.moves.push_back(TestUtils::createTestMove("sandstorm", 0, 100, 10, "rock", "status"));
    weatherUser.moves.push_back(TestUtils::createTestMove("hail", 0, 100, 10, "ice", "status"));
    
    // Target has basic moves
    weatherTarget.moves.clear();
    weatherTarget.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
    weatherTarget.moves.push_back(TestUtils::createTestMove("body-slam", 85, 100, 15, "normal", "physical"));
    
    Team weatherTeam = TestUtils::createTestTeam({weatherUser});
    Team normalTeam = TestUtils::createTestTeam({weatherTarget});
    
    // Test with different AI difficulties
    Battle easyWeatherBattle(weatherTeam, normalTeam, Battle::AIDifficulty::EASY);
    Battle mediumWeatherBattle(weatherTeam, normalTeam, Battle::AIDifficulty::MEDIUM);
    Battle hardWeatherBattle(weatherTeam, normalTeam, Battle::AIDifficulty::HARD);
    
    EXPECT_FALSE(easyWeatherBattle.isBattleOver());
    EXPECT_FALSE(mediumWeatherBattle.isBattleOver());
    EXPECT_FALSE(hardWeatherBattle.isBattleOver());
    
    // All battles should be ongoing
    EXPECT_EQ(easyWeatherBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    EXPECT_EQ(mediumWeatherBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    EXPECT_EQ(hardWeatherBattle.getBattleResult(), Battle::BattleResult::ONGOING);
}

// Test weather integration main battle
TEST_F(WeatherIntegrationTest, WeatherIntegrationMainBattle) {
    EXPECT_FALSE(battle->isBattleOver());
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Both teams should have weather-setting Pokemon
    EXPECT_TRUE(playerTeam.hasAlivePokemon());
    EXPECT_TRUE(opponentTeam.hasAlivePokemon());
    
    // Check that teams have been set up correctly
    EXPECT_EQ(playerTeam.size(), 2);
    EXPECT_EQ(opponentTeam.size(), 2);
    
    // Test that Pokemon have weather-related moves
    Pokemon* playerPokemon = playerTeam.getPokemon(0);
    Pokemon* opponentPokemon = opponentTeam.getPokemon(0);
    
    ASSERT_NE(playerPokemon, nullptr);
    ASSERT_NE(opponentPokemon, nullptr);
    
    EXPECT_GT(playerPokemon->moves.size(), 0);
    EXPECT_GT(opponentPokemon->moves.size(), 0);
    
    // Test weather calculation consistency
    for (int i = 0; i < 5; ++i) {
        EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "water"), 1.5);
        EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::SUN, "fire"), 1.5);
        EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::SANDSTORM, 100), 6);
        EXPECT_EQ(Weather::getWeatherDamage(WeatherCondition::HAIL, 100), 6);
    }
}