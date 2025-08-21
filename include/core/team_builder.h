#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include "pokemon_data.h"
#include "input_validator.h"

/**
 * @brief Comprehensive team building system with validation and suggestions
 * 
 * This class provides tools for building and validating Pokemon teams. It uses
 * PokemonData for access to available Pokemon and moves, and includes comprehensive
 * validation for team composition, type coverage, and battle readiness.
 */
class TeamBuilder {
public:
    /**
     * @brief Information about a Pokemon in a team
     */
    struct TeamPokemon {
        std::string name;
        std::vector<std::string> moves;
        
        TeamPokemon() = default;
        TeamPokemon(const std::string& pokemon_name, const std::vector<std::string>& pokemon_moves)
            : name(pokemon_name), moves(pokemon_moves) {}
    };

    /**
     * @brief A complete team with validation results
     */
    struct Team {
        std::string name;
        std::vector<TeamPokemon> pokemon;
        bool is_valid;
        std::vector<std::string> validation_errors;
        std::vector<std::string> validation_warnings;
        
        Team(const std::string& team_name = "") : name(team_name), is_valid(false) {}
        
        size_t size() const { return pokemon.size(); }
        bool isEmpty() const { return pokemon.empty(); }
        bool isFull() const { return pokemon.size() >= 6; }
    };

    /**
     * @brief Team analysis and suggestions
     */
    struct TeamAnalysis {
        // Type coverage analysis
        std::vector<std::string> offensive_types;
        std::vector<std::string> defensive_weaknesses;
        std::vector<std::string> defensive_resistances;
        
        // Move analysis
        int physical_moves;
        int special_moves;
        int status_moves;
        
        // Stat distribution
        double average_hp;
        double average_attack;
        double average_defense;
        double average_special_attack;
        double average_special_defense;
        double average_speed;
        
        // Team balance score (0-100)
        int balance_score;
        
        // Suggestions
        std::vector<std::string> suggested_pokemon;
        std::vector<std::string> suggested_move_changes;
        std::vector<std::string> coverage_gaps;
        
        TeamAnalysis() : physical_moves(0), special_moves(0), status_moves(0),
                        average_hp(0), average_attack(0), average_defense(0),
                        average_special_attack(0), average_special_defense(0), average_speed(0),
                        balance_score(0) {}
    };

    /**
     * @brief Validation settings for team building
     */
    struct ValidationSettings {
        bool enforce_max_team_size;     // Enforce 6 Pokemon maximum
        bool enforce_min_team_size;     // Enforce minimum team size
        int min_team_size;              // Minimum number of Pokemon (default 1)
        bool enforce_max_moves;         // Enforce 4 moves maximum per Pokemon
        bool enforce_min_moves;         // Enforce minimum moves per Pokemon
        int min_moves_per_pokemon;      // Minimum moves per Pokemon (default 1)
        bool allow_duplicate_pokemon;   // Allow same Pokemon multiple times
        bool allow_duplicate_moves;     // Allow same move on multiple Pokemon
        bool require_type_diversity;    // Require diverse types on team
        int min_unique_types;           // Minimum unique types required
        
        ValidationSettings() 
            : enforce_max_team_size(true), enforce_min_team_size(true), min_team_size(1),
              enforce_max_moves(true), enforce_min_moves(true), min_moves_per_pokemon(1),
              allow_duplicate_pokemon(false), allow_duplicate_moves(true),
              require_type_diversity(true), min_unique_types(3) {}
    };

    /**
     * @brief Template Pokemon information with role and strategy
     */
    struct TemplatePokemon {
        std::string name;
        std::string role;
        std::vector<std::string> moves;
        std::string strategy;
        std::string tips;

        TemplatePokemon() = default;
        TemplatePokemon(const std::string& pokemon_name, const std::string& pokemon_role,
                       const std::vector<std::string>& pokemon_moves, 
                       const std::string& pokemon_strategy, const std::string& pokemon_tips)
            : name(pokemon_name), role(pokemon_role), moves(pokemon_moves), 
              strategy(pokemon_strategy), tips(pokemon_tips) {}
    };

