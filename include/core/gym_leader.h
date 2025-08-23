#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include "team_builder.h"
#include "tournament_manager.h"
#include "ai_factory.h"

// Forward declarations
class Battle;
class Team;

/**
 * @brief Gym Leader battle system with type specialization and badge rewards
 * 
 * This class manages the 8 gym leaders, each specializing in a specific Pokemon type.
 * It handles gym battle execution, badge awarding, attempt tracking, and provides
 * type-specific challenges with appropriate difficulty scaling and AI strategies.
 */
class GymLeader {
public:
    /**
     * @brief Information about a specific gym leader
     */
    struct GymLeaderInfo {
        std::string name;
        std::string gym_name;
        std::string city;
        std::string type_specialization;
        std::string difficulty_level;
        std::string team_template;
        int badge_id;                    // 1-8 for order
        
        // Leader personality and strategy
        std::string personality;
        std::string signature_move;
        std::string battle_style;        // "aggressive", "defensive", "balanced"
        std::vector<std::string> strategy_tips;
        std::vector<std::string> recommended_counters;
        
        // Gym environment
        std::string gym_theme;
        std::string field_effects;      // Special battle conditions
        std::vector<std::string> gym_trainers;  // Other trainers in gym
        
        GymLeaderInfo() : badge_id(0) {}
    };

    /**
     * @brief Badge information awarded by gym leaders
     */
    struct GymBadge {
        std::string badge_name;
        std::string gym_leader_name;
        std::string type_specialization;
        std::string description;
        std::string earned_date;
        int attempts_required;
        double final_score;
        
        // Badge effects (for future expansion)
        std::vector<std::string> battle_bonuses;
        std::string unlock_description;
        
        GymBadge() : attempts_required(1), final_score(0.0) {}
    };

    /**
     * @brief Player's progress against a specific gym leader
     */
    struct GymProgress {
        std::string player_name;
        std::string gym_leader_name;
        
        // Attempt tracking
        int total_attempts;
        int victories;
        int defeats;
        bool badge_earned;
        
        // Performance tracking
        std::vector<double> attempt_scores;
        double best_score;
        double average_score;
        int best_turns;
        std::string last_attempt_date;
        
        // Strategic analysis
        std::vector<std::string> losing_strategies;
        std::vector<std::string> winning_strategies;
        std::unordered_map<std::string, int> pokemon_effectiveness;  // Pokemon name -> effectiveness rating
        
        GymProgress() 
            : total_attempts(0), victories(0), defeats(0), badge_earned(false),
              best_score(0.0), average_score(0.0), best_turns(999) {}
    };

    /**
     * @brief Result of a gym battle
     */
    struct GymBattleResult {
        std::string player_name;
        std::string gym_leader_name;
        std::string player_team_name;
        bool victory;
        
        // Battle details
        int turns_taken;
        double performance_score;
        std::string difficulty_level;
        std::string battle_date;
        std::string battle_duration;
        
        // Team analysis
        std::vector<std::string> effective_pokemon;      // Player's Pokemon that performed well
        std::vector<std::string> ineffective_pokemon;    // Player's Pokemon that struggled
        std::string mvp_pokemon;                         // Most valuable player Pokemon
        
        // Type effectiveness analysis
        std::unordered_map<std::string, double> type_matchup_effectiveness;
        std::vector<std::string> missed_opportunities;   // Strategic mistakes
        std::vector<std::string> good_decisions;         // Strategic successes
        
        // Post-battle team status
        std::vector<std::string> fainted_pokemon;
        bool needs_healing;
        
        GymBattleResult() 
            : victory(false), turns_taken(0), performance_score(0.0), needs_healing(false) {}
    };

    /**
     * @brief Gym system configuration
     */
    struct GymSettings {
        // Battle settings
        bool allow_multiple_attempts;        // Allow retry after defeat
        int max_attempts_per_gym;           // Maximum attempts (0 = unlimited)
        bool heal_between_attempts;         // Heal team between attempts
        bool track_detailed_stats;          // Track comprehensive battle data
        
