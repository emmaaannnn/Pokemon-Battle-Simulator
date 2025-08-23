#include "gym_leader.h"
#include "battle.h"
#include "team.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <random>

using json = nlohmann::json;
namespace fs = std::filesystem;

GymLeader::GymLeader(std::shared_ptr<PokemonData> data,
                     std::shared_ptr<TeamBuilder> team_builder,
                     std::shared_ptr<TournamentManager> tournament_manager)
    : pokemon_data(data), team_builder(team_builder), tournament_manager(tournament_manager) {
    
    if (!pokemon_data || !team_builder || !tournament_manager) {
        throw std::invalid_argument("GymLeader requires valid dependencies");
    }
    
    // Initialize gym leaders and badges
    initializeGymLeaders();
    initializeBadgeTemplates();
    
    // Load configuration and existing data
    loadGymConfiguration();
    loadGymData();
}

std::optional<GymLeader::GymLeaderInfo> 
GymLeader::getGymLeaderInfo(const std::string& gym_leader_name) const {
    const auto* leader = findGymLeader(gym_leader_name);
    if (leader) {
        return *leader;
    }
    return std::nullopt;
}

std::vector<GymLeader::GymLeaderInfo> GymLeader::getAllGymLeaders() const {
    return gym_leaders;
}

std::vector<GymLeader::GymLeaderInfo> 
GymLeader::getGymLeadersByType(const std::string& type_name) const {
    std::vector<GymLeaderInfo> type_leaders;
    
    for (const auto& leader : gym_leaders) {
        if (leader.type_specialization == type_name) {
            type_leaders.push_back(leader);
        }
    }
    
    return type_leaders;
}

std::vector<std::string> GymLeader::getRecommendedGymOrder() const {
    std::vector<std::string> order;
    
    // Sort by badge_id to get recommended order
    std::vector<GymLeaderInfo> sorted_leaders = gym_leaders;
    std::sort(sorted_leaders.begin(), sorted_leaders.end(),
              [](const GymLeaderInfo& a, const GymLeaderInfo& b) {
                  return a.badge_id < b.badge_id;
              });
    
    for (const auto& leader : sorted_leaders) {
        order.push_back(leader.name);
    }
    
    return order;
}

GymLeader::GymBattleResult 
GymLeader::challengeGymLeader(const std::string& player_name,
                             const std::string& gym_leader_name,
                             const TeamBuilder::Team& player_team) {
    GymBattleResult result;
    result.player_name = player_name;
    result.gym_leader_name = gym_leader_name;
    result.player_team_name = player_team.name;
    result.battle_date = getCurrentTimestamp();
    
    if (!canChallengeGymLeader(player_name, gym_leader_name)) {
        return result;  // Invalid challenge
    }
    
    const auto* gym_leader = findGymLeader(gym_leader_name);
    if (!gym_leader) {
        return result;  // Gym leader not found
    }
    
    result.difficulty_level = calculateGymDifficulty(gym_leader_name, player_name);
    
    // Create gym leader's team
    auto gym_team = createGymLeaderTeam(*gym_leader);
    if (!gym_team) {
        return result;  // Failed to create team
    }
    
    // AI difficulty will be handled by Battle constructor
    
    // Convert TeamBuilder::Team to Team for battle
    // This is a simplified implementation - in practice would need proper conversion
    auto battle_player_team = std::make_shared<Team>();
    auto battle_gym_team = std::make_shared<Team>();
    
    // TODO: Implement proper team conversion
    
    // Execute the battle - simplified for now since Battle uses different interface
    // TODO: Integrate properly with Battle system once interface is unified
    // Battle battle(*battle_player_team, *battle_gym_team, Battle::AIDifficulty::MEDIUM);
    
    // TODO: Execute actual battle and capture results
    // For now, we'll simulate the battle results
    
    // Simulate battle results
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Calculate success probability based on type matchups
    const double type_advantage = calculateTypeAdvantage(player_team, gym_leader->type_specialization);
    const double base_win_rate = 0.6; // Base 60% win rate
    const double adjusted_win_rate = std::min(0.95, std::max(0.05, base_win_rate + type_advantage * 0.3));
    
    std::uniform_real_distribution<> victory_dist(0.0, 1.0);
    result.victory = victory_dist(gen) < adjusted_win_rate;
    
    // Generate battle details
    std::uniform_int_distribution<> turns_dist(12, 30);
    result.turns_taken = turns_dist(gen);
    result.battle_duration = formatBattleDuration(result.turns_taken);
    
    // Calculate performance score
    result.performance_score = calculateGymScore(result);
    
    // Team analysis
    if (result.victory) {
        result.effective_pokemon = identifyTeamStrengths(player_team, gym_leader->type_specialization);
        result.good_decisions.push_back("Effective use of type advantages");
        result.mvp_pokemon = result.effective_pokemon.empty() ? 
                           (player_team.pokemon.empty() ? "" : player_team.pokemon[0].name) :
                           result.effective_pokemon[0];
    } else {
        result.ineffective_pokemon = identifyTeamWeaknesses(player_team, gym_leader->type_specialization);
        result.missed_opportunities.push_back("Could have exploited " + gym_leader->type_specialization + " type weaknesses");
        result.needs_healing = true;
        result.fainted_pokemon = {"pokemon1", "pokemon2"};  // Simulate fainted Pokemon
    }
    
    // Type effectiveness analysis
    result.type_matchup_effectiveness[gym_leader->type_specialization] = type_advantage;
    
    // Record the battle result
    recordBattleResult(result);
    updateGymProgress(result);
    
    return result;
}

