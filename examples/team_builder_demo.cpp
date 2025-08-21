#include <iostream>
#include <memory>
#include "pokemon_data.h"
#include "team_builder.h"

/**
 * @brief Demonstration of the new team builder system
 * 
 * This example shows how to use the PokemonData and TeamBuilder classes
 * to create, validate, and manage Pokemon teams with the new security features.
 */
int main() {
    std::cout << "=== Pokemon Team Builder Demo ===" << std::endl;
    
    try {
        // Step 1: Initialize Pokemon Data
        std::cout << "\n1. Initializing Pokemon Data..." << std::endl;
        auto pokemon_data = std::make_shared<PokemonData>();
        
        auto init_result = pokemon_data->initialize();
        if (!init_result.success) {
            std::cerr << "Failed to initialize Pokemon data: " << init_result.error_message << std::endl;
            return 1;
        }
        
        std::cout << "Successfully loaded data!" << std::endl;
        std::cout << pokemon_data->getDataStatistics() << std::endl;
        
        // Step 2: Create Team Builder
        std::cout << "\n2. Creating Team Builder..." << std::endl;
        TeamBuilder team_builder(pokemon_data);
        
        // Step 3: Create a new team
        std::cout << "\n3. Building a new team..." << std::endl;
        auto team = team_builder.createTeam("Demo Team");
        
        // Step 4: Add some Pokemon to the team
        std::cout << "\n4. Adding Pokemon to team..." << std::endl;
        
        // Get some available Pokemon
        auto available_pokemon = pokemon_data->getAvailablePokemon();
        if (available_pokemon.size() >= 3) {
            // Add first Pokemon with suggested moves
            std::string pokemon1 = available_pokemon[0];
            auto moves1 = pokemon_data->suggestMovesForPokemon(pokemon1, 4);
            if (team_builder.addPokemonToTeam(team, pokemon1, moves1)) {
                std::cout << "Added " << pokemon1 << " with moves: ";
                for (const auto& move : moves1) {
                    std::cout << move << " ";
                }
                std::cout << std::endl;
            }
            
            // Add second Pokemon
            std::string pokemon2 = available_pokemon[1];
            auto moves2 = pokemon_data->suggestMovesForPokemon(pokemon2, 3);
            if (team_builder.addPokemonToTeam(team, pokemon2, moves2)) {
                std::cout << "Added " << pokemon2 << " with moves: ";
                for (const auto& move : moves2) {
                    std::cout << move << " ";
                }
                std::cout << std::endl;
            }
            
            // Add third Pokemon
            std::string pokemon3 = available_pokemon[2];
            auto moves3 = pokemon_data->suggestMovesForPokemon(pokemon3, 4);
            if (team_builder.addPokemonToTeam(team, pokemon3, moves3)) {
                std::cout << "Added " << pokemon3 << " with moves: ";
                for (const auto& move : moves3) {
                    std::cout << move << " ";
                }
                std::cout << std::endl;
            }
        }
        
        // Step 5: Validate the team
        std::cout << "\n5. Validating team..." << std::endl;
        bool is_valid = team_builder.validateTeam(team);
        
        std::cout << "Team validation result: " << (is_valid ? "VALID" : "INVALID") << std::endl;
        
        if (!team.validation_errors.empty()) {
            std::cout << "Validation errors:" << std::endl;
            for (const auto& error : team.validation_errors) {
                std::cout << "  - " << error << std::endl;
            }
        }
        
        if (!team.validation_warnings.empty()) {
            std::cout << "Validation warnings:" << std::endl;
            for (const auto& warning : team.validation_warnings) {
                std::cout << "  - " << warning << std::endl;
            }
        }
        
        // Step 6: Analyze the team
        std::cout << "\n6. Analyzing team..." << std::endl;
        auto analysis = team_builder.analyzeTeam(team);
        
        std::cout << "Team Analysis:" << std::endl;
        std::cout << "  Balance Score: " << analysis.balance_score << "/100" << std::endl;
        std::cout << "  Offensive Types: ";
        for (const auto& type : analysis.offensive_types) {
            std::cout << type << " ";
        }
        std::cout << std::endl;
        
        std::cout << "  Move Distribution:" << std::endl;
        std::cout << "    Physical: " << analysis.physical_moves << std::endl;
        std::cout << "    Special: " << analysis.special_moves << std::endl;
        std::cout << "    Status: " << analysis.status_moves << std::endl;
        
        std::cout << "  Average Stats:" << std::endl;
        std::cout << "    HP: " << analysis.average_hp << std::endl;
        std::cout << "    Attack: " << analysis.average_attack << std::endl;
        std::cout << "    Defense: " << analysis.average_defense << std::endl;
        std::cout << "    Special Attack: " << analysis.average_special_attack << std::endl;
        std::cout << "    Special Defense: " << analysis.average_special_defense << std::endl;
        std::cout << "    Speed: " << analysis.average_speed << std::endl;
        
        // Step 7: Get suggestions
        std::cout << "\n7. Getting team suggestions..." << std::endl;
        auto suggestions = team_builder.getTeamSuggestions(team);
        
        if (!suggestions.empty()) {
            std::cout << "Team improvement suggestions:" << std::endl;
            for (const auto& suggestion : suggestions) {
                std::cout << "  - " << suggestion << std::endl;
            }
        }
        
        auto pokemon_suggestions = team_builder.suggestPokemonForTeam(team, 3);
        if (!pokemon_suggestions.empty()) {
            std::cout << "Suggested Pokemon to add:" << std::endl;
            for (const auto& pokemon : pokemon_suggestions) {
                std::cout << "  - " << pokemon << std::endl;
            }
        }
        
        // Step 8: Generate a random balanced team for comparison
        std::cout << "\n8. Generating a balanced team for comparison..." << std::endl;
        auto balanced_team = team_builder.generateBalancedTeam("Balanced Demo", 6);
        
        std::cout << "Generated balanced team with " << balanced_team.size() << " Pokemon:" << std::endl;
        for (const auto& pokemon : balanced_team.pokemon) {
            std::cout << "  " << pokemon.name << " with moves: ";
            for (const auto& move : pokemon.moves) {
                std::cout << move << " ";
            }
            std::cout << std::endl;
        }
        
        auto balanced_analysis = team_builder.analyzeTeam(balanced_team);
        std::cout << "Balanced team score: " << balanced_analysis.balance_score << "/100" << std::endl;
        
        // Step 9: Export team for battle system
        std::cout << "\n9. Exporting team for battle system..." << std::endl;
        auto [selected_teams, selected_moves] = team_builder.exportTeamForBattle(team);
        
        std::cout << "Team exported successfully!" << std::endl;
        std::cout << "  Teams map has " << selected_teams.size() << " entries" << std::endl;
        std::cout << "  Moves map has " << selected_moves.size() << " entries" << std::endl;
        
        // Step 10: Demonstrate type coverage analysis
        std::cout << "\n10. Analyzing type coverage..." << std::endl;
        auto type_coverage = team_builder.calculateTypeCoverage(team);
        
        std::cout << "Type coverage analysis:" << std::endl;
        for (const auto& [type, effectiveness] : type_coverage) {
            if (effectiveness > 1.0) {
                std::cout << "  Strong vs " << type << " (" << effectiveness << "x)" << std::endl;
            } else if (effectiveness < 1.0) {
                std::cout << "  Weak vs " << type << " (" << effectiveness << "x)" << std::endl;
            }
        }
        
        std::cout << "\n=== Demo Complete ===" << std::endl;
        std::cout << "The team builder system is working correctly!" << std::endl;
        std::cout << "Your team is ready for battle with the existing battle system." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during demo: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}