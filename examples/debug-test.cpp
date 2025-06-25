#include "team.h"
#include <iostream>

int main() {
  std::cout << "Testing Team loading..." << std::endl;

  // Simple test data
  std::unordered_map<std::string, std::vector<std::string>> teams = {
      {"Test Team", {"venusaur"}}};

  std::unordered_map<
      std::string,
      std::vector<std::pair<std::string, std::vector<std::string>>>>
      moves = {{"Test Team", {{"venusaur", {"sludge-bomb"}}}}};

  Team testTeam;
  std::cout << "Loading team..." << std::endl;
  testTeam.loadTeams(teams, moves, "Test Team");

  std::cout << "Team loaded successfully! Size: " << testTeam.size()
            << std::endl;

  const Pokemon *pokemon = testTeam.getPokemon(0);
  if (pokemon) {
    std::cout << "Pokemon found: " << pokemon->name << std::endl;
    std::cout << "HP: " << pokemon->hp << std::endl;
    std::cout << "Moves count: " << pokemon->moves.size() << std::endl;

    if (!pokemon->moves.empty()) {
      std::cout << "First move: " << pokemon->moves[0].name << std::endl;
    }
  } else {
    std::cout << "No Pokemon found!" << std::endl;
  }

  return 0;
}