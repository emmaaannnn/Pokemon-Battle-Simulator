#include "team_builder.h"
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <set>
#include <filesystem>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <map>

using json = nlohmann::json;

TeamBuilder::TeamBuilder(std::shared_ptr<PokemonData> data) 
    : pokemon_data(data), validation_settings(), templates_loaded(false) {
    if (!pokemon_data) {
        throw std::invalid_argument("PokemonData cannot be null");
    }
    // Load templates on construction
    loadTemplates();
}

TeamBuilder::Team TeamBuilder::createTeam(const std::string& team_name) {
    // Validate team name
    auto name_validation = InputValidator::validateString(team_name, 1, 50, false);
    if (!name_validation.isValid()) {
        return Team("Invalid_Team_Name");
    }
    
    if (!isValidTeamName(team_name)) {
        return Team("Invalid_Team_Name");
    }
    
    return Team(normalizeTeamName(team_name));
}

bool TeamBuilder::addPokemonToTeam(Team& team, const std::string& pokemon_name, 
                                  const std::vector<std::string>& moves) {
    // Validate Pokemon name
    if (!InputValidator::isValidPokemonName(pokemon_name)) {
        team.validation_errors.push_back("Invalid Pokemon name: " + pokemon_name);
        return false;
    }
    
    if (!pokemon_data->hasPokemon(pokemon_name)) {
        team.validation_errors.push_back("Pokemon not found: " + pokemon_name);
        return false;
    }
    
    // Check team size limit
    if (validation_settings.enforce_max_team_size && team.isFull()) {
        team.validation_errors.push_back("Team is full (maximum 6 Pokemon)");
        return false;
    }
    
    // Check for duplicate Pokemon if not allowed
    if (!validation_settings.allow_duplicate_pokemon) {
        for (const auto& existing_pokemon : team.pokemon) {
            if (existing_pokemon.name == pokemon_name) {
                team.validation_errors.push_back("Duplicate Pokemon not allowed: " + pokemon_name);
                return false;
            }
        }
    }
    
    // Validate moves
    if (validation_settings.enforce_max_moves && moves.size() > 4) {
        team.validation_errors.push_back("Too many moves for " + pokemon_name + " (maximum 4)");
        return false;
    }
    
    if (validation_settings.enforce_min_moves && 
        moves.size() < static_cast<size_t>(validation_settings.min_moves_per_pokemon)) {
        team.validation_errors.push_back("Too few moves for " + pokemon_name + 
                                        " (minimum " + std::to_string(validation_settings.min_moves_per_pokemon) + ")");
        return false;
    }
    
    // Validate each move
    std::vector<std::string> validated_moves;
    for (const auto& move_name : moves) {
        if (!InputValidator::isValidMoveName(move_name)) {
            team.validation_errors.push_back("Invalid move name: " + move_name);
            continue;
        }
        
        if (!pokemon_data->hasMove(move_name)) {
            team.validation_errors.push_back("Move not found: " + move_name);
            continue;
        }
        
        validated_moves.push_back(move_name);
    }
    
    // Check if we have any valid moves
    if (validated_moves.empty() && !moves.empty()) {
        team.validation_errors.push_back("No valid moves found for " + pokemon_name);
        return false;
    }
    
    // Add the Pokemon to the team
    team.pokemon.emplace_back(pokemon_name, validated_moves);
    
    // Clear previous validation state since team changed
    team.is_valid = false;
    
    return true;
}

bool TeamBuilder::removePokemonFromTeam(Team& team, size_t pokemon_index) {
    if (pokemon_index >= team.pokemon.size()) {
        team.validation_errors.push_back("Invalid Pokemon index: " + std::to_string(pokemon_index));
        return false;
    }
    
    team.pokemon.erase(team.pokemon.begin() + pokemon_index);
    team.is_valid = false; // Re-validation needed
    
    return true;
}

bool TeamBuilder::modifyPokemonMoves(Team& team, size_t pokemon_index, 
                                    const std::vector<std::string>& new_moves) {
    if (pokemon_index >= team.pokemon.size()) {
        team.validation_errors.push_back("Invalid Pokemon index: " + std::to_string(pokemon_index));
        return false;
    }
    
    // Validate moves
    if (validation_settings.enforce_max_moves && new_moves.size() > 4) {
        team.validation_errors.push_back("Too many moves (maximum 4)");
        return false;
    }
    
    if (validation_settings.enforce_min_moves && 
        new_moves.size() < static_cast<size_t>(validation_settings.min_moves_per_pokemon)) {
        team.validation_errors.push_back("Too few moves (minimum " + 
                                        std::to_string(validation_settings.min_moves_per_pokemon) + ")");
        return false;
    }
    
    // Validate each move
    std::vector<std::string> validated_moves;
    for (const auto& move_name : new_moves) {
        if (!InputValidator::isValidMoveName(move_name)) {
            team.validation_errors.push_back("Invalid move name: " + move_name);
            continue;
        }
        
        if (!pokemon_data->hasMove(move_name)) {
            team.validation_errors.push_back("Move not found: " + move_name);
            continue;
        }
        
        validated_moves.push_back(move_name);
    }
    
    team.pokemon[pokemon_index].moves = validated_moves;
    team.is_valid = false; // Re-validation needed
    
    return true;
}

bool TeamBuilder::validateTeam(Team& team, const ValidationSettings& settings) {
    // Clear previous validation results
    team.validation_errors.clear();
    team.validation_warnings.clear();
    team.is_valid = false;
    
    // Store current settings for this validation
    ValidationSettings old_settings = validation_settings;
    validation_settings = settings;
    
    bool is_valid = true;
    
    // Validate team size
    is_valid &= validateTeamSize(team, team.validation_errors, team.validation_warnings);
    
    // Validate Pokemon and moves
    is_valid &= validatePokemonMoves(team, team.validation_errors, team.validation_warnings);
    
    // Validate type diversity
    is_valid &= validateTypeDiversity(team, team.validation_errors, team.validation_warnings);
    
    // Validate duplicates
    is_valid &= validateDuplicates(team, team.validation_errors, team.validation_warnings);
    
    // Restore original settings
    validation_settings = old_settings;
    
    team.is_valid = is_valid;
    return is_valid;
}

bool TeamBuilder::validateTeamSize(const Team& team, std::vector<std::string>& errors, 
                                  std::vector<std::string>& warnings) const {
    bool is_valid = true;
    
    if (validation_settings.enforce_min_team_size && 
        team.size() < static_cast<size_t>(validation_settings.min_team_size)) {
        errors.push_back("Team too small (minimum " + std::to_string(validation_settings.min_team_size) + " Pokemon)");
        is_valid = false;
    }
    
    if (validation_settings.enforce_max_team_size && team.size() > 6) {
        errors.push_back("Team too large (maximum 6 Pokemon)");
        is_valid = false;
    }
    
    if (team.size() < 3) {
        warnings.push_back("Small team may lack type coverage");
    }
    
    return is_valid;
}

bool TeamBuilder::validatePokemonMoves(const Team& team, std::vector<std::string>& errors, 
                                      std::vector<std::string>& warnings) const {
    bool is_valid = true;
    
    for (size_t i = 0; i < team.pokemon.size(); ++i) {
        const auto& pokemon = team.pokemon[i];
        
        // Validate Pokemon exists
        if (!pokemon_data->hasPokemon(pokemon.name)) {
            errors.push_back("Pokemon not found: " + pokemon.name);
            is_valid = false;
            continue;
        }
        
        // Validate move count
        if (validation_settings.enforce_min_moves && 
            pokemon.moves.size() < static_cast<size_t>(validation_settings.min_moves_per_pokemon)) {
            errors.push_back(pokemon.name + " has too few moves (minimum " + 
                           std::to_string(validation_settings.min_moves_per_pokemon) + ")");
            is_valid = false;
        }
        
        if (validation_settings.enforce_max_moves && pokemon.moves.size() > 4) {
            errors.push_back(pokemon.name + " has too many moves (maximum 4)");
            is_valid = false;
        }
        
        // Validate each move
        for (const auto& move : pokemon.moves) {
            if (!pokemon_data->hasMove(move)) {
                errors.push_back("Move not found: " + move + " on " + pokemon.name);
                is_valid = false;
            }
        }
        
        // Check for move diversity
        if (pokemon.moves.size() >= 2) {
            std::set<std::string> unique_moves(pokemon.moves.begin(), pokemon.moves.end());
            if (unique_moves.size() != pokemon.moves.size()) {
                warnings.push_back(pokemon.name + " has duplicate moves");
            }
        }
    }
    
    return is_valid;
}

bool TeamBuilder::validateTypeDiversity(const Team& team, std::vector<std::string>& errors, 
                                       std::vector<std::string>& warnings) const {
    if (!validation_settings.require_type_diversity) {
        return true;
    }
    
    bool is_valid = true;
    auto unique_types = getTeamTypes(team);
    
    if (unique_types.size() < static_cast<size_t>(validation_settings.min_unique_types)) {
        errors.push_back("Insufficient type diversity (minimum " + 
                        std::to_string(validation_settings.min_unique_types) + " unique types)");
        is_valid = false;
    }
    
    // Check for type balance
    auto type_counts = getTypeCounts(team);
    for (const auto& [type, count] : type_counts) {
        if (count > static_cast<int>(team.size() / 2)) {
            warnings.push_back("Team heavily weighted toward " + type + " type");
        }
    }
    
    return is_valid;
}

bool TeamBuilder::validateDuplicates(const Team& team, std::vector<std::string>& errors, 
                                     std::vector<std::string>& /* warnings */) const {
    bool is_valid = true;
    
    if (!validation_settings.allow_duplicate_pokemon) {
        std::set<std::string> seen_pokemon;
        for (const auto& pokemon : team.pokemon) {
            if (seen_pokemon.count(pokemon.name)) {
                errors.push_back("Duplicate Pokemon: " + pokemon.name);
                is_valid = false;
            }
            seen_pokemon.insert(pokemon.name);
        }
    }
    
    return is_valid;
}

TeamBuilder::TeamAnalysis TeamBuilder::analyzeTeam(const Team& team) const {
    TeamAnalysis analysis;
    
    if (team.isEmpty()) {
        return analysis;
    }
    
    // Analyze types
    analysis.offensive_types = getTeamTypes(team);
    
    // Analyze moves
    for (const auto& pokemon : team.pokemon) {
        for (const auto& move_name : pokemon.moves) {
            auto move_info = pokemon_data->getMoveInfo(move_name);
            if (move_info.has_value()) {
                if (move_info->damage_class == "physical") {
                    analysis.physical_moves++;
                } else if (move_info->damage_class == "special") {
                    analysis.special_moves++;
                } else {
                    analysis.status_moves++;
                }
            }
        }
    }
    
    // Calculate average stats
    double total_hp = 0, total_attack = 0, total_defense = 0;
    double total_sp_attack = 0, total_sp_defense = 0, total_speed = 0;
    
    for (const auto& pokemon : team.pokemon) {
        auto pokemon_info = pokemon_data->getPokemonInfo(pokemon.name);
        if (pokemon_info.has_value()) {
            total_hp += pokemon_info->hp;
            total_attack += pokemon_info->attack;
            total_defense += pokemon_info->defense;
            total_sp_attack += pokemon_info->special_attack;
            total_sp_defense += pokemon_info->special_defense;
            total_speed += pokemon_info->speed;
        }
    }
    
    double team_size = static_cast<double>(team.size());
    analysis.average_hp = total_hp / team_size;
    analysis.average_attack = total_attack / team_size;
    analysis.average_defense = total_defense / team_size;
    analysis.average_special_attack = total_sp_attack / team_size;
    analysis.average_special_defense = total_sp_defense / team_size;
    analysis.average_speed = total_speed / team_size;
    
    // Calculate balance score
    analysis.balance_score = calculateBalanceScore(team);
    
    // Generate suggestions
    analysis.suggested_pokemon = suggestPokemonForTeam(team, 3);
    analysis.coverage_gaps = getTeamSuggestions(team);
    
    return analysis;
}

