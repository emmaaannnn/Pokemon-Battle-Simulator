#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include "team_builder.h"
#include "tournament_manager.h"
#include "ai_factory.h"

// Forward declaration
class Battle;
class Team;

/**
 * @brief Elite Four and Champion battle management system
 * 
 * This class handles the sequential Elite Four challenges and final Champion battle.
 * It manages team healing, difficulty progression, and victory validation to ensure
 * a proper championship experience with appropriate challenge scaling.
 */
class ChampionshipSystem {
public:
    /**
     * @brief Information about an Elite Four member or Champion
     */
    struct ChampionshipOpponent {
        std::string name;
        std::string title;              // "Elite Four Member" or "Pokemon Champion"
        std::string specialization;     // Type specialization or strategy focus
        std::string difficulty_level;   // AI difficulty level
        std::string team_template;      // Team template to use
        std::vector<std::string> strategy_notes;
        bool is_champion;
        int position_in_sequence;       // 1-4 for Elite Four, 5 for Champion
        
        ChampionshipOpponent() : is_champion(false), position_in_sequence(0) {}
    };

    /**
     * @brief Current state of a championship challenge run
     */
    struct ChampionshipRun {
        std::string player_name;
        std::string player_team_name;
        std::string start_date;
        
        // Progress tracking
        std::vector<std::string> defeated_opponents;
        std::string current_opponent;
        int current_position;           // 1-5 (1-4 Elite Four, 5 Champion)
        bool is_active;
        bool is_completed;
        bool victory;
        
        // Battle statistics
        std::vector<int> battle_turns;
        std::vector<double> battle_scores;
        double total_time_minutes;
        int total_healing_events;
        
        // Run settings
        bool allow_healing_between_battles;
        bool sequential_requirement;
        std::string difficulty_progression;
        
        ChampionshipRun() 
            : current_position(1), is_active(false), is_completed(false), 
              victory(false), total_time_minutes(0.0), total_healing_events(0),
              allow_healing_between_battles(true), sequential_requirement(true),
              difficulty_progression("progressive") {}
    };

    /**
     * @brief Results from a single championship battle
     */
    struct ChampionshipBattleResult {
        std::string player_name;
        std::string opponent_name;
        std::string opponent_type;      // "elite_four" or "champion"
        int opponent_position;
        
        bool victory;
        int turns_taken;
        std::string difficulty_level;
        double performance_score;
        std::string battle_duration;
        
        // Team status after battle
        std::vector<std::string> fainted_pokemon;
        std::vector<std::string> low_health_pokemon;
        bool team_needs_healing;
        
        // Battle analysis
        std::vector<std::string> key_moments;
        std::vector<std::string> strategic_notes;
        std::string mvp_pokemon;        // Most valuable Pokemon
        
        ChampionshipBattleResult() 
            : opponent_position(0), victory(false), turns_taken(0), 
              performance_score(0.0), team_needs_healing(false) {}
    };

    /**
     * @brief Championship system configuration
     */
    struct ChampionshipSettings {
        // Progression requirements
        bool require_sequential_battles;        // Must complete Elite Four in order
        bool require_elite_four_completion;     // Must complete all Elite Four for Champion
        
        // Healing and recovery
        bool allow_healing_between_elite_four; // Full heal between Elite Four members
        bool force_healing_before_champion;     // Mandatory heal before Champion
        bool allow_item_usage;                  // Allow healing items during battles
        
        // Difficulty and AI
        std::string base_difficulty;            // Starting difficulty level
        bool progressive_difficulty;            // Increase difficulty through Elite Four
        std::string champion_difficulty;        // Champion-specific difficulty
        
        // Run management  
        int max_championship_attempts;          // Maximum attempts per run (0 = unlimited)
        bool save_run_progress;                 // Save progress mid-run
        bool allow_run_restart;                 // Allow restarting failed runs
        
        // Scoring and rewards
        bool track_detailed_statistics;        // Track comprehensive battle data
        double time_bonus_multiplier;          // Bonus for faster completion
        double consecutive_victory_bonus;      // Bonus for no defeats
        
