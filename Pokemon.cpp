#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "json.hpp"
#include "Move.cpp"
using namespace std;
using json = nlohmann::json;

class Pokemon {
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

    //Array of Move objects for Move
    std::vector<Move> moves; 

    Pokemon() : name(""), id(0), hp(0), attack(0), defense(0), special_attack(0),
                special_defense(0), speed(0) {
    }

    // Constructor to initialize from a JSON file
    Pokemon(const std::string& pokemonName) {
        //loadFromJson("Pokemon2/" + pokemonName + ".json");
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
        
        // Accessing types which is an array of strings
        for (const auto& type : pokemon_json["types"]) {
            types.push_back(type);  // No need to access 'type' key inside, as it's already a string
        }

        // Accessing base_stats which is an object
        const auto& base_stats = pokemon_json["base_stats"];
        hp = base_stats["hp"];
        attack = base_stats["attack"];
        defense = base_stats["defense"];
        special_attack = base_stats["special-attack"];
        special_defense = base_stats["special-defense"];
        speed = base_stats["speed"];  

        // std::cout << "------------------------------------------------------------------"<< std::endl;
        // std::cout << "name: ";
        // std::cout << name << std::endl;

        // std::cout << "id: ";
        // std::cout << id << std::endl;

        // std::cout << "types: "; 
        // for (const auto& type : types){
        //     std::cout << type << std::endl;
        //  }
        // std::cout << "HP: ";
        // std::cout << hp << std::endl;

        // std::cout << "attack: ";
        // std::cout << attack << std::endl;

        // std::cout << "defense: ";
        // std::cout << defense << std::endl;

        // std::cout << "special_attack: ";
        // std::cout << special_attack << std::endl;

        // std::cout << "special_defense: ";
        // std::cout << special_defense << std::endl;

        // std::cout << "speed: ";
        // std::cout << speed << std::endl;
        // std::cout << "------------------------------------------------------------------" << std::endl;
    }

    // Load moves into a Pokémon
    void loadMoves() {
        std::ifstream file("Moves_Data/" + this->name + ".json");
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << "Moves_Data/" + this->name + ".json"<< std::endl;
            return;
        }
        json move_json;
        file >> move_json;
        for(const auto move : move_json){
            std::string moveName = move["move"]["name"];
            Move MoveObj = Move(moveName); 
            moves.push_back(MoveObj);
            }
        }   
    };
