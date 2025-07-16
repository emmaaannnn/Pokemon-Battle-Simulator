#include <gtest/gtest.h>
#include "test_utils.h"
#include "battle.h"

class BattleTest : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Create battle with easy AI
        battle = std::make_unique<Battle>(playerTeam, opponentTeam, Battle::AIDifficulty::EASY);
        battleMedium = std::make_unique<Battle>(playerTeam, opponentTeam, Battle::AIDifficulty::MEDIUM);
    }
    
    std::unique_ptr<Battle> battle;
    std::unique_ptr<Battle> battleMedium;
};

// Test battle construction
TEST_F(BattleTest, BattleConstruction) {
    EXPECT_NE(battle, nullptr);
    EXPECT_NE(battleMedium, nullptr);
    
    // Battle should not be over initially
    EXPECT_FALSE(battle->isBattleOver());
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
}

// Test battle result states
TEST_F(BattleTest, BattleResultStates) {
    // Initially ongoing
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
    EXPECT_FALSE(battle->isBattleOver());
    
    // Test with all player Pokemon fainted
    for (auto& pair : playerTeam) {
        pair.second.takeDamage(pair.second.hp);
    }
    
    Battle faintedPlayerBattle(playerTeam, opponentTeam);
    EXPECT_TRUE(faintedPlayerBattle.isBattleOver());
    EXPECT_EQ(faintedPlayerBattle.getBattleResult(), Battle::BattleResult::OPPONENT_WINS);
    
    // Test with all opponent Pokemon fainted
    for (auto& pair : opponentTeam) {
        pair.second.takeDamage(pair.second.hp);
    }
    
    Battle faintedOpponentBattle(playerTeam, opponentTeam);
    EXPECT_TRUE(faintedOpponentBattle.isBattleOver());
    EXPECT_EQ(faintedOpponentBattle.getBattleResult(), Battle::BattleResult::PLAYER_WINS);
}

// Test battle with different AI difficulties
TEST_F(BattleTest, BattleAIDifficulties) {
    // Test different AI difficulties can be constructed
    Battle easyBattle(playerTeam, opponentTeam, Battle::AIDifficulty::EASY);
    Battle mediumBattle(playerTeam, opponentTeam, Battle::AIDifficulty::MEDIUM);
    Battle hardBattle(playerTeam, opponentTeam, Battle::AIDifficulty::HARD);
    Battle expertBattle(playerTeam, opponentTeam, Battle::AIDifficulty::EXPERT);
    
    EXPECT_FALSE(easyBattle.isBattleOver());
    EXPECT_FALSE(mediumBattle.isBattleOver());
    EXPECT_FALSE(hardBattle.isBattleOver());
    EXPECT_FALSE(expertBattle.isBattleOver());
}

// Test battle state consistency
TEST_F(BattleTest, BattleStateConsistency) {
    // Battle should be ongoing when both teams have alive Pokemon
    EXPECT_TRUE(playerTeam.hasAlivePokemon());
    EXPECT_TRUE(opponentTeam.hasAlivePokemon());
    EXPECT_FALSE(battle->isBattleOver());
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
}

// Test battle with empty teams
TEST_F(BattleTest, BattleWithEmptyTeams) {
    Team emptyTeam;
    
    // Battle with empty player team
    Battle emptyPlayerBattle(emptyTeam, opponentTeam);
    EXPECT_TRUE(emptyPlayerBattle.isBattleOver());
    EXPECT_EQ(emptyPlayerBattle.getBattleResult(), Battle::BattleResult::OPPONENT_WINS);
    
    // Battle with empty opponent team  
    Battle emptyOpponentBattle(playerTeam, emptyTeam);
    EXPECT_TRUE(emptyOpponentBattle.isBattleOver());
    EXPECT_EQ(emptyOpponentBattle.getBattleResult(), Battle::BattleResult::PLAYER_WINS);
    
    // Battle with both teams empty
    Battle emptyBothBattle(emptyTeam, emptyTeam);
    EXPECT_TRUE(emptyBothBattle.isBattleOver());
    EXPECT_EQ(emptyBothBattle.getBattleResult(), Battle::BattleResult::DRAW);
}