        ChampionshipSettings()
            : require_sequential_battles(true), require_elite_four_completion(true),
              allow_healing_between_elite_four(true), force_healing_before_champion(true),
              allow_item_usage(false), base_difficulty("Hard"), progressive_difficulty(true),
              champion_difficulty("Expert"), max_championship_attempts(0),
              save_run_progress(true), allow_run_restart(true),
              track_detailed_statistics(true), time_bonus_multiplier(1.0),
              consecutive_victory_bonus(1.5) {}
    };

    // Constructor
    ChampionshipSystem(std::shared_ptr<PokemonData> data, 
                      std::shared_ptr<TeamBuilder> team_builder,
                      std::shared_ptr<TournamentManager> tournament_manager);

    // Championship Run Management
    /**
     * @brief Start a new championship run for a player
     * @param player_name Name of the player
     * @param team_name Name of player's team to use
     * @return True if championship run started successfully
     */
    bool startChampionshipRun(const std::string& player_name, const std::string& team_name);

    /**
     * @brief Get current championship run for a player
     * @param player_name Name of the player
     * @return Championship run information or empty if none active
     */
    std::optional<ChampionshipRun> getCurrentRun(const std::string& player_name) const;

    /**
     * @brief Resume an existing championship run
     * @param player_name Name of the player
     * @return True if run resumed successfully
     */
    bool resumeChampionshipRun(const std::string& player_name);

    /**
     * @brief End current championship run (success or failure)
     * @param player_name Name of the player
     * @param victory Whether the run ended in victory
     * @return True if run ended and recorded successfully
     */
    bool endChampionshipRun(const std::string& player_name, bool victory);

    // Battle Management
    /**
     * @brief Get information about the next opponent in the championship run
     * @param player_name Name of the player
     * @return Opponent information or empty if no active run
     */
    std::optional<ChampionshipOpponent> getNextOpponent(const std::string& player_name) const;

    /**
     * @brief Execute a championship battle against the current opponent
     * @param player_name Name of the player
     * @param player_team Player's team for the battle
     * @return Battle result with detailed information
     */
    ChampionshipBattleResult executeBattle(const std::string& player_name, 
                                          const TeamBuilder::Team& player_team);

    /**
     * @brief Record the result of a championship battle
     * @param player_name Name of the player
     * @param battle_result Result of the completed battle
     * @return True if result recorded and progression updated
     */
    bool recordBattleResult(const std::string& player_name, 
                           const ChampionshipBattleResult& battle_result);

    // Team Management
    /**
     * @brief Check if player's team needs healing
     * @param player_name Name of the player
     * @param team Player's team to check
     * @return True if healing is recommended
     */
    bool doesTeamNeedHealing(const std::string& player_name, const TeamBuilder::Team& team) const;

    /**
     * @brief Apply full healing to player's team
     * @param player_name Name of the player
     * @param team Team to heal
     * @return True if healing applied successfully
     */
    bool healPlayerTeam(const std::string& player_name, TeamBuilder::Team& team);

    /**
     * @brief Check if healing is allowed at current position
     * @param player_name Name of the player
     * @return True if healing is permitted
     */
    bool isHealingAllowed(const std::string& player_name) const;

    // Progress and Status Queries
    /**
     * @brief Check if player is eligible to start championship challenge
     * @param player_name Name of the player
     * @return True if player meets requirements
     */
    bool isChampionshipEligible(const std::string& player_name) const;

    /**
     * @brief Get championship progress for a player
     * @param player_name Name of the player
     * @return Progress percentage (0.0 - 1.0)
     */
    double getChampionshipProgress(const std::string& player_name) const;

    /**
     * @brief Check if player has completed championship
     * @param player_name Name of the player
     * @return True if player is current champion
     */
    bool isPlayerChampion(const std::string& player_name) const;

    /**
     * @brief Get list of all championship opponents in order
     * @return Vector of opponent information
     */
    std::vector<ChampionshipOpponent> getChampionshipOpponents() const;

    // Statistics and Analytics
    /**
     * @brief Get championship statistics for a player
     * @param player_name Name of the player
     * @return Map of statistic names to values
     */
    std::unordered_map<std::string, double> getChampionshipStats(const std::string& player_name) const;