bool GymLeader::canChallengeGymLeader(const std::string& player_name, 
                                     const std::string& gym_leader_name) const {
    if (!isValidPlayerName(player_name) || !isValidGymLeaderName(gym_leader_name)) {
        return false;
    }
    
    // Check attempt limits
    if (!canPlayerMakeAttempt(player_name, gym_leader_name)) {
        return false;
    }
    
    // Check if already has badge and rechallenges aren't allowed
    if (!gym_settings.allow_badge_rechallenges && 
        hasPlayerEarnedBadge(player_name, gym_leader_name)) {
        return false;
    }
    
    return true;
}

std::optional<std::string> GymLeader::getNextRecommendedGym(const std::string& player_name) const {
    const auto recommended_order = getRecommendedGymOrder();
    
    for (const auto& gym_name : recommended_order) {
        if (!hasPlayerEarnedBadge(player_name, gym_name)) {
            return gym_name;
        }
    }
    
    return std::nullopt;  // All badges earned
}

std::optional<GymLeader::GymProgress> 
GymLeader::getPlayerGymProgress(const std::string& player_name,
                               const std::string& gym_leader_name) const {
    const auto player_it = player_gym_progress.find(player_name);
    if (player_it == player_gym_progress.end()) {
        return std::nullopt;
    }
    
    const auto gym_it = player_it->second.find(gym_leader_name);
    if (gym_it != player_it->second.end()) {
        return gym_it->second;
    }
    
    return std::nullopt;
}

std::unordered_map<std::string, GymLeader::GymProgress> 
GymLeader::getAllGymProgress(const std::string& player_name) const {
    const auto player_it = player_gym_progress.find(player_name);
    if (player_it != player_gym_progress.end()) {
        return player_it->second;
    }
    
    return std::unordered_map<std::string, GymProgress>();
}

bool GymLeader::updateGymProgress(const GymBattleResult& battle_result) {
    updatePlayerProgress(battle_result.player_name, battle_result.gym_leader_name, battle_result);
    
    if (battle_result.victory) {
        awardBadgeIfEarned(battle_result.player_name, battle_result.gym_leader_name, battle_result);
    }
    
    updateGymStatistics(battle_result.player_name, battle_result.gym_leader_name);
    saveGymData();
    
    return true;
}

bool GymLeader::hasPlayerEarnedBadge(const std::string& player_name, 
                                    const std::string& gym_leader_name) const {
    return tournament_manager->hasPlayerEarnedBadge(player_name, gym_leader_name);
}

std::vector<GymLeader::GymBadge> GymLeader::getPlayerBadges(const std::string& player_name) const {
    std::vector<GymBadge> earned_badges;
    const auto tournament_badges = tournament_manager->getPlayerBadges(player_name);
    
    for (const auto& tournament_badge : tournament_badges) {
        GymBadge gym_badge;
        gym_badge.badge_name = tournament_badge.gym_name + " Badge";
        gym_badge.gym_leader_name = tournament_badge.gym_leader_name;
        gym_badge.type_specialization = tournament_badge.gym_type;
        gym_badge.earned_date = tournament_badge.earned_date;
        gym_badge.attempts_required = tournament_badge.attempts_to_earn;
        gym_badge.final_score = tournament_badge.final_battle_score;
        
        // Get description from badge template
        const auto template_it = badge_templates.find(tournament_badge.gym_leader_name);
        if (template_it != badge_templates.end()) {
            gym_badge.description = template_it->second.description;
            gym_badge.battle_bonuses = template_it->second.battle_bonuses;
            gym_badge.unlock_description = template_it->second.unlock_description;
        }
        
        earned_badges.push_back(gym_badge);
    }
    
    return earned_badges;
}

std::optional<GymLeader::GymBadge> GymLeader::getBadgeInfo(const std::string& gym_leader_name) const {
    const auto it = badge_templates.find(gym_leader_name);
    if (it != badge_templates.end()) {
        return it->second;
    }
    return std::nullopt;
}

int GymLeader::getPlayerBadgeCount(const std::string& player_name) const {
    return tournament_manager->getPlayerBadgeCount(player_name);
}

GymLeader::TeamAnalysis 
GymLeader::analyzeTeamVsGym(const TeamBuilder::Team& player_team,
                           const std::string& gym_leader_name) const {
    TeamAnalysis analysis;
    analysis.gym_leader_name = gym_leader_name;
    
    const auto* gym_leader = findGymLeader(gym_leader_name);
    if (!gym_leader) {
        return analysis;
    }
    
    analysis.gym_type = gym_leader->type_specialization;
    
    // Calculate type advantages
    const double type_advantage = calculateTypeAdvantage(player_team, gym_leader->type_specialization);
    analysis.predicted_success_rate = std::min(95.0, std::max(5.0, 60.0 + type_advantage * 30.0));
    
    // Identify matchups
    analysis.strong_matchups = identifyTeamStrengths(player_team, gym_leader->type_specialization);
    analysis.weak_matchups = identifyTeamWeaknesses(player_team, gym_leader->type_specialization);
    
    // Generate recommendations
    if (analysis.predicted_success_rate < 70.0) {
        analysis.team_suggestions.push_back("Consider adding Pokemon strong against " + 
                                           gym_leader->type_specialization + " types");
        analysis.potential_problems.push_back("Current team may struggle against " + 
                                             gym_leader->type_specialization + " specialists");
    }
    
    // Add general strategy tips
    for (const auto& tip : gym_leader->strategy_tips) {
        analysis.strategy_tips.push_back(tip);
    }
    
    // Add counter recommendations
    for (const auto& counter : gym_leader->recommended_counters) {
        analysis.team_suggestions.push_back("Consider using " + counter + " type Pokemon");
    }
    
    return analysis;
}

