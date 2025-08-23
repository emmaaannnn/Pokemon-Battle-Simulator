#include "tournament_manager.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>

using json = nlohmann::json;
namespace fs = std::filesystem;

TournamentManager::TournamentManager(std::shared_ptr<PokemonData> data, 
                                   std::shared_ptr<TeamBuilder> team_builder)
    : pokemon_data(data), team_builder(team_builder) {
    
    if (!pokemon_data || !team_builder) {
        throw std::invalid_argument("TournamentManager requires valid PokemonData and TeamBuilder instances");
    }
    
    // Initialize gym configuration
    gym_order = {"Brock", "Misty", "Lt. Surge", "Erika", "Koga", "Sabrina", "Blaine", "Giovanni"};
    gym_type_mapping = {
        {"Brock", "rock"}, {"Misty", "water"}, {"Lt. Surge", "electric"},
        {"Erika", "grass"}, {"Koga", "poison"}, {"Sabrina", "psychic"},
        {"Blaine", "fire"}, {"Giovanni", "ground"}
    };
    gym_leader_mapping = {
        {"Brock", "Pewter City Gym"}, {"Misty", "Cerulean City Gym"}, 
        {"Lt. Surge", "Vermilion City Gym"}, {"Erika", "Celadon City Gym"},
        {"Koga", "Fuchsia City Gym"}, {"Sabrina", "Saffron City Gym"},
        {"Blaine", "Cinnabar Island Gym"}, {"Giovanni", "Viridian City Gym"}
    };
    
    // Initialize Elite Four configuration
    elite_four_order = {"Lorelei", "Bruno", "Agatha", "Lance"};
    
    // Load tournament configuration and existing progress
    loadTournamentConfiguration();
    loadTournamentProgress();
}

bool TournamentManager::initializePlayerProgress(const std::string& player_name) {
    if (!isValidPlayerName(player_name)) {
        return false;
    }
    
    const std::string normalized_name = normalizePlayerName(player_name);
    
    // Don't reinitialize existing players
    if (player_progress.find(normalized_name) != player_progress.end()) {
        return true;  // Already initialized
    }
    
    TournamentProgress progress;
    progress.player_name = normalized_name;
    
    player_progress[normalized_name] = progress;
    player_battle_history[normalized_name] = std::vector<TournamentBattleResult>();
    
    saveTournamentProgress(normalized_name);
    return true;
}

std::optional<TournamentManager::TournamentProgress> 
TournamentManager::getPlayerProgress(const std::string& player_name) const {
    const std::string normalized_name = normalizePlayerName(player_name);
    
    const auto it = player_progress.find(normalized_name);
    if (it != player_progress.end()) {
        return it->second;
    }
    
    return std::nullopt;
}

bool TournamentManager::updatePlayerProgress(const std::string& player_name, 
                                           const TournamentBattleResult& battle_result) {
    const std::string normalized_name = normalizePlayerName(player_name);
    
    auto progress_it = player_progress.find(normalized_name);
    if (progress_it == player_progress.end()) {
        if (!initializePlayerProgress(normalized_name)) {
            return false;
        }
        progress_it = player_progress.find(normalized_name);
    }
    
    // Record battle result
    player_battle_history[normalized_name].push_back(battle_result);
    
    // Update progress based on battle type and result
    bool progress_updated = false;
    
    if (battle_result.challenge_type == "gym" && battle_result.victory) {
        // Award badge for gym victory
        Badge badge;
        badge.gym_name = battle_result.challenge_name;
        badge.gym_leader_name = battle_result.opponent_name;
        badge.earned_date = battle_result.battle_date;
        badge.attempts_to_earn = 1; // This will be updated if they had previous attempts
        badge.final_battle_score = battle_result.performance_score;
        
        // Find gym type
        const auto type_it = gym_type_mapping.find(battle_result.opponent_name);
        if (type_it != gym_type_mapping.end()) {
            badge.gym_type = type_it->second;
        }
        
        progress_updated = updateBadgeProgress(normalized_name, badge);
        
    } else if (battle_result.challenge_type == "elite_four" && battle_result.victory) {
        progress_updated = updateEliteFourProgress(normalized_name, battle_result.opponent_name);
        
    } else if (battle_result.challenge_type == "champion" && battle_result.victory) {
        progress_updated = updateChampionProgress(normalized_name);
    }
    
    // Update attempt counters
    if (battle_result.challenge_type == "gym") {
        progress_it->second.total_gym_attempts++;
    } else if (battle_result.challenge_type == "elite_four") {
        progress_it->second.total_elite_four_attempts++;
    } else if (battle_result.challenge_type == "champion") {
        progress_it->second.total_champion_attempts++;
    }
    
    // Recalculate player statistics
    recalculatePlayerStats(normalized_name);
    
    // Save progress
    saveTournamentProgress(normalized_name);
    
    return progress_updated;
}

