#include "battle.h"
#include "move_type_mapping.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

Battle::Battle(const Team &playerTeam, const Team &opponentTeam)
    : playerTeam(playerTeam), opponentTeam(opponentTeam),
      selectedPokemon(nullptr), opponentSelectedPokemon(nullptr),
      rng(std::random_device{}()), criticalDistribution(0.0, 1.0) {
  srand(time(0)); // Seed random number generator once
}

void Battle::displayHealth(const Pokemon &pokemon) const {
  std::cout << pokemon.name << " HP: ";

  int barLength = 20;
  double healthPercent = pokemon.getHealthPercentage();
  int filledBars = static_cast<int>(healthPercent * barLength / 100);

  // Use Unicode blocks on Unix/Mac, ASCII on Windows for compatibility
#ifdef _WIN32
  const char *filledChar = "=";
  const char *emptyChar = "-";
#else
  const char *filledChar = "█";
  const char *emptyChar = "░";
#endif

  std::cout << "[";
  for (int i = 0; i < barLength; ++i) {
    if (i < filledBars) {
      std::cout << filledChar;
    } else {
      std::cout << emptyChar;
    }
  }
  std::cout << "] " << static_cast<int>(healthPercent) << "%";

  // Show status condition
  if (pokemon.hasStatusCondition()) {
    std::cout << " (" << pokemon.getStatusConditionName() << ")";
  }

  std::cout << std::endl;
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
  // Check if attacker can act (not asleep, frozen, or fully paralyzed)
  if (!attacker.canAct()) {
    if (attacker.status == StatusCondition::PARALYSIS) {
      std::cout << attacker.name << " is paralyzed and can't move!"
                << std::endl;
    }
    return;
  }

  if (move.power == -1 || move.power == 0) {
    // Status move or special move
    std::cout << attacker.name << " used " << move.name << "!" << std::endl;

    // Apply status condition if move has one
    StatusCondition statusToApply = move.getStatusCondition();
    if (statusToApply != StatusCondition::NONE) {
      // Check if status effect proc'd based on ailment_chance
      bool statusApplied = false;

      if (move.category == "ailment") {
        // Pure status moves have 100% chance (unless they miss)
        statusApplied = true;
      } else if (move.ailment_chance > 0) {
        // Damage + ailment moves have specified chance
        auto distribution = std::uniform_int_distribution<int>(1, 100);
        statusApplied = distribution(rng) <= move.ailment_chance;
      }

      if (statusApplied && !defender.hasStatusCondition()) {
        defender.applyStatusCondition(statusToApply);
        std::cout << defender.name << " is now "
                  << defender.getStatusConditionName() << "!" << std::endl;
      } else if (statusApplied && defender.hasStatusCondition()) {
        std::cout << "But it failed! " << defender.name
                  << " is already affected by a status condition." << std::endl;
      }
    } else {
      std::cout << "The move applies a status effect!" << std::endl;
    }
  } else {
    // Damage-dealing move
    auto damageResult = calculateDamageWithEffects(attacker, defender, move);

    std::cout << attacker.name << " used " << move.name << "!" << std::endl;
    std::cout << "It dealt " << damageResult.damage << " damage!";

    if (damageResult.hadSTAB) {
      std::cout << " " << attacker.name << " gets STAB!";
    }

    if (damageResult.wasCritical) {
      std::cout << " A critical hit!";
    }

    // Check type effectiveness for message
    auto typeMultiplier = TypeEffectiveness::getEffectivenessMultiplier(
        MoveTypeMapping::getMoveType(move.name), defender.types);

    if (typeMultiplier > 1.0) {
      std::cout << " It's super effective!";
    } else if (typeMultiplier < 1.0 && typeMultiplier > 0.0) {
      std::cout << " It's not very effective...";
    } else if (typeMultiplier == 0.0) {
      std::cout << " It has no effect!";
    }

    std::cout << std::endl;

    defender.takeDamage(damageResult.damage);

    // Apply flinch effect if move has flinch chance and defender is still alive
    if (move.flinch_chance > 0 && defender.isAlive()) {
      auto flinchDistribution = std::uniform_int_distribution<int>(1, 100);
      if (flinchDistribution(rng) <= move.flinch_chance) {
        defender.applyStatusCondition(StatusCondition::FLINCH);
        std::cout << defender.name << " flinched!" << std::endl;
      }
    }

    // Apply status condition from damage moves
    StatusCondition statusToApply = move.getStatusCondition();
    if (statusToApply != StatusCondition::NONE && move.ailment_chance > 0) {
      auto distribution = std::uniform_int_distribution<int>(1, 100);
      if (distribution(rng) <= move.ailment_chance &&
          !defender.hasStatusCondition()) {
        defender.applyStatusCondition(statusToApply);
        std::cout << defender.name << " is now "
                  << defender.getStatusConditionName() << "!" << std::endl;
      }
    }
  }
}

