#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <memory>
#include "team_builder.h"
#include "pokemon_data.h"

/**
 * @brief Comprehensive tournament system managing gym battles, Elite Four, and Champion
 * 
 * This class orchestrates the complete Pokemon tournament experience, tracking player
 * progress through gym battles, Elite Four challenges, and the final Champion battle.
 * It integrates with existing TeamBuilder and AI systems to provide consistent
 * challenge progression and statistics tracking.
 */
class TournamentManager {
public:
    /**
     * @brief Represents a single gym badge earned by the player
     */
    struct Badge {
        std::string gym_name;
        std::string gym_type;
        std::string gym_leader_name;
        std::string earned_date;
        int attempts_to_earn;
        double final_battle_score;
        
        Badge() : attempts_to_earn(0), final_battle_score(0.0) {}
        Badge(const std::string& name, const std::string& type, const std::string& leader,
              const std::string& date, int attempts = 1, double score = 0.0)
            : gym_name(name), gym_type(type), gym_leader_name(leader), 
              earned_date(date), attempts_to_earn(attempts), final_battle_score(score) {}
    };

    /**
     * @brief Tournament progress tracking for a player
     */
    struct TournamentProgress {
        std::string player_name;
        std::vector<Badge> earned_badges;
        std::unordered_set<std::string> defeated_gyms;
        
        // Elite Four progress
        bool elite_four_unlocked;
        std::vector<std::string> defeated_elite_four;
        bool elite_four_completed;
        
        // Champion progress
        bool champion_unlocked;
        bool champion_defeated;
        std::string championship_date;
        
        // Statistics
        int total_gym_attempts;
        int total_elite_four_attempts;
        int total_champion_attempts;
        double average_battle_performance;
        
        TournamentProgress() 
            : elite_four_unlocked(false), elite_four_completed(false),
              champion_unlocked(false), champion_defeated(false),
              total_gym_attempts(0), total_elite_four_attempts(0), 
              total_champion_attempts(0), average_battle_performance(0.0) {}
    };

    /**
     * @brief Challenge difficulty and reward information
     */
    struct ChallengeInfo {
        std::string challenge_name;
        std::string challenge_type;  // "gym", "elite_four", "champion"
        std::string difficulty_level;
        std::vector<std::string> required_badges;
        std::string reward_description;
        bool is_unlocked;
        bool is_completed;
        
        ChallengeInfo() : is_unlocked(false), is_completed(false) {}
    };

    /**
     * @brief Tournament configuration and rules
     */
    struct TournamentSettings {
        // Gym battle settings
        bool require_sequential_gyms;        // Must complete gyms in order
        bool allow_gym_reattempts;          // Allow retrying failed gym battles
        int max_attempts_per_gym;           // Maximum attempts per gym (0 = unlimited)
        bool heal_between_gym_attempts;     // Full heal team between attempts
        
        // Elite Four settings
        bool require_all_badges;            // Need all 8 badges for Elite Four
        bool sequential_elite_four;         // Must complete Elite Four in order
        bool heal_between_elite_battles;    // Heal between Elite Four members
        int max_elite_four_attempts;        // Maximum Elite Four challenge attempts
        
        // Championship settings
        bool require_elite_four_completion; // Need Elite Four for Champion
        bool championship_full_heal;        // Full heal before Champion battle
        int max_championship_attempts;      // Maximum Champion attempts
        
        // Scoring and rewards
        bool track_battle_statistics;       // Track detailed battle performance
        bool award_participation_points;    // Points even for losses
        double difficulty_score_multiplier; // Multiplier for harder difficulties
        
        TournamentSettings()
            : require_sequential_gyms(false), allow_gym_reattempts(true),
              max_attempts_per_gym(0), heal_between_gym_attempts(true),
              require_all_badges(true), sequential_elite_four(true),
              heal_between_elite_battles(true), max_elite_four_attempts(0),
              require_elite_four_completion(true), championship_full_heal(true),
              max_championship_attempts(0), track_battle_statistics(true),
              award_participation_points(true), difficulty_score_multiplier(1.0) {}
    };