std::vector<TournamentManager::ChallengeInfo> 
TournamentManager::getAvailableChallenges(const std::string& player_name) const {
    std::vector<ChallengeInfo> available_challenges;
    const std::string normalized_name = normalizePlayerName(player_name);
    
    const auto progress_it = player_progress.find(normalized_name);
    if (progress_it == player_progress.end()) {
        return available_challenges;  // No progress found
    }
    
    const auto& progress = progress_it->second;
    
    // Add gym challenges
    for (const auto& gym_leader : gym_order) {
        if (progress.defeated_gyms.find(gym_leader) == progress.defeated_gyms.end()) {
            ChallengeInfo challenge;
            challenge.challenge_name = gym_leader_mapping.at(gym_leader);
            challenge.challenge_type = "gym";
            challenge.difficulty_level = "Medium";  // Gyms are medium difficulty
            challenge.reward_description = "Earn the " + gym_type_mapping.at(gym_leader) + " badge";
            challenge.is_unlocked = true;  // All gyms are always available
            challenge.is_completed = false;
            
            available_challenges.push_back(challenge);
        }
    }
    
    // Add Elite Four challenges
    if (isEliteFourUnlocked(normalized_name)) {
        for (const auto& elite_member : elite_four_order) {
            if (std::find(progress.defeated_elite_four.begin(), progress.defeated_elite_four.end(), 
                         elite_member) == progress.defeated_elite_four.end()) {
                
                ChallengeInfo challenge;
                challenge.challenge_name = elite_member + " of the Elite Four";
                challenge.challenge_type = "elite_four";
                challenge.difficulty_level = "Hard";
                challenge.reward_description = "Defeat " + elite_member + " to advance in Elite Four";
                challenge.is_unlocked = true;
                challenge.is_completed = false;
                
                available_challenges.push_back(challenge);
            }
        }
    }
    
    // Add Champion challenge
    if (isChampionshipUnlocked(normalized_name)) {
        ChallengeInfo challenge;
        challenge.challenge_name = "Pokemon Champion Battle";
        challenge.challenge_type = "champion";
        challenge.difficulty_level = "Expert";
        challenge.reward_description = "Become the Pokemon Champion!";
        challenge.is_unlocked = true;
        challenge.is_completed = progress.champion_defeated;
        
        available_challenges.push_back(challenge);
    }
    
    return available_challenges;
}

bool TournamentManager::isChallengeUnlocked(const std::string& player_name, 
                                          const std::string& challenge_name) const {
    const auto challenges = getAvailableChallenges(player_name);
    
    for (const auto& challenge : challenges) {
        if (challenge.challenge_name == challenge_name) {
            return challenge.is_unlocked;
        }
    }
    
    return false;
}

std::optional<TournamentManager::ChallengeInfo> 
TournamentManager::getNextRecommendedChallenge(const std::string& player_name) const {
    const auto challenges = getAvailableChallenges(player_name);
    
    // Prioritize gym challenges first
    for (const auto& challenge : challenges) {
        if (challenge.challenge_type == "gym" && challenge.is_unlocked && !challenge.is_completed) {
            return challenge;
        }
    }
    
    // Then Elite Four
    for (const auto& challenge : challenges) {
        if (challenge.challenge_type == "elite_four" && challenge.is_unlocked && !challenge.is_completed) {
            return challenge;
        }
    }
    
    // Finally Champion
    for (const auto& challenge : challenges) {
        if (challenge.challenge_type == "champion" && challenge.is_unlocked && !challenge.is_completed) {
            return challenge;
        }
    }
    
    return std::nullopt;
}

bool TournamentManager::awardBadge(const std::string& player_name, const Badge& badge) {
    const std::string normalized_name = normalizePlayerName(player_name);
    return updateBadgeProgress(normalized_name, badge);
}