    /**
     * @brief Team template with metadata and strategy information
     */
    struct TeamTemplate {
        // Template metadata
        std::string name;
        std::string description;
        std::string difficulty;        // "beginner", "intermediate", "advanced"
        std::string strategy;          // "balanced", "offensive", "defensive", etc.
        
        // Type coverage information
        std::vector<std::string> offensive_types;
        std::vector<std::string> defensive_coverage;
        
        // Usage information
        std::string usage_notes;
        std::vector<std::string> learning_objectives;
        
        // Team composition
        std::string team_name;
        std::vector<TemplatePokemon> pokemon;

        TeamTemplate() = default;
    };

    /**
     * @brief Random generation settings for flexible team building
     */
    struct RandomGenerationSettings {
        // Basic settings
        int team_size;                          // Number of Pokemon to generate (1-6)
        bool allow_legendaries;                 // Allow legendary Pokemon
        bool allow_duplicates;                  // Allow duplicate Pokemon
        
        // Type restrictions
        std::vector<std::string> required_types;   // Must include these types
        std::vector<std::string> banned_types;     // Cannot include these types
        std::string type_theme;                    // Focus on specific type ("" for none)
        
        // Role requirements
        std::vector<std::string> required_roles;   // Must include these roles
        int min_physical_attackers;             // Minimum physical attackers
        int min_special_attackers;              // Minimum special attackers
        int min_tanks;                          // Minimum defensive Pokemon
        
        // Difficulty scaling
        std::string difficulty;                 // "beginner", "intermediate", "advanced"
        bool optimize_movesets;                 // Use competitive movesets vs basic ones
        
        RandomGenerationSettings() 
            : team_size(6), allow_legendaries(false), allow_duplicates(false),
              type_theme(""), min_physical_attackers(0), min_special_attackers(0),
              min_tanks(0), difficulty("intermediate"), optimize_movesets(true) {}
    };

    // Constructor
    TeamBuilder(std::shared_ptr<PokemonData> data);

    // Team management
    /**
     * @brief Create a new empty team
     * @param team_name Name for the team
     * @return New empty team
     */
    Team createTeam(const std::string& team_name);

    /**
     * @brief Add a Pokemon to a team
     * @param team Team to modify
     * @param pokemon_name Pokemon to add
     * @param moves Moves for the Pokemon (up to 4)
     * @return True if added successfully
     */
    bool addPokemonToTeam(Team& team, const std::string& pokemon_name, 
                         const std::vector<std::string>& moves);

    /**
     * @brief Remove a Pokemon from a team
     * @param team Team to modify
     * @param pokemon_index Index of Pokemon to remove
     * @return True if removed successfully
     */
    bool removePokemonFromTeam(Team& team, size_t pokemon_index);

    /**
     * @brief Modify moves for a Pokemon in the team
     * @param team Team to modify
     * @param pokemon_index Index of Pokemon to modify
     * @param new_moves New moves for the Pokemon
     * @return True if modified successfully
     */
    bool modifyPokemonMoves(Team& team, size_t pokemon_index, 
                           const std::vector<std::string>& new_moves);

    // Team validation
    /**
     * @brief Validate a complete team
     * @param team Team to validate
     * @param settings Validation settings to use
     * @return True if team is valid
     */
    bool validateTeam(Team& team, const ValidationSettings& settings = ValidationSettings());

    /**
     * @brief Get detailed analysis of a team
     * @param team Team to analyze
     * @return TeamAnalysis with detailed information
     */
    TeamAnalysis analyzeTeam(const Team& team) const;

    // Team generation
    /**
     * @brief Generate a random team with good type coverage
     * @param team_name Name for the generated team
     * @param team_size Number of Pokemon to generate (1-6)
     * @return Generated team
     */
    Team generateRandomTeam(const std::string& team_name, int team_size = 6);