        // Difficulty settings
        std::string base_difficulty;        // Default gym difficulty
        bool scale_with_badges;             // Increase difficulty with badge count
        bool adaptive_difficulty;           // Adjust based on player performance
        
        // Badge settings
        bool award_badges_immediately;      // Award badges right after victory
        bool allow_badge_rechallenges;      // Allow battles even after earning badge
        bool track_post_badge_battles;      // Count battles after earning badge
        
        // Team composition rules
        bool enforce_type_restrictions;     // Enforce gym type themes
        int min_type_pokemon;              // Minimum Pokemon of gym type
        bool allow_dual_types;             // Count dual-type Pokemon
        
        GymSettings()
            : allow_multiple_attempts(true), max_attempts_per_gym(0),
              heal_between_attempts(true), track_detailed_stats(true),
              base_difficulty("Medium"), scale_with_badges(true), adaptive_difficulty(false),
              award_badges_immediately(true), allow_badge_rechallenges(true),
              track_post_badge_battles(false), enforce_type_restrictions(true),
              min_type_pokemon(4), allow_dual_types(true) {}
    };

    // Constructor
    GymLeader(std::shared_ptr<PokemonData> data, 
              std::shared_ptr<TeamBuilder> team_builder,
              std::shared_ptr<TournamentManager> tournament_manager);

    // Gym Leader Management
    /**
     * @brief Get information about a specific gym leader
     * @param gym_leader_name Name of the gym leader
     * @return Gym leader information or empty if not found
     */
    std::optional<GymLeaderInfo> getGymLeaderInfo(const std::string& gym_leader_name) const;

    /**
     * @brief Get list of all gym leaders
     * @return Vector of all gym leader information
     */
    std::vector<GymLeaderInfo> getAllGymLeaders() const;

    /**
     * @brief Get gym leaders by type specialization
     * @param type_name Pokemon type
     * @return Vector of gym leaders specializing in that type
     */
    std::vector<GymLeaderInfo> getGymLeadersByType(const std::string& type_name) const;

    /**
     * @brief Get recommended gym leader order for progression
     * @return Vector of gym leader names in recommended challenge order
     */
    std::vector<std::string> getRecommendedGymOrder() const;

    // Battle Management
    /**
     * @brief Challenge a gym leader to battle
     * @param player_name Name of the challenging player
     * @param gym_leader_name Name of the gym leader to challenge
     * @param player_team Player's team for the battle
     * @return Battle result with detailed information
     */
    GymBattleResult challengeGymLeader(const std::string& player_name,
                                      const std::string& gym_leader_name,
                                      const TeamBuilder::Team& player_team);

    /**
     * @brief Check if player can challenge a specific gym leader
     * @param player_name Name of the player
     * @param gym_leader_name Name of the gym leader
     * @return True if challenge is allowed
     */
    bool canChallengeGymLeader(const std::string& player_name, 
                              const std::string& gym_leader_name) const;

    /**
     * @brief Get next recommended gym challenge for a player
     * @param player_name Name of the player
     * @return Gym leader name or empty if none recommended
     */
    std::optional<std::string> getNextRecommendedGym(const std::string& player_name) const;

    // Progress Tracking
    /**
     * @brief Get player's progress against a specific gym leader
     * @param player_name Name of the player
     * @param gym_leader_name Name of the gym leader
     * @return Gym progress information
     */
    std::optional<GymProgress> getPlayerGymProgress(const std::string& player_name,
                                                   const std::string& gym_leader_name) const;

    /**
     * @brief Get player's progress against all gym leaders
     * @param player_name Name of the player
     * @return Map of gym leader names to progress information
     */
    std::unordered_map<std::string, GymProgress> getAllGymProgress(const std::string& player_name) const;

