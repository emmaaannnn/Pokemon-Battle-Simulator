#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include "json.hpp"
#include "input_validator.h"

/**
 * @brief Manages Pokemon and move data loading with security validation
 * 
 * This class provides secure access to Pokemon and move data from the data directory.
 * It uses InputValidator for all file operations and maintains caches of available
 * Pokemon and moves for efficient team building operations.
 */
class PokemonData {
public:
    /**
     * @brief Information about a Pokemon for team building
     */
    struct PokemonInfo {
        std::string name;
        int id;
        std::vector<std::string> types;
        int hp;
        int attack;
        int defense;
        int special_attack;
        int special_defense;
        int speed;
        
        PokemonInfo() = default;
        PokemonInfo(const std::string& name, int id, const std::vector<std::string>& types,
                   int hp, int attack, int defense, int special_attack, int special_defense, int speed)
            : name(name), id(id), types(types), hp(hp), attack(attack), defense(defense),
              special_attack(special_attack), special_defense(special_defense), speed(speed) {}
    };

    /**
     * @brief Information about a move for team building
     */
    struct MoveInfo {
        std::string name;
        int accuracy;
        int power;
        int pp;
        std::string type;
        std::string damage_class;
        std::string category;
        int priority;
        std::string ailment_name;
        int ailment_chance;
        
        MoveInfo() = default;
        MoveInfo(const std::string& name, int accuracy, int power, int pp, 
                const std::string& type, const std::string& damage_class,
                const std::string& category, int priority, const std::string& ailment_name,
                int ailment_chance)
            : name(name), accuracy(accuracy), power(power), pp(pp), type(type),
              damage_class(damage_class), category(category), priority(priority),
              ailment_name(ailment_name), ailment_chance(ailment_chance) {}
    };

    /**
     * @brief Result of data loading operations
     */
    struct LoadResult {
        bool success;
        std::string error_message;
        int loaded_count;
        int failed_count;
        
        LoadResult(bool success = true, const std::string& message = "", 
                  int loaded = 0, int failed = 0)
            : success(success), error_message(message), loaded_count(loaded), failed_count(failed) {}
    };

    // Constructor and initialization
    PokemonData();

    /**
     * @brief Initialize the data loader by scanning data directories
     * @param pokemon_dir Path to pokemon data directory (defaults to "data/pokemon")
     * @param moves_dir Path to moves data directory (defaults to "data/moves")
     * @return LoadResult indicating success/failure and counts
     */
    LoadResult initialize(const std::string& pokemon_dir = "data/pokemon",
                         const std::string& moves_dir = "data/moves");

    /**
     * @brief Reload all data from directories (useful for data updates)
     * @return LoadResult indicating success/failure and counts
     */
    LoadResult reloadData();

    // Pokemon data access
    /**
     * @brief Get list of all available Pokemon names
     * @return Vector of Pokemon names
     */
    std::vector<std::string> getAvailablePokemon() const;

    /**
     * @brief Get information about a specific Pokemon
     * @param name Pokemon name (case-insensitive)
     * @return Optional PokemonInfo if found
     */
    std::optional<PokemonInfo> getPokemonInfo(const std::string& name) const;

    /**
     * @brief Check if a Pokemon exists in the data
     * @param name Pokemon name (case-insensitive)
     * @return True if Pokemon exists
     */
    bool hasPokemon(const std::string& name) const;

    /**
     * @brief Get Pokemon by type
     * @param type Pokemon type (e.g., "fire", "water")
     * @return Vector of Pokemon names of that type
     */
    std::vector<std::string> getPokemonByType(const std::string& type) const;

    // Move data access
    /**
     * @brief Get list of all available move names
     * @return Vector of move names
     */
    std::vector<std::string> getAvailableMoves() const;

    /**
     * @brief Get information about a specific move
     * @param name Move name (case-insensitive)
     * @return Optional MoveInfo if found
     */
    std::optional<MoveInfo> getMoveInfo(const std::string& name) const;

    /**
     * @brief Check if a move exists in the data
     * @param name Move name (case-insensitive)
     * @return True if move exists
     */
    bool hasMove(const std::string& name) const;