std::vector<std::string> TeamBuilder::getTeamTypes(const Team& team) const {
    std::set<std::string> unique_types;
    
    for (const auto& pokemon : team.pokemon) {
        auto pokemon_info = pokemon_data->getPokemonInfo(pokemon.name);
        if (pokemon_info.has_value()) {
            for (const auto& type : pokemon_info->types) {
                unique_types.insert(type);
            }
        }
    }
    
    return std::vector<std::string>(unique_types.begin(), unique_types.end());
}

std::unordered_map<std::string, int> TeamBuilder::getTypeCounts(const Team& team) const {
    std::unordered_map<std::string, int> type_counts;
    
    for (const auto& pokemon : team.pokemon) {
        auto pokemon_info = pokemon_data->getPokemonInfo(pokemon.name);
        if (pokemon_info.has_value()) {
            for (const auto& type : pokemon_info->types) {
                type_counts[type]++;
            }
        }
    }
    
    return type_counts;
}

int TeamBuilder::calculateBalanceScore(const Team& team) const {
    if (team.isEmpty()) {
        return 0;
    }
    
    int score = 50; // Base score
    
    // Type diversity bonus
    auto unique_types = getTeamTypes(team);
    score += static_cast<int>(unique_types.size()) * 5;
    
    // Move diversity bonus
    std::set<std::string> unique_moves;
    for (const auto& pokemon : team.pokemon) {
        for (const auto& move : pokemon.moves) {
            unique_moves.insert(move);
        }
    }
    score += static_cast<int>(unique_moves.size()) * 2;
    
    // Stat balance bonus
    TeamAnalysis analysis;
    double total_hp = 0, total_attack = 0, total_defense = 0;
    double total_sp_attack = 0, total_sp_defense = 0, total_speed = 0;
    
    for (const auto& pokemon : team.pokemon) {
        auto pokemon_info = pokemon_data->getPokemonInfo(pokemon.name);
        if (pokemon_info.has_value()) {
            total_hp += pokemon_info->hp;
            total_attack += pokemon_info->attack;
            total_defense += pokemon_info->defense;
            total_sp_attack += pokemon_info->special_attack;
            total_sp_defense += pokemon_info->special_defense;
            total_speed += pokemon_info->speed;
        }
    }
    
    // Penalize extreme stat imbalances
    std::vector<double> averages = {
        total_hp / team.size(),
        total_attack / team.size(),
        total_defense / team.size(),
        total_sp_attack / team.size(),
        total_sp_defense / team.size(),
        total_speed / team.size()
    };
    
    double min_avg = *std::min_element(averages.begin(), averages.end());
    double max_avg = *std::max_element(averages.begin(), averages.end());
    
    if (max_avg > 0) {
        double balance_ratio = min_avg / max_avg;
        score += static_cast<int>(balance_ratio * 20);
    }
    
    return std::min(100, std::max(0, score));
}

TeamBuilder::Team TeamBuilder::generateRandomTeam(const std::string& team_name, int team_size) {
    auto team = createTeam(team_name);
    team_size = std::min(6, std::max(1, team_size));
    
    auto available_pokemon = pokemon_data->getAvailablePokemon();
    if (available_pokemon.empty()) {
        return team;
    }
    
    auto selected_pokemon = selectRandomPokemon(team_size, available_pokemon);
    
    // Temporarily disable validation for random team generation
    auto original_settings = validation_settings;
    validation_settings.enforce_min_moves = false;
    validation_settings.enforce_min_team_size = false;
    
    for (const auto& pokemon_name : selected_pokemon) {
        auto moves = generateMovesForPokemon(pokemon_name);
        addPokemonToTeam(team, pokemon_name, moves);
    }
    
    // Restore validation settings
    validation_settings = original_settings;
    
    validateTeam(team);
    return team;
}

TeamBuilder::Team TeamBuilder::generateBalancedTeam(const std::string& team_name, int team_size) {
    auto team = createTeam(team_name);
    team_size = std::min(6, std::max(1, team_size));
    
    // Try to get diverse types
    std::vector<std::string> preferred_types = {"fire", "water", "grass", "electric", "psychic", "dragon"};
    std::set<std::string> used_types;
    
    // Temporarily disable validation for balanced team generation
    auto original_settings = validation_settings;
    validation_settings.enforce_min_moves = false;
    validation_settings.enforce_min_team_size = false;
    
    for (int i = 0; i < team_size && i < static_cast<int>(preferred_types.size()); ++i) {
        auto type_pokemon = pokemon_data->getPokemonByType(preferred_types[i]);
        if (!type_pokemon.empty()) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, type_pokemon.size() - 1);
            
            std::string selected = type_pokemon[dis(gen)];
            auto moves = generateMovesForPokemon(selected);
            addPokemonToTeam(team, selected, moves);
            used_types.insert(preferred_types[i]);
        }
    }
    
    // Fill remaining slots with random Pokemon
    while (team.size() < static_cast<size_t>(team_size)) {
        auto available_pokemon = pokemon_data->getAvailablePokemon();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, available_pokemon.size() - 1);
        
        std::string selected = available_pokemon[dis(gen)];
        
        // Check if we already have this Pokemon
        bool already_has = false;
        for (const auto& existing : team.pokemon) {
            if (existing.name == selected) {
                already_has = true;
                break;
            }
        }
        
        if (!already_has) {
            auto moves = generateMovesForPokemon(selected);
            addPokemonToTeam(team, selected, moves);
        }
    }
    
    // Restore validation settings
    validation_settings = original_settings;
    
    validateTeam(team);
    return team;
}

std::vector<std::string> TeamBuilder::selectRandomPokemon(int count, 
                                                         const std::vector<std::string>& available) const {
    std::vector<std::string> selected;
    std::vector<std::string> pool = available;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int i = 0; i < count && !pool.empty(); ++i) {
        std::uniform_int_distribution<> dis(0, pool.size() - 1);
        int index = dis(gen);
        
        selected.push_back(pool[index]);
        pool.erase(pool.begin() + index);
    }
    
    return selected;
}

std::vector<std::string> TeamBuilder::generateMovesForPokemon(const std::string& pokemon_name) const {
    auto suggested_moves = pokemon_data->suggestMovesForPokemon(pokemon_name, 4);
    
    // If we don't have enough moves, fill with random moves
    if (suggested_moves.size() < 4) {
        auto all_moves = pokemon_data->getAvailableMoves();
        std::random_device rd;
        std::mt19937 gen(rd());
        
        while (suggested_moves.size() < 4 && suggested_moves.size() < all_moves.size()) {
            std::uniform_int_distribution<> dis(0, all_moves.size() - 1);
            std::string random_move = all_moves[dis(gen)];
            
            // Check if we already have this move
            if (std::find(suggested_moves.begin(), suggested_moves.end(), random_move) == suggested_moves.end()) {
                suggested_moves.push_back(random_move);
            }
        }
    }
    
    return suggested_moves;
}

std::pair<
    std::unordered_map<std::string, std::vector<std::string>>,
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>>
> TeamBuilder::exportTeamForBattle(const Team& team) const {
    std::unordered_map<std::string, std::vector<std::string>> selected_teams;
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>> selected_moves;
    
    // Build pokemon list
    std::vector<std::string> pokemon_list;
    for (const auto& pokemon : team.pokemon) {
        pokemon_list.push_back(pokemon.name);
    }
    selected_teams[team.name] = pokemon_list;
    
    // Build moves list
    std::vector<std::pair<std::string, std::vector<std::string>>> moves_list;
    for (const auto& pokemon : team.pokemon) {
        moves_list.emplace_back(pokemon.name, pokemon.moves);
    }
    selected_moves[team.name] = moves_list;
    
    return std::make_pair(selected_teams, selected_moves);
}

TeamBuilder::Team TeamBuilder::importTeamFromBattle(
    const std::string& team_name,
    const std::unordered_map<std::string, std::vector<std::string>>& selected_teams,
    const std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>>& selected_moves
) {
    auto team = createTeam(team_name);
    
    // Find the team in selected_teams
    auto team_it = selected_teams.find(team_name);
    if (team_it == selected_teams.end()) {
        team.validation_errors.push_back("Team not found in selected_teams: " + team_name);
        return team;
    }
    
    const auto& pokemon_list = team_it->second;
    
    // Find moves for this team
    auto moves_it = selected_moves.find(team_name);
    std::unordered_map<std::string, std::vector<std::string>> pokemon_moves;
    
    if (moves_it != selected_moves.end()) {
        for (const auto& move_pair : moves_it->second) {
            pokemon_moves[move_pair.first] = move_pair.second;
        }
    }
    
    // Add each Pokemon to the team
    for (const auto& pokemon_name : pokemon_list) {
        std::vector<std::string> moves;
        auto moves_iter = pokemon_moves.find(pokemon_name);
        if (moves_iter != pokemon_moves.end()) {
            moves = moves_iter->second;
        }
        
        addPokemonToTeam(team, pokemon_name, moves);
    }
    
    validateTeam(team);
    return team;
}

bool TeamBuilder::saveTeamToFile(const Team& team, const std::string& file_path) const {
    // Validate file path
    auto path_validation = InputValidator::validateDataFilePath(
        std::filesystem::path(file_path).filename().string(), 
        "teams", 
        ".json"
    );
    
    if (!path_validation.isValid()) {
        return false;
    }
    
    try {
        json team_json;
        team_json["name"] = team.name;
        team_json["is_valid"] = team.is_valid;
        team_json["validation_errors"] = team.validation_errors;
        team_json["validation_warnings"] = team.validation_warnings;
        
        json pokemon_array = json::array();
        for (const auto& pokemon : team.pokemon) {
            json pokemon_json;
            pokemon_json["name"] = pokemon.name;
            pokemon_json["moves"] = pokemon.moves;
            pokemon_array.push_back(pokemon_json);
        }
        team_json["pokemon"] = pokemon_array;
        
        std::ofstream file(path_validation.value);
        file << team_json.dump(2);
        
        return file.good();
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving team to file: " << e.what() << std::endl;
        return false;
    }
}

TeamBuilder::Team TeamBuilder::loadTeamFromFile(const std::string& file_path) {
    // Validate file path
    auto path_validation = InputValidator::validateDataFilePath(
        std::filesystem::path(file_path).filename().string(), 
        "teams", 
        ".json"
    );
    
    if (!path_validation.isValid()) {
        return Team("Invalid_File");
    }
    
    // Validate file accessibility
    auto file_validation = InputValidator::validateFileAccessibility(path_validation.value);
    if (!file_validation.isValid()) {
        return Team("Inaccessible_File");
    }
    
    try {
        std::ifstream file(path_validation.value);
        json team_json;
        file >> team_json;
        
        auto name_result = InputValidator::getJsonString(team_json, "name", 1, 50);
        if (!name_result.isValid()) {
            return Team("Invalid_Name");
        }
        
        Team team = createTeam(name_result.value);
        
        if (team_json.contains("pokemon") && team_json["pokemon"].is_array()) {
            for (const auto& pokemon_json : team_json["pokemon"]) {
                auto pokemon_name_result = InputValidator::getJsonString(pokemon_json, "name", 1, 50);
                if (!pokemon_name_result.isValid()) {
                    continue;
                }
                
                std::vector<std::string> moves;
                if (pokemon_json.contains("moves") && pokemon_json["moves"].is_array()) {
                    for (const auto& move : pokemon_json["moves"]) {
                        if (move.is_string()) {
                            moves.push_back(move.get<std::string>());
                        }
                    }
                }
                
                addPokemonToTeam(team, pokemon_name_result.value, moves);
            }
        }
        
        validateTeam(team);
        return team;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading team from file: " << e.what() << std::endl;
        return Team("Load_Error");
    }
}

