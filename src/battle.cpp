#include "battle.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>

Battle::Battle(const Team &playerTeam, const Team &opponentTeam)
    : playerTeam(playerTeam), opponentTeam(opponentTeam),
      selectedPokemon(nullptr), opponentSelectedPokemon(nullptr),
      rng(std::random_device{}()), criticalDistribution(0.0, 1.0) {
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
    const auto *pokemon = playerTeam.getPokemon(i);
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
      auto *pokemon = playerTeam.getPokemon(chosenPokemonNum - 1);
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
  auto alivePokemon = opponentTeam.getAlivePokemon();
  if (!alivePokemon.empty()) {
    auto randomIndex = rand() % alivePokemon.size();
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

  // Calculate damage with all effects
  auto damageResult = calculateDamageWithEffects(attacker, defender, move);

  // Calculate type effectiveness for display purposes
  auto typeMultiplier =
      TypeEffectiveness::getEffectivenessMultiplier(move.type, defender.types);

  defender.takeDamage(damageResult.damage);

  std::cout << "It dealt " << damageResult.damage << " damage!";

  // Display special effect messages
  if (damageResult.wasCritical) {
    std::cout << " A critical hit!";
  }

  if (damageResult.hadSTAB) {
    std::cout << " " << attacker.name << " gets STAB!";
  }

  // Display type effectiveness message
  if (typeMultiplier > 1.0) {
    std::cout << " It's super effective!";
  } else if (typeMultiplier < 1.0 && typeMultiplier > 0.0) {
    std::cout << " It's not very effective...";
  } else if (typeMultiplier == 0.0) {
    std::cout << " It had no effect!";
  }

  std::cout << "\n";
}

Battle::DamageResult Battle::calculateDamageWithEffects(
    const Pokemon &attacker, const Pokemon &defender, const Move &move) const {
  // Status moves don't deal damage
  if (move.power <= 0) {
    return {0, false, false};
  }

  // Base damage calculation
  int baseDamage = 0;
  if (move.damage_class == "physical") {
    baseDamage = (attacker.attack - defender.defense) + move.power;
  } else if (move.damage_class == "special") {
    baseDamage =
        (attacker.special_attack - defender.special_defense) + move.power;
  }

  // Ensure minimum base damage
  baseDamage = std::max(1, baseDamage);

  // Apply type effectiveness
  double typeMultiplier =
      TypeEffectiveness::getEffectivenessMultiplier(move.type, defender.types);

  // Check for STAB and critical hit
  auto hasStab = hasSTAB(attacker, move);
  auto isCrit = isCriticalHit(move);

  // Apply STAB (Same Type Attack Bonus) - 1.5x damage if move type matches
  // attacker type
  auto stabMultiplier = hasStab ? 1.5 : 1.0;

  // Apply critical hit multiplier - 2x damage on critical hit
  auto criticalMultiplier = isCrit ? 2.0 : 1.0;

  // Calculate final damage with all multipliers
  double finalDamage =
      baseDamage * typeMultiplier * stabMultiplier * criticalMultiplier;

  return {std::max(1, static_cast<int>(finalDamage)), isCrit, hasStab};
}

int Battle::calculateDamage(const Pokemon &attacker, const Pokemon &defender,
                            const Move &move) const {
  // Status moves don't deal damage
  if (move.power <= 0) {
    return 0;
  }

  // Base damage calculation
  int baseDamage = 0;
  if (move.damage_class == "physical") {
    baseDamage = (attacker.attack - defender.defense) + move.power;
  } else if (move.damage_class == "special") {
    baseDamage =
        (attacker.special_attack - defender.special_defense) + move.power;
  }

  // Ensure minimum base damage
  baseDamage = std::max(1, baseDamage);

  // Apply type effectiveness
  double typeMultiplier =
      TypeEffectiveness::getEffectivenessMultiplier(move.type, defender.types);

  // Apply STAB (Same Type Attack Bonus) - 1.5x damage if move type matches
  // attacker type
  double stabMultiplier = calculateSTABMultiplier(attacker, move);

  // Apply critical hit multiplier - 2x damage on critical hit
  double criticalMultiplier = calculateCriticalMultiplier(move);

  // Calculate final damage with all multipliers
  double finalDamage =
      baseDamage * typeMultiplier * stabMultiplier * criticalMultiplier;

  return std::max(1,
                  static_cast<int>(finalDamage)); // Ensure minimum damage is 1
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
              << " (Type: " << move.type << ", Power: " << move.power
              << ", Accuracy: " << move.accuracy
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
      auto *newPokemon = playerTeam.getFirstAlivePokemon();
      if (newPokemon) {
        selectedPokemon = newPokemon;
        std::cout << "\nYou send out " << selectedPokemon->name << "!\n";
      }
    }

    if (!opponentSelectedPokemon->isAlive()) {
      std::cout << "\nOpponent's " << opponentSelectedPokemon->name
                << " has fainted!" << std::endl;
      auto *newPokemon = opponentTeam.getFirstAlivePokemon();
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

// STAB (Same Type Attack Bonus) implementation
bool Battle::hasSTAB(const Pokemon &attacker, const Move &move) const {
  // Check if the move type matches any of the attacker's types
  for (const std::string &type : attacker.types) {
    if (type == move.type) {
      return true;
    }
  }
  return false;
}

double Battle::calculateSTABMultiplier(const Pokemon &attacker,
                                       const Move &move) const {
  return hasSTAB(attacker, move) ? 1.5 : 1.0;
}

// Critical Hit implementation
bool Battle::isCriticalHit(const Move &move) const {
  // Base critical hit ratio is 1/16 (6.25%)
  double criticalRatio = 1.0 / 16.0;

  // Some moves have higher critical hit ratios
  if (move.crit_rate > 0) {
    // Moves with high critical hit ratio (like Slash, Razor Leaf) have 1/8
    // chance
    criticalRatio = 1.0 / 8.0;
  }

  // Generate random number and check if it's a critical hit
  return criticalDistribution(rng) < criticalRatio;
}

double Battle::calculateCriticalMultiplier(const Move &move) const {
  return isCriticalHit(move) ? 2.0 : 1.0;
}