#include "championship_system.h"
#include "battle.h"
#include "team.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <random>

using json = nlohmann::json;
namespace fs = std::filesystem;

ChampionshipSystem::ChampionshipSystem(std::shared_ptr<PokemonData> data,
                                     std::shared_ptr<TeamBuilder> team_builder,
                                     std::shared_ptr<TournamentManager> tournament_manager)
    : pokemon_data(data), team_builder(team_builder), tournament_manager(tournament_manager) {
    
    if (!pokemon_data || !team_builder || !tournament_manager) {
        throw std::invalid_argument("ChampionshipSystem requires valid dependencies");
    }
    
    // Initialize championship opponents
    initializeChampionshipOpponents();
    
    // Load configuration and existing data
    loadChampionshipConfiguration();
    loadChampionshipData();
}

bool ChampionshipSystem::startChampionshipRun(const std::string& player_name, 
                                             const std::string& team_name) {
    if (!isChampionshipEligible(player_name)) {
        return false;
    }
    
    // Check if player already has an active run
    if (active_runs.find(player_name) != active_runs.end()) {
        return false;  // Already has active run
    }
    
    ChampionshipRun run;
    run.player_name = player_name;
    run.player_team_name = team_name;
    run.start_date = getCurrentTimestamp();
    run.current_position = 1;  // Start with first Elite Four member
    run.is_active = true;
    run.allow_healing_between_battles = championship_settings.allow_healing_between_elite_four;
    run.sequential_requirement = championship_settings.require_sequential_battles;
    run.difficulty_progression = championship_settings.progressive_difficulty ? "progressive" : "fixed";
    
    // Set current opponent to first Elite Four member
    if (!elite_four_roster.empty()) {
        run.current_opponent = elite_four_roster[0].name;
    }
    
    active_runs[player_name] = run;
    
    // Initialize player championship stats if needed
    if (player_championship_stats.find(player_name) == player_championship_stats.end()) {
        player_championship_stats[player_name] = std::unordered_map<std::string, double>();
    }
    
    saveChampionshipData();
    return true;
}

std::optional<ChampionshipSystem::ChampionshipRun> 
ChampionshipSystem::getCurrentRun(const std::string& player_name) const {
    const auto it = active_runs.find(player_name);
    if (it != active_runs.end() && it->second.is_active) {
        return it->second;
    }
    return std::nullopt;
}

bool ChampionshipSystem::resumeChampionshipRun(const std::string& player_name) {
    auto it = active_runs.find(player_name);
    if (it == active_runs.end()) {
        return false;
    }
    
    auto& run = it->second;
    if (run.is_completed) {
        return false;  // Can't resume completed run
    }
    
    run.is_active = true;
    saveChampionshipData();
    return true;
}

bool ChampionshipSystem::endChampionshipRun(const std::string& player_name, bool victory) {
    auto it = active_runs.find(player_name);
    if (it == active_runs.end()) {
        return false;
    }
    
    auto& run = it->second;
    run.is_active = false;
    run.is_completed = true;
    run.victory = victory;
    
    // Update tournament manager if victory
    if (victory) {
        TournamentManager::TournamentBattleResult result;
        result.challenge_name = "Pokemon Championship";
        result.challenge_type = "champion";
        result.player_team_name = run.player_team_name;
        result.opponent_name = "Champion";
        result.victory = true;
        result.turns_taken = 0;  // Aggregate turns would need to be calculated
        result.difficulty_level = championship_settings.champion_difficulty;
        result.performance_score = calculateChampionshipScore(player_name);
        result.battle_date = getCurrentTimestamp();
        
        tournament_manager->updatePlayerProgress(player_name, result);
    }
    
    // Update player stats
    updatePlayerChampionshipStats(player_name);
    
    saveChampionshipData();
    return true;
}

std::optional<ChampionshipSystem::ChampionshipOpponent> 
ChampionshipSystem::getNextOpponent(const std::string& player_name) const {
    const auto run = getCurrentRun(player_name);
    if (!run) {
        return std::nullopt;
    }
    
    const int position = run->current_position;
    
    // Elite Four positions 1-4
    if (position >= 1 && position <= 4) {
        const int index = position - 1;
        if (index < static_cast<int>(elite_four_roster.size())) {
            return elite_four_roster[index];
        }
    }
    
    // Champion position 5
    if (position == 5) {
        return champion_opponent;
    }
    
    return std::nullopt;
}