std::vector<std::string> TeamBuilder::suggestPokemonForTeam(const Team& team, int count) const {
    std::vector<std::string> suggestions;
    
    if (team.size() >= 6) {
        return suggestions; // Team is full
    }
    
    // Get current team types
    auto current_types = getTeamTypes(team);
    std::set<std::string> type_set(current_types.begin(), current_types.end());
    
    // Get all available Pokemon
    auto all_pokemon = pokemon_data->getAvailablePokemon();
    
    // Score Pokemon based on type diversity and team composition
    std::vector<std::pair<std::string, int>> pokemon_scores;
    
    for (const auto& pokemon_name : all_pokemon) {
        // Skip if already in team (unless duplicates allowed)
        bool already_in_team = false;
        if (!validation_settings.allow_duplicate_pokemon) {
            for (const auto& team_pokemon : team.pokemon) {
                if (team_pokemon.name == pokemon_name) {
                    already_in_team = true;
                    break;
                }
            }
        }
        
        if (already_in_team) {
            continue;
        }
        
        auto pokemon_info = pokemon_data->getPokemonInfo(pokemon_name);
        if (!pokemon_info.has_value()) {
            continue;
        }
        
        int score = 0;
        
        // Bonus for new types
        for (const auto& type : pokemon_info->types) {
            if (type_set.find(type) == type_set.end()) {
                score += 10; // Bonus for new type
            }
        }
        
        // Bonus for balanced stats
        int total_stats = pokemon_info->hp + pokemon_info->attack + pokemon_info->defense +
                         pokemon_info->special_attack + pokemon_info->special_defense + pokemon_info->speed;
        if (total_stats > 450) { // Good stat total
            score += 5;
        }
        
        // Small random factor for variety
        score += rand() % 3;
        
        pokemon_scores.emplace_back(pokemon_name, score);
    }
    
    // Sort by score (descending)
    std::sort(pokemon_scores.begin(), pokemon_scores.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Take top suggestions
    for (int i = 0; i < count && i < static_cast<int>(pokemon_scores.size()); ++i) {
        suggestions.push_back(pokemon_scores[i].first);
    }
    
    return suggestions;
}

std::vector<std::string> TeamBuilder::suggestMovesForTeamPokemon(const Team& team, 
                                                               size_t pokemon_index, int count) const {
    std::vector<std::string> suggestions;
    
    if (pokemon_index >= team.pokemon.size()) {
        return suggestions;
    }
    
    const auto& pokemon = team.pokemon[pokemon_index];
    
    // Get moves from PokemonData suggestions
    auto base_suggestions = pokemon_data->suggestMovesForPokemon(pokemon.name, count);
    
    // Filter out moves already known by this Pokemon
    std::set<std::string> current_moves(pokemon.moves.begin(), pokemon.moves.end());
    
    for (const auto& move : base_suggestions) {
        if (current_moves.find(move) == current_moves.end()) {
            suggestions.push_back(move);
        }
    }
    
    // If we need more suggestions, add some popular moves
    if (suggestions.size() < static_cast<size_t>(count)) {
        auto all_moves = pokemon_data->getAvailableMoves();
        
        for (const auto& move : all_moves) {
            if (suggestions.size() >= static_cast<size_t>(count)) break;
            
            if (current_moves.find(move) == current_moves.end() &&
                std::find(suggestions.begin(), suggestions.end(), move) == suggestions.end()) {
                auto move_info = pokemon_data->getMoveInfo(move);
                if (move_info.has_value() && move_info->power > 0) {
                    suggestions.push_back(move);
                }
            }
        }
    }
    
    return suggestions;
}

std::vector<std::string> TeamBuilder::getTeamSuggestions(const Team& team) const {
    std::vector<std::string> suggestions;
    
    if (team.isEmpty()) {
        suggestions.push_back("Add Pokemon to your team");
        return suggestions;
    }
    
    // Check type coverage
    auto team_types = getTeamTypes(team);
    if (team_types.size() < 3) {
        suggestions.push_back("Consider adding more type diversity");
    }
    
    // Check for common weaknesses
    std::unordered_map<std::string, int> weakness_count;
    for (const auto& pokemon : team.pokemon) {
        auto pokemon_info = pokemon_data->getPokemonInfo(pokemon.name);
        if (pokemon_info.has_value()) {
            auto weak_types = getWeakTypes(pokemon_info->types);
            for (const auto& weakness : weak_types) {
                weakness_count[weakness]++;
            }
        }
    }
    
    for (const auto& [type, count] : weakness_count) {
        if (count > static_cast<int>(team.size()) / 2) {
            suggestions.push_back("Team is weak to " + type + " type moves");
        }
    }
    
    // Check move diversity
    std::unordered_map<std::string, int> move_type_count;
    for (const auto& pokemon : team.pokemon) {
        for (const auto& move_name : pokemon.moves) {
            auto move_info = pokemon_data->getMoveInfo(move_name);
            if (move_info.has_value()) {
                move_type_count[move_info->type]++;
            }
        }
    }
    
    if (move_type_count.size() < team_types.size()) {
        suggestions.push_back("Consider moves that match your Pokemon types for STAB bonus");
    }
    
    return suggestions;
}

std::unordered_map<std::string, double> TeamBuilder::calculateTypeCoverage(const Team& team) const {
    std::unordered_map<std::string, double> coverage;
    
    // Get all possible target types
    std::vector<std::string> all_types = {"normal", "fire", "water", "electric", "grass", "ice",
                                         "fighting", "poison", "ground", "flying", "psychic",
                                         "bug", "rock", "ghost", "dragon", "dark", "steel", "fairy"};
    
    // Initialize coverage map
    for (const auto& type : all_types) {
        coverage[type] = 1.0; // Neutral effectiveness by default
    }
    
    // Calculate best effectiveness against each type from team's moves
    for (const auto& pokemon : team.pokemon) {
        for (const auto& move_name : pokemon.moves) {
            auto move_info = pokemon_data->getMoveInfo(move_name);
            if (move_info.has_value()) {
                for (const auto& target_type : all_types) {
                    double effectiveness = pokemon_data->getTypeEffectiveness(
                        move_info->type, {target_type}
                    );
                    // Keep the best effectiveness for each target type
                    coverage[target_type] = std::max(coverage[target_type], effectiveness);
                }
            }
        }
    }
    
    return coverage;
}

std::string TeamBuilder::normalizeTeamName(const std::string& name) const {
    return InputValidator::sanitizeString(name);
}

bool TeamBuilder::isValidTeamName(const std::string& name) const {
    return InputValidator::isAlphanumericSafe(name);
}

std::unordered_set<std::string> TeamBuilder::getWeakTypes(const std::vector<std::string>& pokemon_types) const {
    // Simplified weakness calculation - in reality this would be more complex
    std::unordered_set<std::string> weaknesses;
    
    for (const auto& type : pokemon_types) {
        if (type == "fire") {
            weaknesses.insert("water");
            weaknesses.insert("ground");
            weaknesses.insert("rock");
        } else if (type == "water") {
            weaknesses.insert("grass");
            weaknesses.insert("electric");
        } else if (type == "grass") {
            weaknesses.insert("fire");
            weaknesses.insert("ice");
            weaknesses.insert("poison");
            weaknesses.insert("flying");
            weaknesses.insert("bug");
        }
        // Add more type relationships as needed
    }
    
    return weaknesses;
}

// ========== TEMPLATE SYSTEM IMPLEMENTATION ==========

bool TeamBuilder::loadTemplates() {
    try {
        templates.clear();
        templates_loaded = false;
        
        const std::string template_base_path = "data/team_templates/";
        
        // Check if templates directory exists
        if (!std::filesystem::exists(template_base_path)) {
            return false;
        }
        
        // Load templates from each category directory
        std::vector<std::string> categories = {"starter_teams", "type_themed", "competitive"};
        
        for (const auto& category : categories) {
            std::string category_path = template_base_path + category + "/";
            
            if (!std::filesystem::exists(category_path)) {
                continue;
            }
            
            for (const auto& entry : std::filesystem::directory_iterator(category_path)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    std::string file_path = entry.path().string();
                    
                    if (isValidTemplateFile(file_path)) {
                        TeamTemplate template_data = parseTemplateFromJson(file_path);
                        if (!template_data.name.empty()) {
                            std::string template_name = getTemplateNameFromFile(file_path);
                            templates[category][template_name] = template_data;
                        }
                    }
                }
            }
        }
        
        templates_loaded = true;
        return true;
    } catch (const std::exception& e) {
        templates_loaded = false;
        return false;
    }
}

std::vector<std::string> TeamBuilder::getTemplateCategories() const {
    std::vector<std::string> categories;
    for (const auto& category_pair : templates) {
        categories.push_back(category_pair.first);
    }
    std::sort(categories.begin(), categories.end());
    return categories;
}

std::vector<std::string> TeamBuilder::getTemplatesInCategory(const std::string& category) const {
    // Validate category input
    auto category_validation = InputValidator::validateString(category, 1, 50, false);
    if (!category_validation.isValid()) {
        return {};
    }
    
    std::vector<std::string> template_names;
    auto category_it = templates.find(category);
    if (category_it != templates.end()) {
        for (const auto& template_pair : category_it->second) {
            template_names.push_back(template_pair.first);
        }
        std::sort(template_names.begin(), template_names.end());
    }
    return template_names;
}

std::optional<TeamBuilder::TeamTemplate> TeamBuilder::getTemplate(const std::string& category, 
                                                                  const std::string& template_name) const {
    // Validate inputs
    auto category_validation = InputValidator::validateString(category, 1, 50, false);
    auto name_validation = InputValidator::validateString(template_name, 1, 50, false);
    
    if (!category_validation.isValid() || !name_validation.isValid()) {
        return std::nullopt;
    }
    
    auto category_it = templates.find(category);
    if (category_it != templates.end()) {
        auto template_it = category_it->second.find(template_name);
        if (template_it != category_it->second.end()) {
            return template_it->second;
        }
    }
    return std::nullopt;
}

TeamBuilder::Team TeamBuilder::generateTeamFromTemplate(const std::string& category, 
                                                       const std::string& template_name,
                                                       const std::string& custom_name) {
    auto template_data = getTemplate(category, template_name);
    if (!template_data) {
        return Team("Template_Not_Found");
    }
    
    std::string team_name = custom_name.empty() ? template_data->team_name : custom_name;
    return convertTemplateToTeam(*template_data, team_name);
}

