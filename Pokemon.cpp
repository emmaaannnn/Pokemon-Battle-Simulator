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
        name = pokemon_json["name"];
        id = pokemon_json["id"];
        types = pokemon_json["types"];

        // Base stats mapping
        const auto& base_stats = pokemon_json["base_stats"];
        hp = base_stats["hp"];
        attack = base_stats["attack"];
        defense = base_stats["defense"];
        special_attack = base_stats["special-attack"];
        special_defense = base_stats["special-defense"];
        speed = base_stats["speed"];
    }
};