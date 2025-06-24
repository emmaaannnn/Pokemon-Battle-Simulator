#include "Pokemon.h"

using json = nlohmann::json;

Pokemon::Pokemon()
    : name(""), id(0), hp(0), attack(0), defense(0), special_attack(0),
      special_defense(0), speed(0), fainted(false) {}

Pokemon::Pokemon(const std::string &pokemonName) {
  loadFromJson("Pokemon2/" + pokemonName + ".json");
}

void Pokemon::loadFromJson(const std::string &file_path) {
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
  for (const auto &type : pokemon_json["types"]) {
    types.push_back(
        type); // No need to access 'type' key inside, as it's already a string
  }

  // Accessing base_stats which is an object
  const auto &base_stats = pokemon_json["base_stats"];
  hp = base_stats["hp"];
  current_hp = hp;
  attack = base_stats["attack"];
  defense = base_stats["defense"];
  special_attack = base_stats["special-attack"];
  special_defense = base_stats["special-defense"];
  speed = base_stats["speed"];
  fainted = false;
}

void Pokemon::loadMoves() {
  std::ifstream file("Moves_Data/" + name + ".json");
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << "Moves_Data/" + name + ".json"
              << std::endl;
    return;
  }
  json move_json;
  file >> move_json;
  for (const auto &move : move_json) {
    std::string moveName = move["move"]["name"];
    Move MoveObj = Move(moveName);
    moves.push_back(MoveObj);
  }
}

double Pokemon::getHealthPercentage() const {
  return (static_cast<double>(current_hp) / hp) * 100.0;
}

void Pokemon::takeDamage(int damage) {
  current_hp = std::max(0, current_hp - damage);
  if (current_hp == 0) {
    fainted = true;
  }
}

void Pokemon::heal(int amount) {
  current_hp = std::min(hp, current_hp + amount);
  if (current_hp > 0) {
    fainted = false;
  }
}