    /**
     * @brief Battle result from tournament challenges
     */
    struct TournamentBattleResult {
        std::string challenge_name;
        std::string challenge_type;
        std::string player_team_name;
        std::string opponent_name;
        bool victory;
        int turns_taken;
        std::string difficulty_level;
        double performance_score;
        std::string battle_date;
        std::vector<std::string> strategy_notes;
        
        TournamentBattleResult()
            : victory(false), turns_taken(0), performance_score(0.0) {}
    };

    // Constructor
    TournamentManager(std::shared_ptr<PokemonData> data, std::shared_ptr<TeamBuilder> team_builder);

    // Tournament Progress Management
    /**
     * @brief Initialize tournament progress for a new player
     * @param player_name Name of the player
     * @return True if initialization successful
     */
    bool initializePlayerProgress(const std::string& player_name);

    /**
     * @brief Get current tournament progress for a player
     * @param player_name Name of the player
     * @return Tournament progress or empty optional if player not found
     */
    std::optional<TournamentProgress> getPlayerProgress(const std::string& player_name) const;

    /**
     * @brief Update player progress after a successful challenge
     * @param player_name Name of the player
     * @param battle_result Result of the completed battle
     * @return True if progress updated successfully
     */
    bool updatePlayerProgress(const std::string& player_name, 
                             const TournamentBattleResult& battle_result);

    // Challenge Management
    /**
     * @brief Get list of available challenges for a player
     * @param player_name Name of the player
     * @return Vector of available challenge information
     */
    std::vector<ChallengeInfo> getAvailableChallenges(const std::string& player_name) const;

    /**
     * @brief Check if a specific challenge is unlocked for a player
     * @param player_name Name of the player
     * @param challenge_name Name of the challenge
     * @return True if challenge is unlocked
     */
    bool isChallengeUnlocked(const std::string& player_name, const std::string& challenge_name) const;

    /**
     * @brief Get next recommended challenge for a player
     * @param player_name Name of the player
     * @return Challenge info for next recommended challenge, empty if none
     */
    std::optional<ChallengeInfo> getNextRecommendedChallenge(const std::string& player_name) const;

    // Badge Management
    /**
     * @brief Award a badge to a player
     * @param player_name Name of the player
     * @param badge Badge to award
     * @return True if badge awarded successfully
     */
    bool awardBadge(const std::string& player_name, const Badge& badge);

    /**
     * @brief Get all badges earned by a player
     * @param player_name Name of the player
     * @return Vector of earned badges
     */
    std::vector<Badge> getPlayerBadges(const std::string& player_name) const;

    /**
     * @brief Check if player has earned a specific badge
     * @param player_name Name of the player
     * @param gym_name Name of the gym
     * @return True if player has the badge
     */
    bool hasPlayerEarnedBadge(const std::string& player_name, const std::string& gym_name) const;

    /**
     * @brief Get count of badges earned by a player
     * @param player_name Name of the player
     * @return Number of badges earned (0-8)
     */
    int getPlayerBadgeCount(const std::string& player_name) const;

    // Tournament State Queries
    /**
     * @brief Check if Elite Four is unlocked for a player
     * @param player_name Name of the player
     * @return True if Elite Four is available
     */
    bool isEliteFourUnlocked(const std::string& player_name) const;

    /**
     * @brief Check if Champion battle is unlocked for a player
     * @param player_name Name of the player
     * @return True if Champion battle is available
     */
    bool isChampionshipUnlocked(const std::string& player_name) const;

    /**
     * @brief Get tournament completion status for a player
     * @param player_name Name of the player
     * @return Completion percentage (0.0 - 1.0)
     */
    double getTournamentCompletionPercentage(const std::string& player_name) const;

    // Statistics and Analytics
    /**
     * @brief Get comprehensive tournament statistics for a player
     * @param player_name Name of the player
     * @return Map of statistic names to values
     */
    std::unordered_map<std::string, double> getPlayerTournamentStats(const std::string& player_name) const;

