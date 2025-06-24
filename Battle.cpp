#include "Battle.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>

Battle::Battle(const Team &playerTeam, const Team &opponentTeam)
    : playerTeam(playerTeam), opponentTeam(opponentTeam),
      selectedPokemon(nullptr), opponentSelectedPokemon(nullptr) {
  srand(time(0)); // Seed random number generator once
}

void Battle::displayHealth(const Pokemon &pokemon) const {
  double healthPercentage = pokemon.getHealthPercentage();
  std::cout << "\n"
            << pokemon.name << " HP: " << healthPercentage << "%" << std::endl;
}

void Battle::selectPokemon() {
  std::cout << "\nSelect the Pokémon you want to send out first:" << std::endl;

  // Display available Pokemon
  for (int i = 0; i < static_cast<int>(playerTeam.size()); ++i) {
    const Pokemon *pokemon = playerTeam.getPokemon(i);
    if (pokemon && pokemon->isAlive()) {
      std::cout << "[" << i + 1 << "] - " << pokemon->name << std::endl;
    }
  }

  int chosenPokemonNum;
  while (true) {
    std::cout << "\nEnter the number of the Pokémon you want to send out: ";
    std::cin >> chosenPokemonNum;

    if (chosenPokemonNum >= 1 &&
        chosenPokemonNum <= static_cast<int>(playerTeam.size())) {
      Pokemon *pokemon = playerTeam.getPokemon(chosenPokemonNum - 1);
      if (pokemon && pokemon->isAlive()) {
        selectedPokemon = pokemon;
        std::cout << "\nYou have selected " << selectedPokemon->name
                  << " to send out!" << std::endl;
        break;
      }
    }
    std::cout << "Invalid selection - try again." << std::endl;
  }
}

void Battle::selectOpponentPokemon() {
  std::vector<Pokemon *> alivePokemon = opponentTeam.getAlivePokemon();
  if (!alivePokemon.empty()) {
    int randomIndex = rand() % alivePokemon.size();
    opponentSelectedPokemon = alivePokemon[randomIndex];
    std::cout << "\nThe opponent has selected " << opponentSelectedPokemon->name
              << " to send out!" << std::endl;
  }
}

void Battle::executeMove(Pokemon &attacker, Pokemon &defender,
                         const Move &move) {
  std::cout << attacker.name << " used " << move.name << "!\n";

  if (move.damage_class == "status") {
    std::cout << "The move applies a status effect!\n";
    return;
  }

  // Check for accuracy
  int hitChance = rand() % 100 + 1;
  if (hitChance > move.accuracy) {
    std::cout << "The move missed!\n";
    return;
  }

  int damage = calculateDamage(attacker, defender, move);
  defender.takeDamage(damage);

  std::cout << "It dealt " << damage << " damage!\n";
}

int Battle::calculateDamage(const Pokemon &attacker, const Pokemon &defender,
                            const Move &move) const {
  int damage = 0;
  if (move.damage_class == "physical") {
    damage = (attacker.attack - defender.defense) + move.power;
  } else if (move.damage_class == "special") {
    damage = (attacker.special_attack - defender.special_defense) + move.power;
  }
  return std::max(1, damage); // Ensure minimum damage is 1
}

bool Battle::playerFirst(const Move &playerMove,
                         const Move &opponentMove) const {
  if (playerMove.priority != opponentMove.priority) {
    return playerMove.priority > opponentMove.priority;
  }
  if (selectedPokemon->speed != opponentSelectedPokemon->speed) {
    return selectedPokemon->speed > opponentSelectedPokemon->speed;
  }
  return rand() % 2; // Randomize if speeds are equal
}

