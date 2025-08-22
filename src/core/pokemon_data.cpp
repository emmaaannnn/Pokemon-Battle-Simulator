#include "pokemon_data.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

PokemonData::PokemonData() : is_initialized(false) {
    // Initialize empty containers
    pokemon_data.clear();
    move_data.clear();
    pokemon_by_type.clear();
    moves_by_type.clear();
    moves_by_damage_class.clear();
}

PokemonData::LoadResult PokemonData::initialize(const std::string& pokemon_dir, 
                                               const std::string& moves_dir) {
    // Clear any existing data
    clearCache();
    
    // Store directory paths
    pokemon_directory = pokemon_dir;
    moves_directory = moves_dir;
    
    // Validate directory paths
    auto pokemon_path_result = InputValidator::validatePathWithinDataDirectory(pokemon_dir, {"pokemon"});
    if (!pokemon_path_result.isValid()) {
        return LoadResult(false, "Invalid Pokemon directory: " + pokemon_path_result.errorMessage);
    }
    
    auto moves_path_result = InputValidator::validatePathWithinDataDirectory(moves_dir, {"moves"});
    if (!moves_path_result.isValid()) {
        return LoadResult(false, "Invalid moves directory: " + moves_path_result.errorMessage);
    }
    
    // Load Pokemon data
    auto pokemon_result = loadPokemonData(pokemon_dir);
    if (!pokemon_result.success) {
        return LoadResult(false, "Failed to load Pokemon data: " + pokemon_result.error_message);
    }
    
    // Load move data
    auto move_result = loadMoveData(moves_dir);
    if (!move_result.success) {
        return LoadResult(false, "Failed to load move data: " + move_result.error_message);
    }
    
    // Organize data for efficient lookups
    organizeDataByTypes();
    
    is_initialized = true;
    
    return LoadResult(true, "Data loaded successfully", 
                     pokemon_result.loaded_count + move_result.loaded_count,
                     pokemon_result.failed_count + move_result.failed_count);
}

PokemonData::LoadResult PokemonData::reloadData() {
    if (!is_initialized) {
        return LoadResult(false, "PokemonData not initialized. Call initialize() first.");
    }
    
    return initialize(pokemon_directory, moves_directory);
}

PokemonData::LoadResult PokemonData::loadPokemonData(const std::string& directory) {
    int loaded_count = 0;
    int failed_count = 0;
    
    try {
        // Check if directory exists
        if (!std::filesystem::exists(directory)) {
            return LoadResult(false, "Pokemon directory does not exist: " + directory);
        }
        
        if (!std::filesystem::is_directory(directory)) {
            return LoadResult(false, "Path is not a directory: " + directory);
        }
        
        // Iterate through all JSON files in the directory
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                std::string file_path = entry.path().string();
                
                // Validate file accessibility
                auto file_validation = InputValidator::validateFileAccessibility(file_path);
                if (!file_validation.isValid()) {
                    std::cerr << "Skipping inaccessible Pokemon file: " << file_path 
                             << " (" << file_validation.errorMessage << ")" << std::endl;
                    failed_count++;
                    continue;
                }
                
                // Load the Pokemon file
                if (loadPokemonFile(file_path)) {
                    loaded_count++;
                } else {
                    failed_count++;
                }
            }
        }
        
        return LoadResult(true, "Pokemon data loaded", loaded_count, failed_count);
    }
    catch (const std::filesystem::filesystem_error& e) {
        return LoadResult(false, "Filesystem error loading Pokemon data: " + std::string(e.what()));
    }
    catch (const std::exception& e) {
        return LoadResult(false, "Error loading Pokemon data: " + std::string(e.what()));
    }
}