    /**
     * @brief Update player progress after a gym battle
     * @param battle_result Result of the completed gym battle
     * @return True if progress updated successfully
     */
    bool updateGymProgress(const GymBattleResult& battle_result);

    // Badge Management
    /**
     * @brief Check if player has earned a specific badge
     * @param player_name Name of the player
     * @param gym_leader_name Name of the gym leader
     * @return True if badge has been earned
     */
    bool hasPlayerEarnedBadge(const std::string& player_name, 
                             const std::string& gym_leader_name) const;

    /**
     * @brief Get all badges earned by a player
     * @param player_name Name of the player
     * @return Vector of earned badges
     */
    std::vector<GymBadge> getPlayerBadges(const std::string& player_name) const;

    /**
     * @brief Get badge information for a specific gym
     * @param gym_leader_name Name of the gym leader
     * @return Badge information or empty if not found
     */
    std::optional<GymBadge> getBadgeInfo(const std::string& gym_leader_name) const;

    /**
     * @brief Count badges earned by a player
     * @param player_name Name of the player
     * @return Number of badges earned (0-8)
     */
    int getPlayerBadgeCount(const std::string& player_name) const;

    // Team Analysis and Recommendations
    /**
     * @brief Analyze player's team effectiveness against a gym leader
     * @param player_team Player's team to analyze
     * @param gym_leader_name Name of the gym leader
     * @return Analysis with recommendations and predictions
     */
    struct TeamAnalysis {
        std::string gym_leader_name;
        std::string gym_type;
        double predicted_success_rate;
        
        // Team composition analysis
        std::vector<std::string> strong_matchups;
        std::vector<std::string> weak_matchups;
        std::vector<std::string> neutral_matchups;
        
        // Recommendations
        std::vector<std::string> team_suggestions;
        std::vector<std::string> move_suggestions;
        std::vector<std::string> strategy_tips;
        
        // Risk assessment
        std::vector<std::string> potential_problems;
        std::vector<std::string> safety_nets;
        
        TeamAnalysis() : predicted_success_rate(0.0) {}
    };
    
    TeamAnalysis analyzeTeamVsGym(const TeamBuilder::Team& player_team,
                                 const std::string& gym_leader_name) const;

    /**
     * @brief Get recommended team modifications for a gym challenge
     * @param current_team Player's current team
     * @param gym_leader_name Name of the gym leader
     * @return Suggestions for team improvement
     */
    std::vector<std::string> getTeamRecommendations(const TeamBuilder::Team& current_team,
                                                   const std::string& gym_leader_name) const;

    /**
     * @brief Get counter-strategies for a specific gym leader
     * @param gym_leader_name Name of the gym leader
     * @param difficulty_level Desired strategy complexity
     * @return Vector of strategic recommendations
     */
    std::vector<std::string> getGymCounterStrategies(const std::string& gym_leader_name,
                                                    const std::string& difficulty_level = "intermediate") const;

    // Statistics and Analytics
    /**
     * @brief Get comprehensive gym statistics for a player
     * @param player_name Name of the player
     * @return Map of statistic names to values
     */
    std::unordered_map<std::string, double> getPlayerGymStats(const std::string& player_name) const;

    /**
     * @brief Get gym battle history for a player
     * @param player_name Name of the player
     * @param gym_leader_name Optional filter by gym leader
     * @return Vector of gym battle results
     */
    std::vector<GymBattleResult> getPlayerGymHistory(const std::string& player_name,
                                                    const std::string& gym_leader_name = "") const;

    /**
     * @brief Get gym challenge leaderboard
     * @param gym_leader_name Name of the gym leader
     * @param sort_by Sorting criteria ("attempts", "score", "time")
     * @param max_results Maximum results to return
     * @return Vector of player rankings for this gym
     */
    std::vector<std::pair<std::string, double>> getGymLeaderboard(
        const std::string& gym_leader_name,
        const std::string& sort_by = "score", 
        int max_results = 10) const;