std::vector<std::string> 
GymLeader::getTeamRecommendations(const TeamBuilder::Team& current_team,
                                 const std::string& gym_leader_name) const {
    const auto analysis = analyzeTeamVsGym(current_team, gym_leader_name);
    return analysis.team_suggestions;
}

std::vector<std::string> 
GymLeader::getGymCounterStrategies(const std::string& gym_leader_name,
                                  const std::string& difficulty_level) const {
    const auto* gym_leader = findGymLeader(gym_leader_name);
    if (!gym_leader) {
        return std::vector<std::string>();
    }
    
    std::vector<std::string> strategies;
    
    // Basic strategies
    strategies.push_back("Use " + gym_leader->recommended_counters[0] + 
                        " types to exploit " + gym_leader->type_specialization + " weaknesses");
    
    // Add leader-specific strategies
    for (const auto& tip : gym_leader->strategy_tips) {
        strategies.push_back(tip);
    }
    
    // Difficulty-specific advice
    if (difficulty_level == "beginner") {
        strategies.push_back("Focus on type advantages - bring Pokemon strong against " + 
                           gym_leader->type_specialization);
        strategies.push_back("Level up your Pokemon before challenging");
    } else if (difficulty_level == "advanced") {
        strategies.push_back("Consider " + gym_leader->battle_style + " battle tactics");
        strategies.push_back("Watch for " + gym_leader->signature_move + " - it's their signature move");
    }
    
    return strategies;
}

std::unordered_map<std::string, double> 
GymLeader::getPlayerGymStats(const std::string& player_name) const {
    std::unordered_map<std::string, double> stats;
    
    int total_attempts = 0;
    int total_victories = 0;
    int total_defeats = 0;
    double total_score = 0.0;
    int score_count = 0;
    
    const auto player_progress = getAllGymProgress(player_name);
    for (const auto& [gym_name, progress] : player_progress) {
        total_attempts += progress.total_attempts;
        total_victories += progress.victories;
        total_defeats += progress.defeats;
        
        if (!progress.attempt_scores.empty()) {
            for (const auto& score : progress.attempt_scores) {
                total_score += score;
                score_count++;
            }
        }
    }
    
    stats["total_gym_attempts"] = static_cast<double>(total_attempts);
    stats["total_gym_victories"] = static_cast<double>(total_victories);
    stats["total_gym_defeats"] = static_cast<double>(total_defeats);
    stats["badges_earned"] = static_cast<double>(getPlayerBadgeCount(player_name));
    
    if (total_attempts > 0) {
        stats["gym_win_rate"] = (static_cast<double>(total_victories) / 
                                static_cast<double>(total_attempts)) * 100.0;
    }
    
    if (score_count > 0) {
        stats["average_gym_score"] = total_score / static_cast<double>(score_count);
    }
    
    return stats;
}

std::vector<GymLeader::GymBattleResult> 
GymLeader::getPlayerGymHistory(const std::string& player_name,
                              const std::string& gym_leader_name) const {
    const auto it = player_gym_history.find(player_name);
    if (it == player_gym_history.end()) {
        return std::vector<GymBattleResult>();
    }
    
    if (gym_leader_name.empty()) {
        return it->second;
    }
    
    // Filter by gym leader
    std::vector<GymBattleResult> filtered_history;
    for (const auto& result : it->second) {
        if (result.gym_leader_name == gym_leader_name) {
            filtered_history.push_back(result);
        }
    }
    
    return filtered_history;
}

std::vector<std::pair<std::string, double>> 
GymLeader::getGymLeaderboard(const std::string& gym_leader_name,
                            const std::string& sort_by,
                            int max_results) const {
    std::vector<std::pair<std::string, double>> leaderboard;
    
    for (const auto& [player_name, gym_progress_map] : player_gym_progress) {
        const auto it = gym_progress_map.find(gym_leader_name);
        if (it == gym_progress_map.end()) {
            continue;  // Player hasn't challenged this gym
        }
        
        const auto& progress = it->second;
        double score = 0.0;
        
        if (sort_by == "attempts") {
            score = static_cast<double>(progress.total_attempts);
        } else if (sort_by == "time") {
            score = static_cast<double>(progress.best_turns);
        } else { // "score"
            score = progress.best_score;
        }
        
        leaderboard.emplace_back(player_name, score);
    }
    
    // Sort appropriately
    if (sort_by == "attempts" || sort_by == "time") {
        std::sort(leaderboard.begin(), leaderboard.end(),
                 [](const auto& a, const auto& b) { return a.second < b.second; });
    } else {
        std::sort(leaderboard.begin(), leaderboard.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });
    }
    
    if (max_results > 0 && static_cast<int>(leaderboard.size()) > max_results) {
        leaderboard.resize(max_results);
    }
    
    return leaderboard;
}