std::vector<TournamentManager::Badge> 
TournamentManager::getPlayerBadges(const std::string& player_name) const {
    const std::string normalized_name = normalizePlayerName(player_name);
    
    const auto it = player_progress.find(normalized_name);
    if (it != player_progress.end()) {
        return it->second.earned_badges;
    }
    
    return std::vector<Badge>();
}

bool TournamentManager::hasPlayerEarnedBadge(const std::string& player_name, 
                                            const std::string& gym_name) const {
    const std::string normalized_name = normalizePlayerName(player_name);
    
    const auto it = player_progress.find(normalized_name);
    if (it != player_progress.end()) {
        return it->second.defeated_gyms.find(gym_name) != it->second.defeated_gyms.end();
    }
    
    return false;
}

int TournamentManager::getPlayerBadgeCount(const std::string& player_name) const {
    const std::string normalized_name = normalizePlayerName(player_name);
    
    const auto it = player_progress.find(normalized_name);
    if (it != player_progress.end()) {
        return static_cast<int>(it->second.earned_badges.size());
    }
    
    return 0;
}

bool TournamentManager::isEliteFourUnlocked(const std::string& player_name) const {
    if (!tournament_settings.require_all_badges) {
        return true;  // Elite Four always available if not requiring badges
    }
    
    return getPlayerBadgeCount(player_name) >= 8;
}

bool TournamentManager::isChampionshipUnlocked(const std::string& player_name) const {
    if (!tournament_settings.require_elite_four_completion) {
        return true;  // Champion always available if not requiring Elite Four
    }
    
    const std::string normalized_name = normalizePlayerName(player_name);
    const auto it = player_progress.find(normalized_name);
    if (it != player_progress.end()) {
        return it->second.elite_four_completed;
    }
    
    return false;
}

double TournamentManager::getTournamentCompletionPercentage(const std::string& player_name) const {
    const std::string normalized_name = normalizePlayerName(player_name);
    
    const auto it = player_progress.find(normalized_name);
    if (it == player_progress.end()) {
        return 0.0;
    }
    
    const auto& progress = it->second;
    double completion = 0.0;
    const double total_challenges = 8.0 + 4.0 + 1.0;  // 8 gyms + 4 Elite Four + 1 Champion
    
    // Count completed challenges
    completion += static_cast<double>(progress.earned_badges.size());  // Gyms
    completion += static_cast<double>(progress.defeated_elite_four.size());  // Elite Four
    if (progress.champion_defeated) {
        completion += 1.0;  // Champion
    }
    
    return completion / total_challenges;
}

std::unordered_map<std::string, double> 
TournamentManager::getPlayerTournamentStats(const std::string& player_name) const {
    std::unordered_map<std::string, double> stats;
    const std::string normalized_name = normalizePlayerName(player_name);
    
    const auto progress_it = player_progress.find(normalized_name);
    if (progress_it == player_progress.end()) {
        return stats;
    }
    
    const auto& progress = progress_it->second;
    
    stats["badges_earned"] = static_cast<double>(progress.earned_badges.size());
    stats["elite_four_defeated"] = static_cast<double>(progress.defeated_elite_four.size());
    stats["champion_defeated"] = progress.champion_defeated ? 1.0 : 0.0;
    stats["completion_percentage"] = getTournamentCompletionPercentage(normalized_name) * 100.0;
    
    stats["total_gym_attempts"] = static_cast<double>(progress.total_gym_attempts);
    stats["total_elite_four_attempts"] = static_cast<double>(progress.total_elite_four_attempts);
    stats["total_champion_attempts"] = static_cast<double>(progress.total_champion_attempts);
    
    stats["average_battle_performance"] = progress.average_battle_performance;
    
    // Calculate success rates
    if (progress.total_gym_attempts > 0) {
        stats["gym_success_rate"] = (static_cast<double>(progress.earned_badges.size()) / 
                                   static_cast<double>(progress.total_gym_attempts)) * 100.0;
    }
    
    if (progress.total_elite_four_attempts > 0) {
        stats["elite_four_success_rate"] = (static_cast<double>(progress.defeated_elite_four.size()) / 
                                          static_cast<double>(progress.total_elite_four_attempts)) * 100.0;
    }
    
    return stats;
}

