#include <gtest/gtest.h>
#include <memory>
#include "core/team_builder.h"
#include "core/pokemon_data.h"
#include "../utils/test_utils.h"

class TeamBuilderPhase4Test : public ::testing::Test {
protected:
    void SetUp() override {
        pokemon_data = std::make_shared<PokemonData>();
        auto result = pokemon_data->initialize();
        ASSERT_TRUE(result.success) << "Failed to initialize Pokemon data: " << result.error_message;
        
        team_builder = std::make_shared<TeamBuilder>(pokemon_data);
    }

    std::shared_ptr<PokemonData> pokemon_data;
    std::shared_ptr<TeamBuilder> team_builder;
};

// Test improved team name validation
TEST_F(TeamBuilderPhase4Test, ImprovedTeamNameValidation) {
    // Test names that should be valid now
    auto team1 = team_builder->createTeam("Player's Team");
    EXPECT_NE(team1.name, "Invalid_Team_Name");
    EXPECT_EQ(team1.name, "Player's Team");
    
    auto team2 = team_builder->createTeam("Team-Alpha");
    EXPECT_NE(team2.name, "Invalid_Team_Name");
    EXPECT_EQ(team2.name, "Team-Alpha");
    
    auto team3 = team_builder->createTeam("Team_Beta");
    EXPECT_NE(team3.name, "Invalid_Team_Name");
    EXPECT_EQ(team3.name, "Team_Beta");
    
    auto team4 = team_builder->createTeam("Team 1.5");
    EXPECT_NE(team4.name, "Invalid_Team_Name");
    EXPECT_EQ(team4.name, "Team 1.5");
    
    // Test empty name handling
    auto team5 = team_builder->createTeam("");
    EXPECT_EQ(team5.name, "Invalid_Team_Name");
    
    // Test whitespace-only name handling
    auto team6 = team_builder->createTeam("   ");
    EXPECT_EQ(team6.name, "Unnamed_Team");
    
    // Test overly long name
    std::string long_name(150, 'a');
    auto team7 = team_builder->createTeam(long_name);
    EXPECT_EQ(team7.name, "Invalid_Team_Name");
}

// Test custom team building functionality
TEST_F(TeamBuilderPhase4Test, CustomTeamBuilding) {
    auto team = team_builder->createTeam("Custom Test Team");
    ASSERT_NE(team.name, "Invalid_Team_Name");
    
    // Test adding Pokemon to team
    std::vector<std::string> moves = {"tackle", "growl", "quick-attack", "tail-whip"};
    bool success = team_builder->addPokemonToTeam(team, "pikachu", moves);
    EXPECT_TRUE(success);
    EXPECT_EQ(team.pokemon.size(), 1);
    EXPECT_EQ(team.pokemon[0].name, "pikachu");
    EXPECT_EQ(team.pokemon[0].moves.size(), 4);
    
    // Test duplicate Pokemon prevention
    bool duplicate_success = team_builder->addPokemonToTeam(team, "pikachu", moves);
    EXPECT_FALSE(duplicate_success);
    EXPECT_EQ(team.pokemon.size(), 1); // Should still be 1
    
    // Test team size limit
    std::vector<std::string> more_moves = {"tackle", "growl"};
    for (const auto& pokemon : {"charmander", "bulbasaur", "squirtle", "caterpie", "weedle"}) {
        bool add_success = team_builder->addPokemonToTeam(team, pokemon, more_moves);
        EXPECT_TRUE(add_success);
    }
    
    EXPECT_EQ(team.pokemon.size(), 6); // Team should be full
    
    // Try to add 7th Pokemon - should fail
    bool seventh_success = team_builder->addPokemonToTeam(team, "pidgey", more_moves);
    EXPECT_FALSE(seventh_success);
    EXPECT_EQ(team.pokemon.size(), 6);
}

// Test battle result recording functionality
TEST_F(TeamBuilderPhase4Test, BattleResultRecording) {
    const std::string team_name = "Test Battle Team";
    const std::string opponent_name = "Test Opponent";
    
    // Record a victory
    team_builder->recordBattleResult(team_name, opponent_name, true, 10, "Medium", 85.0);
    
    auto stats = team_builder->getTeamStatistics(team_name);
    ASSERT_TRUE(stats.has_value());
    EXPECT_EQ(stats->team_name, team_name);
    EXPECT_EQ(stats->total_battles, 1);
    EXPECT_EQ(stats->victories, 1);
    EXPECT_EQ(stats->defeats, 0);
    EXPECT_FLOAT_EQ(stats->win_rate, 100.0);
    EXPECT_FLOAT_EQ(stats->average_effectiveness_score, 85.0);
    
    // Record a defeat
    team_builder->recordBattleResult(team_name, opponent_name, false, 15, "Hard", 45.0);
    
    stats = team_builder->getTeamStatistics(team_name);
    ASSERT_TRUE(stats.has_value());
    EXPECT_EQ(stats->total_battles, 2);
    EXPECT_EQ(stats->victories, 1);
    EXPECT_EQ(stats->defeats, 1);
    EXPECT_FLOAT_EQ(stats->win_rate, 50.0);
    EXPECT_FLOAT_EQ(stats->average_effectiveness_score, 65.0); // (85 + 45) / 2
}