int Battle::getMoveChoice() const {
  std::cout << "\nChoose a move:\n";
  for (size_t i = 0; i < selectedPokemon->moves.size(); ++i) {
    const Move &move = selectedPokemon->moves[i];
    std::cout << "    " << (i + 1) << ". " << move.name
              << " (Power: " << move.power << ", Accuracy: " << move.accuracy
              << ", Class: " << move.damage_class << ")\n";
  }

  int chosenMoveIndex;
  while (true) {
    std::cout << "\nSelect a move (1-" << selectedPokemon->moves.size()
              << "): ";
    std::cin >> chosenMoveIndex;

    if (chosenMoveIndex >= 1 &&
        chosenMoveIndex <= static_cast<int>(selectedPokemon->moves.size())) {
      return chosenMoveIndex - 1; // Return 0-based index
    }
    std::cout << "Invalid choice. Please select a valid move.\n";
  }
}

Battle::BattleResult Battle::getBattleResult() const {
  bool playerHasAlive = playerTeam.hasAlivePokemon();
  bool opponentHasAlive = opponentTeam.hasAlivePokemon();

  if (!playerHasAlive && !opponentHasAlive) {
    return BattleResult::DRAW;
  } else if (!playerHasAlive) {
    return BattleResult::OPPONENT_WINS;
  } else if (!opponentHasAlive) {
    return BattleResult::PLAYER_WINS;
  }
  return BattleResult::ONGOING;
}

bool Battle::isBattleOver() const {
  return getBattleResult() != BattleResult::ONGOING;
}

void Battle::startBattle() {
  std::cout << "\n====================== BATTLE START ======================"
            << std::endl;

  // Initial Pokemon selection
  selectOpponentPokemon();
  if (opponentSelectedPokemon) {
    displayHealth(*opponentSelectedPokemon);
  }

  selectPokemon();
  if (selectedPokemon) {
    displayHealth(*selectedPokemon);
  }

  // Main battle loop
  while (!isBattleOver()) {
    // Player chooses move
    int playerMoveIndex = getMoveChoice();
    Move playerMove = selectedPokemon->moves[playerMoveIndex];

    // Opponent chooses random move
    int opponentMoveIndex = rand() % opponentSelectedPokemon->moves.size();
    Move opponentMove = opponentSelectedPokemon->moves[opponentMoveIndex];

    // Determine turn order and execute moves
    if (playerFirst(playerMove, opponentMove)) {
      executeMove(*selectedPokemon, *opponentSelectedPokemon, playerMove);
      if (opponentSelectedPokemon->isAlive()) {
        executeMove(*opponentSelectedPokemon, *selectedPokemon, opponentMove);
      }
    } else {
      executeMove(*opponentSelectedPokemon, *selectedPokemon, opponentMove);
      if (selectedPokemon->isAlive()) {
        executeMove(*selectedPokemon, *opponentSelectedPokemon, playerMove);
      }
    }

    // Handle fainted Pokemon (simplified for now)
    if (!selectedPokemon->isAlive()) {
      std::cout << "\n"
                << selectedPokemon->name << " has fainted!" << std::endl;
      Pokemon *newPokemon = playerTeam.getFirstAlivePokemon();
      if (newPokemon) {
        selectedPokemon = newPokemon;
        std::cout << "\nYou send out " << selectedPokemon->name << "!\n";
      }
    }

    if (!opponentSelectedPokemon->isAlive()) {
      std::cout << "\nOpponent's " << opponentSelectedPokemon->name
                << " has fainted!" << std::endl;
      Pokemon *newPokemon = opponentTeam.getFirstAlivePokemon();
      if (newPokemon) {
        opponentSelectedPokemon = newPokemon;
        std::cout << "\nOpponent sends out " << opponentSelectedPokemon->name
                  << "!\n";
      }
    }
  }

  // Display battle result
  BattleResult result = getBattleResult();
  switch (result) {
  case BattleResult::PLAYER_WINS:
    std::cout << "\nAll opponent's Pokémon have fainted! You won the battle!\n";
    break;
  case BattleResult::OPPONENT_WINS:
    std::cout << "\nAll your Pokémon have fainted! You lost the battle.\n";
    break;
  case BattleResult::DRAW:
    std::cout << "\nIt's a draw! All Pokémon have fainted.\n";
    break;
  default:
    break;
  }
}