Battle::DamageResult Battle::calculateDamageWithEffects(
    const Pokemon &attacker, const Pokemon &defender, const Move &move) const {
  // Status moves don't deal damage
  if (move.power <= 0) {
    return {0, false, false};
  }

  // Base damage calculation using effective stats
  int baseDamage = 0;
  if (move.damage_class == "physical") {
    baseDamage =
        (attacker.getEffectiveAttack() - defender.defense) + move.power;
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
  if (move.power <= 0) {
    return 0;
  }

  // Use effective stats (modified by status conditions)
  int effectiveAttack = attacker.getEffectiveAttack();
  int level = 50; // Assuming level 50
  int defense = defender.defense;

  // Determine which attack stat to use
  int attackStat;
  if (move.damage_class == "physical") {
    attackStat = effectiveAttack;
  } else {
    attackStat = attacker.special_attack;
  }

  // Determine which defense stat to use
  int defenseStat;
  if (move.damage_class == "physical") {
    defenseStat = defense;
  } else {
    defenseStat = defender.special_defense;
  }

  // Basic damage calculation
  double damage = ((2.0 * level + 10.0) / 250.0) *
                      (attackStat / (double)defenseStat) * move.power +
                  2;

  return static_cast<int>(damage);
}

bool Battle::playerFirst(const Move &playerMove,
                         const Move &opponentMove) const {
  if (playerMove.priority != opponentMove.priority) {
    return playerMove.priority > opponentMove.priority;
  }
  if (selectedPokemon->getEffectiveSpeed() !=
      opponentSelectedPokemon->getEffectiveSpeed()) {
    return selectedPokemon->getEffectiveSpeed() >
           opponentSelectedPokemon->getEffectiveSpeed();
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
    // Process status conditions at start of turn
    if (selectedPokemon->hasStatusCondition()) {
      selectedPokemon->processStatusCondition();
    }
    if (opponentSelectedPokemon->hasStatusCondition()) {
      opponentSelectedPokemon->processStatusCondition();
    }

    // Check if either Pokemon fainted from status damage
    if (!selectedPokemon->isAlive() || !opponentSelectedPokemon->isAlive()) {
      // Handle fainted Pokemon below...
    } else {
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

      // Display health after moves are executed
      std::cout << std::endl;
      if (opponentSelectedPokemon->isAlive()) {
        displayHealth(*opponentSelectedPokemon);
      }
      if (selectedPokemon->isAlive()) {
        displayHealth(*selectedPokemon);
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
        displayHealth(*selectedPokemon);
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
        displayHealth(*opponentSelectedPokemon);
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

void Battle::executeTurn() {
  // Process status conditions at start of turn
  if (selectedPokemon->hasStatusCondition()) {
    selectedPokemon->processStatusCondition();
  }
  if (opponentSelectedPokemon->hasStatusCondition()) {
    opponentSelectedPokemon->processStatusCondition();
  }

  // Check if either Pokemon fainted from status damage
  if (!selectedPokemon->isAlive() || !opponentSelectedPokemon->isAlive()) {
    handlePokemonFainted();
    return;
  }

  int moveChoice = getMoveChoice();
  const auto &playerMove = selectedPokemon->moves[moveChoice - 1];

  // Simple AI: opponent chooses a random damage-dealing move
  auto opponentMoves = std::vector<Move>{};
  for (const auto &move : opponentSelectedPokemon->moves) {
    if (move.power > 0) {
      opponentMoves.push_back(move);
    }
  }

  if (opponentMoves.empty()) {
    opponentMoves = opponentSelectedPokemon->moves;
  }

  auto distribution =
      std::uniform_int_distribution<size_t>(0, opponentMoves.size() - 1);
  const auto &opponentMove = opponentMoves[distribution(rng)];

  // Determine turn order based on effective speed (modified by status)
  bool playerFirst = selectedPokemon->getEffectiveSpeed() >=
                     opponentSelectedPokemon->getEffectiveSpeed();

  if (playerFirst) {
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

  handlePokemonFainted();
}

void Battle::handlePokemonFainted() {
  // Implementation of handlePokemonFainted method
}