PokemonData::LoadResult PokemonData::loadMoveData(const std::string& directory) {
    int loaded_count = 0;
    int failed_count = 0;
    
    try {
        // Check if directory exists
        if (!std::filesystem::exists(directory)) {
            return LoadResult(false, "Moves directory does not exist: " + directory);
        }
        
        if (!std::filesystem::is_directory(directory)) {
            return LoadResult(false, "Path is not a directory: " + directory);
        }
        
        // Iterate through all JSON files in the directory
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                std::string file_path = entry.path().string();
                
                // Validate file accessibility
                auto file_validation = InputValidator::validateFileAccessibility(file_path);
                if (!file_validation.isValid()) {
                    std::cerr << "Skipping inaccessible move file: " << file_path 
                             << " (" << file_validation.errorMessage << ")" << std::endl;
                    failed_count++;
                    continue;
                }
                
                // Load the move file
                if (loadMoveFile(file_path)) {
                    loaded_count++;
                } else {
                    failed_count++;
                }
            }
        }
        
        return LoadResult(true, "Move data loaded", loaded_count, failed_count);
    }
    catch (const std::filesystem::filesystem_error& e) {
        return LoadResult(false, "Filesystem error loading move data: " + std::string(e.what()));
    }
    catch (const std::exception& e) {
        return LoadResult(false, "Error loading move data: " + std::string(e.what()));
    }
}

bool PokemonData::loadPokemonFile(const std::string& file_path) {
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open Pokemon file: " << file_path << std::endl;
            return false;
        }
        
        json pokemon_json;
        file >> pokemon_json;
        
        // Validate JSON structure
        if (!validatePokemonJson(pokemon_json)) {
            std::cerr << "Invalid Pokemon JSON structure: " << file_path << std::endl;
            return false;
        }
        
        // Extract and validate Pokemon data using InputValidator
        auto name_result = InputValidator::getJsonString(pokemon_json, "name", 1, 50);
        if (!name_result.isValid()) {
            std::cerr << "Invalid Pokemon name in file: " << file_path 
                     << " (" << name_result.errorMessage << ")" << std::endl;
            return false;
        }
        
        auto id_result = InputValidator::getJsonInt(pokemon_json, "id", 1, 9999);
        if (!id_result.isValid()) {
            std::cerr << "Invalid Pokemon ID in file: " << file_path 
                     << " (" << id_result.errorMessage << ")" << std::endl;
            return false;
        }
        
        // Extract types
        std::vector<std::string> types;
        if (pokemon_json.contains("types") && pokemon_json["types"].is_array()) {
            for (const auto& type : pokemon_json["types"]) {
                if (type.is_string()) {
                    std::string type_str = type.get<std::string>();
                    if (InputValidator::isAlphanumericSafe(type_str)) {
                        types.push_back(type_str);
                    }
                }
            }
        }
        
        if (types.empty()) {
            std::cerr << "No valid types found for Pokemon: " << file_path << std::endl;
            return false;
        }
        
        // Extract base stats
        if (!pokemon_json.contains("base_stats") || !pokemon_json["base_stats"].is_object()) {
            std::cerr << "Missing base_stats in Pokemon file: " << file_path << std::endl;
            return false;
        }
        
        const auto& base_stats = pokemon_json["base_stats"];
        
        auto hp_result = InputValidator::getJsonInt(base_stats, "hp", 1, 255);
        auto attack_result = InputValidator::getJsonInt(base_stats, "attack", 1, 255);
        auto defense_result = InputValidator::getJsonInt(base_stats, "defense", 1, 255);
        auto sp_attack_result = InputValidator::getJsonInt(base_stats, "special-attack", 1, 255);
        auto sp_defense_result = InputValidator::getJsonInt(base_stats, "special-defense", 1, 255);
        auto speed_result = InputValidator::getJsonInt(base_stats, "speed", 1, 255);
        
        if (!hp_result.isValid() || !attack_result.isValid() || !defense_result.isValid() ||
            !sp_attack_result.isValid() || !sp_defense_result.isValid() || !speed_result.isValid()) {
            std::cerr << "Invalid base stats in Pokemon file: " << file_path << std::endl;
            return false;
        }
        
        // Create PokemonInfo and store it
        PokemonInfo pokemon_info(
            name_result.value,
            id_result.value,
            types,
            hp_result.value,
            attack_result.value,
            defense_result.value,
            sp_attack_result.value,
            sp_defense_result.value,
            speed_result.value
        );
        
        std::string normalized_name = normalizeName(name_result.value);
        pokemon_data[normalized_name] = pokemon_info;
        
        return true;
    }
    catch (const json::exception& e) {
        std::cerr << "JSON parsing error in Pokemon file " << file_path << ": " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading Pokemon file " << file_path << ": " << e.what() << std::endl;
        return false;
    }
}

