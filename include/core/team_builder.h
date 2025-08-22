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

    // ────────────────────────────────────────────────────────────────────────
    // Tournament/Draft Mode System
    // ────────────────────────────────────────────────────────────────────────

    /**
     * @brief Draft session settings and rules
     */
    struct DraftSettings {
        int team_size;                                  // Number of Pokemon per team (1-6)
        int player_count;                              // Number of players (2-8)
        
        // Ban/Pick rules
        int ban_phase_picks_per_player;                // Bans per player (0-5)
        int pick_phase_picks_per_turn;                 // Picks per turn (1-3)
        bool allow_pick_same_turn;                     // Allow multiple picks in one turn
        
        // Team restrictions
        int max_legendaries_per_team;                  // Maximum legendary Pokemon (0-6)
        int max_same_type_per_team;                    // Maximum Pokemon of same type (0-6)
        std::vector<std::string> banned_pokemon;       // Globally banned Pokemon
        std::vector<std::string> banned_types;         // Globally banned types
        
        // Draft format
        bool snake_draft;                              // Snake draft (true) vs linear (false)
        bool reveal_picks;                             // Show picks to all players
        bool allow_trade_phase;                        // Allow trading after draft
        
        DraftSettings() 
            : team_size(6), player_count(2), ban_phase_picks_per_player(2),
              pick_phase_picks_per_turn(1), allow_pick_same_turn(false),
              max_legendaries_per_team(1), max_same_type_per_team(2),
              snake_draft(true), reveal_picks(true), allow_trade_phase(false) {}
    };

    /**
     * @brief Draft session state and history
     */
    struct DraftSession {
        DraftSettings settings;
        
        // Session state
        std::string session_id;
        bool is_active;
        int current_phase;                             // 0=ban, 1=pick, 2=trade, 3=complete
        int current_player;
        int current_turn;
        
        // Draft pools
        std::vector<std::string> available_pokemon;    // Available for picking
        std::vector<std::string> banned_pokemon;       // Banned during session
        
        // Player teams and data
        std::vector<std::string> player_names;
        std::vector<std::vector<std::string>> player_teams;  // Pokemon picked by each player
        std::vector<std::vector<std::string>> player_bans;   // Pokemon banned by each player
        
        // Draft history for strategy analysis
        struct DraftAction {
            int player_id;
            std::string action_type;                   // "ban", "pick", "trade"
            std::string pokemon_name;
            int turn_number;
            std::string timestamp;
            std::string strategy_note;                 // Optional strategy explanation
        };
        std::vector<DraftAction> draft_history;
        
        // Team validation results
        std::vector<bool> teams_valid;
        std::vector<std::vector<std::string>> team_errors;
        
        DraftSession() : is_active(false), current_phase(0), current_player(0), current_turn(0) {}
    };

    /**
     * @brief Team sharing and import/export functionality
     */
    struct TeamShareCode {
        std::string team_name;
        std::vector<TeamPokemon> pokemon;
        std::string creator_name;
        std::string creation_date;
        std::string description;
        std::string format_version;                    // For backward compatibility
        
        TeamShareCode() : format_version("1.0") {}
    };

    /**
     * @brief Battle history tracking for team performance
     */
    struct BattleRecord {
        std::string team_name;
        std::string opponent_team;
        std::string battle_date;
        bool victory;
        int turns_taken;
        std::string difficulty_level;
        std::vector<std::string> strategies_used;
        double team_effectiveness_score;               // 0-100 based on performance
        
        BattleRecord() : victory(false), turns_taken(0), team_effectiveness_score(0.0) {}
    };

    /**
     * @brief Team statistics and performance metrics
     */
    struct TeamStatistics {
        std::string team_name;
        int total_battles;
        int victories;
        int defeats;
        double win_rate;
        double average_battle_length;
        double average_effectiveness_score;
        
        // Type matchup performance
        std::map<std::string, double> type_matchup_performance;
        
        // Most/least effective Pokemon
        std::vector<std::pair<std::string, double>> pokemon_effectiveness;
        
        // Recommended improvements
        std::vector<std::string> improvement_suggestions;
        
        TeamStatistics() : total_battles(0), victories(0), defeats(0), win_rate(0.0),
                          average_battle_length(0.0), average_effectiveness_score(0.0) {}
    };

    // Tournament/Draft Mode Methods
    /**
     * @brief Create a new draft session with specified settings
     * @param settings Draft configuration and rules
     * @param player_names Names of participating players
     * @return New draft session ready to start
     */
    DraftSession createDraftSession(const DraftSettings& settings, 
                                   const std::vector<std::string>& player_names);

    /**
     * @brief Execute a ban action in the current draft session
     * @param session Draft session to modify
     * @param player_id Player making the ban (0-based index)
     * @param pokemon_name Pokemon to ban
     * @return True if ban was successful
     */
    bool executeDraftBan(DraftSession& session, int player_id, const std::string& pokemon_name);

    /**
     * @brief Execute a pick action in the current draft session
     * @param session Draft session to modify
     * @param player_id Player making the pick (0-based index)
     * @param pokemon_name Pokemon to pick
     * @return True if pick was successful
     */
    bool executeDraftPick(DraftSession& session, int player_id, const std::string& pokemon_name);

    /**
     * @brief Get current available picks for a player
     * @param session Current draft session
     * @param player_id Player to get picks for
     * @param filter_by_strategy Optional strategy filter
     * @return Vector of available Pokemon names
     */
    std::vector<std::string> getAvailablePicks(const DraftSession& session, int player_id,
                                              const std::string& filter_by_strategy = "") const;

    /**
     * @brief Get draft suggestions for current player
     * @param session Current draft session
     * @param suggestion_count Number of suggestions to provide
     * @return Vector of suggested Pokemon with reasoning
     */
    std::vector<std::pair<std::string, std::string>> getDraftSuggestions(
        const DraftSession& session, int suggestion_count = 3) const;

    /**
     * @brief Advance draft session to next phase/player
     * @param session Draft session to advance
     * @return True if advancement was successful
     */
    bool advanceDraftTurn(DraftSession& session);

    /**
     * @brief Validate team composition according to draft rules
     * @param session Draft session with rules
     * @param player_id Player whose team to validate
     * @return ValidationResult with team validity and errors
     */
    InputValidator::ValidationResult<bool> validateDraftTeam(const DraftSession& session, int player_id) const;

    /**
     * @brief Convert draft results to battle-ready teams
     * @param session Completed draft session
     * @return Vector of teams ready for battle
     */
    std::vector<Team> finalizeDraftTeams(const DraftSession& session);

    /**
     * @brief Get detailed draft analysis and strategy insights
     * @param session Draft session to analyze
     * @return Map of player IDs to strategy analysis
     */
    std::map<int, std::vector<std::string>> analyzeDraftStrategy(const DraftSession& session) const;

    // Team Sharing System Methods
    /**
     * @brief Export team as shareable base64 encoded string
     * @param team Team to export
     * @param creator_name Name of team creator
     * @param description Optional team description
     * @return Base64 encoded team share code
     */
    std::string exportTeamShareCode(const Team& team, const std::string& creator_name,
                                   const std::string& description = "") const;

    /**
     * @brief Import team from base64 encoded share code
     * @param share_code Base64 encoded team data
     * @param validate_team Whether to validate imported team
     * @return Imported team or empty team if import failed
     */
    Team importTeamFromShareCode(const std::string& share_code, bool validate_team = true);

    /**
     * @brief Save team to custom teams directory
     * @param team Team to save
     * @param custom_filename Optional custom filename (auto-generated if empty)
     * @return True if saved successfully
     */
    bool saveCustomTeam(const Team& team, const std::string& custom_filename = "");

    /**
     * @brief Load team from custom teams directory
     * @param filename Filename in custom teams directory
     * @return Loaded team or empty team if failed
     */
    Team loadCustomTeam(const std::string& filename);

    /**
     * @brief Get list of all custom teams
     * @return Vector of custom team filenames
     */
    std::vector<std::string> getCustomTeamsList() const;

    /**
     * @brief Delete custom team file
     * @param filename Custom team filename to delete
     * @return True if deletion was successful
     */
    bool deleteCustomTeam(const std::string& filename);

    // Team Comparison and Analysis Methods
    /**
     * @brief Compare two teams and provide detailed analysis
     * @param team1 First team to compare
     * @param team2 Second team to compare
     * @return Detailed comparison analysis
     */
    struct TeamComparison {
        std::string team1_name;
        std::string team2_name;
        
        // Type effectiveness analysis
        std::map<std::string, double> team1_vs_team2_effectiveness;
        std::map<std::string, double> team2_vs_team1_effectiveness;
        
        // Balance comparison
        int team1_balance_score;
        int team2_balance_score;
        
        // Coverage analysis
        std::vector<std::string> team1_coverage_advantages;
        std::vector<std::string> team2_coverage_advantages;
        std::vector<std::string> mutual_weaknesses;
        
        // Predicted battle outcome
        double team1_win_probability;
        std::string battle_prediction_reasoning;
        
        // Improvement suggestions
        std::vector<std::string> team1_improvement_suggestions;
        std::vector<std::string> team2_improvement_suggestions;
    };
    
    TeamComparison compareTeams(const Team& team1, const Team& team2) const;

    // Battle History and Statistics Methods
    /**
     * @brief Record a battle result for team statistics
     * @param team_name Name of team that battled
     * @param opponent_name Name of opponent team
     * @param victory Whether the team won
     * @param turns_taken Number of turns the battle lasted
     * @param difficulty AI difficulty level
     * @param effectiveness_score Performance score (0-100)
     */
    void recordBattleResult(const std::string& team_name, const std::string& opponent_name,
                           bool victory, int turns_taken, const std::string& difficulty,
                           double effectiveness_score = 50.0);

    /**
     * @brief Get comprehensive statistics for a team
     * @param team_name Name of team to get statistics for
     * @return Team statistics or empty if no data found
     */
    std::optional<TeamStatistics> getTeamStatistics(const std::string& team_name) const;

    /**
     * @brief Get battle history for a team
     * @param team_name Name of team to get history for
     * @param max_records Maximum number of records to return (0 for all)
     * @return Vector of battle records
     */
    std::vector<BattleRecord> getTeamBattleHistory(const std::string& team_name, 
                                                  int max_records = 10) const;

    /**
     * @brief Clear battle history for a team
     * @param team_name Name of team to clear history for
     * @return True if clearing was successful
     */
    bool clearTeamBattleHistory(const std::string& team_name);

    // Enhanced Random Generation Methods
    /**
     * @brief Generate random team with advanced constraints and meta considerations
     * @param settings Enhanced random generation settings
     * @param meta_analysis Whether to consider current meta when generating
     * @return Generated team optimized for current meta
     */
    Team generateMetaOptimizedTeam(const RandomGenerationSettings& settings,
                                  bool meta_analysis = true) const;

    /**
     * @brief Generate counter team specifically designed to counter another team
     * @param target_team Team to counter
     * @param team_name Name for the counter team
     * @param strictness How strictly to counter (0.0-1.0, higher = more focused)
     * @return Generated counter team
     */
    Team generateCounterTeam(const Team& target_team, const std::string& team_name,
                            double strictness = 0.7) const;