std::vector<TournamentManager::TournamentBattleResult> 
TournamentManager::getPlayerBattleHistory(const std::string& player_name, 
                                        const std::string& challenge_type) const {
    const std::string normalized_name = normalizePlayerName(player_name);
    
    const auto it = player_battle_history.find(normalized_name);
    if (it == player_battle_history.end()) {
        return std::vector<TournamentBattleResult>();
    }
    
    if (challenge_type.empty()) {
        return it->second;
    }
    
    // Filter by challenge type
    std::vector<TournamentBattleResult> filtered_history;
    for (const auto& result : it->second) {
        if (result.challenge_type == challenge_type) {
            filtered_history.push_back(result);
        }
    }
    
    return filtered_history;
}

std::vector<std::pair<std::string, double>> 
TournamentManager::getTournamentLeaderboard(const std::string& sort_by, int max_results) const {
    std::vector<std::pair<std::string, double>> leaderboard;
    
    for (const auto& [player_name, progress] : player_progress) {
        double score = 0.0;
        
        if (sort_by == "badges") {
            score = static_cast<double>(progress.earned_badges.size());
        } else if (sort_by == "completion") {
            score = getTournamentCompletionPercentage(player_name) * 100.0;
        } else if (sort_by == "score") {
            score = calculateOverallPlayerScore(player_name);
        }
        
        leaderboard.emplace_back(player_name, score);
    }
    
    // Sort descending by score
    std::sort(leaderboard.begin(), leaderboard.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Limit results
    if (max_results > 0 && static_cast<int>(leaderboard.size()) > max_results) {
        leaderboard.resize(max_results);
    }
    
    return leaderboard;
}

bool TournamentManager::saveTournamentProgress(const std::string& player_name) const {
    try {
        if (!ensureTournamentDataDirectoryExists()) {
            return false;
        }
        
        json tournament_data;
        
        // Save player progress
        json players_json;
        for (const auto& [name, progress] : player_progress) {
            if (!player_name.empty() && name != normalizePlayerName(player_name)) {
                continue;  // Skip other players if specific player requested
            }
            
            json player_json;
            player_json["player_name"] = progress.player_name;
            
            // Badges
            json badges_json = json::array();
            for (const auto& badge : progress.earned_badges) {
                json badge_json;
                badge_json["gym_name"] = badge.gym_name;
                badge_json["gym_type"] = badge.gym_type;
                badge_json["gym_leader_name"] = badge.gym_leader_name;
                badge_json["earned_date"] = badge.earned_date;
                badge_json["attempts_to_earn"] = badge.attempts_to_earn;
                badge_json["final_battle_score"] = badge.final_battle_score;
                badges_json.push_back(badge_json);
            }
            player_json["earned_badges"] = badges_json;
            
            // Defeated gyms
            player_json["defeated_gyms"] = json::array();
            for (const auto& gym : progress.defeated_gyms) {
                player_json["defeated_gyms"].push_back(gym);
            }
            
            // Elite Four progress
            player_json["elite_four_unlocked"] = progress.elite_four_unlocked;
            player_json["defeated_elite_four"] = progress.defeated_elite_four;
            player_json["elite_four_completed"] = progress.elite_four_completed;
            
            // Champion progress
            player_json["champion_unlocked"] = progress.champion_unlocked;
            player_json["champion_defeated"] = progress.champion_defeated;
            player_json["championship_date"] = progress.championship_date;
            
            // Statistics
            player_json["total_gym_attempts"] = progress.total_gym_attempts;
            player_json["total_elite_four_attempts"] = progress.total_elite_four_attempts;
            player_json["total_champion_attempts"] = progress.total_champion_attempts;
            player_json["average_battle_performance"] = progress.average_battle_performance;
            
            players_json[name] = player_json;
        }
        tournament_data["players"] = players_json;
        
        // Save battle history
        json history_json;
        for (const auto& [name, history] : player_battle_history) {
            if (!player_name.empty() && name != normalizePlayerName(player_name)) {
                continue;
            }
            
            json player_history = json::array();
            for (const auto& result : history) {
                json result_json;
                result_json["challenge_name"] = result.challenge_name;
                result_json["challenge_type"] = result.challenge_type;
                result_json["player_team_name"] = result.player_team_name;
                result_json["opponent_name"] = result.opponent_name;
                result_json["victory"] = result.victory;
                result_json["turns_taken"] = result.turns_taken;
                result_json["difficulty_level"] = result.difficulty_level;
                result_json["performance_score"] = result.performance_score;
                result_json["battle_date"] = result.battle_date;
                result_json["strategy_notes"] = result.strategy_notes;
                player_history.push_back(result_json);
            }
            history_json[name] = player_history;
        }
        tournament_data["battle_history"] = history_json;
        
        // Write to file
        const std::string file_path = getTournamentDataFilePath();
        std::ofstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        file << tournament_data.dump(2);
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool TournamentManager::loadTournamentProgress(const std::string& player_name) {
    try {
        const std::string file_path = getTournamentDataFilePath();
        
        if (!fs::exists(file_path)) {
            return true;  // No data to load yet, that's okay
        }
        
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        json tournament_data;
        file >> tournament_data;
        
        // Clear existing data if loading all players
        if (player_name.empty()) {
            player_progress.clear();
            player_battle_history.clear();
        }
        
        // Load player progress
        if (tournament_data.contains("players")) {
            for (const auto& [name, player_json] : tournament_data["players"].items()) {
                if (!player_name.empty() && name != normalizePlayerName(player_name)) {
                    continue;
                }
                
                TournamentProgress progress;
                progress.player_name = player_json.value("player_name", "");
                
                // Load badges
                if (player_json.contains("earned_badges")) {
                    for (const auto& badge_json : player_json["earned_badges"]) {
                        Badge badge;
                        badge.gym_name = badge_json.value("gym_name", "");
                        badge.gym_type = badge_json.value("gym_type", "");
                        badge.gym_leader_name = badge_json.value("gym_leader_name", "");
                        badge.earned_date = badge_json.value("earned_date", "");
                        badge.attempts_to_earn = badge_json.value("attempts_to_earn", 1);
                        badge.final_battle_score = badge_json.value("final_battle_score", 0.0);
                        progress.earned_badges.push_back(badge);
                    }
                }
                
                // Load defeated gyms
                if (player_json.contains("defeated_gyms")) {
                    for (const auto& gym : player_json["defeated_gyms"]) {
                        progress.defeated_gyms.insert(gym);
                    }
                }
                
                // Load Elite Four progress
                progress.elite_four_unlocked = player_json.value("elite_four_unlocked", false);
                progress.defeated_elite_four = player_json.value("defeated_elite_four", std::vector<std::string>());
                progress.elite_four_completed = player_json.value("elite_four_completed", false);
                
                // Load Champion progress
                progress.champion_unlocked = player_json.value("champion_unlocked", false);
                progress.champion_defeated = player_json.value("champion_defeated", false);
                progress.championship_date = player_json.value("championship_date", "");
                
                // Load statistics
                progress.total_gym_attempts = player_json.value("total_gym_attempts", 0);
                progress.total_elite_four_attempts = player_json.value("total_elite_four_attempts", 0);
                progress.total_champion_attempts = player_json.value("total_champion_attempts", 0);
                progress.average_battle_performance = player_json.value("average_battle_performance", 0.0);
                
                player_progress[name] = progress;
            }
        }
        
        // Load battle history
        if (tournament_data.contains("battle_history")) {
            for (const auto& [name, history_json] : tournament_data["battle_history"].items()) {
                if (!player_name.empty() && name != normalizePlayerName(player_name)) {
                    continue;
                }
                
                std::vector<TournamentBattleResult> history;
                for (const auto& result_json : history_json) {
                    TournamentBattleResult result;
                    result.challenge_name = result_json.value("challenge_name", "");
                    result.challenge_type = result_json.value("challenge_type", "");
                    result.player_team_name = result_json.value("player_team_name", "");
                    result.opponent_name = result_json.value("opponent_name", "");
                    result.victory = result_json.value("victory", false);
                    result.turns_taken = result_json.value("turns_taken", 0);
                    result.difficulty_level = result_json.value("difficulty_level", "");
                    result.performance_score = result_json.value("performance_score", 0.0);
                    result.battle_date = result_json.value("battle_date", "");
                    result.strategy_notes = result_json.value("strategy_notes", std::vector<std::string>());
                    history.push_back(result);
                }
                player_battle_history[name] = history;
            }
        }
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool TournamentManager::resetPlayerProgress(const std::string& player_name, bool confirm_reset) {
    if (!confirm_reset) {
        return false;  // Safety check
    }
    
    const std::string normalized_name = normalizePlayerName(player_name);
    
    // Remove from data structures
    player_progress.erase(normalized_name);
    player_battle_history.erase(normalized_name);
    
    // Reinitialize
    return initializePlayerProgress(normalized_name);
}

std::vector<std::string> TournamentManager::getAllPlayers() const {
    std::vector<std::string> players;
    for (const auto& [player_name, _] : player_progress) {
        players.push_back(player_name);
    }
    return players;
}

bool TournamentManager::validateTournamentData() const {
    // Validate all player progress data
    for (const auto& [player_name, progress] : player_progress) {
        if (!isConsistentTournamentState(progress)) {
            return false;
        }
    }
    
    // Validate battle history consistency
    for (const auto& [player_name, history] : player_battle_history) {
        if (player_progress.find(player_name) == player_progress.end()) {
            return false;  // History without progress
        }
    }
    
    return true;
}

std::unordered_map<std::string, std::string> 
TournamentManager::getTournamentSystemStatus() const {
    std::unordered_map<std::string, std::string> status;
    
    status["total_players"] = std::to_string(player_progress.size());
    status["data_file_exists"] = fs::exists(getTournamentDataFilePath()) ? "true" : "false";
    status["data_valid"] = validateTournamentData() ? "true" : "false";
    
    // Count completed challenges across all players
    int total_badges = 0;
    int total_elite_four = 0;
    int total_champions = 0;
    
    for (const auto& [_, progress] : player_progress) {
        total_badges += static_cast<int>(progress.earned_badges.size());
        total_elite_four += static_cast<int>(progress.defeated_elite_four.size());
        if (progress.champion_defeated) total_champions++;
    }
    
    status["total_badges_earned"] = std::to_string(total_badges);
    status["total_elite_four_defeated"] = std::to_string(total_elite_four);
    status["total_champions"] = std::to_string(total_champions);
    
    return status;
}

// Private helper method implementations

bool TournamentManager::updateBadgeProgress(const std::string& player_name, const Badge& badge) {
    auto progress_it = player_progress.find(player_name);
    if (progress_it == player_progress.end()) {
        return false;
    }
    
    auto& progress = progress_it->second;
    
    // Check if already has this badge
    for (const auto& existing_badge : progress.earned_badges) {
        if (existing_badge.gym_name == badge.gym_name) {
            return false;  // Already earned
        }
    }
    
    // Add badge and update defeated gyms
    progress.earned_badges.push_back(badge);
    progress.defeated_gyms.insert(badge.gym_leader_name);
    
    // Check if Elite Four should be unlocked
    if (static_cast<int>(progress.earned_badges.size()) >= 8) {
        progress.elite_four_unlocked = true;
    }
    
    return true;
}

bool TournamentManager::updateEliteFourProgress(const std::string& player_name, 
                                              const std::string& elite_member) {
    auto progress_it = player_progress.find(player_name);
    if (progress_it == player_progress.end()) {
        return false;
    }
    
    auto& progress = progress_it->second;
    
    // Check if already defeated
    if (std::find(progress.defeated_elite_four.begin(), progress.defeated_elite_four.end(), 
                  elite_member) != progress.defeated_elite_four.end()) {
        return false;  // Already defeated
    }
    
    progress.defeated_elite_four.push_back(elite_member);
    
    // Check if Elite Four completed
    if (static_cast<int>(progress.defeated_elite_four.size()) >= 4) {
        progress.elite_four_completed = true;
        progress.champion_unlocked = true;
    }
    
    return true;
}

bool TournamentManager::updateChampionProgress(const std::string& player_name) {
    auto progress_it = player_progress.find(player_name);
    if (progress_it == player_progress.end()) {
        return false;
    }
    
    auto& progress = progress_it->second;
    
    if (progress.champion_defeated) {
        return false;  // Already champion
    }
    
    progress.champion_defeated = true;
    progress.championship_date = getCurrentTimestamp();
    
    return true;
}

void TournamentManager::recalculatePlayerStats(const std::string& player_name) {
    const auto history_it = player_battle_history.find(player_name);
    if (history_it == player_battle_history.end()) {
        return;
    }
    
    auto progress_it = player_progress.find(player_name);
    if (progress_it == player_progress.end()) {
        return;
    }
    
    const auto& history = history_it->second;
    auto& progress = progress_it->second;
    
    if (history.empty()) {
        return;
    }
    
    // Calculate average performance
    double total_performance = 0.0;
    for (const auto& result : history) {
        total_performance += result.performance_score;
    }
    
    progress.average_battle_performance = total_performance / static_cast<double>(history.size());
}

std::string TournamentManager::getTournamentDataFilePath() const {
    return "data/tournaments/tournament_progress.json";
}

bool TournamentManager::ensureTournamentDataDirectoryExists() const {
    try {
        const std::string dir_path = "data/tournaments";
        return fs::create_directories(dir_path);
    } catch (const std::exception&) {
        return false;
    }
}

void TournamentManager::loadTournamentConfiguration() {
    // Tournament configuration is currently hardcoded
    // In the future, this could load from configuration files
}

double TournamentManager::calculateBattlePerformanceScore(const TournamentBattleResult& result) const {
    double base_score = result.victory ? 100.0 : 50.0;
    
    // Adjust for difficulty
    if (result.difficulty_level == "Easy") {
        base_score *= 0.8;
    } else if (result.difficulty_level == "Hard") {
        base_score *= 1.2;
    } else if (result.difficulty_level == "Expert") {
        base_score *= 1.5;
    }
    
    // Adjust for turn efficiency (fewer turns = better score)
    if (result.turns_taken > 0) {
        const double turn_efficiency = std::max(0.5, 1.0 - (result.turns_taken - 10.0) / 50.0);
        base_score *= turn_efficiency;
    }
    
    return std::min(150.0, std::max(0.0, base_score));
}

double TournamentManager::calculateOverallPlayerScore(const std::string& player_name) const {
    const auto progress_it = player_progress.find(player_name);
    if (progress_it == player_progress.end()) {
        return 0.0;
    }
    
    const auto& progress = progress_it->second;
    double score = 0.0;
    
    // Score for badges (100 points each)
    score += static_cast<double>(progress.earned_badges.size()) * 100.0;
    
    // Score for Elite Four (200 points each)
    score += static_cast<double>(progress.defeated_elite_four.size()) * 200.0;
    
    // Score for Champion (500 points)
    if (progress.champion_defeated) {
        score += 500.0;
    }
    
    // Bonus for efficiency (fewer attempts = higher score)
    const int total_attempts = progress.total_gym_attempts + 
                              progress.total_elite_four_attempts + 
                              progress.total_champion_attempts;
    if (total_attempts > 0) {
        const double efficiency_bonus = std::max(0.0, 100.0 - static_cast<double>(total_attempts));
        score += efficiency_bonus;
    }
    
    // Performance bonus
    score += progress.average_battle_performance;
    
    return score;
}

bool TournamentManager::isValidPlayerName(const std::string& player_name) const {
    return !player_name.empty() && player_name.length() <= 50;
}

bool TournamentManager::isValidChallengeName(const std::string& challenge_name) const {
    return !challenge_name.empty() && challenge_name.length() <= 100;
}

bool TournamentManager::isConsistentTournamentState(const TournamentProgress& progress) const {
    // Validate badge count vs. defeated gyms
    if (progress.earned_badges.size() != progress.defeated_gyms.size()) {
        return false;
    }
    
    // Validate Elite Four unlock requirements
    if (tournament_settings.require_all_badges && progress.elite_four_unlocked) {
        if (static_cast<int>(progress.earned_badges.size()) < 8) {
            return false;
        }
    }
    
    // Validate Elite Four completion
    if (progress.elite_four_completed && static_cast<int>(progress.defeated_elite_four.size()) < 4) {
        return false;
    }
    
    // Validate Champion unlock requirements
    if (tournament_settings.require_elite_four_completion && progress.champion_unlocked) {
        if (!progress.elite_four_completed) {
            return false;
        }
    }
    
    return true;
}

std::string TournamentManager::getCurrentTimestamp() const {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string TournamentManager::normalizePlayerName(const std::string& player_name) const {
    std::string normalized = player_name;
    
    // Trim whitespace
    normalized.erase(0, normalized.find_first_not_of(" \t\n\r"));
    normalized.erase(normalized.find_last_not_of(" \t\n\r") + 1);
    
    return normalized;
}