    /**
     * @brief Generate a team focused on a specific type
     * @param team_name Name for the generated team
     * @param focus_type Primary type to focus on
     * @param team_size Number of Pokemon to generate (1-6)
     * @return Generated team with type focus
     */
    Team generateTypeFocusedTeam(const std::string& team_name, 
                                const std::string& focus_type, int team_size = 6);

    /**
     * @brief Generate a balanced team with optimal type coverage
     * @param team_name Name for the generated team
     * @param team_size Number of Pokemon to generate (1-6)
     * @return Generated balanced team
     */
    Team generateBalancedTeam(const std::string& team_name, int team_size = 6);

    // Team export/import
    /**
     * @brief Convert team to format compatible with Team::loadTeams()
     * @param team Team to convert
     * @return Pair of maps compatible with existing team loading system
     */
    std::pair<
        std::unordered_map<std::string, std::vector<std::string>>,
        std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>>
    > exportTeamForBattle(const Team& team) const;

    /**
     * @brief Import a team from the legacy format
     * @param team_name Name of the team
     * @param selected_teams Pokemon selection map
     * @param selected_moves Moves selection map
     * @return Imported team
     */
    Team importTeamFromBattle(
        const std::string& team_name,
        const std::unordered_map<std::string, std::vector<std::string>>& selected_teams,
        const std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>>& selected_moves
    );

    /**
     * @brief Save team to JSON file
     * @param team Team to save
     * @param file_path Path to save file
     * @return True if saved successfully
     */
    bool saveTeamToFile(const Team& team, const std::string& file_path) const;

    /**
     * @brief Load team from JSON file
     * @param file_path Path to team file
     * @return Loaded team or empty team if failed
     */
    Team loadTeamFromFile(const std::string& file_path);

    // Utility methods
    /**
     * @brief Get suggestions for improving a team
     * @param team Team to analyze
     * @return Vector of improvement suggestions
     */
    std::vector<std::string> getTeamSuggestions(const Team& team) const;

    /**
     * @brief Get Pokemon suggestions to fill team gaps
     * @param team Current team
     * @param count Number of suggestions to return
     * @return Vector of Pokemon suggestions
     */
    std::vector<std::string> suggestPokemonForTeam(const Team& team, int count = 3) const;

    /**
     * @brief Get move suggestions for a specific Pokemon in the team
     * @param team Team containing the Pokemon
     * @param pokemon_index Index of Pokemon to suggest moves for
     * @param count Number of move suggestions
     * @return Vector of move suggestions
     */
    std::vector<std::string> suggestMovesForTeamPokemon(const Team& team, 
                                                       size_t pokemon_index, int count = 4) const;

    /**
     * @brief Calculate type coverage of a team
     * @param team Team to analyze
     * @return Map of types to effectiveness multipliers
     */
    std::unordered_map<std::string, double> calculateTypeCoverage(const Team& team) const;

    /**
     * @brief Get validation settings
     * @return Current validation settings
     */
    const ValidationSettings& getValidationSettings() const { return validation_settings; }

    /**
     * @brief Set validation settings
     * @param settings New validation settings
     */
    void setValidationSettings(const ValidationSettings& settings) { validation_settings = settings; }

    // Template system methods
    /**
     * @brief Load all available team templates from the data directory
     * @return True if templates loaded successfully
     */
    bool loadTemplates();

    /**
     * @brief Get list of available template categories
     * @return Vector of category names (e.g., "starter_teams", "type_themed", "competitive")
     */
    std::vector<std::string> getTemplateCategories() const;

    /**
     * @brief Get list of templates in a specific category
     * @param category Category to list templates from
     * @return Vector of template names in the category
     */
    std::vector<std::string> getTemplatesInCategory(const std::string& category) const;

    /**
     * @brief Get detailed information about a specific template
     * @param category Template category
     * @param template_name Name of the template
     * @return Template information or empty optional if not found
     */
    std::optional<TeamTemplate> getTemplate(const std::string& category, const std::string& template_name) const;