private:
    std::shared_ptr<PokemonData> pokemon_data;
    ValidationSettings validation_settings;
    
    // Template system storage - mutable for lazy loading in const methods
    mutable std::unordered_map<std::string, std::unordered_map<std::string, TeamTemplate>> templates;
    mutable bool templates_loaded;
    
    // Battle history and statistics storage
    mutable std::unordered_map<std::string, std::vector<BattleRecord>> battle_history;
    mutable std::unordered_map<std::string, TeamStatistics> team_statistics;
    
    // Draft session management
    std::unordered_map<std::string, DraftSession> active_draft_sessions;
    
    // Performance optimization caches
    mutable std::unordered_map<std::string, std::vector<std::string>> pokemon_type_cache;
    mutable std::unordered_map<std::string, std::vector<std::string>> pokemon_moves_cache;

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
    void ensureTemplatesLoaded() const;
    TeamTemplate parseTemplateFromJson(const std::string& file_path);
    bool isValidTemplateFile(const std::string& file_path) const;
    std::string getTemplateNameFromFile(const std::string& file_path) const;
    Team convertTemplateToTeam(const TeamTemplate& template_data, const std::string& custom_name) const;
    std::vector<std::string> getPokemonByRole(const std::string& role, int count) const;
    std::vector<std::string> getLegendaryPokemon() const;
    bool isPokemonLegendary(const std::string& pokemon_name) const;
    std::vector<std::string> generateRoleBasedMoves(const std::string& pokemon_name, const std::string& role) const;
    bool teamMeetsRoleRequirements(const Team& team, const RandomGenerationSettings& settings) const;

    // Draft system helper methods
    std::string generateSessionId() const;
    bool isDraftActionValid(const DraftSession& session, int player_id, 
                           const std::string& action_type, const std::string& pokemon_name) const;
    void updateDraftPhase(DraftSession& session) const;
    int getNextPlayer(const DraftSession& session) const;
    std::vector<std::string> getPlayerTeamTypes(const DraftSession& session, int player_id) const;
    bool exceedsTypeLimit(const DraftSession& session, int player_id, const std::string& pokemon_name) const;
    bool exceedsLegendaryLimit(const DraftSession& session, int player_id, const std::string& pokemon_name) const;
    std::string getCurrentTimestamp() const;

    // Team sharing helper methods
    std::string encodeTeamToBase64(const TeamShareCode& share_code) const;
    TeamShareCode decodeTeamFromBase64(const std::string& base64_data) const;
    std::string getCustomTeamsDirectory() const;
    std::string sanitizeCustomFilename(const std::string& filename) const;
    bool ensureCustomTeamsDirectoryExists() const;

    // Team comparison helper methods
    double calculateTypeMatchupAdvantage(const Team& attacker, const Team& defender) const;
    std::vector<std::string> findCoverageGaps(const Team& team) const;
    std::vector<std::string> findCoverageStrengths(const Team& team) const;
    double predictBattleOutcome(const Team& team1, const Team& team2) const;

    // Battle history helper methods
    void loadBattleHistory() const;
    void saveBattleHistory() const;
    void updateTeamStatistics(const std::string& team_name) const;
    std::string getBattleHistoryFilePath() const;
    std::string getTeamStatisticsFilePath() const;

    // Performance optimization helper methods
    void preloadPokemonData() const;
    void clearPerformanceCaches() const;
    std::vector<std::string> getCachedPokemonTypes(const std::string& pokemon_name) const;
    std::vector<std::string> getCachedPokemonMoves(const std::string& pokemon_name) const;

    // Enhanced generation helper methods
    std::vector<std::string> getMetaTierPokemon(const std::string& tier) const;
    std::vector<std::string> getCounterPokemon(const std::string& target_pokemon) const;
    double calculatePokemonSynergy(const std::vector<std::string>& team_pokemon) const;
    std::vector<std::string> optimizeTeamComposition(const std::vector<std::string>& base_team) const;
};