ChampionshipSystem::ChampionshipBattleResult 
ChampionshipSystem::executeBattle(const std::string& player_name, 
                                 const TeamBuilder::Team& /* player_team */) {
    ChampionshipBattleResult result;
    result.player_name = player_name;
    
    const auto opponent_opt = getNextOpponent(player_name);
    if (!opponent_opt) {
        return result;  // Invalid state
    }
    
    const auto& opponent = *opponent_opt;
    result.opponent_name = opponent.name;
    result.opponent_type = opponent.is_champion ? "champion" : "elite_four";
    result.opponent_position = opponent.position_in_sequence;
    result.difficulty_level = calculateBattleDifficulty(opponent.position_in_sequence);
    
    // Create opponent team
    auto opponent_team = createOpponentTeam(opponent);
    if (!opponent_team) {
        return result;  // Failed to create opponent team
    }
    
    // AI difficulty will be handled by Battle constructor
    
    // Convert TeamBuilder::Team to Team for battle
    // This is a simplified conversion - in practice, you'd need proper integration
    auto battle_team = std::make_shared<Team>();
    // TODO: Implement proper team conversion from TeamBuilder::Team to battle Team
    
    auto battle_opponent_team = std::make_shared<Team>();
    // TODO: Implement proper team conversion for opponent
    
    // Execute the battle - simplified for now since Battle uses different interface
    // TODO: Integrate properly with Battle system once interface is unified
    // Battle battle(*battle_team, *battle_opponent_team, Battle::AIDifficulty::HARD);
    
    // TODO: Execute battle and capture results
    // For now, we'll simulate results
    
    // Simulate battle results
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> victory_dist(0, 1);
    std::uniform_int_distribution<> turns_dist(15, 35);
    std::uniform_real_distribution<> score_dist(60.0, 95.0);
    
    result.victory = victory_dist(gen) == 1;
    result.turns_taken = turns_dist(gen);
    result.performance_score = score_dist(gen);
    result.battle_duration = formatBattleDuration(static_cast<double>(result.turns_taken) * 0.5);
    
    // Simulate team status
    if (!result.victory) {
        result.team_needs_healing = true;
        result.fainted_pokemon = {"pokemon1", "pokemon2"};  // Simulate fainted
    } else {
        result.team_needs_healing = result.turns_taken > 25;
        if (result.team_needs_healing) {
            result.low_health_pokemon = {"pokemon3"};  // Simulate low health
        }
    }
    
    // Add strategic notes
    result.strategic_notes.push_back("Opponent used " + opponent.specialization + " type advantage");
    result.key_moments.push_back("Critical hit at turn " + std::to_string(result.turns_taken / 2));
    
    return result;
}

bool ChampionshipSystem::recordBattleResult(const std::string& player_name,
                                          const ChampionshipBattleResult& battle_result) {
    auto run_it = active_runs.find(player_name);
    if (run_it == active_runs.end()) {
        return false;
    }
    
    auto& run = run_it->second;
    
    // Record battle in history
    if (championship_history.find(player_name) == championship_history.end()) {
        championship_history[player_name] = std::vector<ChampionshipBattleResult>();
    }
    championship_history[player_name].push_back(battle_result);
    
    // Update run statistics
    updateRunStatistics(run, battle_result);
    
    if (battle_result.victory) {
        // Record defeated opponent
        run.defeated_opponents.push_back(battle_result.opponent_name);
        
        // Advance to next position
        if (!advanceRunPosition(run)) {
            // Championship completed!
            endChampionshipRun(player_name, true);
        }
    } else {
        // Battle lost - end run unless settings allow retry
        if (!championship_settings.allow_run_restart || 
            (championship_settings.max_championship_attempts > 0 && 
             run.battle_turns.size() >= static_cast<size_t>(championship_settings.max_championship_attempts))) {
            endChampionshipRun(player_name, false);
        }
    }
    
    saveChampionshipData();
    return true;
}

bool ChampionshipSystem::doesTeamNeedHealing(const std::string& player_name, 
                                           const TeamBuilder::Team& /* team */) const {
    // Simple heuristic - in practice this would check actual Pokemon HP
    const auto run = getCurrentRun(player_name);
    if (!run) {
        return false;
    }
    
    // If we just had a battle, team likely needs healing
    if (!run->battle_turns.empty()) {
        return true;
    }
    
    return false;
}