bool PokemonData::loadMoveFile(const std::string& file_path) {
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open move file: " << file_path << std::endl;
            return false;
        }
        
        json move_json;
        file >> move_json;
        
        // Validate JSON structure
        if (!validateMoveJson(move_json)) {
            std::cerr << "Invalid move JSON structure: " << file_path << std::endl;
            return false;
        }
        
        // Extract and validate move data using InputValidator
        auto name_result = InputValidator::getJsonString(move_json, "name", 1, 50);
        if (!name_result.isValid()) {
            std::cerr << "Invalid move name in file: " << file_path 
                     << " (" << name_result.errorMessage << ")" << std::endl;
            return false;
        }
        
        // Handle nullable fields with proper defaults
        auto accuracy_result = InputValidator::getJsonInt(move_json, "accuracy", 0, 100, 100);
        
        // Handle power field which can be null (for status moves)
        auto power_result = InputValidator::ValidationResult<int>(0);
        if (move_json.contains("power") && !move_json["power"].is_null()) {
            power_result = InputValidator::getJsonInt(move_json, "power", 0, 250, 0);
        }
        
        auto pp_result = InputValidator::getJsonInt(move_json, "pp", 1, 40, 10);
        auto priority_result = InputValidator::getJsonInt(move_json, "priority", -6, 6, 0);
        
        if (!accuracy_result.isValid() || !power_result.isValid() || 
            !pp_result.isValid() || !priority_result.isValid()) {
            std::cerr << "Invalid move stats in file: " << file_path << std::endl;
            return false;
        }
        
        // Extract damage class and type
        std::string damage_class = "physical";
        std::string move_type = "normal";
        
        if (move_json.contains("damage_class") && move_json["damage_class"].is_object()) {
            auto dc_result = InputValidator::getJsonString(move_json["damage_class"], "name", 1, 20, "physical");
            if (dc_result.isValid()) {
                damage_class = dc_result.value;
            }
        }
        
        if (move_json.contains("type") && move_json["type"].is_object()) {
            auto type_result = InputValidator::getJsonString(move_json["type"], "name", 1, 20, "normal");
            if (type_result.isValid()) {
                move_type = type_result.value;
            }
        }
        
        // Extract additional move info
        std::string category = "damage";
        std::string ailment_name = "none";
        int ailment_chance = 0;
        
        if (move_json.contains("Info") && move_json["Info"].is_object()) {
            const auto& info = move_json["Info"];
            
            if (info.contains("category") && info["category"].is_object()) {
                auto cat_result = InputValidator::getJsonString(info["category"], "name", 1, 30, "damage");
                if (cat_result.isValid()) {
                    category = cat_result.value;
                }
            }
            
            if (info.contains("ailment") && info["ailment"].is_object()) {
                auto ailment_result = InputValidator::getJsonString(info["ailment"], "name", 1, 20, "none");
                if (ailment_result.isValid()) {
                    ailment_name = ailment_result.value;
                }
            }
            
            auto ailment_chance_result = InputValidator::getJsonInt(info, "ailment_chance", 0, 100, 0);
            if (ailment_chance_result.isValid()) {
                ailment_chance = ailment_chance_result.value;
            }
        }
        
        // Create MoveInfo and store it
        MoveInfo move_info(
            name_result.value,
            accuracy_result.value,
            power_result.value,
            pp_result.value,
            move_type,
            damage_class,
            category,
            priority_result.value,
            ailment_name,
            ailment_chance
        );
        
        std::string normalized_name = normalizeName(name_result.value);
        move_data[normalized_name] = move_info;
        
        return true;
    }
    catch (const json::exception& e) {
        std::cerr << "JSON parsing error in move file " << file_path << ": " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading move file " << file_path << ": " << e.what() << std::endl;
        return false;
    }
}

void PokemonData::organizeDataByTypes() {
    // Clear existing type mappings
    pokemon_by_type.clear();
    moves_by_type.clear();
    moves_by_damage_class.clear();
    
    // Organize Pokemon by type
    for (const auto& [name, pokemon] : pokemon_data) {
        for (const auto& type : pokemon.types) {
            pokemon_by_type[type].push_back(pokemon.name);
        }
    }
    
    // Organize moves by type and damage class
    for (const auto& [name, move] : move_data) {
        moves_by_type[move.type].push_back(move.name);
        moves_by_damage_class[move.damage_class].push_back(move.name);
    }
}