// Test battle result transitions
TEST_F(BattleTest, BattleResultTransitions) {
    // Start with ongoing battle
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Faint player team gradually
    std::vector<Pokemon*> playerPokemon = playerTeam.getAlivePokemon();
    ASSERT_GT(playerPokemon.size(), 0);
    
    // Faint all but one player Pokemon
    for (size_t i = 0; i < playerPokemon.size() - 1; ++i) {
        playerPokemon[i]->takeDamage(playerPokemon[i]->hp);
    }
    
    // Should still be ongoing
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Faint the last player Pokemon
    playerPokemon.back()->takeDamage(playerPokemon.back()->hp);
    
    // Now opponent should win
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::OPPONENT_WINS);
    EXPECT_TRUE(battle->isBattleOver());
}

// Test battle with single Pokemon teams
TEST_F(BattleTest, SinglePokemonTeams) {
    // Create teams with single Pokemon
    std::unordered_map<std::string, std::vector<std::string>> singleTeamData;
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>> singleMovesData;
    
    singleTeamData["SingleTeam"] = {"testmon1"};
    singleMovesData["SingleTeam"] = {{"testmon1", {"testmove"}}};
    
    Team singlePlayerTeam;
    singlePlayerTeam.loadTeams(singleTeamData, singleMovesData, "SingleTeam");
    
    Team singleOpponentTeam;
    singleOpponentTeam.loadTeams(singleTeamData, singleMovesData, "SingleTeam");
    
    Battle singleBattle(singlePlayerTeam, singleOpponentTeam);
    
    EXPECT_FALSE(singleBattle.isBattleOver());
    EXPECT_EQ(singleBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Faint player's single Pokemon
    Pokemon* playerPokemon = singlePlayerTeam.getPokemon(0);
    ASSERT_NE(playerPokemon, nullptr);
    playerPokemon->takeDamage(playerPokemon->hp);
    
    EXPECT_TRUE(singleBattle.isBattleOver());
    EXPECT_EQ(singleBattle.getBattleResult(), Battle::BattleResult::OPPONENT_WINS);
}

// Test battle copy semantics
TEST_F(BattleTest, BattleCopySemantics) {
    // Test that battle can be constructed with team copies
    Team playerTeamCopy = playerTeam;
    Team opponentTeamCopy = opponentTeam;
    
    Battle battleCopy(playerTeamCopy, opponentTeamCopy);
    
    EXPECT_FALSE(battleCopy.isBattleOver());
    EXPECT_EQ(battleCopy.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Modifications to original teams shouldn't affect the copy
    for (auto& pair : playerTeam) {
        pair.second.takeDamage(pair.second.hp);
    }
    
    // Copy should still be ongoing
    EXPECT_FALSE(battleCopy.isBattleOver());
    EXPECT_EQ(battleCopy.getBattleResult(), Battle::BattleResult::ONGOING);
}

// Test battle with damaged Pokemon
TEST_F(BattleTest, BattleWithDamagedPokemon) {
    // Damage some Pokemon before battle
    Pokemon* playerPokemon = playerTeam.getPokemon(0);
    Pokemon* opponentPokemon = opponentTeam.getPokemon(0);
    
    ASSERT_NE(playerPokemon, nullptr);
    ASSERT_NE(opponentPokemon, nullptr);
    
    playerPokemon->takeDamage(50);
    opponentPokemon->takeDamage(30);
    
    Battle damagedBattle(playerTeam, opponentTeam);
    
    EXPECT_FALSE(damagedBattle.isBattleOver());
    EXPECT_EQ(damagedBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Pokemon should still be alive
    EXPECT_TRUE(playerPokemon->isAlive());
    EXPECT_TRUE(opponentPokemon->isAlive());
    EXPECT_LT(playerPokemon->getHealthPercentage(), 100.0);
    EXPECT_LT(opponentPokemon->getHealthPercentage(), 100.0);
}

// Test battle state queries
TEST_F(BattleTest, BattleStateQueries) {
    // Test initial state
    EXPECT_FALSE(battle->isBattleOver());
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Test consistency between isBattleOver() and getBattleResult()
    EXPECT_EQ(battle->isBattleOver(), (battle->getBattleResult() != Battle::BattleResult::ONGOING));
    
    // Test after making changes
    Pokemon* pokemon = playerTeam.getPokemon(0);
    ASSERT_NE(pokemon, nullptr);
    pokemon->takeDamage(pokemon->hp);
    
    // Still should be ongoing (other Pokemon alive)
    EXPECT_EQ(battle->getBattleResult(), Battle::BattleResult::ONGOING);
    EXPECT_FALSE(battle->isBattleOver());
}

// Test battle with status-affected Pokemon
TEST_F(BattleTest, BattleWithStatusEffects) {
    // Apply status conditions to Pokemon
    Pokemon* playerPokemon = playerTeam.getPokemon(0);
    Pokemon* opponentPokemon = opponentTeam.getPokemon(0);
    
    ASSERT_NE(playerPokemon, nullptr);
    ASSERT_NE(opponentPokemon, nullptr);
    
    playerPokemon->applyStatusCondition(StatusCondition::POISON);
    opponentPokemon->applyStatusCondition(StatusCondition::PARALYSIS);
    
    Battle statusBattle(playerTeam, opponentTeam);
    
    EXPECT_FALSE(statusBattle.isBattleOver());
    EXPECT_EQ(statusBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Status conditions shouldn't affect battle state directly
    EXPECT_TRUE(playerPokemon->hasStatusCondition());
    EXPECT_TRUE(opponentPokemon->hasStatusCondition());
    EXPECT_TRUE(playerPokemon->isAlive());
    EXPECT_TRUE(opponentPokemon->isAlive());
}

// Test battle result enum values
TEST_F(BattleTest, BattleResultEnumValues) {
    // Test that enum values are distinct
    EXPECT_NE(Battle::BattleResult::ONGOING, Battle::BattleResult::PLAYER_WINS);
    EXPECT_NE(Battle::BattleResult::ONGOING, Battle::BattleResult::OPPONENT_WINS);
    EXPECT_NE(Battle::BattleResult::ONGOING, Battle::BattleResult::DRAW);
    EXPECT_NE(Battle::BattleResult::PLAYER_WINS, Battle::BattleResult::OPPONENT_WINS);
    EXPECT_NE(Battle::BattleResult::PLAYER_WINS, Battle::BattleResult::DRAW);
    EXPECT_NE(Battle::BattleResult::OPPONENT_WINS, Battle::BattleResult::DRAW);
}

// Test AI difficulty enum values
TEST_F(BattleTest, AIDifficultyEnumValues) {
    // Test that enum values are distinct
    EXPECT_NE(Battle::AIDifficulty::EASY, Battle::AIDifficulty::MEDIUM);
    EXPECT_NE(Battle::AIDifficulty::EASY, Battle::AIDifficulty::HARD);
    EXPECT_NE(Battle::AIDifficulty::EASY, Battle::AIDifficulty::EXPERT);
    EXPECT_NE(Battle::AIDifficulty::MEDIUM, Battle::AIDifficulty::HARD);
    EXPECT_NE(Battle::AIDifficulty::MEDIUM, Battle::AIDifficulty::EXPERT);
    EXPECT_NE(Battle::AIDifficulty::HARD, Battle::AIDifficulty::EXPERT);
}

// Test battle with mixed team states
TEST_F(BattleTest, BattleWithMixedTeamStates) {
    // Create a team with some fainted Pokemon
    Pokemon* playerPokemon1 = playerTeam.getPokemon(0);
    Pokemon* playerPokemon2 = playerTeam.getPokemon(1);
    
    ASSERT_NE(playerPokemon1, nullptr);
    ASSERT_NE(playerPokemon2, nullptr);
    
    // Faint first Pokemon
    playerPokemon1->takeDamage(playerPokemon1->hp);
    
    Battle mixedBattle(playerTeam, opponentTeam);
    
    EXPECT_FALSE(mixedBattle.isBattleOver());
    EXPECT_EQ(mixedBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Should still have alive Pokemon
    EXPECT_TRUE(playerTeam.hasAlivePokemon());
    EXPECT_TRUE(playerPokemon2->isAlive());
    EXPECT_FALSE(playerPokemon1->isAlive());
}