    /**
     * @brief Generate a team from a template
     * @param category Template category  
     * @param template_name Name of the template to use
     * @param custom_name Optional custom team name (uses template name if empty)
     * @return Generated team based on the template
     */
    Team generateTeamFromTemplate(const std::string& category, const std::string& template_name, 
                                 const std::string& custom_name = "");

    /**
     * @brief Generate a random team with advanced settings
     * @param settings Random generation settings
     * @param custom_name Optional custom team name
     * @return Generated team based on the settings
     */
    Team generateAdvancedRandomTeam(const RandomGenerationSettings& settings,
                                  const std::string& custom_name = "");

    /**
     * @brief Get suggested templates based on user preferences
     * @param difficulty Preferred difficulty level
     * @param strategy Preferred strategy type
     * @param max_suggestions Maximum number of suggestions
     * @return Vector of suggested templates with category and name
     */
    std::vector<std::pair<std::string, std::string>> getSuggestedTemplates(
        const std::string& difficulty = "", const std::string& strategy = "", 
        int max_suggestions = 5) const;

    /**
     * @brief Search templates by keywords
     * @param keywords Search terms to look for in template names/descriptions
     * @return Vector of matching templates with category and name
     */
    std::vector<std::pair<std::string, std::string>> searchTemplates(
        const std::vector<std::string>& keywords) const;

private:
    std::shared_ptr<PokemonData> pokemon_data;
    ValidationSettings validation_settings;
    
    // Template system storage
    std::unordered_map<std::string, std::unordered_map<std::string, TeamTemplate>> templates;
    bool templates_loaded;

    // Validation helper methods
    bool validateTeamSize(const Team& team, std::vector<std::string>& errors, 
                         std::vector<std::string>& warnings) const;
    bool validatePokemonMoves(const Team& team, std::vector<std::string>& errors, 
                             std::vector<std::string>& warnings) const;
    bool validateTypeDiversity(const Team& team, std::vector<std::string>& errors, 
                              std::vector<std::string>& warnings) const;
    bool validateDuplicates(const Team& team, std::vector<std::string>& errors, 
                           std::vector<std::string>& warnings) const;

    // Analysis helper methods
    std::vector<std::string> getTeamTypes(const Team& team) const;
    std::unordered_map<std::string, int> getTypeCounts(const Team& team) const;
    std::unordered_map<std::string, int> getMoveTypeCounts(const Team& team) const;
    int calculateBalanceScore(const Team& team) const;

    // Generation helper methods
    std::vector<std::string> selectRandomPokemon(int count, const std::vector<std::string>& available) const;
    std::vector<std::string> selectPokemonByType(const std::string& type, int count) const;
    std::vector<std::string> generateMovesForPokemon(const std::string& pokemon_name) const;
    bool isTeamTypeBalanced(const Team& team) const;

    // Utility helper methods
    std::string normalizeTeamName(const std::string& name) const;
    bool isValidTeamName(const std::string& name) const;
    std::unordered_set<std::string> getWeakTypes(const std::vector<std::string>& pokemon_types) const;
    std::unordered_set<std::string> getResistantTypes(const std::vector<std::string>& pokemon_types) const;

    // Template system helper methods
    TeamTemplate parseTemplateFromJson(const std::string& file_path);
    bool isValidTemplateFile(const std::string& file_path) const;
    std::string getTemplateNameFromFile(const std::string& file_path) const;
    Team convertTemplateToTeam(const TeamTemplate& template_data, const std::string& custom_name) const;
    std::vector<std::string> getPokemonByRole(const std::string& role, int count) const;
    std::vector<std::string> getLegendaryPokemon() const;
    bool isPokemonLegendary(const std::string& pokemon_name) const;
    std::vector<std::string> generateRoleBasedMoves(const std::string& pokemon_name, const std::string& role) const;
    bool teamMeetsRoleRequirements(const Team& team, const RandomGenerationSettings& settings) const;
};