std::string PokemonData::normalizeName(const std::string& name) const {
    std::string normalized = name;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    return normalized;
}

bool PokemonData::validatePokemonJson(const nlohmann::json& json_data) const {
    // Check required fields
    std::vector<std::string> required_fields = {"name", "id", "types", "base_stats"};
    auto validation_result = InputValidator::validateRequiredFields(json_data, required_fields);
    if (!validation_result.isValid()) {
        return false;
    }
    
    // Check base_stats structure
    if (!json_data["base_stats"].is_object()) {
        return false;
    }
    
    std::vector<std::string> required_stats = {"hp", "attack", "defense", "special-attack", "special-defense", "speed"};
    auto stats_validation = InputValidator::validateRequiredFields(json_data["base_stats"], required_stats);
    return stats_validation.isValid();
}

bool PokemonData::validateMoveJson(const nlohmann::json& json_data) const {
    // Check required fields
    std::vector<std::string> required_fields = {"name"};
    auto validation_result = InputValidator::validateRequiredFields(json_data, required_fields);
    return validation_result.isValid();
}

// Public interface methods
std::vector<std::string> PokemonData::getAvailablePokemon() const {
    std::vector<std::string> pokemon_names;
    pokemon_names.reserve(pokemon_data.size());
    
    for (const auto& [name, pokemon] : pokemon_data) {
        pokemon_names.push_back(pokemon.name);
    }
    
    std::sort(pokemon_names.begin(), pokemon_names.end());
    return pokemon_names;
}