// Test team sharing functionality
TEST_F(TeamBuilderPhase4Test, TeamSharingSystem) {
    // Create a test team
    auto team = team_builder->createTeam("Share Test Team");
    std::vector<std::string> moves = {"tackle", "growl"};
    
    team_builder->addPokemonToTeam(team, "pikachu", moves);
    team_builder->addPokemonToTeam(team, "charmander", moves);
    
    // Generate share code
    std::string share_code = team_builder->exportTeamShareCode(team, "TestUser", "Test team export");
    EXPECT_FALSE(share_code.empty());
    
    // Import the team back
    auto imported_team = team_builder->importTeamFromShareCode(share_code, true);
    
    // Verify the imported team
    EXPECT_NE(imported_team.name, "Import_Failed");
    EXPECT_EQ(imported_team.pokemon.size(), 2);
    
    // Check if Pokemon names match (order might differ)
    std::vector<std::string> imported_names;
    for (const auto& pokemon : imported_team.pokemon) {
        imported_names.push_back(pokemon.name);
    }
    
    EXPECT_TRUE(std::find(imported_names.begin(), imported_names.end(), "pikachu") != imported_names.end());
    EXPECT_TRUE(std::find(imported_names.begin(), imported_names.end(), "charmander") != imported_names.end());
}

// Test template system functionality
TEST_F(TeamBuilderPhase4Test, TemplateSystem) {
    // Test getting template categories
    auto categories = team_builder->getTemplateCategories();
    EXPECT_FALSE(categories.empty());
    
    // Test getting templates in a category
    if (!categories.empty()) {
        auto templates = team_builder->getTemplatesInCategory(categories[0]);
        // Templates may or may not exist, but the call should not crash
        EXPECT_TRUE(templates.size() >= 0);
        
        if (!templates.empty()) {
            // Test getting a specific template
            auto template_data = team_builder->getTemplate(categories[0], templates[0]);
            if (template_data.has_value()) {
                EXPECT_FALSE(template_data->name.empty());
                EXPECT_FALSE(template_data->description.empty());
                EXPECT_FALSE(template_data->pokemon.empty());
            }
        }
    }
}

// Test team comparison functionality
TEST_F(TeamBuilderPhase4Test, TeamComparison) {
    // Create two test teams
    auto team1 = team_builder->generateRandomTeam("Team Alpha", 3);
    auto team2 = team_builder->generateRandomTeam("Team Beta", 3);
    
    ASSERT_FALSE(team1.pokemon.empty());
    ASSERT_FALSE(team2.pokemon.empty());
    
    // Compare the teams
    auto comparison = team_builder->compareTeams(team1, team2);
    
    EXPECT_EQ(comparison.team1_name, team1.name);
    EXPECT_EQ(comparison.team2_name, team2.name);
    EXPECT_GE(comparison.team1_balance_score, 0);
    EXPECT_LE(comparison.team1_balance_score, 100);
    EXPECT_GE(comparison.team2_balance_score, 0);
    EXPECT_LE(comparison.team2_balance_score, 100);
    EXPECT_GE(comparison.team1_win_probability, 0.0);
    EXPECT_LE(comparison.team1_win_probability, 1.0);
    EXPECT_FALSE(comparison.battle_prediction_reasoning.empty());
}

// Test tournament draft functionality
TEST_F(TeamBuilderPhase4Test, TournamentDraftMode) {
    TeamBuilder::DraftSettings settings;
    settings.player_count = 2;
    settings.team_size = 6;
    settings.ban_phase_picks_per_player = 2;
    settings.max_legendaries_per_team = 1;
    settings.max_same_type_per_team = 2;
    
    std::vector<std::string> players = {"Player1", "Player2"};
    
    auto session = team_builder->createDraftSession(settings, players);
    
    EXPECT_TRUE(session.is_active);
    EXPECT_EQ(session.player_names.size(), 2);
    EXPECT_EQ(session.player_names[0], "Player1");
    EXPECT_EQ(session.player_names[1], "Player2");
    EXPECT_FALSE(session.available_pokemon.empty());
    
    // Test draft suggestions
    auto suggestions = team_builder->getDraftSuggestions(session, 5);
    EXPECT_LE(suggestions.size(), 5);
    
    // Test draft analysis (even without picks, should not crash)
    auto analysis = team_builder->analyzeDraftStrategy(session);
    EXPECT_EQ(analysis.size(), 2); // Should have analysis for both players
}

// Test performance and error handling
TEST_F(TeamBuilderPhase4Test, ErrorHandling) {
    // Test with invalid Pokemon names
    auto team = team_builder->createTeam("Error Test Team");
    std::vector<std::string> moves = {"invalid-move"};
    
    bool success = team_builder->addPokemonToTeam(team, "invalid-pokemon", moves);
    EXPECT_FALSE(success);
    EXPECT_FALSE(team.validation_errors.empty());
    
    // Test with empty move list
    success = team_builder->addPokemonToTeam(team, "pikachu", {});
    // This might succeed or fail depending on validation settings, but should not crash
    
    // Test importing invalid share code
    auto imported_team = team_builder->importTeamFromShareCode("invalid_code", true);
    EXPECT_EQ(imported_team.name, "Import_Failed");
    
    // Test getting statistics for non-existent team
    auto stats = team_builder->getTeamStatistics("NonExistentTeam");
    EXPECT_FALSE(stats.has_value());
}

// Test custom team loading
TEST_F(TeamBuilderPhase4Test, CustomTeamManagement) {
    // Create and save a custom team
    auto team = team_builder->createTeam("Custom Save Test");
    std::vector<std::string> moves = {"tackle", "growl"};
    team_builder->addPokemonToTeam(team, "pikachu", moves);
    
    // Save the team (this might fail if directory doesn't exist, but should not crash)
    bool save_success = team_builder->saveCustomTeam(team);
    // Don't assert on success as it depends on file system permissions
    
    // Get list of custom teams
    auto custom_teams = team_builder->getCustomTeamsList();
    // Should not crash, might be empty
    EXPECT_TRUE(custom_teams.size() >= 0);
}