    // Configuration Management
    /**
     * @brief Get current gym system settings
     * @return Gym system configuration
     */
    const GymSettings& getGymSettings() const { return gym_settings; }

    /**
     * @brief Update gym system settings
     * @param settings New gym system configuration
     */
    void setGymSettings(const GymSettings& settings) { gym_settings = settings; }

    // Data Persistence
    /**
     * @brief Save gym data to file
     * @return True if save successful
     */
    bool saveGymData() const;

    /**
     * @brief Load gym data from file
     * @return True if load successful
     */
    bool loadGymData();

    /**
     * @brief Reset gym progress for a player
     * @param player_name Name of the player
     * @param gym_leader_name Optional specific gym leader (empty for all gyms)
     * @param confirm_reset Confirmation to prevent accidental resets
     * @return True if reset successful
     */
    bool resetPlayerGymProgress(const std::string& player_name,
                               const std::string& gym_leader_name = "",
                               bool confirm_reset = false);

    // Utility Methods
    /**
     * @brief Validate gym system data integrity
     * @return True if all data is consistent
     */
    bool validateGymData() const;

    /**
     * @brief Get gym system status and health
     * @return Map of system status indicators
     */
    std::unordered_map<std::string, std::string> getGymSystemStatus() const;

private:
    // Core dependencies
    std::shared_ptr<PokemonData> pokemon_data;
    std::shared_ptr<TeamBuilder> team_builder;
    std::shared_ptr<TournamentManager> tournament_manager;
    
    // Gym configuration
    GymSettings gym_settings;
    std::vector<GymLeaderInfo> gym_leaders;
    std::unordered_map<std::string, GymBadge> badge_templates;
    
    // Player progress tracking
    std::unordered_map<std::string, std::unordered_map<std::string, GymProgress>> player_gym_progress;
    std::unordered_map<std::string, std::vector<GymBattleResult>> player_gym_history;
    
    // Initialization and configuration
    void initializeGymLeaders();
    void initializeBadgeTemplates();
    void loadGymConfiguration();
    
    // Battle execution helpers
    std::unique_ptr<TeamBuilder::Team> createGymLeaderTeam(const GymLeaderInfo& gym_leader);
    std::string calculateGymDifficulty(const std::string& gym_leader_name, 
                                      const std::string& player_name) const;
    
    // Progress tracking helpers
    void recordBattleResult(const GymBattleResult& result);
    void updatePlayerProgress(const std::string& player_name, 
                             const std::string& gym_leader_name,
                             const GymBattleResult& result);
    void awardBadgeIfEarned(const std::string& player_name,
                           const std::string& gym_leader_name,
                           const GymBattleResult& result);
    
    // Analysis helpers
    double calculateTypeAdvantage(const TeamBuilder::Team& player_team, 
                                 const std::string& gym_type) const;
    std::vector<std::string> identifyTeamWeaknesses(const TeamBuilder::Team& player_team,
                                                   const std::string& gym_type) const;
    std::vector<std::string> identifyTeamStrengths(const TeamBuilder::Team& player_team,
                                                  const std::string& gym_type) const;
    
    // Statistics calculation helpers
    double calculateGymScore(const GymBattleResult& result) const;
    void updateGymStatistics(const std::string& player_name, const std::string& gym_leader_name);
    
    // Data persistence helpers
    std::string getGymDataFilePath() const;
    bool ensureGymDataDirectoryExists() const;
    
    // Validation helpers
    bool isValidGymLeaderName(const std::string& gym_leader_name) const;
    bool isValidPlayerName(const std::string& player_name) const;
    bool canPlayerMakeAttempt(const std::string& player_name, 
                             const std::string& gym_leader_name) const;
    
    // Utility helpers
    std::string getCurrentTimestamp() const;
    std::string formatBattleDuration(int turns) const;
    GymLeaderInfo* findGymLeader(const std::string& gym_leader_name);
    const GymLeaderInfo* findGymLeader(const std::string& gym_leader_name) const;
};