    /**
     * @brief Get battle history for a player's tournament progress
     * @param player_name Name of the player
     * @param challenge_type Optional filter by challenge type
     * @return Vector of battle results
     */
    std::vector<TournamentBattleResult> getPlayerBattleHistory(
        const std::string& player_name, 
        const std::string& challenge_type = "") const;

    /**
     * @brief Get leaderboard of tournament progress across all players
     * @param sort_by Sorting criteria ("badges", "completion", "score")
     * @param max_results Maximum number of results to return
     * @return Vector of player names with their scores/progress
     */
    std::vector<std::pair<std::string, double>> getTournamentLeaderboard(
        const std::string& sort_by = "completion", int max_results = 10) const;

    // Data Persistence
    /**
     * @brief Save tournament progress to JSON file
     * @param player_name Name of the player (empty for all players)
     * @return True if save successful
     */
    bool saveTournamentProgress(const std::string& player_name = "") const;

    /**
     * @brief Load tournament progress from JSON file
     * @param player_name Name of the player (empty for all players)
     * @return True if load successful
     */
    bool loadTournamentProgress(const std::string& player_name = "");

    /**
     * @brief Reset tournament progress for a player
     * @param player_name Name of the player
     * @param confirm_reset Confirmation flag to prevent accidental resets
     * @return True if reset successful
     */
    bool resetPlayerProgress(const std::string& player_name, bool confirm_reset = false);

    // Tournament Settings
    /**
     * @brief Get current tournament settings
     * @return Current tournament configuration
     */
    const TournamentSettings& getTournamentSettings() const { return tournament_settings; }

    /**
     * @brief Update tournament settings
     * @param settings New tournament configuration
     */
    void setTournamentSettings(const TournamentSettings& settings) { tournament_settings = settings; }

    // Utility Methods
    /**
     * @brief Get list of all registered players
     * @return Vector of player names
     */
    std::vector<std::string> getAllPlayers() const;

    /**
     * @brief Validate tournament data integrity
     * @return True if all tournament data is consistent
     */
    bool validateTournamentData() const;

    /**
     * @brief Get tournament system status and health
     * @return Map of system status indicators
     */
    std::unordered_map<std::string, std::string> getTournamentSystemStatus() const;

private:
    // Core dependencies
    std::shared_ptr<PokemonData> pokemon_data;
    std::shared_ptr<TeamBuilder> team_builder;
    
    // Tournament state
    TournamentSettings tournament_settings;
    std::unordered_map<std::string, TournamentProgress> player_progress;
    std::unordered_map<std::string, std::vector<TournamentBattleResult>> player_battle_history;
    
    // Tournament configuration data
    std::vector<std::string> gym_order;
    std::vector<std::string> elite_four_order;
    std::unordered_map<std::string, std::string> gym_type_mapping;
    std::unordered_map<std::string, std::string> gym_leader_mapping;
    
    // Helper methods for progress tracking
    bool updateBadgeProgress(const std::string& player_name, const Badge& badge);
    bool updateEliteFourProgress(const std::string& player_name, const std::string& elite_member);
    bool updateChampionProgress(const std::string& player_name);
    void recalculatePlayerStats(const std::string& player_name);
    
    // Helper methods for challenge validation
    bool meetsGymRequirements(const std::string& player_name, const std::string& gym_name) const;
    bool meetsEliteFourRequirements(const std::string& player_name) const;
    bool meetsChampionRequirements(const std::string& player_name) const;
    
    // Data persistence helpers
    std::string getTournamentDataFilePath() const;
    bool ensureTournamentDataDirectoryExists() const;
    void loadTournamentConfiguration();
    
    // Statistics calculation helpers
    double calculateBattlePerformanceScore(const TournamentBattleResult& result) const;
    double calculateOverallPlayerScore(const std::string& player_name) const;
    
    // Validation helpers
    bool isValidPlayerName(const std::string& player_name) const;
    bool isValidChallengeName(const std::string& challenge_name) const;
    bool isConsistentTournamentState(const TournamentProgress& progress) const;
    
    // Utility helpers
    std::string getCurrentTimestamp() const;
    std::string normalizePlayerName(const std::string& player_name) const;
};