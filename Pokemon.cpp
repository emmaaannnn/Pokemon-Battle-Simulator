#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "json.hpp"
#include "Move.cpp"

using json = nlohmann::json;

class Pokemon {
public:
    public:
    // Basic info
    std::string name;
    int id;
    std::vector<std::string> types;

    // Base stats
    int hp;
    int attack;
    int defense;
    int special_attack;
    int special_defense;
    int speed;

    
    std::vector<Move> moves; 

    // Constructor to initialize from a JSON file
    Pokemon(const std::string& pokemonName) {
        // Construct the file path dynamically using the pokemonName
        loadFromJson("Pokemon2/" + pokemonName + ".json");
    }

    void loadFromJson(const std::string& file_path) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << file_path << std::endl;
            return;
        }

        json pokemon_json;
        file >> pokemon_json;

        // Map JSON data to class members
        name = pokemon_json.value("name", "");
        id = pokemon_json.value("id", 0);
        types = pokemon_json.value("types", std::vector<std::string>{});

        // Base stats mapping
        const auto& base_stats = pokemon_json["base_stats"];
        hp = base_stats.value("hp", 0);
        attack = base_stats.value("attack", 0);
        defense = base_stats.value("defense", 0);
        special_attack = base_stats.value("special-attack", 0);
        special_defense = base_stats.value("special-defense", 0);
        speed = base_stats.value("speed", 0);
    }
};