    /**
     * @brief Get championship battle history for a player
     * @param player_name Name of the player
     * @return Vector of championship battle results
     */
    std::vector<ChampionshipBattleResult> getChampionshipHistory(const std::string& player_name) const;

    /**
     * @brief Get championship leaderboard
     * @param sort_by Sorting criteria ("time", "attempts", "score")
     * @param max_results Maximum results to return
     * @return Vector of player rankings
     */
    std::vector<std::pair<std::string, double>> getChampionshipLeaderboard(
        const std::string& sort_by = "score", int max_results = 10) const;

    // Configuration Management
    /**
     * @brief Get current championship settings
     * @return Championship system configuration
     */
    const ChampionshipSettings& getSettings() const { return championship_settings; }

    /**
     * @brief Update championship settings
     * @param settings New championship configuration
     */
    void setSettings(const ChampionshipSettings& settings) { championship_settings = settings; }

    // Data Persistence
    /**
     * @brief Save championship data to file
     * @return True if save successful
     */
    bool saveChampionshipData() const;

    /**
     * @brief Load championship data from file
     * @return True if load successful
     */
    bool loadChampionshipData();

    /**
     * @brief Reset championship progress for a player
     * @param player_name Name of the player
     * @param confirm_reset Confirmation to prevent accidental resets
     * @return True if reset successful
     */
    bool resetPlayerChampionshipProgress(const std::string& player_name, bool confirm_reset = false);

    // Utility Methods
    /**
     * @brief Validate championship system integrity
     * @return True if all data is consistent
     */
    bool validateChampionshipData() const;

    /**
     * @brief Get championship system status
     * @return Map of system status indicators
     */
    std::unordered_map<std::string, std::string> getChampionshipSystemStatus() const;

private:
    // Core dependencies
    std::shared_ptr<PokemonData> pokemon_data;
    std::shared_ptr<TeamBuilder> team_builder;
    std::shared_ptr<TournamentManager> tournament_manager;
    
    // Championship configuration
    ChampionshipSettings championship_settings;
    std::vector<ChampionshipOpponent> elite_four_roster;
    ChampionshipOpponent champion_opponent;
    
    // Active championship runs
    std::unordered_map<std::string, ChampionshipRun> active_runs;
    std::unordered_map<std::string, std::vector<ChampionshipBattleResult>> championship_history;
    
    // Championship statistics
    std::unordered_map<std::string, std::unordered_map<std::string, double>> player_championship_stats;
    
    // Initialization and configuration
    void initializeChampionshipOpponents();
    void loadChampionshipConfiguration();
    
    // Battle execution helpers
    std::unique_ptr<TeamBuilder::Team> createOpponentTeam(const ChampionshipOpponent& opponent);
    std::string calculateBattleDifficulty(int position) const;
    
    // Progress tracking helpers
    bool advanceRunPosition(ChampionshipRun& run);
    void updateRunStatistics(ChampionshipRun& run, const ChampionshipBattleResult& result);
    bool validateRunProgression(const ChampionshipRun& run) const;
    
    // Team status helpers
    bool hasAnyFaintedPokemon(const TeamBuilder::Team& team) const;
    bool hasAnyLowHealthPokemon(const TeamBuilder::Team& team) const;
    double calculateTeamHealthPercentage(const TeamBuilder::Team& team) const;
    
    // Statistics calculation helpers
    double calculateChampionshipScore(const std::string& player_name) const;
    double calculateCompletionTime(const ChampionshipRun& run) const;
    void updatePlayerChampionshipStats(const std::string& player_name);
    
    // Data persistence helpers
    std::string getChampionshipDataFilePath() const;
    bool ensureChampionshipDataDirectoryExists() const;
    
    // Validation helpers
    bool isValidRun(const ChampionshipRun& run) const;
    bool canPlayerStartRun(const std::string& player_name) const;
    bool meetsSequentialRequirements(const std::string& player_name, int position) const;
    
    // Utility helpers
    std::string getCurrentTimestamp() const;
    std::string generateRunId(const std::string& player_name) const;
    std::string formatBattleDuration(double minutes) const;
};