#include "team.h"

using json = nlohmann::json;

void Team::loadTeams(
    const std::unordered_map<std::string, std::vector<std::string>>
        &selectedTeams,
    const std::unordered_map<
        std::string,
        std::vector<std::pair<std::string, std::vector<std::string>>>>
        &selectedMoves,
    const std::string &selectedTeamName) {
  int PokemonCount = 0;

  // Get the selected Pokémon names for the chosen team
  auto teamIt = selectedTeams.find(selectedTeamName);
  if (teamIt != selectedTeams.end()) {
    const auto &teamPokemons = teamIt->second;

    // Loop over each Pokémon in the selected team
    for (const auto &pokemonName : teamPokemons) {
      // Create a Pokémon object and load its details from JSON
      auto pokeObj = Pokemon(pokemonName);

      // Check if moves for this Pokémon exist in selectedMoves
      auto movesIt = selectedMoves.find(selectedTeamName);
      if (movesIt != selectedMoves.end()) {
        for (const auto &movePair : movesIt->second) {
          if (movePair.first == pokemonName) {
            for (const auto &move : movePair.second) {
              // Load moves for this Pokémon
              auto moveObj = Move(move);
              pokeObj.moves.push_back(moveObj);
            }
          }
        }
      }

      // Add the Pokémon to the team
      pokemonTeam[PokemonCount] = pokeObj;
      PokemonCount++;
    }
  }
}

void Team::addPokemon(const Pokemon& pokemon) {
  int index = pokemonTeam.size();
  pokemonTeam[index] = pokemon;
}

Pokemon *Team::getPokemon(int index) {
  auto it = pokemonTeam.find(index);
  return (it != pokemonTeam.end()) ? &it->second : nullptr;
}

const Pokemon *Team::getPokemon(int index) const {
  auto it = pokemonTeam.find(index);
  return (it != pokemonTeam.end()) ? &it->second : nullptr;
}

bool Team::hasAlivePokemon() const {
  for (const auto &pair : pokemonTeam) {
    if (pair.second.isAlive()) {
      return true;
    }
  }
  return false;
}

std::vector<Pokemon *> Team::getAlivePokemon() {
  auto alivePokemon = std::vector<Pokemon *>{};
  for (auto &pair : pokemonTeam) {
    if (pair.second.isAlive()) {
      alivePokemon.push_back(&pair.second);
    }
  }
  return alivePokemon;
}

Pokemon *Team::getFirstAlivePokemon() {
  for (auto &pair : pokemonTeam) {
    if (pair.second.isAlive()) {
      return &pair.second;
    }
  }
  return nullptr;
}