bool ChampionshipSystem::healPlayerTeam(const std::string& player_name, TeamBuilder::Team& /* team */) {
    if (!isHealingAllowed(player_name)) {
        return false;
    }
    
    // Record healing event
    auto run_it = active_runs.find(player_name);
    if (run_it != active_runs.end()) {
        run_it->second.total_healing_events++;
    }
    
    // TODO: Implement actual team healing
    // For now, we'll just mark it as healed
    
    saveChampionshipData();
    return true;
}

bool ChampionshipSystem::isHealingAllowed(const std::string& player_name) const {
    const auto run = getCurrentRun(player_name);
    if (!run) {
        return false;
    }
    
    // Always allow healing between Elite Four members
    if (run->current_position <= 4 && championship_settings.allow_healing_between_elite_four) {
        return true;
    }
    
    // Force healing before Champion
    if (run->current_position == 5 && championship_settings.force_healing_before_champion) {
        return true;
    }
    
    return false;
}

bool ChampionshipSystem::isChampionshipEligible(const std::string& player_name) const {
    if (championship_settings.require_elite_four_completion) {
        return tournament_manager->isChampionshipUnlocked(player_name);
    }
    
    // If not requiring Elite Four completion, just check if tournament is available
    return tournament_manager->getPlayerProgress(player_name).has_value();
}

double ChampionshipSystem::getChampionshipProgress(const std::string& player_name) const {
    const auto run = getCurrentRun(player_name);
    if (!run) {
        // Check if player has completed championship
        if (isPlayerChampion(player_name)) {
            return 1.0;
        }
        return 0.0;
    }
    
    // Progress is based on current position (1-5)
    const double total_battles = 5.0;  // 4 Elite Four + 1 Champion
    return static_cast<double>(run->current_position - 1) / total_battles;
}

bool ChampionshipSystem::isPlayerChampion(const std::string& player_name) const {
    // Check tournament manager first
    const auto progress = tournament_manager->getPlayerProgress(player_name);
    if (progress && progress->champion_defeated) {
        return true;
    }
    
    // Check championship history for completed victorious runs
    const auto history_it = championship_history.find(player_name);
    if (history_it != championship_history.end()) {
        for (const auto& result : history_it->second) {
            if (result.opponent_type == "champion" && result.victory) {
                return true;
            }
        }
    }
    
    return false;
}

std::vector<ChampionshipSystem::ChampionshipOpponent> 
ChampionshipSystem::getChampionshipOpponents() const {
    std::vector<ChampionshipOpponent> all_opponents = elite_four_roster;
    all_opponents.push_back(champion_opponent);
    return all_opponents;
}

std::unordered_map<std::string, double> 
ChampionshipSystem::getChampionshipStats(const std::string& player_name) const {
    std::unordered_map<std::string, double> stats;
    
    const auto stats_it = player_championship_stats.find(player_name);
    if (stats_it != player_championship_stats.end()) {
        return stats_it->second;
    }
    
    // Return default stats
    stats["total_attempts"] = 0.0;
    stats["victories"] = 0.0;
    stats["defeats"] = 0.0;
    stats["best_completion_time"] = 0.0;
    stats["average_performance_score"] = 0.0;
    
    return stats;
}

std::vector<ChampionshipSystem::ChampionshipBattleResult> 
ChampionshipSystem::getChampionshipHistory(const std::string& player_name) const {
    const auto it = championship_history.find(player_name);
    if (it != championship_history.end()) {
        return it->second;
    }
    return std::vector<ChampionshipBattleResult>();
}