    /**
     * @brief Get moves by type
     * @param type Move type (e.g., "fire", "water")
     * @return Vector of move names of that type
     */
    std::vector<std::string> getMovesByType(const std::string& type) const;

    /**
     * @brief Get moves by damage class
     * @param damage_class Move damage class ("physical", "special", "status")
     * @return Vector of move names of that damage class
     */
    std::vector<std::string> getMovesByDamageClass(const std::string& damage_class) const;

    // Team building utilities
    /**
     * @brief Check if Pokemon and move names are compatible for team building
     * @param pokemon_name Pokemon name
     * @param move_names Vector of move names
     * @return True if all names are valid and exist in data
     */
    bool validateTeamEntry(const std::string& pokemon_name, 
                          const std::vector<std::string>& move_names) const;

    /**
     * @brief Get suggested moves for a Pokemon based on type effectiveness
     * @param pokemon_name Pokemon name
     * @param count Number of moves to suggest (max 4)
     * @return Vector of suggested move names
     */
    std::vector<std::string> suggestMovesForPokemon(const std::string& pokemon_name, 
                                                   int count = 4) const;

    /**
     * @brief Get type effectiveness information
     * @param attacking_type The attacking move type
     * @param defending_types Vector of defending Pokemon types
     * @return Effectiveness multiplier (0.0, 0.5, 1.0, 2.0)
     */
    double getTypeEffectiveness(const std::string& attacking_type,
                               const std::vector<std::string>& defending_types) const;

    // Data statistics
    /**
     * @brief Get statistics about loaded data
     * @return String with data statistics
     */
    std::string getDataStatistics() const;

    /**
     * @brief Clear all cached data
     */
    void clearCache();

private:
    // Data storage
    std::unordered_map<std::string, PokemonInfo> pokemon_data;
    std::unordered_map<std::string, MoveInfo> move_data;
    
    // Directory paths
    std::string pokemon_directory;
    std::string moves_directory;
    
    // Type organization for quick lookups
    std::unordered_map<std::string, std::vector<std::string>> pokemon_by_type;
    std::unordered_map<std::string, std::vector<std::string>> moves_by_type;
    std::unordered_map<std::string, std::vector<std::string>> moves_by_damage_class;
    
    // Loading state
    bool is_initialized;
    
    // Helper methods
    /**
     * @brief Load all Pokemon data from directory
     * @param directory Path to Pokemon data directory
     * @return LoadResult with success/failure information
     */
    LoadResult loadPokemonData(const std::string& directory);
    
    /**
     * @brief Load all move data from directory
     * @param directory Path to moves data directory
     * @return LoadResult with success/failure information
     */
    LoadResult loadMoveData(const std::string& directory);
    
    /**
     * @brief Load a single Pokemon JSON file with error handling
     * @param file_path Path to Pokemon JSON file
     * @return True if loaded successfully
     */
    bool loadPokemonFile(const std::string& file_path);
    
    /**
     * @brief Load a single move JSON file with error handling
     * @param file_path Path to move JSON file
     * @return True if loaded successfully
     */
    bool loadMoveFile(const std::string& file_path);
    
    /**
     * @brief Organize data by types for quick lookups
     */
    void organizeDataByTypes();
    
    /**
     * @brief Normalize name for case-insensitive lookups
     * @param name Input name
     * @return Normalized lowercase name
     */
    std::string normalizeName(const std::string& name) const;
    
    /**
     * @brief Validate JSON has required fields for Pokemon
     * @param json_data JSON object to validate
     * @return True if valid Pokemon JSON
     */
    bool validatePokemonJson(const nlohmann::json& json_data) const;
    
    /**
     * @brief Validate JSON has required fields for Move
     * @param json_data JSON object to validate
     * @return True if valid Move JSON
     */
    bool validateMoveJson(const nlohmann::json& json_data) const;
    
    /**
     * @brief Get type effectiveness data (hardcoded for now)
     * @return Map of type effectiveness multipliers
     */
    std::unordered_map<std::string, std::unordered_map<std::string, double>> getTypeChart() const;
};