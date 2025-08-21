#include "team_builder.h"
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <set>
#include <filesystem>
#include <cstdlib>

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
    
    for (const auto& pokemon_name : selected_pokemon) {
        auto moves = generateMovesForPokemon(pokemon_name);
        addPokemonToTeam(team, pokemon_name, moves);
    }
    
    validateTeam(team);
    return team;
}

TeamBuilder::Team TeamBuilder::generateBalancedTeam(const std::string& team_name, int team_size) {
    auto team = createTeam(team_name);
    team_size = std::min(6, std::max(1, team_size));
    
    // Try to get diverse types
    std::vector<std::string> preferred_types = {"fire", "water", "grass", "electric", "psychic", "dragon"};
    std::set<std::string> used_types;
    
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
    
    for (int i = 0; i < settings.team_size && i < static_cast<int>(available_pokemon.size()); ++i) {
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
        
        addPokemonToTeam(team, pokemon_name, moves);
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