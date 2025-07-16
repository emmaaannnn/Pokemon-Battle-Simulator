#include <gtest/gtest.h>
#include "test_utils.h"
#include "team.h"

class TeamTest : public TestUtils::PokemonTestFixture {
protected:
    void SetUp() override {
        TestUtils::PokemonTestFixture::SetUp();
        
        // Create test Pokemon
        testPokemon1 = TestUtils::createTestPokemon("testmon1", 100, 80, 70, 90, 85, 75, {"normal"});
        testPokemon2 = TestUtils::createTestPokemon("testmon2", 90, 85, 65, 95, 80, 85, {"fire"});
        testPokemon3 = TestUtils::createTestPokemon("testmon3", 110, 75, 80, 85, 90, 70, {"water"});
        
        // Create test team data
        teamData["TestTeam"] = {"testmon1", "testmon2", "testmon3"};
        
        // Create moves data
        movesData["TestTeam"] = {
            {"testmon1", {"testmove"}},
            {"testmon2", {"testmove"}},
            {"testmon3", {"testmove"}}
        };
    }
    
    Pokemon testPokemon1;
    Pokemon testPokemon2;
    Pokemon testPokemon3;
    
    std::unordered_map<std::string, std::vector<std::string>> teamData;
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>> movesData;
};

// Test team creation and basic properties
TEST_F(TeamTest, TeamCreation) {
    Team team;
    EXPECT_TRUE(team.isEmpty());
    EXPECT_EQ(team.size(), 0);
    EXPECT_FALSE(team.hasAlivePokemon());
}

// Test loading teams from configuration
TEST_F(TeamTest, LoadTeamFromConfiguration) {
    Team team;
    team.loadTeams(teamData, movesData, "TestTeam");
    
    EXPECT_FALSE(team.isEmpty());
    EXPECT_EQ(team.size(), 3);
    EXPECT_TRUE(team.hasAlivePokemon());
    
    // Check that Pokemon were loaded correctly
    Pokemon* pokemon1 = team.getPokemon(0);
    Pokemon* pokemon2 = team.getPokemon(1);
    Pokemon* pokemon3 = team.getPokemon(2);
    
    ASSERT_NE(pokemon1, nullptr);
    ASSERT_NE(pokemon2, nullptr);
    ASSERT_NE(pokemon3, nullptr);
    
    EXPECT_EQ(pokemon1->name, "testmon1");
    EXPECT_EQ(pokemon2->name, "testmon2");
    EXPECT_EQ(pokemon3->name, "testmon3");
}

// Test getPokemon method
TEST_F(TeamTest, GetPokemon) {
    Team team;
    team.loadTeams(teamData, movesData, "TestTeam");
    
    // Test valid indices
    Pokemon* pokemon = team.getPokemon(0);
    ASSERT_NE(pokemon, nullptr);
    EXPECT_EQ(pokemon->name, "testmon1");
    
    pokemon = team.getPokemon(1);
    ASSERT_NE(pokemon, nullptr);
    EXPECT_EQ(pokemon->name, "testmon2");
    
    pokemon = team.getPokemon(2);
    ASSERT_NE(pokemon, nullptr);
    EXPECT_EQ(pokemon->name, "testmon3");
    
    // Test invalid index
    pokemon = team.getPokemon(10);
    EXPECT_EQ(pokemon, nullptr);
    
    // Test negative index
    pokemon = team.getPokemon(-1);
    EXPECT_EQ(pokemon, nullptr);
}

// Test const getPokemon method
TEST_F(TeamTest, GetPokemonConst) {
    Team team;
    team.loadTeams(teamData, movesData, "TestTeam");
    
    const Team& constTeam = team;
    
    // Test valid indices
    const Pokemon* pokemon = constTeam.getPokemon(0);
    ASSERT_NE(pokemon, nullptr);
    EXPECT_EQ(pokemon->name, "testmon1");
    
    // Test invalid index
    pokemon = constTeam.getPokemon(10);
    EXPECT_EQ(pokemon, nullptr);
}