std::vector<std::pair<std::string, double>> 
ChampionshipSystem::getChampionshipLeaderboard(const std::string& sort_by, int max_results) const {
    std::vector<std::pair<std::string, double>> leaderboard;
    
    for (const auto& [player_name, stats] : player_championship_stats) {
        double score = 0.0;
        
        if (sort_by == "time") {
            const auto it = stats.find("best_completion_time");
            if (it != stats.end()) {
                score = it->second;
            }
        } else if (sort_by == "attempts") {
            const auto it = stats.find("total_attempts");
            if (it != stats.end()) {
                score = it->second;
            }
        } else { // "score"
            score = calculateChampionshipScore(player_name);
        }
        
        leaderboard.emplace_back(player_name, score);
    }
    
    // Sort appropriately
    if (sort_by == "time" || sort_by == "attempts") {
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

bool ChampionshipSystem::saveChampionshipData() const {
    try {
        if (!ensureChampionshipDataDirectoryExists()) {
            return false;
        }
        
        json championship_data;
        
        // Save active runs
        json runs_json;
        for (const auto& [player_name, run] : active_runs) {
            json run_json;
            run_json["player_name"] = run.player_name;
            run_json["player_team_name"] = run.player_team_name;
            run_json["start_date"] = run.start_date;
            run_json["defeated_opponents"] = run.defeated_opponents;
            run_json["current_opponent"] = run.current_opponent;
            run_json["current_position"] = run.current_position;
            run_json["is_active"] = run.is_active;
            run_json["is_completed"] = run.is_completed;
            run_json["victory"] = run.victory;
            run_json["battle_turns"] = run.battle_turns;
            run_json["battle_scores"] = run.battle_scores;
            run_json["total_time_minutes"] = run.total_time_minutes;
            run_json["total_healing_events"] = run.total_healing_events;
            run_json["allow_healing_between_battles"] = run.allow_healing_between_battles;
            run_json["sequential_requirement"] = run.sequential_requirement;
            run_json["difficulty_progression"] = run.difficulty_progression;
            
            runs_json[player_name] = run_json;
        }
        championship_data["active_runs"] = runs_json;
        
        // Save championship history
        json history_json;
        for (const auto& [player_name, history] : championship_history) {
            json player_history = json::array();
            for (const auto& result : history) {
                json result_json;
                result_json["player_name"] = result.player_name;
                result_json["opponent_name"] = result.opponent_name;
                result_json["opponent_type"] = result.opponent_type;
                result_json["opponent_position"] = result.opponent_position;
                result_json["victory"] = result.victory;
                result_json["turns_taken"] = result.turns_taken;
                result_json["difficulty_level"] = result.difficulty_level;
                result_json["performance_score"] = result.performance_score;
                result_json["battle_duration"] = result.battle_duration;
                result_json["fainted_pokemon"] = result.fainted_pokemon;
                result_json["low_health_pokemon"] = result.low_health_pokemon;
                result_json["team_needs_healing"] = result.team_needs_healing;
                result_json["key_moments"] = result.key_moments;
                result_json["strategic_notes"] = result.strategic_notes;
                result_json["mvp_pokemon"] = result.mvp_pokemon;
                player_history.push_back(result_json);
            }
            history_json[player_name] = player_history;
        }
        championship_data["championship_history"] = history_json;
        
        // Save player stats
        championship_data["player_championship_stats"] = player_championship_stats;
        
        // Write to file
        const std::string file_path = getChampionshipDataFilePath();
        std::ofstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        file << championship_data.dump(2);
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool ChampionshipSystem::loadChampionshipData() {
    try {
        const std::string file_path = getChampionshipDataFilePath();
        
        if (!fs::exists(file_path)) {
            return true;  // No data to load yet
        }
        
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        json championship_data;
        file >> championship_data;
        
        // Load active runs
        if (championship_data.contains("active_runs")) {
            for (const auto& [player_name, run_json] : championship_data["active_runs"].items()) {
                ChampionshipRun run;
                run.player_name = run_json.value("player_name", "");
                run.player_team_name = run_json.value("player_team_name", "");
                run.start_date = run_json.value("start_date", "");
                run.defeated_opponents = run_json.value("defeated_opponents", std::vector<std::string>());
                run.current_opponent = run_json.value("current_opponent", "");
                run.current_position = run_json.value("current_position", 1);
                run.is_active = run_json.value("is_active", false);
                run.is_completed = run_json.value("is_completed", false);
                run.victory = run_json.value("victory", false);
                run.battle_turns = run_json.value("battle_turns", std::vector<int>());
                run.battle_scores = run_json.value("battle_scores", std::vector<double>());
                run.total_time_minutes = run_json.value("total_time_minutes", 0.0);
                run.total_healing_events = run_json.value("total_healing_events", 0);
                run.allow_healing_between_battles = run_json.value("allow_healing_between_battles", true);
                run.sequential_requirement = run_json.value("sequential_requirement", true);
                run.difficulty_progression = run_json.value("difficulty_progression", "progressive");
                
                active_runs[player_name] = run;
            }
        }
        
        // Load championship history
        if (championship_data.contains("championship_history")) {
            for (const auto& [player_name, history_json] : championship_data["championship_history"].items()) {
                std::vector<ChampionshipBattleResult> history;
                for (const auto& result_json : history_json) {
                    ChampionshipBattleResult result;
                    result.player_name = result_json.value("player_name", "");
                    result.opponent_name = result_json.value("opponent_name", "");
                    result.opponent_type = result_json.value("opponent_type", "");
                    result.opponent_position = result_json.value("opponent_position", 0);
                    result.victory = result_json.value("victory", false);
                    result.turns_taken = result_json.value("turns_taken", 0);
                    result.difficulty_level = result_json.value("difficulty_level", "");
                    result.performance_score = result_json.value("performance_score", 0.0);
                    result.battle_duration = result_json.value("battle_duration", "");
                    result.fainted_pokemon = result_json.value("fainted_pokemon", std::vector<std::string>());
                    result.low_health_pokemon = result_json.value("low_health_pokemon", std::vector<std::string>());
                    result.team_needs_healing = result_json.value("team_needs_healing", false);
                    result.key_moments = result_json.value("key_moments", std::vector<std::string>());
                    result.strategic_notes = result_json.value("strategic_notes", std::vector<std::string>());
                    result.mvp_pokemon = result_json.value("mvp_pokemon", "");
                    history.push_back(result);
                }
                championship_history[player_name] = history;
            }
        }
        
        // Load player stats
        if (championship_data.contains("player_championship_stats")) {
            player_championship_stats = championship_data["player_championship_stats"];
        }
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool ChampionshipSystem::resetPlayerChampionshipProgress(const std::string& player_name, 
                                                       bool confirm_reset) {
    if (!confirm_reset) {
        return false;
    }
    
    // Remove from all data structures
    active_runs.erase(player_name);
    championship_history.erase(player_name);
    player_championship_stats.erase(player_name);
    
    saveChampionshipData();
    return true;
}

bool ChampionshipSystem::validateChampionshipData() const {
    // Validate all active runs
    for (const auto& [player_name, run] : active_runs) {
        if (!isValidRun(run)) {
            return false;
        }
    }
    
    return true;
}

std::unordered_map<std::string, std::string> 
ChampionshipSystem::getChampionshipSystemStatus() const {
    std::unordered_map<std::string, std::string> status;
    
    status["active_runs"] = std::to_string(active_runs.size());
    status["total_players_with_history"] = std::to_string(championship_history.size());
    status["data_valid"] = validateChampionshipData() ? "true" : "false";
    status["data_file_exists"] = fs::exists(getChampionshipDataFilePath()) ? "true" : "false";
    
    // Count champions
    int champion_count = 0;
    for (const auto& [player_name, _] : player_championship_stats) {
        if (isPlayerChampion(player_name)) {
            champion_count++;
        }
    }
    status["total_champions"] = std::to_string(champion_count);
    
    return status;
}

// Private method implementations

void ChampionshipSystem::initializeChampionshipOpponents() {
    // Initialize Elite Four
    elite_four_roster.clear();
    
    ChampionshipOpponent lorelei;
    lorelei.name = "Lorelei";
    lorelei.title = "Elite Four Member";
    lorelei.specialization = "ice";
    lorelei.difficulty_level = "Hard";
    lorelei.team_template = "ice_team";
    lorelei.is_champion = false;
    lorelei.position_in_sequence = 1;
    lorelei.strategy_notes.push_back("Ice-type specialist with strong defensive walls");
    lorelei.strategy_notes.push_back("Weak to Fire, Fighting, Rock, and Steel types");
    elite_four_roster.push_back(lorelei);
    
    ChampionshipOpponent bruno;
    bruno.name = "Bruno";
    bruno.title = "Elite Four Member";
    bruno.specialization = "fighting";
    bruno.difficulty_level = "Hard";
    bruno.team_template = "fighting_team";
    bruno.is_champion = false;
    bruno.position_in_sequence = 2;
    bruno.strategy_notes.push_back("Fighting-type specialist with high physical attack");
    bruno.strategy_notes.push_back("Weak to Flying, Psychic, and Fairy types");
    elite_four_roster.push_back(bruno);
    
    ChampionshipOpponent agatha;
    agatha.name = "Agatha";
    agatha.title = "Elite Four Member";
    agatha.specialization = "ghost";
    agatha.difficulty_level = "Hard";
    agatha.team_template = "dark_team";  // Using dark team as proxy for ghost
    agatha.is_champion = false;
    agatha.position_in_sequence = 3;
    agatha.strategy_notes.push_back("Ghost-type specialist with status effects");
    agatha.strategy_notes.push_back("Immune to Normal and Fighting, weak to Dark and Ghost");
    elite_four_roster.push_back(agatha);
    
    ChampionshipOpponent lance;
    lance.name = "Lance";
    lance.title = "Elite Four Member";
    lance.specialization = "dragon";
    lance.difficulty_level = "Hard";
    lance.team_template = "balanced_meta";  // Using balanced as proxy for dragon team
    lance.is_champion = false;
    lance.position_in_sequence = 4;
    lance.strategy_notes.push_back("Dragon-type specialist with powerful attacks");
    lance.strategy_notes.push_back("Weak to Ice, Dragon, and Fairy types");
    elite_four_roster.push_back(lance);
    
    // Initialize Champion
    champion_opponent.name = "Champion";
    champion_opponent.title = "Pokemon Champion";
    champion_opponent.specialization = "balanced";
    champion_opponent.difficulty_level = "Expert";
    champion_opponent.team_template = "balanced_meta";
    champion_opponent.is_champion = true;
    champion_opponent.position_in_sequence = 5;
    champion_opponent.strategy_notes.push_back("Champion with diverse team composition");
    champion_opponent.strategy_notes.push_back("Uses advanced strategies and type coverage");
}

void ChampionshipSystem::loadChampionshipConfiguration() {
    // Configuration is currently using defaults
    // In future, this could load from config files
}

std::unique_ptr<TeamBuilder::Team> 
ChampionshipSystem::createOpponentTeam(const ChampionshipOpponent& opponent) {
    auto team = std::make_unique<TeamBuilder::Team>();
    
    // Generate team based on opponent's template
    if (opponent.team_template == "ice_team") {
        *team = team_builder->generateTeamFromTemplate("type_themed", "ice_team");
    } else if (opponent.team_template == "fighting_team") {
        *team = team_builder->generateTeamFromTemplate("type_themed", "fighting_team");
    } else if (opponent.team_template == "dark_team") {
        *team = team_builder->generateTeamFromTemplate("type_themed", "dark_team");
    } else {
        *team = team_builder->generateTeamFromTemplate("competitive", "balanced_meta");
    }
    
    // Set team name to opponent name
    team->name = opponent.name + "'s Team";
    
    return team;
}


std::string ChampionshipSystem::calculateBattleDifficulty(int position) const {
    if (!championship_settings.progressive_difficulty) {
        return championship_settings.base_difficulty;
    }
    
    // Progressive difficulty from base to champion level
    if (position <= 2) {
        return championship_settings.base_difficulty;
    } else if (position <= 4) {
        return "Hard";
    } else {
        return championship_settings.champion_difficulty;
    }
}

bool ChampionshipSystem::advanceRunPosition(ChampionshipRun& run) {
    run.current_position++;
    
    if (run.current_position > 5) {
        // Championship completed
        return false;
    }
    
    // Update current opponent
    const auto next_opponent = getNextOpponent(run.player_name);
    if (next_opponent) {
        run.current_opponent = next_opponent->name;
    }
    
    return true;
}

void ChampionshipSystem::updateRunStatistics(ChampionshipRun& run, 
                                            const ChampionshipBattleResult& battle_result) {
    run.battle_turns.push_back(battle_result.turns_taken);
    run.battle_scores.push_back(battle_result.performance_score);
    
    // Update total time (simplified - would need actual battle timing)
    run.total_time_minutes += static_cast<double>(battle_result.turns_taken) * 0.5;
    
    if (battle_result.team_needs_healing) {
        run.total_healing_events++;
    }
}

bool ChampionshipSystem::validateRunProgression(const ChampionshipRun& run) const {
    // Validate that run progression makes sense
    if (run.current_position < 1 || run.current_position > 5) {
        return false;
    }
    
    if (run.defeated_opponents.size() >= static_cast<size_t>(run.current_position)) {
        return false;  // Can't have defeated more opponents than current position allows
    }
    
    return true;
}

double ChampionshipSystem::calculateChampionshipScore(const std::string& player_name) const {
    const auto stats_it = player_championship_stats.find(player_name);
    if (stats_it == player_championship_stats.end()) {
        return 0.0;
    }
    
    const auto& stats = stats_it->second;
    double score = 0.0;
    
    // Base score for victories
    const auto victories_it = stats.find("victories");
    if (victories_it != stats.end()) {
        score += victories_it->second * 1000.0;
    }
    
    // Time bonus
    const auto time_it = stats.find("best_completion_time");
    if (time_it != stats.end() && time_it->second > 0.0) {
        const double time_bonus = std::max(0.0, 100.0 - time_it->second);
        score += time_bonus * championship_settings.time_bonus_multiplier;
    }
    
    // Performance bonus
    const auto performance_it = stats.find("average_performance_score");
    if (performance_it != stats.end()) {
        score += performance_it->second * 2.0;
    }
    
    return score;
}

double ChampionshipSystem::calculateCompletionTime(const ChampionshipRun& run) const {
    return run.total_time_minutes;
}

void ChampionshipSystem::updatePlayerChampionshipStats(const std::string& player_name) {
    auto& stats = player_championship_stats[player_name];
    
    // Count total attempts
    const auto run_it = active_runs.find(player_name);
    if (run_it != active_runs.end()) {
        stats["total_attempts"] = stats["total_attempts"] + 1.0;
        
        if (run_it->second.victory) {
            stats["victories"] = stats["victories"] + 1.0;
            
            // Update best completion time
            const double completion_time = calculateCompletionTime(run_it->second);
            const auto current_best = stats.find("best_completion_time");
            if (current_best == stats.end() || completion_time < current_best->second) {
                stats["best_completion_time"] = completion_time;
            }
        } else {
            stats["defeats"] = stats["defeats"] + 1.0;
        }
    }
    
    // Calculate average performance score
    const auto history_it = championship_history.find(player_name);
    if (history_it != championship_history.end() && !history_it->second.empty()) {
        double total_score = 0.0;
        for (const auto& result : history_it->second) {
            total_score += result.performance_score;
        }
        stats["average_performance_score"] = total_score / static_cast<double>(history_it->second.size());
    }
}

std::string ChampionshipSystem::getChampionshipDataFilePath() const {
    return "data/tournaments/championship_data.json";
}

bool ChampionshipSystem::ensureChampionshipDataDirectoryExists() const {
    try {
        const std::string dir_path = "data/tournaments";
        return fs::create_directories(dir_path);
    } catch (const std::exception&) {
        return false;
    }
}

bool ChampionshipSystem::isValidRun(const ChampionshipRun& run) const {
    return validateRunProgression(run) && !run.player_name.empty() && 
           !run.player_team_name.empty();
}

bool ChampionshipSystem::canPlayerStartRun(const std::string& player_name) const {
    return isChampionshipEligible(player_name) && 
           active_runs.find(player_name) == active_runs.end();
}

bool ChampionshipSystem::meetsSequentialRequirements(const std::string& player_name, 
                                                    int position) const {
    if (!championship_settings.require_sequential_battles) {
        return true;
    }
    
    const auto run = getCurrentRun(player_name);
    if (!run) {
        return false;
    }
    
    return static_cast<int>(run->defeated_opponents.size()) >= (position - 1);
}

std::string ChampionshipSystem::getCurrentTimestamp() const {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string ChampionshipSystem::generateRunId(const std::string& player_name) const {
    return player_name + "_" + getCurrentTimestamp();
}

std::string ChampionshipSystem::formatBattleDuration(double minutes) const {
    const int total_minutes = static_cast<int>(minutes);
    const int hours = total_minutes / 60;
    const int mins = total_minutes % 60;
    
    if (hours > 0) {
        return std::to_string(hours) + "h " + std::to_string(mins) + "m";
    } else {
        return std::to_string(mins) + "m";
    }
}