std::optional<PokemonData::PokemonInfo> PokemonData::getPokemonInfo(const std::string& name) const {
    std::string normalized = normalizeName(name);
    auto it = pokemon_data.find(normalized);
    if (it != pokemon_data.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool PokemonData::hasPokemon(const std::string& name) const {
    std::string normalized = normalizeName(name);
    return pokemon_data.find(normalized) != pokemon_data.end();
}

std::vector<std::string> PokemonData::getPokemonByType(const std::string& type) const {
    auto it = pokemon_by_type.find(type);
    if (it != pokemon_by_type.end()) {
        return it->second;
    }
    return {};
}

std::vector<std::string> PokemonData::getAvailableMoves() const {
    std::vector<std::string> move_names;
    move_names.reserve(move_data.size());
    
    for (const auto& [name, move] : move_data) {
        move_names.push_back(move.name);
    }
    
    std::sort(move_names.begin(), move_names.end());
    return move_names;
}

std::optional<PokemonData::MoveInfo> PokemonData::getMoveInfo(const std::string& name) const {
    std::string normalized = normalizeName(name);
    auto it = move_data.find(normalized);
    if (it != move_data.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool PokemonData::hasMove(const std::string& name) const {
    std::string normalized = normalizeName(name);
    return move_data.find(normalized) != move_data.end();
}

std::vector<std::string> PokemonData::getMovesByType(const std::string& type) const {
    auto it = moves_by_type.find(type);
    if (it != moves_by_type.end()) {
        return it->second;
    }
    return {};
}

std::vector<std::string> PokemonData::getMovesByDamageClass(const std::string& damage_class) const {
    auto it = moves_by_damage_class.find(damage_class);
    if (it != moves_by_damage_class.end()) {
        return it->second;
    }
    return {};
}

bool PokemonData::validateTeamEntry(const std::string& pokemon_name, 
                                   const std::vector<std::string>& move_names) const {
    // Validate Pokemon name
    if (!hasPokemon(pokemon_name)) {
        return false;
    }
    
    // Validate all move names
    for (const auto& move_name : move_names) {
        if (!hasMove(move_name)) {
            return false;
        }
    }
    
    return true;
}

std::vector<std::string> PokemonData::suggestMovesForPokemon(const std::string& pokemon_name, int count) const {
    std::vector<std::string> suggested_moves;
    
    auto pokemon_info = getPokemonInfo(pokemon_name);
    if (!pokemon_info.has_value()) {
        return suggested_moves;
    }
    
    count = std::min(count, 4); // Maximum 4 moves
    
    // Get moves of the same type as the Pokemon (STAB moves)
    for (const auto& type : pokemon_info->types) {
        auto type_moves = getMovesByType(type);
        for (const auto& move : type_moves) {
            auto move_info = getMoveInfo(move);
            if (move_info.has_value() && move_info->power > 0) { // Damaging moves
                suggested_moves.push_back(move);
                if (suggested_moves.size() >= static_cast<size_t>(count)) {
                    return suggested_moves;
                }
            }
        }
    }
    
    // Fill remaining slots with high-power moves of other types
    auto all_moves = getAvailableMoves();
    for (const auto& move : all_moves) {
        auto move_info = getMoveInfo(move);
        if (move_info.has_value() && move_info->power >= 80) {
            // Check if we already have this move
            if (std::find(suggested_moves.begin(), suggested_moves.end(), move) == suggested_moves.end()) {
                suggested_moves.push_back(move);
                if (suggested_moves.size() >= static_cast<size_t>(count)) {
                    break;
                }
            }
        }
    }
    
    return suggested_moves;
}

double PokemonData::getTypeEffectiveness(const std::string& attacking_type,
                                        const std::vector<std::string>& defending_types) const {
    auto type_chart = getTypeChart();
    double effectiveness = 1.0;
    
    for (const auto& defending_type : defending_types) {
        auto attacking_it = type_chart.find(attacking_type);
        if (attacking_it != type_chart.end()) {
            auto defending_it = attacking_it->second.find(defending_type);
            if (defending_it != attacking_it->second.end()) {
                effectiveness *= defending_it->second;
            }
        }
    }
    
    return effectiveness;
}

std::string PokemonData::getDataStatistics() const {
    std::ostringstream stats;
    stats << "Pokemon Data Statistics:\n";
    stats << "  Pokemon loaded: " << pokemon_data.size() << "\n";
    stats << "  Moves loaded: " << move_data.size() << "\n";
    stats << "  Types represented: " << pokemon_by_type.size() << "\n";
    stats << "  Move damage classes: " << moves_by_damage_class.size() << "\n";
    return stats.str();
}

void PokemonData::clearCache() {
    pokemon_data.clear();
    move_data.clear();
    pokemon_by_type.clear();
    moves_by_type.clear();
    moves_by_damage_class.clear();
    is_initialized = false;
}

std::unordered_map<std::string, std::unordered_map<std::string, double>> PokemonData::getTypeChart() const {
    // Simplified type effectiveness chart - in a real implementation this would be more complete
    std::unordered_map<std::string, std::unordered_map<std::string, double>> type_chart;
    
    // Fire type effectiveness
    type_chart["fire"]["grass"] = 2.0;
    type_chart["fire"]["ice"] = 2.0;
    type_chart["fire"]["bug"] = 2.0;
    type_chart["fire"]["steel"] = 2.0;
    type_chart["fire"]["fire"] = 0.5;
    type_chart["fire"]["water"] = 0.5;
    type_chart["fire"]["rock"] = 0.5;
    type_chart["fire"]["dragon"] = 0.5;
    
    // Water type effectiveness
    type_chart["water"]["fire"] = 2.0;
    type_chart["water"]["ground"] = 2.0;
    type_chart["water"]["rock"] = 2.0;
    type_chart["water"]["water"] = 0.5;
    type_chart["water"]["grass"] = 0.5;
    type_chart["water"]["dragon"] = 0.5;
    
    // Grass type effectiveness
    type_chart["grass"]["water"] = 2.0;
    type_chart["grass"]["ground"] = 2.0;
    type_chart["grass"]["rock"] = 2.0;
    type_chart["grass"]["fire"] = 0.5;
    type_chart["grass"]["grass"] = 0.5;
    type_chart["grass"]["poison"] = 0.5;
    type_chart["grass"]["flying"] = 0.5;
    type_chart["grass"]["bug"] = 0.5;
    type_chart["grass"]["dragon"] = 0.5;
    type_chart["grass"]["steel"] = 0.5;
    
    // Electric type effectiveness
    type_chart["electric"]["water"] = 2.0;
    type_chart["electric"]["flying"] = 2.0;
    type_chart["electric"]["electric"] = 0.5;
    type_chart["electric"]["grass"] = 0.5;
    type_chart["electric"]["dragon"] = 0.5;
    type_chart["electric"]["ground"] = 0.0;
    
    // Add more type relationships as needed...
    
    return type_chart;
}