// Test hasAlivePokemon method
TEST_F(TeamTest, HasAlivePokemon) {
    Team team;
    team.loadTeams(teamData, movesData, "TestTeam");
    
    // All Pokemon should be alive initially
    EXPECT_TRUE(team.hasAlivePokemon());
    
    // Faint all Pokemon
    for (size_t i = 0; i < team.size(); ++i) {
        Pokemon* pokemon = team.getPokemon(i);
        ASSERT_NE(pokemon, nullptr);
        pokemon->takeDamage(pokemon->hp); // Faint the Pokemon
    }
    
    // No Pokemon should be alive now
    EXPECT_FALSE(team.hasAlivePokemon());
}

// Test getAlivePokemon method
TEST_F(TeamTest, GetAlivePokemon) {
    Team team;
    team.loadTeams(teamData, movesData, "TestTeam");
    
    // All Pokemon should be alive initially
    std::vector<Pokemon*> alivePokemon = team.getAlivePokemon();
    EXPECT_EQ(alivePokemon.size(), 3);
    
    // Faint first Pokemon
    Pokemon* pokemon1 = team.getPokemon(0);
    ASSERT_NE(pokemon1, nullptr);
    pokemon1->takeDamage(pokemon1->hp);
    
    // Should have 2 alive Pokemon
    alivePokemon = team.getAlivePokemon();
    EXPECT_EQ(alivePokemon.size(), 2);
    
    // Verify the alive Pokemon are correct
    EXPECT_EQ(alivePokemon[0]->name, "testmon2");
    EXPECT_EQ(alivePokemon[1]->name, "testmon3");
    
    // Faint all Pokemon
    for (size_t i = 0; i < team.size(); ++i) {
        Pokemon* pokemon = team.getPokemon(i);
        if (pokemon && pokemon->isAlive()) {
            pokemon->takeDamage(pokemon->hp);
        }
    }
    
    // Should have no alive Pokemon
    alivePokemon = team.getAlivePokemon();
    EXPECT_EQ(alivePokemon.size(), 0);
}

// Test getFirstAlivePokemon method
TEST_F(TeamTest, GetFirstAlivePokemon) {
    Team team;
    team.loadTeams(teamData, movesData, "TestTeam");
    
    // First Pokemon should be alive
    Pokemon* firstAlive = team.getFirstAlivePokemon();
    ASSERT_NE(firstAlive, nullptr);
    EXPECT_EQ(firstAlive->name, "testmon1");
    
    // Faint first Pokemon
    Pokemon* pokemon1 = team.getPokemon(0);
    ASSERT_NE(pokemon1, nullptr);
    pokemon1->takeDamage(pokemon1->hp);
    
    // Second Pokemon should now be first alive
    firstAlive = team.getFirstAlivePokemon();
    ASSERT_NE(firstAlive, nullptr);
    EXPECT_EQ(firstAlive->name, "testmon2");
    
    // Faint all Pokemon
    for (size_t i = 0; i < team.size(); ++i) {
        Pokemon* pokemon = team.getPokemon(i);
        if (pokemon && pokemon->isAlive()) {
            pokemon->takeDamage(pokemon->hp);
        }
    }
    
    // Should have no alive Pokemon
    firstAlive = team.getFirstAlivePokemon();
    EXPECT_EQ(firstAlive, nullptr);
}

// Test team size and isEmpty methods
TEST_F(TeamTest, TeamSizeAndEmpty) {
    Team team;
    
    // Empty team
    EXPECT_TRUE(team.isEmpty());
    EXPECT_EQ(team.size(), 0);
    
    // Load team
    team.loadTeams(teamData, movesData, "TestTeam");
    EXPECT_FALSE(team.isEmpty());
    EXPECT_EQ(team.size(), 3);
    
    // Test with empty team data
    std::unordered_map<std::string, std::vector<std::string>> emptyTeamData;
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>> emptyMovesData;
    
    Team emptyTeam;
    emptyTeam.loadTeams(emptyTeamData, emptyMovesData, "NonExistentTeam");
    EXPECT_TRUE(emptyTeam.isEmpty());
    EXPECT_EQ(emptyTeam.size(), 0);
}

