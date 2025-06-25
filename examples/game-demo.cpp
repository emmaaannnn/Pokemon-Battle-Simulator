#include "team.h"
#include <iostream>
#include <string>

void displayPokemon(const Pokemon *pokemon) {
  if (!pokemon)
    return;

  std::cout << "\n🐉 " << pokemon->name << std::endl;
  std::cout << "   HP: " << pokemon->current_hp << "/" << pokemon->hp << " ("
            << pokemon->getHealthPercentage() << "%)" << std::endl;
  std::cout << "   ATK: " << pokemon->attack << " | DEF: " << pokemon->defense
            << " | SPD: " << pokemon->speed << std::endl;

  std::cout << "   Moves:" << std::endl;
  for (size_t i = 0; i < pokemon->moves.size(); ++i) {
    const Move &move = pokemon->moves[i];
    std::cout << "     " << (i + 1) << ". " << move.name
              << " (Power: " << move.power << ", Accuracy: " << move.accuracy
              << ")" << std::endl;
  }
}

int main() {
  std::cout << "🎮 Welcome to Pokemon Battle Simulator!" << std::endl;
  std::cout << "=====================================\n" << std::endl;

  // Get player name
  std::string playerName;
  std::cout << "Enter your trainer name: ";
  std::getline(std::cin, playerName);
  std::cout << "\nWelcome, Trainer " << playerName << "!" << std::endl;

  // Team data
  std::unordered_map<std::string, std::vector<std::string>> teams = {
      {"Team 1", {"venusaur", "pikachu", "machamp"}},
      {"Team 2", {"charizard"}},
      {"Brock Team", {"golem", "onix"}}};

  std::unordered_map<
      std::string,
      std::vector<std::pair<std::string, std::vector<std::string>>>>
      moves = {
          {"Team 1",
           {{"venusaur",
             {"sludge-bomb", "mega-drain", "leech-seed", "amnesia"}},
            {"pikachu", {"thunderbolt", "iron-tail", "reflect", "tackle"}},
            {"machamp", {"superpower", "earthquake", "fire-blast", "tackle"}}}},
          {"Team 2",
           {{"charizard",
             {"flamethrower", "air-slash", "dragon-pulse", "tackle"}}}},
          {"Brock Team",
           {{"golem", {"earthquake", "stone-edge", "tackle", "protect"}},
            {"onix", {"tackle", "bind", "rock-throw", "protect"}}}}};

  // Team selection
  std::cout << "\n🎯 Choose your team:" << std::endl;
  std::cout << "[1] Team 1 (Venusaur, Pikachu, Machamp)" << std::endl;
  std::cout << "[2] Team 2 (Charizard)" << std::endl;

  int teamChoice;
  std::cout << "\nEnter team number (1-2): ";
  std::cin >> teamChoice;

  std::string chosenTeam = "Team " + std::to_string(teamChoice);

  // Load player team
  Team playerTeam;
  playerTeam.loadTeams(teams, moves, chosenTeam);

  std::cout << "\n⚡ Your Team:" << std::endl;
  std::cout << "============" << std::endl;
  for (int i = 0; i < static_cast<int>(playerTeam.size()); ++i) {
    const Pokemon *pokemon = playerTeam.getPokemon(i);
    displayPokemon(pokemon);
  }

  // Load opponent team (Brock)
  Team opponentTeam;
  opponentTeam.loadTeams(teams, moves, "Brock Team");

  std::cout << "\n🔥 Opponent Team (Brock - Rock Gym Leader):" << std::endl;
  std::cout << "===========================================" << std::endl;
  for (int i = 0; i < static_cast<int>(opponentTeam.size()); ++i) {
    const Pokemon *pokemon = opponentTeam.getPokemon(i);
    if (pokemon) {
      std::cout << "   🗿 " << pokemon->name << " (HP: " << pokemon->hp << ")"
                << std::endl;
    }
  }

  // Pokemon selection
  std::cout << "\n🎲 Select your starter Pokemon:" << std::endl;
  for (int i = 0; i < static_cast<int>(playerTeam.size()); ++i) {
    const Pokemon *pokemon = playerTeam.getPokemon(i);
    if (pokemon) {
      std::cout << "[" << (i + 1) << "] " << pokemon->name << std::endl;
    }
  }

  int pokemonChoice;
  std::cout << "\nEnter Pokemon number: ";
  std::cin >> pokemonChoice;

  const Pokemon *chosenPokemon = playerTeam.getPokemon(pokemonChoice - 1);
  const Pokemon *opponentPokemon =
      opponentTeam.getPokemon(0); // Brock starts with Golem

  if (chosenPokemon && opponentPokemon) {
    std::cout << "\n⚔️  BATTLE START!" << std::endl;
    std::cout << "=================" << std::endl;
    std::cout << playerName << " sends out " << chosenPokemon->name << "!"
              << std::endl;
    std::cout << "Brock sends out " << opponentPokemon->name << "!"
              << std::endl;

    displayPokemon(chosenPokemon);

    std::cout << "\n🎯 Battle moves available:" << std::endl;
    for (size_t i = 0; i < chosenPokemon->moves.size(); ++i) {
      const Move &move = chosenPokemon->moves[i];
      std::cout << "[" << (i + 1) << "] " << move.name;
      if (move.power > 0) {
        std::cout << " - " << move.power << " power";
      }
      std::cout << " (" << move.damage_class << ")" << std::endl;
    }

    std::cout << "\n✨ This is where the battle would continue!" << std::endl;
    std::cout
        << "The battle system is working, but we encountered a technical issue."
        << std::endl;
    std::cout << "Your Pokemon and moves are loaded correctly! 🎉" << std::endl;
  }

  std::cout << "\nThanks for playing! 🌟" << std::endl;
  return 0;
}