TeamBuilder::Team TeamBuilder::generateAdvancedRandomTeam(const RandomGenerationSettings& settings,
                                                         const std::string& custom_name) {
    // Validate settings
    if (settings.team_size < 1 || settings.team_size > 6) {
        return Team("Invalid_Team_Size");
    }
    
    // Create team with appropriate name
    std::string team_name = custom_name.empty() ? "Random Team" : custom_name;
    Team team = createTeam(team_name);
    
    // Get available Pokemon
    auto all_pokemon = pokemon_data->getAvailablePokemon();
    if (all_pokemon.empty()) {
        return team;
    }
    
    // Filter Pokemon based on settings
    std::vector<std::string> available_pokemon;
    for (const auto& pokemon : all_pokemon) {
        // Skip legendaries if not allowed
        if (!settings.allow_legendaries && isPokemonLegendary(pokemon)) {
            continue;
        }
        
        // Check banned types
        bool is_banned = false;
        auto pokemon_info = pokemon_data->getPokemonInfo(pokemon);
        std::vector<std::string> pokemon_types;
        if (pokemon_info.has_value()) {
            pokemon_types = pokemon_info->types;
        }
        for (const auto& banned_type : settings.banned_types) {
            if (std::find(pokemon_types.begin(), pokemon_types.end(), banned_type) != pokemon_types.end()) {
                is_banned = true;
                break;
            }
        }
        
        if (!is_banned) {
            available_pokemon.push_back(pokemon);
        }
    }
    
    if (available_pokemon.empty()) {
        return team;
    }
    
    // Generate random team
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(available_pokemon.begin(), available_pokemon.end(), gen);
    
    int pokemon_added = 0;
    for (int i = 0; i < static_cast<int>(available_pokemon.size()) && pokemon_added < settings.team_size; ++i) {
        const auto& pokemon_name = available_pokemon[i];
        
        // Skip duplicates if not allowed
        if (!settings.allow_duplicates) {
            bool already_exists = false;
            for (const auto& existing_pokemon : team.pokemon) {
                if (existing_pokemon.name == pokemon_name) {
                    already_exists = true;
                    break;
                }
            }
            if (already_exists) {
                continue;
            }
        }
        
        // Generate moves for the Pokemon
        std::vector<std::string> moves;
        if (settings.optimize_movesets) {
            moves = generateMovesForPokemon(pokemon_name);
        } else {
            // Generate basic random moves
            auto available_moves = pokemon_data->getAvailableMoves();
            std::shuffle(available_moves.begin(), available_moves.end(), gen);
            
            int move_count = std::min(4, static_cast<int>(available_moves.size()));
            for (int j = 0; j < move_count; ++j) {
                moves.push_back(available_moves[j]);
            }
        }
        
        // Temporarily disable validation for random team generation
        auto original_settings = validation_settings;
        validation_settings.enforce_min_moves = false;
        validation_settings.enforce_min_team_size = false;
        
        if (addPokemonToTeam(team, pokemon_name, moves)) {
            pokemon_added++;
        }
        
        // Restore validation settings
        validation_settings = original_settings;
    }
    
    return team;
}

std::vector<std::pair<std::string, std::string>> TeamBuilder::getSuggestedTemplates(
    const std::string& difficulty, const std::string& strategy, int max_suggestions) const {
    
    std::vector<std::pair<std::string, std::string>> suggestions;
    
    for (const auto& category_pair : templates) {
        const auto& category = category_pair.first;
        
        for (const auto& template_pair : category_pair.second) {
            const auto& template_name = template_pair.first;
            const auto& template_data = template_pair.second;
            
            // Filter by difficulty if specified
            if (!difficulty.empty() && template_data.difficulty != difficulty) {
                continue;
            }
            
            // Filter by strategy if specified  
            if (!strategy.empty() && template_data.strategy != strategy) {
                continue;
            }
            
            suggestions.emplace_back(category, template_name);
            
            if (static_cast<int>(suggestions.size()) >= max_suggestions) {
                break;
            }
        }
        
        if (static_cast<int>(suggestions.size()) >= max_suggestions) {
            break;
        }
    }
    
    return suggestions;
}

std::vector<std::pair<std::string, std::string>> TeamBuilder::searchTemplates(
    const std::vector<std::string>& keywords) const {
    
    std::vector<std::pair<std::string, std::string>> results;
    
    for (const auto& category_pair : templates) {
        const auto& category = category_pair.first;
        
        for (const auto& template_pair : category_pair.second) {
            const auto& template_name = template_pair.first;
            const auto& template_data = template_pair.second;
            
            bool matches = false;
            
            // Check if any keyword matches template name or description
            for (const auto& keyword : keywords) {
                std::string lower_keyword = keyword;
                std::transform(lower_keyword.begin(), lower_keyword.end(), lower_keyword.begin(), ::tolower);
                
                std::string lower_name = template_data.name;
                std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
                
                std::string lower_desc = template_data.description;
                std::transform(lower_desc.begin(), lower_desc.end(), lower_desc.begin(), ::tolower);
                
                if (lower_name.find(lower_keyword) != std::string::npos ||
                    lower_desc.find(lower_keyword) != std::string::npos) {
                    matches = true;
                    break;
                }
            }
            
            if (matches) {
                results.emplace_back(category, template_name);
            }
        }
    }
    
    return results;
}

// Template Helper Methods Implementation

TeamBuilder::TeamTemplate TeamBuilder::parseTemplateFromJson(const std::string& file_path) {
    TeamTemplate template_data;
    
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return template_data;
        }
        
        json j;
        file >> j;
        
        // Parse template info
        if (j.contains("template_info")) {
            const auto& info = j["template_info"];
            template_data.name = info.value("name", "");
            template_data.description = info.value("description", "");
            template_data.difficulty = info.value("difficulty", "intermediate");
            template_data.strategy = info.value("strategy", "balanced");
            template_data.usage_notes = info.value("usage_notes", "");
            
            if (info.contains("learning_objectives")) {
                for (const auto& objective : info["learning_objectives"]) {
                    template_data.learning_objectives.push_back(objective);
                }
            }
            
            if (info.contains("type_coverage")) {
                const auto& coverage = info["type_coverage"];
                if (coverage.contains("offensive_types")) {
                    for (const auto& type : coverage["offensive_types"]) {
                        template_data.offensive_types.push_back(type);
                    }
                }
                if (coverage.contains("defensive_coverage")) {
                    for (const auto& type : coverage["defensive_coverage"]) {
                        template_data.defensive_coverage.push_back(type);
                    }
                }
            }
        }
        
        // Parse team data
        if (j.contains("team")) {
            const auto& team = j["team"];
            template_data.team_name = team.value("name", "Template Team");
            
            if (team.contains("pokemon")) {
                for (const auto& pokemon : team["pokemon"]) {
                    TemplatePokemon temp_pokemon;
                    temp_pokemon.name = pokemon.value("name", "");
                    temp_pokemon.role = pokemon.value("role", "");
                    temp_pokemon.strategy = pokemon.value("strategy", "");
                    temp_pokemon.tips = pokemon.value("tips", "");
                    
                    if (pokemon.contains("moves")) {
                        for (const auto& move : pokemon["moves"]) {
                            temp_pokemon.moves.push_back(move);
                        }
                    }
                    
                    template_data.pokemon.push_back(temp_pokemon);
                }
            }
        }
        
    } catch (const std::exception& e) {
        // Return empty template on error
        return TeamTemplate();
    }
    
    return template_data;
}

bool TeamBuilder::isValidTemplateFile(const std::string& file_path) const {
    // Basic file validation - check if the file exists and is a JSON file
    if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
        return false;
    }
    
    // Check if it's a JSON file (C++11 compatible way)
    return file_path.length() >= 5 && file_path.substr(file_path.length() - 5) == ".json";
}

std::string TeamBuilder::getTemplateNameFromFile(const std::string& file_path) const {
    std::filesystem::path path(file_path);
    return path.stem().string();
}

TeamBuilder::Team TeamBuilder::convertTemplateToTeam(const TeamTemplate& template_data, 
                                                    const std::string& custom_name) const {
    std::string team_name = custom_name.empty() ? template_data.team_name : custom_name;
    Team team = const_cast<TeamBuilder*>(this)->createTeam(team_name);
    
    for (const auto& template_pokemon : template_data.pokemon) {
        const_cast<TeamBuilder*>(this)->addPokemonToTeam(team, template_pokemon.name, template_pokemon.moves);
    }
    
    return team;
}

std::vector<std::string> TeamBuilder::getPokemonByRole(const std::string& role, int count) const {
    // This is a simplified implementation - in a full version, Pokemon would have role metadata
    auto all_pokemon = pokemon_data->getAvailablePokemon();
    std::vector<std::string> role_pokemon;
    
    // Basic role categorization based on known Pokemon characteristics
    for (const auto& pokemon : all_pokemon) {
        bool matches_role = false;
        
        if (role == "tank" || role == "wall") {
            // High defense Pokemon
            if (pokemon == "snorlax" || pokemon == "chansey" || pokemon == "lapras" || 
                pokemon == "cloyster" || pokemon == "golem" || pokemon == "slowbro") {
                matches_role = true;
            }
        } else if (role == "sweeper" || role == "special_attacker") {
            // High special attack Pokemon
            if (pokemon == "alakazam" || pokemon == "gengar" || pokemon == "charizard" ||
                pokemon == "jolteon" || pokemon == "starmie") {
                matches_role = true;
            }
        } else if (role == "physical_attacker") {
            // High attack Pokemon
            if (pokemon == "machamp" || pokemon == "dragonite" || pokemon == "gyarados" ||
                pokemon == "arcanine" || pokemon == "hitmonlee") {
                matches_role = true;
            }
        }
        
        if (matches_role) {
            role_pokemon.push_back(pokemon);
        }
    }
    
    // Return up to 'count' Pokemon
    if (static_cast<int>(role_pokemon.size()) > count) {
        role_pokemon.resize(count);
    }
    
    return role_pokemon;
}

std::vector<std::string> TeamBuilder::getLegendaryPokemon() const {
    // List of known legendary Pokemon in the dataset
    std::vector<std::string> legendaries = {
        "mewtwo", "mew", "articuno", "zapdos", "moltres"
    };
    
    // Filter to only include Pokemon that actually exist in the data
    std::vector<std::string> available_legendaries;
    for (const auto& legendary : legendaries) {
        if (pokemon_data->hasPokemon(legendary)) {
            available_legendaries.push_back(legendary);
        }
    }
    
    return available_legendaries;
}

bool TeamBuilder::isPokemonLegendary(const std::string& pokemon_name) const {
    auto legendaries = getLegendaryPokemon();
    return std::find(legendaries.begin(), legendaries.end(), pokemon_name) != legendaries.end();
}

std::vector<std::string> TeamBuilder::generateRoleBasedMoves(const std::string& pokemon_name, 
                                                            const std::string& /* role */) const {
    // This would ideally access a move database with role information
    // For now, return the standard move generation
    return generateMovesForPokemon(pokemon_name);
}