// Test iterator support
TEST_F(TeamTest, IteratorSupport) {
    Team team;
    team.loadTeams(teamData, movesData, "TestTeam");
    
    // Test non-const iterator
    int count = 0;
    for (auto& pair : team) {
        EXPECT_TRUE(pair.second.isAlive());
        count++;
    }
    EXPECT_EQ(count, 3);
    
    // Test const iterator
    const Team& constTeam = team;
    count = 0;
    for (const auto& pair : constTeam) {
        EXPECT_TRUE(pair.second.isAlive());
        count++;
    }
    EXPECT_EQ(count, 3);
}

// Test loading team with no moves
TEST_F(TeamTest, LoadTeamWithNoMoves) {
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>> emptyMovesData;
    
    Team team;
    team.loadTeams(teamData, emptyMovesData, "TestTeam");
    
    EXPECT_FALSE(team.isEmpty());
    EXPECT_EQ(team.size(), 3);
    
    // Check that Pokemon were loaded but have no moves
    for (size_t i = 0; i < team.size(); ++i) {
        Pokemon* pokemon = team.getPokemon(i);
        ASSERT_NE(pokemon, nullptr);
        EXPECT_EQ(pokemon->moves.size(), 0);
    }
}

// Test loading non-existent team
TEST_F(TeamTest, LoadNonExistentTeam) {
    Team team;
    team.loadTeams(teamData, movesData, "NonExistentTeam");
    
    EXPECT_TRUE(team.isEmpty());
    EXPECT_EQ(team.size(), 0);
    EXPECT_FALSE(team.hasAlivePokemon());
}

// Test team with mixed alive/fainted Pokemon
TEST_F(TeamTest, MixedAliveAndFaintedPokemon) {
    Team team;
    team.loadTeams(teamData, movesData, "TestTeam");
    
    // Faint middle Pokemon
    Pokemon* pokemon2 = team.getPokemon(1);
    ASSERT_NE(pokemon2, nullptr);
    pokemon2->takeDamage(pokemon2->hp);
    
    // Check alive Pokemon
    EXPECT_TRUE(team.hasAlivePokemon());
    
    std::vector<Pokemon*> alivePokemon = team.getAlivePokemon();
    EXPECT_EQ(alivePokemon.size(), 2);
    EXPECT_EQ(alivePokemon[0]->name, "testmon1");
    EXPECT_EQ(alivePokemon[1]->name, "testmon3");
    
    // First alive should still be testmon1
    Pokemon* firstAlive = team.getFirstAlivePokemon();
    ASSERT_NE(firstAlive, nullptr);
    EXPECT_EQ(firstAlive->name, "testmon1");
}

// Test team with Pokemon that have moves
TEST_F(TeamTest, TeamWithPokemonMoves) {
    Team team;
    team.loadTeams(teamData, movesData, "TestTeam");
    
    // Check that Pokemon have moves
    for (size_t i = 0; i < team.size(); ++i) {
        Pokemon* pokemon = team.getPokemon(i);
        ASSERT_NE(pokemon, nullptr);
        EXPECT_GT(pokemon->moves.size(), 0);
        
        // Check that moves are properly loaded
        for (const auto& move : pokemon->moves) {
            EXPECT_FALSE(move.name.empty());
            EXPECT_TRUE(move.canUse());
        }
    }
}

// Test team memory management
TEST_F(TeamTest, TeamMemoryManagement) {
    Team team;
    team.loadTeams(teamData, movesData, "TestTeam");
    
    // Store pointers to Pokemon
    std::vector<Pokemon*> pokemonPointers;
    for (size_t i = 0; i < team.size(); ++i) {
        pokemonPointers.push_back(team.getPokemon(i));
    }
    
    // Verify pointers are valid
    for (Pokemon* pokemon : pokemonPointers) {
        ASSERT_NE(pokemon, nullptr);
        EXPECT_TRUE(pokemon->isAlive());
    }
    
    // After team destruction, pointers should still be valid within scope
    // This tests that the team properly manages Pokemon objects
}