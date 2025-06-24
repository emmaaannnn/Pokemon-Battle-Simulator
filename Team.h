#pragma once

#include "Pokemon.h"
#include <string>
#include <unordered_map>
#include <vector>

class Team {
public:
  // Constructor
  Team() = default;

  // Team management
  void
  loadTeams(const std::unordered_map<std::string, std::vector<std::string>>
                &selectedTeams,
            const std::unordered_map<
                std::string,
                std::vector<std::pair<std::string, std::vector<std::string>>>>
                &selectedMoves,
            const std::string &selectedTeamName);

  // Getters
  Pokemon *getPokemon(int index);
  const Pokemon *getPokemon(int index) const;
  size_t size() const { return pokemonTeam.size(); }
  bool isEmpty() const { return pokemonTeam.empty(); }

  // Battle utilities
  bool hasAlivePokemon() const;
  std::vector<Pokemon *> getAlivePokemon();
  Pokemon *getFirstAlivePokemon();

  // Iterator support for range-based loops
  std::unordered_map<int, Pokemon>::iterator begin() {
    return pokemonTeam.begin();
  }
  std::unordered_map<int, Pokemon>::iterator end() { return pokemonTeam.end(); }
  std::unordered_map<int, Pokemon>::const_iterator begin() const {
    return pokemonTeam.begin();
  }
  std::unordered_map<int, Pokemon>::const_iterator end() const {
    return pokemonTeam.end();
  }

private:
  std::unordered_map<int, Pokemon> pokemonTeam;
};