bool GymLeader::saveGymData() const {
    try {
        if (!ensureGymDataDirectoryExists()) {
            return false;
        }
        
        json gym_data;
        
        // Save player progress
        json progress_json;
        for (const auto& [player_name, gym_map] : player_gym_progress) {
            json player_progress;
            for (const auto& [gym_name, progress] : gym_map) {
                json progress_json_entry;
                progress_json_entry["player_name"] = progress.player_name;
                progress_json_entry["gym_leader_name"] = progress.gym_leader_name;
                progress_json_entry["total_attempts"] = progress.total_attempts;
                progress_json_entry["victories"] = progress.victories;
                progress_json_entry["defeats"] = progress.defeats;
                progress_json_entry["badge_earned"] = progress.badge_earned;
                progress_json_entry["attempt_scores"] = progress.attempt_scores;
                progress_json_entry["best_score"] = progress.best_score;
                progress_json_entry["average_score"] = progress.average_score;
                progress_json_entry["best_turns"] = progress.best_turns;
                progress_json_entry["last_attempt_date"] = progress.last_attempt_date;
                progress_json_entry["losing_strategies"] = progress.losing_strategies;
                progress_json_entry["winning_strategies"] = progress.winning_strategies;
                progress_json_entry["pokemon_effectiveness"] = progress.pokemon_effectiveness;
                
                player_progress[gym_name] = progress_json_entry;
            }
            progress_json[player_name] = player_progress;
        }
        gym_data["player_gym_progress"] = progress_json;
        
        // Save battle history
        json history_json;
        for (const auto& [player_name, history] : player_gym_history) {
            json player_history = json::array();
            for (const auto& result : history) {
                json result_json;
                result_json["player_name"] = result.player_name;
                result_json["gym_leader_name"] = result.gym_leader_name;
                result_json["player_team_name"] = result.player_team_name;
                result_json["victory"] = result.victory;
                result_json["turns_taken"] = result.turns_taken;
                result_json["performance_score"] = result.performance_score;
                result_json["difficulty_level"] = result.difficulty_level;
                result_json["battle_date"] = result.battle_date;
                result_json["battle_duration"] = result.battle_duration;
                result_json["effective_pokemon"] = result.effective_pokemon;
                result_json["ineffective_pokemon"] = result.ineffective_pokemon;
                result_json["mvp_pokemon"] = result.mvp_pokemon;
                result_json["type_matchup_effectiveness"] = result.type_matchup_effectiveness;
                result_json["missed_opportunities"] = result.missed_opportunities;
                result_json["good_decisions"] = result.good_decisions;
                result_json["fainted_pokemon"] = result.fainted_pokemon;
                result_json["needs_healing"] = result.needs_healing;
                player_history.push_back(result_json);
            }
            history_json[player_name] = player_history;
        }
        gym_data["player_gym_history"] = history_json;
        
        // Write to file
        const std::string file_path = getGymDataFilePath();
        std::ofstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        file << gym_data.dump(2);
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool GymLeader::loadGymData() {
    try {
        const std::string file_path = getGymDataFilePath();
        
        if (!fs::exists(file_path)) {
            return true;  // No data to load yet
        }
        
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        json gym_data;
        file >> gym_data;
        
        // Load player progress
        if (gym_data.contains("player_gym_progress")) {
            for (const auto& [player_name, player_progress_json] : gym_data["player_gym_progress"].items()) {
                std::unordered_map<std::string, GymProgress> gym_map;
                for (const auto& [gym_name, progress_json] : player_progress_json.items()) {
                    GymProgress progress;
                    progress.player_name = progress_json.value("player_name", "");
                    progress.gym_leader_name = progress_json.value("gym_leader_name", "");
                    progress.total_attempts = progress_json.value("total_attempts", 0);
                    progress.victories = progress_json.value("victories", 0);
                    progress.defeats = progress_json.value("defeats", 0);
                    progress.badge_earned = progress_json.value("badge_earned", false);
                    progress.attempt_scores = progress_json.value("attempt_scores", std::vector<double>());
                    progress.best_score = progress_json.value("best_score", 0.0);
                    progress.average_score = progress_json.value("average_score", 0.0);
                    progress.best_turns = progress_json.value("best_turns", 999);
                    progress.last_attempt_date = progress_json.value("last_attempt_date", "");
                    progress.losing_strategies = progress_json.value("losing_strategies", std::vector<std::string>());
                    progress.winning_strategies = progress_json.value("winning_strategies", std::vector<std::string>());
                    progress.pokemon_effectiveness = progress_json.value("pokemon_effectiveness", std::unordered_map<std::string, int>());
                    
                    gym_map[gym_name] = progress;
                }
                player_gym_progress[player_name] = gym_map;
            }
        }
        
        // Load battle history
        if (gym_data.contains("player_gym_history")) {
            for (const auto& [player_name, history_json] : gym_data["player_gym_history"].items()) {
                std::vector<GymBattleResult> history;
                for (const auto& result_json : history_json) {
                    GymBattleResult result;
                    result.player_name = result_json.value("player_name", "");
                    result.gym_leader_name = result_json.value("gym_leader_name", "");
                    result.player_team_name = result_json.value("player_team_name", "");
                    result.victory = result_json.value("victory", false);
                    result.turns_taken = result_json.value("turns_taken", 0);
                    result.performance_score = result_json.value("performance_score", 0.0);
                    result.difficulty_level = result_json.value("difficulty_level", "");
                    result.battle_date = result_json.value("battle_date", "");
                    result.battle_duration = result_json.value("battle_duration", "");
                    result.effective_pokemon = result_json.value("effective_pokemon", std::vector<std::string>());
                    result.ineffective_pokemon = result_json.value("ineffective_pokemon", std::vector<std::string>());
                    result.mvp_pokemon = result_json.value("mvp_pokemon", "");
                    result.type_matchup_effectiveness = result_json.value("type_matchup_effectiveness", std::unordered_map<std::string, double>());
                    result.missed_opportunities = result_json.value("missed_opportunities", std::vector<std::string>());
                    result.good_decisions = result_json.value("good_decisions", std::vector<std::string>());
                    result.fainted_pokemon = result_json.value("fainted_pokemon", std::vector<std::string>());
                    result.needs_healing = result_json.value("needs_healing", false);
                    history.push_back(result);
                }
                player_gym_history[player_name] = history;
            }
        }
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool GymLeader::resetPlayerGymProgress(const std::string& player_name,
                                      const std::string& gym_leader_name,
                                      bool confirm_reset) {
    if (!confirm_reset) {
        return false;
    }
    
    if (gym_leader_name.empty()) {
        // Reset all gym progress for player
        player_gym_progress.erase(player_name);
        player_gym_history.erase(player_name);
    } else {
        // Reset specific gym progress
        auto player_it = player_gym_progress.find(player_name);
        if (player_it != player_gym_progress.end()) {
            player_it->second.erase(gym_leader_name);
        }
        
        // Remove specific gym battles from history
        auto history_it = player_gym_history.find(player_name);
        if (history_it != player_gym_history.end()) {
            auto& history = history_it->second;
            history.erase(
                std::remove_if(history.begin(), history.end(),
                              [&gym_leader_name](const GymBattleResult& result) {
                                  return result.gym_leader_name == gym_leader_name;
                              }),
                history.end());
        }
    }
    
    saveGymData();
    return true;
}

bool GymLeader::validateGymData() const {
    // Validate gym leader configuration
    if (gym_leaders.size() != 8) {
        return false;  // Should have exactly 8 gym leaders
    }
    
    // Validate badge templates match gym leaders
    for (const auto& leader : gym_leaders) {
        if (badge_templates.find(leader.name) == badge_templates.end()) {
            return false;  // Missing badge template
        }
    }
    
    // Validate player progress consistency
    for (const auto& [player_name, gym_map] : player_gym_progress) {
        for (const auto& [gym_name, progress] : gym_map) {
            if (progress.total_attempts != (progress.victories + progress.defeats)) {
                return false;  // Inconsistent attempt counts
            }
        }
    }
    
    return true;
}

std::unordered_map<std::string, std::string> GymLeader::getGymSystemStatus() const {
    std::unordered_map<std::string, std::string> status;
    
    status["total_gym_leaders"] = std::to_string(gym_leaders.size());
    status["total_badge_templates"] = std::to_string(badge_templates.size());
    status["players_with_progress"] = std::to_string(player_gym_progress.size());
    status["players_with_history"] = std::to_string(player_gym_history.size());
    status["data_valid"] = validateGymData() ? "true" : "false";
    status["data_file_exists"] = fs::exists(getGymDataFilePath()) ? "true" : "false";
    
    // Count total battles across all players
    int total_battles = 0;
    for (const auto& [player_name, history] : player_gym_history) {
        total_battles += static_cast<int>(history.size());
    }
    status["total_gym_battles"] = std::to_string(total_battles);
    
    return status;
}

// Private method implementations

void GymLeader::initializeGymLeaders() {
    gym_leaders.clear();
    
    // Brock - Rock type
    GymLeaderInfo brock;
    brock.name = "Brock";
    brock.gym_name = "Pewter City Gym";
    brock.city = "Pewter City";
    brock.type_specialization = "rock";
    brock.difficulty_level = "Easy";
    brock.team_template = "rock_team";
    brock.badge_id = 1;
    brock.personality = "Calm and defensive";
    brock.signature_move = "rock-slide";
    brock.battle_style = "defensive";
    brock.strategy_tips.push_back("Rock types are weak to Water, Grass, Fighting, Ground, and Steel");
    brock.strategy_tips.push_back("Bring Pokemon with high Attack to break through his defenses");
    brock.recommended_counters = {"water", "grass", "fighting"};
    brock.gym_theme = "Rocky cavern environment";
    brock.field_effects = "Rock terrain - boosts Rock-type moves";
    gym_leaders.push_back(brock);
    
    // Misty - Water type
    GymLeaderInfo misty;
    misty.name = "Misty";
    misty.gym_name = "Cerulean City Gym";
    misty.city = "Cerulean City";
    misty.type_specialization = "water";
    misty.difficulty_level = "Easy";
    misty.team_template = "water_team";
    misty.badge_id = 2;
    misty.personality = "Energetic and flowing";
    misty.signature_move = "hydro-pump";
    misty.battle_style = "balanced";
    misty.strategy_tips.push_back("Water types are weak to Electric and Grass");
    misty.strategy_tips.push_back("Watch out for her powerful special attacks");
    misty.recommended_counters = {"electric", "grass"};
    misty.gym_theme = "Aquatic pool arena";
    misty.field_effects = "Water terrain - boosts Water-type moves";
    gym_leaders.push_back(misty);
    
    // Lt. Surge - Electric type
    GymLeaderInfo surge;
    surge.name = "Lt. Surge";
    surge.gym_name = "Vermilion City Gym";
    surge.city = "Vermilion City";
    surge.type_specialization = "electric";
    surge.difficulty_level = "Medium";
    surge.team_template = "electric_team";
    surge.badge_id = 3;
    surge.personality = "Aggressive and shocking";
    surge.signature_move = "thunderbolt";
    surge.battle_style = "aggressive";
    surge.strategy_tips.push_back("Electric types are weak to Ground");
    surge.strategy_tips.push_back("Ground types are immune to Electric attacks");
    surge.recommended_counters = {"ground"};
    surge.gym_theme = "Electrified battlefield";
    surge.field_effects = "Electric terrain - boosts Electric-type moves";
    gym_leaders.push_back(surge);
    
    // Erika - Grass type
    GymLeaderInfo erika;
    erika.name = "Erika";
    erika.gym_name = "Celadon City Gym";
    erika.city = "Celadon City";
    erika.type_specialization = "grass";
    erika.difficulty_level = "Medium";
    erika.team_template = "grass_team";
    erika.badge_id = 4;
    erika.personality = "Gentle and nature-loving";
    erika.signature_move = "petal-dance";
    erika.battle_style = "defensive";
    erika.strategy_tips.push_back("Grass types are weak to Fire, Ice, Poison, Flying, and Bug");
    erika.strategy_tips.push_back("Be careful of status moves like Sleep Powder");
    erika.recommended_counters = {"fire", "ice", "flying"};
    erika.gym_theme = "Lush greenhouse garden";
    erika.field_effects = "Grassy terrain - boosts Grass-type moves";
    gym_leaders.push_back(erika);
    
    // Koga - Poison type
    GymLeaderInfo koga;
    koga.name = "Koga";
    koga.gym_name = "Fuchsia City Gym";
    koga.city = "Fuchsia City";
    koga.type_specialization = "poison";
    koga.difficulty_level = "Hard";
    koga.team_template = "psychic_team";  // Using psychic as proxy for poison
    koga.badge_id = 5;
    koga.personality = "Sneaky and tactical";
    koga.signature_move = "toxic";
    koga.battle_style = "defensive";
    koga.strategy_tips.push_back("Poison types are weak to Ground and Psychic");
    koga.strategy_tips.push_back("Watch out for status conditions and poison damage");
    koga.recommended_counters = {"ground", "psychic"};
    koga.gym_theme = "Ninja dojo with invisible walls";
    koga.field_effects = "Toxic spikes - may poison Pokemon";
    gym_leaders.push_back(koga);
    
    // Sabrina - Psychic type
    GymLeaderInfo sabrina;
    sabrina.name = "Sabrina";
    sabrina.gym_name = "Saffron City Gym";
    sabrina.city = "Saffron City";
    sabrina.type_specialization = "psychic";
    sabrina.difficulty_level = "Hard";
    sabrina.team_template = "psychic_team";
    sabrina.badge_id = 6;
    sabrina.personality = "Mysterious and powerful";
    sabrina.signature_move = "psychic";
    sabrina.battle_style = "aggressive";
    sabrina.strategy_tips.push_back("Psychic types are weak to Bug, Ghost, and Dark");
    sabrina.strategy_tips.push_back("Powerful special attacks and teleportation");
    sabrina.recommended_counters = {"bug", "ghost", "dark"};
    sabrina.gym_theme = "Teleportation maze";
    sabrina.field_effects = "Psychic terrain - boosts Psychic-type moves";
    gym_leaders.push_back(sabrina);
    
    // Blaine - Fire type
    GymLeaderInfo blaine;
    blaine.name = "Blaine";
    blaine.gym_name = "Cinnabar Island Gym";
    blaine.city = "Cinnabar Island";
    blaine.type_specialization = "fire";
    blaine.difficulty_level = "Hard";
    blaine.team_template = "fire_team";
    blaine.badge_id = 7;
    blaine.personality = "Hot-headed and intense";
    blaine.signature_move = "fire-blast";
    blaine.battle_style = "aggressive";
    blaine.strategy_tips.push_back("Fire types are weak to Water, Ground, and Rock");
    blaine.strategy_tips.push_back("Intense heat and powerful fire attacks");
    blaine.recommended_counters = {"water", "ground", "rock"};
    blaine.gym_theme = "Volcanic arena with lava";
    blaine.field_effects = "Heat - may burn Pokemon";
    gym_leaders.push_back(blaine);
    
    // Giovanni - Ground type
    GymLeaderInfo giovanni;
    giovanni.name = "Giovanni";
    giovanni.gym_name = "Viridian City Gym";
    giovanni.city = "Viridian City";
    giovanni.type_specialization = "ground";
    giovanni.difficulty_level = "Expert";
    giovanni.team_template = "balanced_meta";  // Using balanced for ground
    giovanni.badge_id = 8;
    giovanni.personality = "Ruthless and strategic";
    giovanni.signature_move = "earthquake";
    giovanni.battle_style = "aggressive";
    giovanni.strategy_tips.push_back("Ground types are weak to Water, Grass, and Ice");
    giovanni.strategy_tips.push_back("Powerful physical attacks and earthquake damage");
    giovanni.recommended_counters = {"water", "grass", "ice"};
    giovanni.gym_theme = "Earth-based battlefield";
    giovanni.field_effects = "Earthquake tremors - may cause flinching";
    gym_leaders.push_back(giovanni);
}

void GymLeader::initializeBadgeTemplates() {
    badge_templates.clear();
    
    for (const auto& leader : gym_leaders) {
        GymBadge badge_template;
        badge_template.badge_name = leader.type_specialization + " Badge";
        badge_template.gym_leader_name = leader.name;
        badge_template.type_specialization = leader.type_specialization;
        badge_template.description = "Earned by defeating " + leader.name + " at " + leader.gym_name;
        badge_template.battle_bonuses.push_back("Increased effectiveness against " + 
                                              leader.type_specialization + " types");
        badge_template.unlock_description = "Proves mastery over " + leader.type_specialization + 
                                          " type challenges";
        
        badge_templates[leader.name] = badge_template;
    }
}

void GymLeader::loadGymConfiguration() {
    // Configuration is currently hardcoded
    // Future versions could load from config files
}

std::unique_ptr<TeamBuilder::Team> GymLeader::createGymLeaderTeam(const GymLeaderInfo& gym_leader) {
    auto team = std::make_unique<TeamBuilder::Team>();
    
    // Generate team based on gym leader's template
    if (gym_leader.team_template == "rock_team") {
        *team = team_builder->generateTeamFromTemplate("type_themed", "rock_team");
    } else if (gym_leader.team_template == "water_team") {
        *team = team_builder->generateTeamFromTemplate("type_themed", "water_team");
    } else if (gym_leader.team_template == "electric_team") {
        *team = team_builder->generateTeamFromTemplate("type_themed", "electric_team");
    } else if (gym_leader.team_template == "grass_team") {
        *team = team_builder->generateTeamFromTemplate("type_themed", "grass_team");
    } else if (gym_leader.team_template == "psychic_team") {
        *team = team_builder->generateTeamFromTemplate("type_themed", "psychic_team");
    } else if (gym_leader.team_template == "fire_team") {
        *team = team_builder->generateTeamFromTemplate("type_themed", "fire_team");
    } else {
        *team = team_builder->generateTeamFromTemplate("competitive", "balanced_meta");
    }
    
    // Set team name
    team->name = gym_leader.name + "'s Gym Team";
    
    return team;
}


std::string GymLeader::calculateGymDifficulty(const std::string& gym_leader_name, 
                                             const std::string& player_name) const {
    const auto* gym_leader = findGymLeader(gym_leader_name);
    if (!gym_leader) {
        return "Medium";
    }
    
    std::string base_difficulty = gym_leader->difficulty_level;
    
    // Adjust difficulty based on player's badge count if enabled
    if (gym_settings.scale_with_badges) {
        const int badge_count = getPlayerBadgeCount(player_name);
        if (badge_count >= 6) {
            if (base_difficulty == "Easy") base_difficulty = "Medium";
            else if (base_difficulty == "Medium") base_difficulty = "Hard";
        } else if (badge_count >= 4) {
            if (base_difficulty == "Easy") base_difficulty = "Medium";
        }
    }
    
    return base_difficulty;
}

void GymLeader::recordBattleResult(const GymBattleResult& result) {
    if (player_gym_history.find(result.player_name) == player_gym_history.end()) {
        player_gym_history[result.player_name] = std::vector<GymBattleResult>();
    }
    
    player_gym_history[result.player_name].push_back(result);
}

void GymLeader::updatePlayerProgress(const std::string& player_name, 
                                   const std::string& gym_leader_name,
                                   const GymBattleResult& result) {
    if (player_gym_progress.find(player_name) == player_gym_progress.end()) {
        player_gym_progress[player_name] = std::unordered_map<std::string, GymProgress>();
    }
    
    auto& gym_map = player_gym_progress[player_name];
    if (gym_map.find(gym_leader_name) == gym_map.end()) {
        gym_map[gym_leader_name] = GymProgress();
        gym_map[gym_leader_name].player_name = player_name;
        gym_map[gym_leader_name].gym_leader_name = gym_leader_name;
    }
    
    auto& progress = gym_map[gym_leader_name];
    progress.total_attempts++;
    progress.last_attempt_date = result.battle_date;
    progress.attempt_scores.push_back(result.performance_score);
    
    if (result.victory) {
        progress.victories++;
        if (!result.good_decisions.empty()) {
            progress.winning_strategies.insert(progress.winning_strategies.end(),
                                             result.good_decisions.begin(),
                                             result.good_decisions.end());
        }
    } else {
        progress.defeats++;
        if (!result.missed_opportunities.empty()) {
            progress.losing_strategies.insert(progress.losing_strategies.end(),
                                            result.missed_opportunities.begin(),
                                            result.missed_opportunities.end());
        }
    }
    
    // Update best performance
    if (result.performance_score > progress.best_score) {
        progress.best_score = result.performance_score;
    }
    
    if (result.turns_taken < progress.best_turns) {
        progress.best_turns = result.turns_taken;
    }
    
    // Update average score
    if (!progress.attempt_scores.empty()) {
        const double total = std::accumulate(progress.attempt_scores.begin(), 
                                           progress.attempt_scores.end(), 0.0);
        progress.average_score = total / static_cast<double>(progress.attempt_scores.size());
    }
}

void GymLeader::awardBadgeIfEarned(const std::string& player_name,
                                  const std::string& gym_leader_name,
                                  const GymBattleResult& result) {
    if (!result.victory || !gym_settings.award_badges_immediately) {
        return;
    }
    
    // Create tournament badge
    TournamentManager::Badge badge;
    badge.gym_name = gym_leader_name + " Gym";
    badge.gym_leader_name = gym_leader_name;
    badge.earned_date = result.battle_date;
    badge.attempts_to_earn = 1;  // This would need to be calculated from progress
    badge.final_battle_score = result.performance_score;
    
    // Get gym type
    const auto* gym_leader = findGymLeader(gym_leader_name);
    if (gym_leader) {
        badge.gym_type = gym_leader->type_specialization;
    }
    
    // Award badge through tournament manager
    tournament_manager->awardBadge(player_name, badge);
    
    // Update local progress
    auto& progress = player_gym_progress[player_name][gym_leader_name];
    progress.badge_earned = true;
}

double GymLeader::calculateTypeAdvantage(const TeamBuilder::Team& player_team, 
                                        const std::string& gym_type) const {
    // Simplified type advantage calculation
    // In practice, this would use proper type effectiveness tables
    
    double advantage = 0.0;
    int pokemon_count = 0;
    
    for (const auto& pokemon : player_team.pokemon) {
        // This is a simplified implementation
        // Would need to check actual Pokemon types from pokemon_data
        pokemon_count++;
        
        // Simple heuristic based on common type matchups
        if (gym_type == "rock" && (pokemon.name.find("water") != std::string::npos ||
                                  pokemon.name.find("grass") != std::string::npos)) {
            advantage += 0.5;
        } else if (gym_type == "water" && pokemon.name.find("electric") != std::string::npos) {
            advantage += 0.5;
        } else if (gym_type == "electric" && pokemon.name.find("ground") != std::string::npos) {
            advantage += 0.5;
        }
        // Add more type matchup logic here
    }
    
    return pokemon_count > 0 ? advantage / static_cast<double>(pokemon_count) : 0.0;
}

std::vector<std::string> 
GymLeader::identifyTeamWeaknesses(const TeamBuilder::Team& player_team,
                                 const std::string& gym_type) const {
    std::vector<std::string> weaknesses;
    
    // Simple heuristic for team weaknesses
    if (gym_type == "fire" && player_team.pokemon.size() > 0) {
        bool has_water = false;
        bool has_rock = false;
        for (const auto& pokemon : player_team.pokemon) {
            if (pokemon.name.find("water") != std::string::npos) has_water = true;
            if (pokemon.name.find("rock") != std::string::npos) has_rock = true;
        }
        
        if (!has_water && !has_rock) {
            weaknesses.push_back("No water or rock types to counter fire");
        }
    }
    
    return weaknesses;
}

std::vector<std::string> 
GymLeader::identifyTeamStrengths(const TeamBuilder::Team& player_team,
                                const std::string& gym_type) const {
    std::vector<std::string> strengths;
    
    // Simple heuristic for team strengths
    for (const auto& pokemon : player_team.pokemon) {
        if ((gym_type == "rock" && pokemon.name.find("water") != std::string::npos) ||
            (gym_type == "fire" && pokemon.name.find("water") != std::string::npos) ||
            (gym_type == "electric" && pokemon.name.find("ground") != std::string::npos)) {
            strengths.push_back(pokemon.name);
        }
    }
    
    return strengths;
}

double GymLeader::calculateGymScore(const GymBattleResult& result) const {
    double base_score = result.victory ? 100.0 : 50.0;
    
    // Adjust for difficulty
    if (result.difficulty_level == "Easy") {
        base_score *= 0.8;
    } else if (result.difficulty_level == "Hard") {
        base_score *= 1.2;
    } else if (result.difficulty_level == "Expert") {
        base_score *= 1.5;
    }
    
    // Adjust for turn efficiency
    if (result.turns_taken > 0) {
        const double turn_efficiency = std::max(0.5, 1.0 - (result.turns_taken - 15.0) / 30.0);
        base_score *= turn_efficiency;
    }
    
    return std::min(150.0, std::max(0.0, base_score));
}

void GymLeader::updateGymStatistics(const std::string& /* player_name */, 
                                   const std::string& /* gym_leader_name */) {
    // Statistics are updated automatically through progress tracking
    // Additional complex statistics could be calculated here
}

std::string GymLeader::getGymDataFilePath() const {
    return "data/tournaments/gym_data.json";
}

bool GymLeader::ensureGymDataDirectoryExists() const {
    try {
        const std::string dir_path = "data/tournaments";
        return fs::create_directories(dir_path);
    } catch (const std::exception&) {
        return false;
    }
}

bool GymLeader::isValidGymLeaderName(const std::string& gym_leader_name) const {
    return findGymLeader(gym_leader_name) != nullptr;
}

bool GymLeader::isValidPlayerName(const std::string& player_name) const {
    return !player_name.empty() && player_name.length() <= 50;
}

bool GymLeader::canPlayerMakeAttempt(const std::string& player_name, 
                                    const std::string& gym_leader_name) const {
    if (gym_settings.max_attempts_per_gym <= 0) {
        return true;  // Unlimited attempts
    }
    
    const auto progress = getPlayerGymProgress(player_name, gym_leader_name);
    if (!progress) {
        return true;  // No previous attempts
    }
    
    return progress->total_attempts < gym_settings.max_attempts_per_gym;
}

std::string GymLeader::getCurrentTimestamp() const {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string GymLeader::formatBattleDuration(int turns) const {
    const int minutes = turns / 2;  // Simplified: 2 turns per minute
    return std::to_string(minutes) + "m";
}

GymLeader::GymLeaderInfo* GymLeader::findGymLeader(const std::string& gym_leader_name) {
    const auto it = std::find_if(gym_leaders.begin(), gym_leaders.end(),
                                [&gym_leader_name](const GymLeaderInfo& leader) {
                                    return leader.name == gym_leader_name;
                                });
    
    return it != gym_leaders.end() ? &(*it) : nullptr;
}

const GymLeader::GymLeaderInfo* GymLeader::findGymLeader(const std::string& gym_leader_name) const {
    const auto it = std::find_if(gym_leaders.begin(), gym_leaders.end(),
                                [&gym_leader_name](const GymLeaderInfo& leader) {
                                    return leader.name == gym_leader_name;
                                });
    
    return it != gym_leaders.end() ? &(*it) : nullptr;
}