bool TeamBuilder::teamMeetsRoleRequirements(const Team& /* team */, 
                                           const RandomGenerationSettings& /* settings */) const {
    // This would need role detection logic based on Pokemon stats/movesets
    // For now, return true (simplified implementation)
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tournament/Draft Mode System Implementation
// ═══════════════════════════════════════════════════════════════════════════════

TeamBuilder::DraftSession TeamBuilder::createDraftSession(const DraftSettings& settings, 
                                                          const std::vector<std::string>& player_names) {
    DraftSession session;
    session.settings = settings;
    session.session_id = generateSessionId();
    session.is_active = true;
    session.current_phase = 0; // Start with ban phase
    session.current_player = 0;
    session.current_turn = 1;
    
    // Validate player count
    if (static_cast<int>(player_names.size()) != settings.player_count) {
        session.is_active = false;
        return session;
    }
    
    // Initialize player data
    session.player_names = player_names;
    session.player_teams.resize(settings.player_count);
    session.player_bans.resize(settings.player_count);
    session.teams_valid.resize(settings.player_count, false);
    session.team_errors.resize(settings.player_count);
    
    // Initialize available Pokemon pool (exclude globally banned)
    session.available_pokemon = pokemon_data->getAvailablePokemon();
    for (const auto& banned : settings.banned_pokemon) {
        auto it = std::find(session.available_pokemon.begin(), session.available_pokemon.end(), banned);
        if (it != session.available_pokemon.end()) {
            session.available_pokemon.erase(it);
        }
    }
    
    // Remove Pokemon of banned types
    for (const auto& banned_type : settings.banned_types) {
        session.available_pokemon.erase(
            std::remove_if(session.available_pokemon.begin(), session.available_pokemon.end(),
                [this, &banned_type](const std::string& pokemon_name) {
                    auto types = getCachedPokemonTypes(pokemon_name);
                    return std::find(types.begin(), types.end(), banned_type) != types.end();
                }),
            session.available_pokemon.end()
        );
    }
    
    // Store session for management
    active_draft_sessions[session.session_id] = session;
    
    return session;
}

bool TeamBuilder::executeDraftBan(DraftSession& session, int player_id, const std::string& pokemon_name) {
    if (!isDraftActionValid(session, player_id, "ban", pokemon_name)) {
        return false;
    }
    
    // Add to banned list
    session.banned_pokemon.push_back(pokemon_name);
    session.player_bans[player_id].push_back(pokemon_name);
    
    // Remove from available pool
    auto it = std::find(session.available_pokemon.begin(), session.available_pokemon.end(), pokemon_name);
    if (it != session.available_pokemon.end()) {
        session.available_pokemon.erase(it);
    }
    
    // Record action in history
    DraftSession::DraftAction action;
    action.player_id = player_id;
    action.action_type = "ban";
    action.pokemon_name = pokemon_name;
    action.turn_number = session.current_turn;
    action.timestamp = getCurrentTimestamp();
    action.strategy_note = "Banned to prevent opponent strategy";
    session.draft_history.push_back(action);
    
    return true;
}

bool TeamBuilder::executeDraftPick(DraftSession& session, int player_id, const std::string& pokemon_name) {
    if (!isDraftActionValid(session, player_id, "pick", pokemon_name)) {
        return false;
    }
    
    // Add to player team
    session.player_teams[player_id].push_back(pokemon_name);
    
    // Remove from available pool
    auto it = std::find(session.available_pokemon.begin(), session.available_pokemon.end(), pokemon_name);
    if (it != session.available_pokemon.end()) {
        session.available_pokemon.erase(it);
    }
    
    // Record action in history
    DraftSession::DraftAction action;
    action.player_id = player_id;
    action.action_type = "pick";
    action.pokemon_name = pokemon_name;
    action.turn_number = session.current_turn;
    action.timestamp = getCurrentTimestamp();
    action.strategy_note = "Strategic pick for team composition";
    session.draft_history.push_back(action);
    
    return true;
}

std::vector<std::string> TeamBuilder::getAvailablePicks(const DraftSession& session, int /* player_id */,
                                                       const std::string& filter_by_strategy) const {
    std::vector<std::string> available = session.available_pokemon;
    
    if (filter_by_strategy.empty()) {
        return available;
    }
    
    // Filter by strategy (simplified implementation)
    std::vector<std::string> filtered;
    for (const auto& pokemon : available) {
        bool matches_strategy = false;
        
        if (filter_by_strategy == "offensive") {
            // High attack Pokemon
            auto types = getCachedPokemonTypes(pokemon);
            if (std::find(types.begin(), types.end(), "fire") != types.end() ||
                std::find(types.begin(), types.end(), "dragon") != types.end() ||
                std::find(types.begin(), types.end(), "fighting") != types.end()) {
                matches_strategy = true;
            }
        } else if (filter_by_strategy == "defensive") {
            // Tank-like Pokemon
            if (pokemon == "snorlax" || pokemon == "chansey" || pokemon == "cloyster") {
                matches_strategy = true;
            }
        } else if (filter_by_strategy == "utility") {
            // Support Pokemon
            auto types = getCachedPokemonTypes(pokemon);
            if (std::find(types.begin(), types.end(), "psychic") != types.end() ||
                std::find(types.begin(), types.end(), "grass") != types.end()) {
                matches_strategy = true;
            }
        }
        
        if (matches_strategy) {
            filtered.push_back(pokemon);
        }
    }
    
    return filtered;
}

std::vector<std::pair<std::string, std::string>> TeamBuilder::getDraftSuggestions(
    const DraftSession& session, int suggestion_count) const {
    std::vector<std::pair<std::string, std::string>> suggestions;
    
    if (session.current_player >= static_cast<int>(session.player_teams.size())) {
        return suggestions;
    }
    
    auto current_team = session.player_teams[session.current_player];
    auto available = session.available_pokemon;
    
    // Analyze current team composition
    std::vector<std::string> team_types;
    for (const auto& pokemon : current_team) {
        auto types = getCachedPokemonTypes(pokemon);
        team_types.insert(team_types.end(), types.begin(), types.end());
    }
    
    // Suggest Pokemon that fill gaps
    for (const auto& pokemon : available) {
        if (suggestions.size() >= static_cast<size_t>(suggestion_count)) break;
        
        auto types = getCachedPokemonTypes(pokemon);
        std::string reasoning;
        
        // Check for type coverage
        bool fills_gap = false;
        for (const auto& type : types) {
            if (std::find(team_types.begin(), team_types.end(), type) == team_types.end()) {
                fills_gap = true;
                reasoning = "Adds " + type + " type coverage";
                break;
            }
        }
        
        // Check for legendary status
        if (isPokemonLegendary(pokemon) && !exceedsLegendaryLimit(session, session.current_player, pokemon)) {
            reasoning += " (Legendary - high power)";
            fills_gap = true;
        }
        
        if (fills_gap) {
            suggestions.emplace_back(pokemon, reasoning);
        }
    }
    
    // If we don't have enough suggestions, add some top-tier Pokemon
    if (suggestions.size() < static_cast<size_t>(suggestion_count)) {
        std::vector<std::string> meta_picks = {"mewtwo", "alakazam", "gengar", "dragonite", "snorlax"};
        for (const auto& pokemon : meta_picks) {
            if (suggestions.size() >= static_cast<size_t>(suggestion_count)) break;
            if (std::find(available.begin(), available.end(), pokemon) != available.end()) {
                suggestions.emplace_back(pokemon, "Meta pick - strong overall Pokemon");
            }
        }
    }
    
    return suggestions;
}

bool TeamBuilder::advanceDraftTurn(DraftSession& session) {
    // Move to next player
    if (session.settings.snake_draft && session.current_turn % 2 == 0) {
        // Snake draft: reverse direction on even turns
        session.current_player = (session.current_player == 0) ? 
            session.settings.player_count - 1 : session.current_player - 1;
    } else {
        // Linear draft or odd turns in snake draft
        session.current_player = (session.current_player + 1) % session.settings.player_count;
    }
    
    // Check if we need to advance phase
    updateDraftPhase(session);
    
    session.current_turn++;
    
    // Check if draft is complete
    bool all_teams_full = true;
    for (const auto& team : session.player_teams) {
        if (static_cast<int>(team.size()) < session.settings.team_size) {
            all_teams_full = false;
            break;
        }
    }
    
    if (all_teams_full) {
        session.current_phase = 3; // Complete
        session.is_active = false;
    }
    
    return true;
}

InputValidator::ValidationResult<bool> TeamBuilder::validateDraftTeam(const DraftSession& session, 
                                                                     int player_id) const {
    if (player_id < 0 || player_id >= static_cast<int>(session.player_teams.size())) {
        return InputValidator::ValidationResult<bool>(InputValidator::ValidationError::OUT_OF_RANGE, 
                                                     "Invalid player ID");
    }
    
    const auto& team = session.player_teams[player_id];
    
    // Check team size
    if (static_cast<int>(team.size()) != session.settings.team_size) {
        return InputValidator::ValidationResult<bool>(InputValidator::ValidationError::INVALID_INPUT,
                                                     "Team size does not match requirements");
    }
    
    // Check legendary limit
    int legendary_count = 0;
    for (const auto& pokemon : team) {
        if (isPokemonLegendary(pokemon)) {
            legendary_count++;
        }
    }
    if (legendary_count > session.settings.max_legendaries_per_team) {
        return InputValidator::ValidationResult<bool>(InputValidator::ValidationError::INVALID_INPUT,
                                                     "Too many legendary Pokemon");
    }
    
    // Check type limits
    std::map<std::string, int> type_counts;
    for (const auto& pokemon : team) {
        auto types = getCachedPokemonTypes(pokemon);
        for (const auto& type : types) {
            type_counts[type]++;
        }
    }
    
    for (const auto& [type, count] : type_counts) {
        if (count > session.settings.max_same_type_per_team) {
            return InputValidator::ValidationResult<bool>(InputValidator::ValidationError::INVALID_INPUT,
                                                         "Too many " + type + " type Pokemon");
        }
    }
    
    return InputValidator::ValidationResult<bool>(true);
}

std::vector<TeamBuilder::Team> TeamBuilder::finalizeDraftTeams(const DraftSession& session) {
    std::vector<Team> teams;
    
    for (size_t i = 0; i < session.player_teams.size(); ++i) {
        const auto& player_pokemon = session.player_teams[i];
        const auto& player_name = session.player_names[i];
        
        Team team = const_cast<TeamBuilder*>(this)->createTeam(player_name + "'s Draft Team");
        
        for (const auto& pokemon_name : player_pokemon) {
            auto moves = generateMovesForPokemon(pokemon_name);
            const_cast<TeamBuilder*>(this)->addPokemonToTeam(team, pokemon_name, moves);
        }
        
        teams.push_back(team);
    }
    
    return teams;
}

std::map<int, std::vector<std::string>> TeamBuilder::analyzeDraftStrategy(const DraftSession& session) const {
    std::map<int, std::vector<std::string>> analysis;
    
    for (size_t player_id = 0; player_id < session.player_teams.size(); ++player_id) {
        std::vector<std::string> player_analysis;
        const auto& team = session.player_teams[player_id];
        const auto& bans = session.player_bans[player_id];
        
        // Analyze team composition
        std::map<std::string, int> type_distribution;
        for (const auto& pokemon : team) {
            auto types = getCachedPokemonTypes(pokemon);
            for (const auto& type : types) {
                type_distribution[type]++;
            }
        }
        
        // Determine strategy
        std::string primary_strategy = "Balanced";
        int max_type_count = 0;
        std::string dominant_type;
        
        for (const auto& [type, count] : type_distribution) {
            if (count > max_type_count) {
                max_type_count = count;
                dominant_type = type;
            }
        }
        
        if (max_type_count >= 3) {
            primary_strategy = dominant_type + " specialist";
        }
        
        player_analysis.push_back("Primary Strategy: " + primary_strategy);
        player_analysis.push_back("Team Type Distribution: " + std::to_string(type_distribution.size()) + " unique types");
        
        // Analyze bans
        if (!bans.empty()) {
            player_analysis.push_back("Banned " + std::to_string(bans.size()) + " Pokemon to disrupt opponents");
        }
        
        // Check for legendary usage
        int legendary_count = 0;
        for (const auto& pokemon : team) {
            if (isPokemonLegendary(pokemon)) {
                legendary_count++;
            }
        }
        if (legendary_count > 0) {
            player_analysis.push_back("Using " + std::to_string(legendary_count) + " legendary Pokemon");
        }
        
        analysis[static_cast<int>(player_id)] = player_analysis;
    }
    
    return analysis;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Team Sharing System Implementation
// ═══════════════════════════════════════════════════════════════════════════════

std::string TeamBuilder::exportTeamShareCode(const Team& team, const std::string& creator_name,
                                            const std::string& description) const {
    TeamShareCode share_code;
    share_code.team_name = team.name;
    share_code.pokemon = team.pokemon;
    share_code.creator_name = InputValidator::sanitizeString(creator_name);
    share_code.creation_date = getCurrentTimestamp();
    share_code.description = InputValidator::sanitizeString(description);
    share_code.format_version = "1.0";
    
    return encodeTeamToBase64(share_code);
}

TeamBuilder::Team TeamBuilder::importTeamFromShareCode(const std::string& share_code, bool validate_team) {
    try {
        TeamShareCode decoded = decodeTeamFromBase64(share_code);
        
        Team team = const_cast<TeamBuilder*>(this)->createTeam(decoded.team_name);
        
        for (const auto& pokemon : decoded.pokemon) {
            const_cast<TeamBuilder*>(this)->addPokemonToTeam(team, pokemon.name, pokemon.moves);
        }
        
        if (validate_team) {
            const_cast<TeamBuilder*>(this)->validateTeam(team);
        }
        
        return team;
    } catch (const std::exception& e) {
        return Team("Import_Failed");
    }
}

bool TeamBuilder::saveCustomTeam(const Team& team, const std::string& custom_filename) {
    if (!ensureCustomTeamsDirectoryExists()) {
        return false;
    }
    
    std::string filename = custom_filename;
    if (filename.empty()) {
        filename = sanitizeCustomFilename(team.name) + ".json";
    } else {
        filename = sanitizeCustomFilename(filename);
        if (filename.find(".json") == std::string::npos) {
            filename += ".json";
        }
    }
    
    std::string file_path = getCustomTeamsDirectory() + "/" + filename;
    return saveTeamToFile(team, file_path);
}

TeamBuilder::Team TeamBuilder::loadCustomTeam(const std::string& filename) {
    std::string safe_filename = sanitizeCustomFilename(filename);
    if (safe_filename.find(".json") == std::string::npos) {
        safe_filename += ".json";
    }
    
    std::string file_path = getCustomTeamsDirectory() + "/" + safe_filename;
    return loadTeamFromFile(file_path);
}

std::vector<std::string> TeamBuilder::getCustomTeamsList() const {
    std::vector<std::string> team_files;
    
    try {
        std::string custom_dir = getCustomTeamsDirectory();
        if (!std::filesystem::exists(custom_dir)) {
            return team_files;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(custom_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                team_files.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::exception& e) {
        // Return empty list on error
    }
    
    return team_files;
}

bool TeamBuilder::deleteCustomTeam(const std::string& filename) {
    std::string safe_filename = sanitizeCustomFilename(filename);
    if (safe_filename.find(".json") == std::string::npos) {
        safe_filename += ".json";
    }
    
    std::string file_path = getCustomTeamsDirectory() + "/" + safe_filename;
    
    try {
        return std::filesystem::remove(file_path);
    } catch (const std::exception& e) {
        return false;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Team Comparison and Analysis Implementation
// ═══════════════════════════════════════════════════════════════════════════════

TeamBuilder::TeamComparison TeamBuilder::compareTeams(const Team& team1, const Team& team2) const {
    TeamComparison comparison;
    comparison.team1_name = team1.name;
    comparison.team2_name = team2.name;
    
    // Calculate type effectiveness between teams
    comparison.team1_vs_team2_effectiveness = {};
    comparison.team2_vs_team1_effectiveness = {};
    
    // Simplified type effectiveness calculation
    double team1_advantage = calculateTypeMatchupAdvantage(team1, team2);
    double team2_advantage = calculateTypeMatchupAdvantage(team2, team1);
    
    comparison.team1_vs_team2_effectiveness["overall"] = team1_advantage;
    comparison.team2_vs_team1_effectiveness["overall"] = team2_advantage;
    
    // Balance comparison
    auto analysis1 = analyzeTeam(team1);
    auto analysis2 = analyzeTeam(team2);
    comparison.team1_balance_score = analysis1.balance_score;
    comparison.team2_balance_score = analysis2.balance_score;
    
    // Coverage analysis
    comparison.team1_coverage_advantages = findCoverageStrengths(team1);
    comparison.team2_coverage_advantages = findCoverageStrengths(team2);
    comparison.mutual_weaknesses = findCoverageGaps(team1);
    
    // Filter mutual weaknesses to only include those shared by both teams
    auto team2_gaps = findCoverageGaps(team2);
    comparison.mutual_weaknesses.erase(
        std::remove_if(comparison.mutual_weaknesses.begin(), comparison.mutual_weaknesses.end(),
            [&team2_gaps](const std::string& gap) {
                return std::find(team2_gaps.begin(), team2_gaps.end(), gap) == team2_gaps.end();
            }),
        comparison.mutual_weaknesses.end()
    );
    
    // Predict battle outcome
    comparison.team1_win_probability = predictBattleOutcome(team1, team2);
    
    if (comparison.team1_win_probability > 0.6) {
        comparison.battle_prediction_reasoning = team1.name + " has significant type and balance advantages";
    } else if (comparison.team1_win_probability < 0.4) {
        comparison.battle_prediction_reasoning = team2.name + " has significant type and balance advantages";
    } else {
        comparison.battle_prediction_reasoning = "Teams are well-matched, battle outcome depends on strategy";
    }
    
    // Generate improvement suggestions
    comparison.team1_improvement_suggestions = getTeamSuggestions(team1);
    comparison.team2_improvement_suggestions = getTeamSuggestions(team2);
    
    return comparison;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Battle History and Statistics Implementation
// ═══════════════════════════════════════════════════════════════════════════════

void TeamBuilder::recordBattleResult(const std::string& team_name, const std::string& opponent_name,
                                     bool victory, int turns_taken, const std::string& difficulty,
                                     double effectiveness_score) {
    // Load existing battle history
    loadBattleHistory();
    
    // Create battle record
    BattleRecord record;
    record.team_name = InputValidator::sanitizeString(team_name);
    record.opponent_team = InputValidator::sanitizeString(opponent_name);
    record.battle_date = getCurrentTimestamp();
    record.victory = victory;
    record.turns_taken = turns_taken;
    record.difficulty_level = difficulty;
    record.team_effectiveness_score = std::max(0.0, std::min(100.0, effectiveness_score));
    
    // Add to history
    battle_history[record.team_name].push_back(record);
    
    // Update statistics
    updateTeamStatistics(record.team_name);
    
    // Save to file
    saveBattleHistory();
}

std::optional<TeamBuilder::TeamStatistics> TeamBuilder::getTeamStatistics(const std::string& team_name) const {
    loadBattleHistory();
    
    auto it = team_statistics.find(team_name);
    if (it != team_statistics.end()) {
        return it->second;
    }
    
    return std::nullopt;
}

std::vector<TeamBuilder::BattleRecord> TeamBuilder::getTeamBattleHistory(const std::string& team_name, 
                                                                        int max_records) const {
    loadBattleHistory();
    
    auto it = battle_history.find(team_name);
    if (it == battle_history.end()) {
        return {};
    }
    
    const auto& records = it->second;
    if (max_records <= 0 || static_cast<int>(records.size()) <= max_records) {
        return records;
    }
    
    // Return most recent records
    auto start_it = records.end() - max_records;
    return std::vector<BattleRecord>(start_it, records.end());
}

bool TeamBuilder::clearTeamBattleHistory(const std::string& team_name) {
    loadBattleHistory();
    
    auto history_it = battle_history.find(team_name);
    if (history_it != battle_history.end()) {
        battle_history.erase(history_it);
    }
    
    auto stats_it = team_statistics.find(team_name);
    if (stats_it != team_statistics.end()) {
        team_statistics.erase(stats_it);
    }
    
    saveBattleHistory();
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Enhanced Random Generation Implementation
// ═══════════════════════════════════════════════════════════════════════════════

TeamBuilder::Team TeamBuilder::generateMetaOptimizedTeam(const RandomGenerationSettings& settings,
                                                         bool meta_analysis) const {
    std::string team_name = "Meta-Optimized Team";
    Team team = const_cast<TeamBuilder*>(this)->createTeam(team_name);
    
    std::vector<std::string> selected_pokemon;
    
    if (meta_analysis) {
        // Get meta-tier Pokemon
        auto meta_tier_pokemon = getMetaTierPokemon("S");
        auto high_tier_pokemon = getMetaTierPokemon("A");
        
        // Combine tiers with preference for meta tier
        std::vector<std::string> priority_pokemon = meta_tier_pokemon;
        priority_pokemon.insert(priority_pokemon.end(), high_tier_pokemon.begin(), high_tier_pokemon.end());
        
        // Select from priority list first
        int priority_picks = std::min(settings.team_size / 2, static_cast<int>(priority_pokemon.size()));
        for (int i = 0; i < priority_picks && selected_pokemon.size() < static_cast<size_t>(settings.team_size); ++i) {
            if (std::find(selected_pokemon.begin(), selected_pokemon.end(), priority_pokemon[i]) == selected_pokemon.end()) {
                selected_pokemon.push_back(priority_pokemon[i]);
            }
        }
    }
    
    // Fill remaining slots with balanced selection
    auto all_pokemon = pokemon_data->getAvailablePokemon();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(all_pokemon.begin(), all_pokemon.end(), gen);
    
    for (const auto& pokemon : all_pokemon) {
        if (static_cast<int>(selected_pokemon.size()) >= settings.team_size) {
            break;
        }
        
        if (std::find(selected_pokemon.begin(), selected_pokemon.end(), pokemon) != selected_pokemon.end()) {
            continue;
        }
        
        // Check constraints
        if (!settings.allow_legendaries && isPokemonLegendary(pokemon)) {
            continue;
        }
        
        // Check banned types
        bool is_banned_type = false;
        auto types = getCachedPokemonTypes(pokemon);
        for (const auto& type : types) {
            if (std::find(settings.banned_types.begin(), settings.banned_types.end(), type) != settings.banned_types.end()) {
                is_banned_type = true;
                break;
            }
        }
        if (is_banned_type) {
            continue;
        }
        
        selected_pokemon.push_back(pokemon);
    }
    
    // Optimize team composition
    selected_pokemon = optimizeTeamComposition(selected_pokemon);
    
    // Add Pokemon to team with optimized movesets
    for (const auto& pokemon_name : selected_pokemon) {
        auto moves = generateMovesForPokemon(pokemon_name);
        const_cast<TeamBuilder*>(this)->addPokemonToTeam(team, pokemon_name, moves);
    }
    
    return team;
}

TeamBuilder::Team TeamBuilder::generateCounterTeam(const Team& target_team, const std::string& team_name,
                                                   double strictness) const {
    Team counter_team = const_cast<TeamBuilder*>(this)->createTeam(team_name);
    
    // Analyze target team weaknesses
    std::vector<std::string> target_weaknesses;
    for (const auto& pokemon : target_team.pokemon) {
        auto types = getCachedPokemonTypes(pokemon.name);
        for (const auto& type : types) {
            // Add types that are weak to common attacking types
            if (type == "grass") target_weaknesses.push_back("fire");
            if (type == "fire") target_weaknesses.push_back("water");
            if (type == "water") target_weaknesses.push_back("electric");
            if (type == "psychic") target_weaknesses.push_back("ghost");
            if (type == "fighting") target_weaknesses.push_back("psychic");
        }
    }
    
    // Remove duplicates
    std::sort(target_weaknesses.begin(), target_weaknesses.end());
    target_weaknesses.erase(std::unique(target_weaknesses.begin(), target_weaknesses.end()), target_weaknesses.end());
    
    // Select counter Pokemon
    auto all_pokemon = pokemon_data->getAvailablePokemon();
    std::vector<std::string> counter_pokemon;
    
    for (const auto& weakness_type : target_weaknesses) {
        for (const auto& pokemon : all_pokemon) {
            if (counter_pokemon.size() >= 6) break;
            
            auto types = getCachedPokemonTypes(pokemon);
            if (std::find(types.begin(), types.end(), weakness_type) != types.end()) {
                if (std::find(counter_pokemon.begin(), counter_pokemon.end(), pokemon) == counter_pokemon.end()) {
                    counter_pokemon.push_back(pokemon);
                }
            }
        }
    }
    
    // If we need more Pokemon or strictness is low, add balanced picks
    if (strictness < 0.8) {
        std::vector<std::string> balanced_picks = {"snorlax", "alakazam", "gengar", "dragonite"};
        for (const auto& pokemon : balanced_picks) {
            if (counter_pokemon.size() >= 6) break;
            if (std::find(counter_pokemon.begin(), counter_pokemon.end(), pokemon) == counter_pokemon.end()) {
                counter_pokemon.push_back(pokemon);
            }
        }
    }
    
    // Ensure we have enough Pokemon
    while (counter_pokemon.size() < 6 && counter_pokemon.size() < all_pokemon.size()) {
        for (const auto& pokemon : all_pokemon) {
            if (counter_pokemon.size() >= 6) break;
            if (std::find(counter_pokemon.begin(), counter_pokemon.end(), pokemon) == counter_pokemon.end()) {
                counter_pokemon.push_back(pokemon);
            }
        }
    }
    
    // Add to team
    for (size_t i = 0; i < std::min(counter_pokemon.size(), size_t(6)); ++i) {
        auto moves = generateMovesForPokemon(counter_pokemon[i]);
        const_cast<TeamBuilder*>(this)->addPokemonToTeam(counter_team, counter_pokemon[i], moves);
    }
    
    return counter_team;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Helper Methods Implementation
// ═══════════════════════════════════════════════════════════════════════════════

// Draft system helper methods
std::string TeamBuilder::generateSessionId() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "draft_" << time_t << "_" << (rand() % 10000);
    return ss.str();
}

bool TeamBuilder::isDraftActionValid(const DraftSession& session, int player_id, 
                                     const std::string& action_type, const std::string& pokemon_name) const {
    // Check if session is active
    if (!session.is_active) return false;
    
    // Check if it's the player's turn
    if (player_id != session.current_player) return false;
    
    // Check if Pokemon exists and is available
    if (!pokemon_data->hasPokemon(pokemon_name)) return false;
    
    if (action_type == "pick") {
        // Check if Pokemon is available for picking
        if (std::find(session.available_pokemon.begin(), session.available_pokemon.end(), pokemon_name) 
            == session.available_pokemon.end()) {
            return false;
        }
        
        // Check if adding this Pokemon would exceed limits
        if (exceedsLegendaryLimit(session, player_id, pokemon_name)) return false;
        if (exceedsTypeLimit(session, player_id, pokemon_name)) return false;
        
    } else if (action_type == "ban") {
        // Check if in ban phase
        if (session.current_phase != 0) return false;
        
        // Check if Pokemon is available for banning
        if (std::find(session.available_pokemon.begin(), session.available_pokemon.end(), pokemon_name) 
            == session.available_pokemon.end()) {
            return false;
        }
    }
    
    return true;
}

void TeamBuilder::updateDraftPhase(DraftSession& session) const {
    // Count total bans
    int total_bans = 0;
    for (const auto& player_bans : session.player_bans) {
        total_bans += static_cast<int>(player_bans.size());
    }
    
    int expected_bans = session.settings.player_count * session.settings.ban_phase_picks_per_player;
    
    if (session.current_phase == 0 && total_bans >= expected_bans) {
        session.current_phase = 1; // Move to pick phase
    }
}

int TeamBuilder::getNextPlayer(const DraftSession& session) const {
    if (session.settings.snake_draft && session.current_turn % 2 == 0) {
        return (session.current_player == 0) ? 
            session.settings.player_count - 1 : session.current_player - 1;
    } else {
        return (session.current_player + 1) % session.settings.player_count;
    }
}

std::vector<std::string> TeamBuilder::getPlayerTeamTypes(const DraftSession& session, int player_id) const {
    std::vector<std::string> types;
    if (player_id < 0 || player_id >= static_cast<int>(session.player_teams.size())) {
        return types;
    }
    
    for (const auto& pokemon_name : session.player_teams[player_id]) {
        auto pokemon_types = getCachedPokemonTypes(pokemon_name);
        types.insert(types.end(), pokemon_types.begin(), pokemon_types.end());
    }
    
    return types;
}

bool TeamBuilder::exceedsTypeLimit(const DraftSession& session, int player_id, const std::string& pokemon_name) const {
    auto pokemon_types = getCachedPokemonTypes(pokemon_name);
    auto current_types = getPlayerTeamTypes(session, player_id);
    
    for (const auto& type : pokemon_types) {
        int type_count = std::count(current_types.begin(), current_types.end(), type);
        if (type_count >= session.settings.max_same_type_per_team) {
            return true;
        }
    }
    
    return false;
}

bool TeamBuilder::exceedsLegendaryLimit(const DraftSession& session, int player_id, const std::string& pokemon_name) const {
    if (!isPokemonLegendary(pokemon_name)) {
        return false;
    }
    
    if (player_id < 0 || player_id >= static_cast<int>(session.player_teams.size())) {
        return true;
    }
    
    int legendary_count = 0;
    for (const auto& team_pokemon : session.player_teams[player_id]) {
        if (isPokemonLegendary(team_pokemon)) {
            legendary_count++;
        }
    }
    
    return legendary_count >= session.settings.max_legendaries_per_team;
}

std::string TeamBuilder::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Team sharing helper methods
std::string TeamBuilder::encodeTeamToBase64(const TeamShareCode& share_code) const {
    // Create JSON representation
    json j;
    j["team_name"] = share_code.team_name;
    j["creator_name"] = share_code.creator_name;
    j["creation_date"] = share_code.creation_date;
    j["description"] = share_code.description;
    j["format_version"] = share_code.format_version;
    
    j["pokemon"] = json::array();
    for (const auto& pokemon : share_code.pokemon) {
        json pokemon_json;
        pokemon_json["name"] = pokemon.name;
        pokemon_json["moves"] = pokemon.moves;
        j["pokemon"].push_back(pokemon_json);
    }
    
    std::string json_str = j.dump();
    
    // Simple base64 encoding (for demonstration - in production, use a proper base64 library)
    std::string encoded;
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    for (size_t i = 0; i < json_str.length(); i += 3) {
        unsigned char b1 = json_str[i];
        unsigned char b2 = (i + 1 < json_str.length()) ? json_str[i + 1] : 0;
        unsigned char b3 = (i + 2 < json_str.length()) ? json_str[i + 2] : 0;
        
        encoded += chars[b1 >> 2];
        encoded += chars[((b1 & 0x03) << 4) | ((b2 & 0xf0) >> 4)];
        encoded += (i + 1 < json_str.length()) ? chars[((b2 & 0x0f) << 2) | ((b3 & 0xc0) >> 6)] : '=';
        encoded += (i + 2 < json_str.length()) ? chars[b3 & 0x3f] : '=';
    }
    
    return encoded;
}

TeamBuilder::TeamShareCode TeamBuilder::decodeTeamFromBase64(const std::string& base64_data) const {
    TeamShareCode share_code;
    
    try {
        // Simple base64 decoding (for demonstration)
        std::string decoded;
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        for (size_t i = 0; i < base64_data.length(); i += 4) {
            unsigned char c1 = chars.find(base64_data[i]);
            unsigned char c2 = chars.find(base64_data[i + 1]);
            unsigned char c3 = (base64_data[i + 2] != '=') ? chars.find(base64_data[i + 2]) : 0;
            unsigned char c4 = (base64_data[i + 3] != '=') ? chars.find(base64_data[i + 3]) : 0;
            
            decoded += static_cast<char>((c1 << 2) | (c2 >> 4));
            if (base64_data[i + 2] != '=') {
                decoded += static_cast<char>((c2 << 4) | (c3 >> 2));
            }
            if (base64_data[i + 3] != '=') {
                decoded += static_cast<char>((c3 << 6) | c4);
            }
        }
        
        // Parse JSON
        json j = json::parse(decoded);
        
        share_code.team_name = j["team_name"];
        share_code.creator_name = j["creator_name"];
        share_code.creation_date = j["creation_date"];
        share_code.description = j["description"];
        share_code.format_version = j["format_version"];
        
        for (const auto& pokemon_json : j["pokemon"]) {
            TeamPokemon pokemon;
            pokemon.name = pokemon_json["name"];
            pokemon.moves = pokemon_json["moves"].get<std::vector<std::string>>();
            share_code.pokemon.push_back(pokemon);
        }
    } catch (const std::exception& e) {
        // Return empty share code on error
        share_code = TeamShareCode();
    }
    
    return share_code;
}

std::string TeamBuilder::getCustomTeamsDirectory() const {
    return "data/teams/custom";
}

std::string TeamBuilder::sanitizeCustomFilename(const std::string& filename) const {
    return InputValidator::sanitizeFileName(filename);
}

bool TeamBuilder::ensureCustomTeamsDirectoryExists() const {
    try {
        std::string dir = getCustomTeamsDirectory();
        if (!std::filesystem::exists(dir)) {
            return std::filesystem::create_directories(dir);
        }
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

// Team comparison helper methods
double TeamBuilder::calculateTypeMatchupAdvantage(const Team& attacker, const Team& defender) const {
    double total_advantage = 0.0;
    int comparisons = 0;
    
    for (const auto& attacker_pokemon : attacker.pokemon) {
        auto attacker_types = getCachedPokemonTypes(attacker_pokemon.name);
        
        for (const auto& defender_pokemon : defender.pokemon) {
            auto defender_types = getCachedPokemonTypes(defender_pokemon.name);
            
            // Simplified type effectiveness calculation
            for (const auto& attack_type : attacker_types) {
                for (const auto& defend_type : defender_types) {
                    double effectiveness = 1.0;
                    
                    // Basic type effectiveness rules
                    if (attack_type == "fire" && defend_type == "grass") effectiveness = 2.0;
                    else if (attack_type == "water" && defend_type == "fire") effectiveness = 2.0;
                    else if (attack_type == "grass" && defend_type == "water") effectiveness = 2.0;
                    else if (attack_type == "electric" && defend_type == "water") effectiveness = 2.0;
                    else if (attack_type == "psychic" && defend_type == "fighting") effectiveness = 2.0;
                    else if (attack_type == "ghost" && defend_type == "psychic") effectiveness = 2.0;
                    // Add more type matchups as needed
                    
                    total_advantage += effectiveness;
                    comparisons++;
                }
            }
        }
    }
    
    return comparisons > 0 ? total_advantage / comparisons : 1.0;
}

std::vector<std::string> TeamBuilder::findCoverageGaps(const Team& team) const {
    std::vector<std::string> gaps;
    std::vector<std::string> team_types;
    
    // Collect all types in the team
    for (const auto& pokemon : team.pokemon) {
        auto types = getCachedPokemonTypes(pokemon.name);
        team_types.insert(team_types.end(), types.begin(), types.end());
    }
    
    // Check for common types that are missing
    std::vector<std::string> common_types = {
        "fire", "water", "grass", "electric", "psychic", "fighting", 
        "rock", "ground", "flying", "poison", "bug", "ghost", "ice", "dragon"
    };
    
    for (const auto& type : common_types) {
        if (std::find(team_types.begin(), team_types.end(), type) == team_types.end()) {
            gaps.push_back(type);
        }
    }
    
    return gaps;
}

std::vector<std::string> TeamBuilder::findCoverageStrengths(const Team& team) const {
    std::vector<std::string> strengths;
    std::map<std::string, int> type_counts;
    
    // Count type occurrences
    for (const auto& pokemon : team.pokemon) {
        auto types = getCachedPokemonTypes(pokemon.name);
        for (const auto& type : types) {
            type_counts[type]++;
        }
    }
    
    // Types with multiple Pokemon are strengths
    for (const auto& [type, count] : type_counts) {
        if (count >= 2) {
            strengths.push_back(type);
        }
    }
    
    return strengths;
}

double TeamBuilder::predictBattleOutcome(const Team& team1, const Team& team2) const {
    double team1_score = 0.0;
    double team2_score = 0.0;
    
    // Factor in type advantages
    team1_score += calculateTypeMatchupAdvantage(team1, team2) * 0.4;
    team2_score += calculateTypeMatchupAdvantage(team2, team1) * 0.4;
    
    // Factor in team balance
    auto analysis1 = analyzeTeam(team1);
    auto analysis2 = analyzeTeam(team2);
    team1_score += (analysis1.balance_score / 100.0) * 0.3;
    team2_score += (analysis2.balance_score / 100.0) * 0.3;
    
    // Factor in legendary Pokemon count
    int team1_legendaries = 0, team2_legendaries = 0;
    for (const auto& pokemon : team1.pokemon) {
        if (isPokemonLegendary(pokemon.name)) team1_legendaries++;
    }
    for (const auto& pokemon : team2.pokemon) {
        if (isPokemonLegendary(pokemon.name)) team2_legendaries++;
    }
    
    team1_score += (team1_legendaries * 0.05);
    team2_score += (team2_legendaries * 0.05);
    
    // Convert to probability for team1
    double total_score = team1_score + team2_score;
    return total_score > 0 ? team1_score / total_score : 0.5;
}

// Battle history helper methods
void TeamBuilder::loadBattleHistory() const {
    try {
        std::string history_file = getBattleHistoryFilePath();
        if (!std::filesystem::exists(history_file)) {
            return;
        }
        
        std::ifstream file(history_file);
        if (!file.is_open()) {
            return;
        }
        
        json j;
        file >> j;
        
        battle_history.clear();
        team_statistics.clear();
        
        if (j.contains("battle_history")) {
            for (const auto& [team_name, records] : j["battle_history"].items()) {
                std::vector<BattleRecord> team_records;
                for (const auto& record_json : records) {
                    BattleRecord record;
                    record.team_name = record_json["team_name"];
                    record.opponent_team = record_json["opponent_team"];
                    record.battle_date = record_json["battle_date"];
                    record.victory = record_json["victory"];
                    record.turns_taken = record_json["turns_taken"];
                    record.difficulty_level = record_json["difficulty_level"];
                    record.team_effectiveness_score = record_json["team_effectiveness_score"];
                    team_records.push_back(record);
                }
                battle_history[team_name] = team_records;
            }
        }
        
        if (j.contains("team_statistics")) {
            for (const auto& [team_name, stats_json] : j["team_statistics"].items()) {
                TeamStatistics stats;
                stats.team_name = stats_json["team_name"];
                stats.total_battles = stats_json["total_battles"];
                stats.victories = stats_json["victories"];
                stats.defeats = stats_json["defeats"];
                stats.win_rate = stats_json["win_rate"];
                stats.average_battle_length = stats_json["average_battle_length"];
                stats.average_effectiveness_score = stats_json["average_effectiveness_score"];
                team_statistics[team_name] = stats;
            }
        }
    } catch (const std::exception& e) {
        // Ignore errors in loading
    }
}

void TeamBuilder::saveBattleHistory() const {
    try {
        json j;
        
        // Save battle history
        j["battle_history"] = json::object();
        for (const auto& [team_name, records] : battle_history) {
            j["battle_history"][team_name] = json::array();
            for (const auto& record : records) {
                json record_json;
                record_json["team_name"] = record.team_name;
                record_json["opponent_team"] = record.opponent_team;
                record_json["battle_date"] = record.battle_date;
                record_json["victory"] = record.victory;
                record_json["turns_taken"] = record.turns_taken;
                record_json["difficulty_level"] = record.difficulty_level;
                record_json["team_effectiveness_score"] = record.team_effectiveness_score;
                j["battle_history"][team_name].push_back(record_json);
            }
        }
        
        // Save team statistics
        j["team_statistics"] = json::object();
        for (const auto& [team_name, stats] : team_statistics) {
            json stats_json;
            stats_json["team_name"] = stats.team_name;
            stats_json["total_battles"] = stats.total_battles;
            stats_json["victories"] = stats.victories;
            stats_json["defeats"] = stats.defeats;
            stats_json["win_rate"] = stats.win_rate;
            stats_json["average_battle_length"] = stats.average_battle_length;
            stats_json["average_effectiveness_score"] = stats.average_effectiveness_score;
            j["team_statistics"][team_name] = stats_json;
        }
        
        std::string history_file = getBattleHistoryFilePath();
        std::ofstream file(history_file);
        if (file.is_open()) {
            file << j.dump(2);
        }
    } catch (const std::exception& e) {
        // Ignore save errors
    }
}

void TeamBuilder::updateTeamStatistics(const std::string& team_name) const {
    auto it = battle_history.find(team_name);
    if (it == battle_history.end()) {
        return;
    }
    
    const auto& records = it->second;
    TeamStatistics stats;
    stats.team_name = team_name;
    stats.total_battles = static_cast<int>(records.size());
    stats.victories = 0;
    stats.defeats = 0;
    
    double total_turns = 0.0;
    double total_effectiveness = 0.0;
    
    for (const auto& record : records) {
        if (record.victory) {
            stats.victories++;
        } else {
            stats.defeats++;
        }
        total_turns += record.turns_taken;
        total_effectiveness += record.team_effectiveness_score;
    }
    
    stats.win_rate = stats.total_battles > 0 ? 
        (static_cast<double>(stats.victories) / stats.total_battles) * 100.0 : 0.0;
    stats.average_battle_length = stats.total_battles > 0 ? 
        total_turns / stats.total_battles : 0.0;
    stats.average_effectiveness_score = stats.total_battles > 0 ? 
        total_effectiveness / stats.total_battles : 0.0;
    
    team_statistics[team_name] = stats;
}

std::string TeamBuilder::getBattleHistoryFilePath() const {
    return "data/teams/battle_history.json";
}

std::string TeamBuilder::getTeamStatisticsFilePath() const {
    return "data/teams/team_statistics.json";
}

// Performance optimization helper methods
void TeamBuilder::preloadPokemonData() const {
    auto all_pokemon = pokemon_data->getAvailablePokemon();
    for (const auto& pokemon : all_pokemon) {
        getCachedPokemonTypes(pokemon);
        getCachedPokemonMoves(pokemon);
    }
}

void TeamBuilder::clearPerformanceCaches() const {
    pokemon_type_cache.clear();
    pokemon_moves_cache.clear();
}

std::vector<std::string> TeamBuilder::getCachedPokemonTypes(const std::string& pokemon_name) const {
    auto it = pokemon_type_cache.find(pokemon_name);
    if (it != pokemon_type_cache.end()) {
        return it->second;
    }
    
    // Load from Pokemon data and cache
    std::vector<std::string> types;
    if (pokemon_data->hasPokemon(pokemon_name)) {
        auto pokemon_info = pokemon_data->getPokemonInfo(pokemon_name);
        if (pokemon_info) {
            types = pokemon_info->types;
        }
    }
    
    pokemon_type_cache[pokemon_name] = types;
    return types;
}

std::vector<std::string> TeamBuilder::getCachedPokemonMoves(const std::string& pokemon_name) const {
    auto it = pokemon_moves_cache.find(pokemon_name);
    if (it != pokemon_moves_cache.end()) {
        return it->second;
    }
    
    // Generate moves and cache
    std::vector<std::string> moves = generateMovesForPokemon(pokemon_name);
    pokemon_moves_cache[pokemon_name] = moves;
    return moves;
}

// Enhanced generation helper methods
std::vector<std::string> TeamBuilder::getMetaTierPokemon(const std::string& tier) const {
    std::vector<std::string> meta_pokemon;
    
    if (tier == "S") {
        // S-tier Pokemon (strongest in meta)
        meta_pokemon = {"mewtwo", "alakazam", "gengar", "dragonite", "snorlax"};
    } else if (tier == "A") {
        // A-tier Pokemon (very strong)
        meta_pokemon = {"charizard", "blastoise", "venusaur", "gyarados", "lapras", "articuno", "zapdos", "moltres"};
    } else if (tier == "B") {
        // B-tier Pokemon (good)
        meta_pokemon = {"machamp", "golem", "arcanine", "cloyster", "starmie", "jolteon", "vaporeon", "flareon"};
    }
    
    // Filter to only include Pokemon that exist in the data
    std::vector<std::string> available_meta;
    for (const auto& pokemon : meta_pokemon) {
        if (pokemon_data->hasPokemon(pokemon)) {
            available_meta.push_back(pokemon);
        }
    }
    
    return available_meta;
}

std::vector<std::string> TeamBuilder::getCounterPokemon(const std::string& target_pokemon) const {
    std::vector<std::string> counters;
    auto target_types = getCachedPokemonTypes(target_pokemon);
    
    auto all_pokemon = pokemon_data->getAvailablePokemon();
    for (const auto& pokemon : all_pokemon) {
        auto pokemon_types = getCachedPokemonTypes(pokemon);
        
        // Check if this Pokemon has type advantage
        bool has_advantage = false;
        for (const auto& attack_type : pokemon_types) {
            for (const auto& defend_type : target_types) {
                // Simplified type effectiveness check
                if ((attack_type == "fire" && defend_type == "grass") ||
                    (attack_type == "water" && defend_type == "fire") ||
                    (attack_type == "electric" && defend_type == "water") ||
                    (attack_type == "psychic" && defend_type == "fighting") ||
                    (attack_type == "ghost" && defend_type == "psychic")) {
                    has_advantage = true;
                    break;
                }
            }
            if (has_advantage) break;
        }
        
        if (has_advantage) {
            counters.push_back(pokemon);
        }
    }
    
    return counters;
}

double TeamBuilder::calculatePokemonSynergy(const std::vector<std::string>& team_pokemon) const {
    if (team_pokemon.empty()) return 0.0;
    
    double synergy_score = 0.0;
    
    // Calculate type diversity bonus
    std::set<std::string> unique_types;
    for (const auto& pokemon : team_pokemon) {
        auto types = getCachedPokemonTypes(pokemon);
        unique_types.insert(types.begin(), types.end());
    }
    
    synergy_score += unique_types.size() * 10.0; // Bonus for type diversity
    
    // Calculate coverage bonus
    std::vector<std::string> important_types = {"fire", "water", "grass", "electric", "psychic"};
    int coverage_count = 0;
    for (const auto& type : important_types) {
        if (unique_types.count(type) > 0) {
            coverage_count++;
        }
    }
    
    synergy_score += coverage_count * 15.0; // Bonus for important type coverage
    
    return std::min(synergy_score, 100.0);
}

std::vector<std::string> TeamBuilder::optimizeTeamComposition(const std::vector<std::string>& base_team) const {
    std::vector<std::string> optimized = base_team;
    
    // Ensure we have good type coverage
    std::set<std::string> team_types;
    for (const auto& pokemon : optimized) {
        auto types = getCachedPokemonTypes(pokemon);
        team_types.insert(types.begin(), types.end());
    }
    
    // If we're missing important types, try to replace some Pokemon
    std::vector<std::string> important_types = {"fire", "water", "electric", "psychic"};
    auto all_pokemon = pokemon_data->getAvailablePokemon();
    
    for (const auto& needed_type : important_types) {
        if (team_types.count(needed_type) == 0 && optimized.size() < 6) {
            // Find a Pokemon with this type
            for (const auto& pokemon : all_pokemon) {
                if (std::find(optimized.begin(), optimized.end(), pokemon) != optimized.end()) {
                    continue;
                }
                
                auto types = getCachedPokemonTypes(pokemon);
                if (std::find(types.begin(), types.end(), needed_type) != types.end()) {
                    if (optimized.size() < 6) {
                        optimized.push_back(pokemon);
                    } else {
                        // Replace a duplicate type if possible
                        for (size_t i = 0; i < optimized.size(); ++i) {
                            auto existing_types = getCachedPokemonTypes(optimized[i]);
                            bool has_duplicate = false;
                            for (const auto& existing_type : existing_types) {
                                if (existing_type != needed_type && team_types.count(existing_type) > 1) {
                                    has_duplicate = true;
                                    break;
                                }
                            }
                            if (has_duplicate) {
                                optimized[i] = pokemon;
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    
    return optimized;
}