#include "battle.h"
#include "move_type_mapping.h"
#include "weather.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

Battle::Battle(const Team &playerTeam, const Team &opponentTeam)
    : playerTeam(playerTeam), opponentTeam(opponentTeam),
      selectedPokemon(nullptr), opponentSelectedPokemon(nullptr),
      currentWeather(WeatherCondition::NONE), weatherTurnsRemaining(0),
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
  std::cout << std::endl;

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
        std::cout << std::endl;
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

void Battle::executeMove(Pokemon &attacker, Pokemon &defender, int moveIndex) {
  // Check if attacker can act (not asleep, frozen, or fully paralyzed)
  if (!attacker.canAct()) {
    if (attacker.status == StatusCondition::PARALYSIS) {
      std::cout << attacker.name << " is paralyzed and can't move!"
                << std::endl;
    }
    return;
  }

  // Get the move and check if it can be used
  Move &move = attacker.moves[moveIndex];

  if (!move.canUse()) {
    std::cout << attacker.name << " tried to use " << move.name
              << " but it has no PP left!" << std::endl;
    return;
  }

  // Announce the move attempt
  std::cout << attacker.name << " used " << move.name << "!" << std::endl;

  // Consume PP
  move.usePP();

  // Check if the move hits
  if (!checkMoveAccuracy(move)) {
    std::cout << attacker.name << "'s attack missed!" << std::endl;
    return;
  }

  // Handle OHKO moves first (Guillotine, Sheer Cold, etc.)
  if (move.category == "ohko") {
    // OHKO moves ignore normal damage calculation
    // In real Pokemon, OHKO accuracy is based on level difference, but we'll
    // use base accuracy
    std::cout << "It's a one-hit KO!" << std::endl;
    defender.takeDamage(defender.current_hp); // Deal enough damage to KO
    return; // OHKO moves don't have other effects
  }

  // Handle healing moves (Recover, Soft-Boiled, etc.)
  if (move.healing > 0) {
    int healAmount = (attacker.hp * move.healing) / 100;
    int actualHeal = std::min(healAmount, attacker.hp - attacker.current_hp);

    if (actualHeal > 0) {
      attacker.heal(actualHeal);
      std::cout << attacker.name << " restored " << actualHeal << " HP! ("
                << healAmount << "% heal)" << std::endl;
    } else {
      std::cout << attacker.name << "'s HP is already full!" << std::endl;
    }
    return; // Healing moves don't do damage or apply other effects
  }

  if (move.power == -1 || move.power == 0) {
    // Status move or special move - move announcement already done above

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
    }

    // Handle stat modification moves (Swords Dance, Growl, etc.)
    if (move.category == "net-good-stats") {
      applyStatModification(attacker, defender, move);
    }
    // Handle weather-setting moves
    else if (move.name == "rain-dance") {
      setWeather(WeatherCondition::RAIN, 5);
    } else if (move.name == "sunny-day") {
      setWeather(WeatherCondition::SUN, 5);
    } else if (move.name == "sandstorm") {
      setWeather(WeatherCondition::SANDSTORM, 5);
    } else if (move.name == "hail") {
      setWeather(WeatherCondition::HAIL, 5);
    } else if (statusToApply == StatusCondition::NONE) {
      std::cout << "The move had no effect!" << std::endl;
    }
  } else {
    // Damage-dealing move

    // Determine number of hits for multi-hit moves
    int numHits = 1;
    if (move.min_hits > 0 && move.max_hits > 0) {
      auto hitDistribution =
          std::uniform_int_distribution<int>(move.min_hits, move.max_hits);
      numHits = hitDistribution(rng);
    }

    int totalDamage = 0;
    bool hadSTAB = false;
    bool wasCritical = false;
    bool showEffectiveness = true;

    // Execute each hit
    for (int hit = 0; hit < numHits && defender.isAlive(); ++hit) {
      auto damageResult = calculateDamageWithEffects(attacker, defender, move);

      if (numHits > 1) {
        std::cout << "Hit " << (hit + 1) << ": ";
      }

      std::cout << "It dealt " << damageResult.damage << " damage!";

      // Show weather boost if applicable
      double weatherMultiplier =
          Weather::getWeatherDamageMultiplier(currentWeather, move.type);
      if (weatherMultiplier > 1.0) {
        std::cout << " (Boosted by " << Weather::getWeatherName(currentWeather)
                  << "!)";
      } else if (weatherMultiplier < 1.0) {
        std::cout << " (Weakened by " << Weather::getWeatherName(currentWeather)
                  << "!)";
      }

      // Track overall move properties
      totalDamage += damageResult.damage;
      if (damageResult.hadSTAB)
        hadSTAB = true;
      if (damageResult.wasCritical)
        wasCritical = true;

      if (damageResult.wasCritical) {
        std::cout << " A critical hit!";
      }

      // Show type effectiveness only once for multi-hit moves
      if (showEffectiveness) {
        auto typeMultiplier = TypeEffectiveness::getEffectivenessMultiplier(
            MoveTypeMapping::getMoveType(move.name), defender.types);

        if (typeMultiplier > 1.0) {
          std::cout << " It's super effective!";
        } else if (typeMultiplier < 1.0 && typeMultiplier > 0.0) {
          std::cout << " It's not very effective...";
        } else if (typeMultiplier == 0.0) {
          std::cout << " It has no effect!";
        }
        showEffectiveness = false;
      }

      std::cout << std::endl;
      defender.takeDamage(damageResult.damage);
    }

    // Show multi-hit summary
    if (numHits > 1) {
      std::cout << "Hit " << numHits << " time(s) for " << totalDamage
                << " total damage!";
      if (hadSTAB) {
        std::cout << " " << attacker.name << " gets STAB!";
      }
      std::cout << std::endl;
    } else if (hadSTAB) {
      std::cout << attacker.name << " gets STAB!" << std::endl;
    }

    // Handle draining moves (Mega Drain, Absorb, etc.)
    if (move.drain > 0 && totalDamage > 0) {
      int drainAmount = (totalDamage * move.drain) / 100;
      int actualHeal = std::min(drainAmount, attacker.hp - attacker.current_hp);

      if (actualHeal > 0) {
        attacker.heal(actualHeal);
        std::cout << attacker.name << " absorbed " << actualHeal << " HP! ("
                  << move.drain << "% of damage dealt)" << std::endl;
      }
    }

    // Handle recoil moves (Double Edge, Take Down, etc.)
    if (move.drain < 0 && totalDamage > 0) {
      int recoilPercent =
          -move.drain; // Convert negative drain to positive percentage
      int recoilDamage = (totalDamage * recoilPercent) / 100;

      if (recoilDamage > 0) {
        attacker.takeDamage(recoilDamage);
        std::cout << attacker.name << " is hit with recoil! (" << recoilPercent
                  << "% of damage dealt = " << recoilDamage << " HP)"
                  << std::endl;
      }
    }

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
        (attacker.getEffectiveAttack() - defender.getEffectiveDefense()) +
        move.power;
  } else if (move.damage_class == "special") {
    baseDamage = (attacker.getEffectiveSpecialAttack() -
                  defender.getEffectiveSpecialDefense()) +
                 move.power;
  }

  // Ensure minimum base damage
  baseDamage = std::max(1, baseDamage);

  // Apply type effectiveness
  double typeMultiplier =
      TypeEffectiveness::getEffectivenessMultiplier(move.type, defender.types);

  // Apply weather effects
  double weatherMultiplier =
      Weather::getWeatherDamageMultiplier(currentWeather, move.type);

  // Check for STAB and critical hit
  auto hasStab = hasSTAB(attacker, move);
  auto isCrit = isCriticalHit(move);

  // Apply STAB (Same Type Attack Bonus) - 1.5x damage if move type matches
  // attacker type
  auto stabMultiplier = hasStab ? 1.5 : 1.0;

  // Apply critical hit multiplier - 2x damage on critical hit
  auto criticalMultiplier = isCrit ? 2.0 : 1.0;

  // Calculate final damage with all multipliers
  double finalDamage = baseDamage * typeMultiplier * weatherMultiplier *
                       stabMultiplier * criticalMultiplier;

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
              << ", PP: " << move.getRemainingPP() << "/" << move.getMaxPP()
              << ", Class: " << move.damage_class << ")";

    // Show "No PP!" if move can't be used
    if (!move.canUse()) {
      std::cout << " [No PP!]";
    }
    std::cout << "\n";
  }

  int chosenMoveIndex;
  while (true) {
    std::cout << "\nSelect a move (1-" << selectedPokemon->moves.size()
              << "): ";
    std::cin >> chosenMoveIndex;

    if (chosenMoveIndex >= 1 &&
        chosenMoveIndex <= static_cast<int>(selectedPokemon->moves.size())) {

      const Move &selectedMove = selectedPokemon->moves[chosenMoveIndex - 1];

      // Check if the move has PP remaining
      if (!selectedMove.canUse()) {
        std::cout << selectedMove.name
                  << " has no PP left! Choose another move.\n";
        continue;
      }

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
  std::cout << "\n======================================================== BATTLE "
  "START ========================================================="    << std::endl;

  // Initial Pokemon selection
  selectOpponentPokemon();
  selectPokemon();
  
  // Main battle loop
  while (!isBattleOver()) {
    std::cout << "==============================================================="
    "===============================================================" << std::endl;
    std::cout << std::endl;

    // Process status conditions at start of turn
    if (selectedPokemon->hasStatusCondition()) {
      selectedPokemon->processStatusCondition();
    }
    if (opponentSelectedPokemon->hasStatusCondition()) {
      opponentSelectedPokemon->processStatusCondition();
    }

    // Process weather conditions
    processWeather();

    // Show health bars before move selection
    if (opponentSelectedPokemon->isAlive()) {
      displayHealth(*opponentSelectedPokemon);
    }
    if (selectedPokemon->isAlive()) {
      displayHealth(*selectedPokemon);
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
        executeMove(*selectedPokemon, *opponentSelectedPokemon,
                    playerMoveIndex);
        if (opponentSelectedPokemon->isAlive()) {
          executeMove(*opponentSelectedPokemon, *selectedPokemon,
                      opponentMoveIndex);
        }
      } else {
        executeMove(*opponentSelectedPokemon, *selectedPokemon,
                    opponentMoveIndex);
        if (selectedPokemon->isAlive()) {
          executeMove(*selectedPokemon, *opponentSelectedPokemon,
                      playerMoveIndex);
        }
      }
      
      // Always display health after moves, even if fainted
      displayHealth(*opponentSelectedPokemon);
      displayHealth(*selectedPokemon);
    }

    // Handle fainted Pokemon (simplified for now)
    if (!selectedPokemon->isAlive()) {
      std::cout << "\n"
                << selectedPokemon->name << " has fainted!" << std::endl;
      auto *newPokemon = playerTeam.getFirstAlivePokemon();
      if (newPokemon) {
        selectedPokemon = newPokemon;
        std::cout << "\nYou send out " << selectedPokemon->name << "!\n";
        //displayHealth(*selectedPokemon);
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
        //displayHealth(*opponentSelectedPokemon);
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

// Accuracy checking implementation
bool Battle::checkMoveAccuracy(const Move &move) const {
  // Moves with accuracy = 0 never miss (like Swift, Aerial Ace)
  if (move.accuracy == 0) {
    return true;
  }

  // Roll 1-100 vs accuracy value
  auto accuracyDistribution = std::uniform_int_distribution<int>(1, 100);
  return accuracyDistribution(rng) <= move.accuracy;
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

  // Simple AI: opponent chooses a random move with PP
  int opponentMoveIndex = -1;
  for (int i = 0; i < static_cast<int>(opponentSelectedPokemon->moves.size());
       ++i) {
    if (opponentSelectedPokemon->moves[i].canUse()) {
      if (opponentMoveIndex == -1 || rand() % 2) {
        opponentMoveIndex = i;
      }
    }
  }

  // If no moves have PP, use first move anyway (will fail in executeMove)
  if (opponentMoveIndex == -1) {
    opponentMoveIndex = 0;
  }

  // Determine turn order based on effective speed (modified by status)
  bool playerFirst = selectedPokemon->getEffectiveSpeed() >=
                     opponentSelectedPokemon->getEffectiveSpeed();

  if (playerFirst) {
    executeMove(*selectedPokemon, *opponentSelectedPokemon, moveChoice);
    if (opponentSelectedPokemon->isAlive()) {
      executeMove(*opponentSelectedPokemon, *selectedPokemon,
                  opponentMoveIndex);
    }
  } else {
    executeMove(*opponentSelectedPokemon, *selectedPokemon, opponentMoveIndex);
    if (selectedPokemon->isAlive()) {
      executeMove(*selectedPokemon, *opponentSelectedPokemon, moveChoice);
    }
  }

  handlePokemonFainted();
}

void Battle::handlePokemonFainted() {
  // Implementation of handlePokemonFainted method
}

void Battle::applyStatModification(Pokemon &attacker, Pokemon &defender,
                                   const Move &move) {
  // Map move names to stat modifications
  // Format: {stat, stages, target} where target: true=self, false=opponent

  if (move.name == "swords-dance") {
    attacker.modifyAttack(2);
    std::cout << attacker.name << "'s Attack rose sharply!" << std::endl;
  } else if (move.name == "growl") {
    defender.modifyAttack(-1);
    std::cout << defender.name << "'s Attack fell!" << std::endl;
  } else if (move.name == "agility") {
    attacker.modifySpeed(2);
    std::cout << attacker.name << "'s Speed rose sharply!" << std::endl;
  } else if (move.name == "harden") {
    attacker.modifyDefense(1);
    std::cout << attacker.name << "'s Defense rose!" << std::endl;
  } else if (move.name == "defense-curl") {
    attacker.modifyDefense(1);
    std::cout << attacker.name << "'s Defense rose!" << std::endl;
  } else if (move.name == "iron-defense") {
    attacker.modifyDefense(2);
    std::cout << attacker.name << "'s Defense rose sharply!" << std::endl;
  } else if (move.name == "calm-mind") {
    attacker.modifySpecialAttack(1);
    attacker.modifySpecialDefense(1);
    std::cout << attacker.name << "'s Special Attack and Special Defense rose!"
              << std::endl;
  } else if (move.name == "leer") {
    defender.modifyDefense(-1);
    std::cout << defender.name << "'s Defense fell!" << std::endl;
  } else if (move.name == "tail-whip") {
    defender.modifyDefense(-1);
    std::cout << defender.name << "'s Defense fell!" << std::endl;
  } else if (move.name == "amnesia") {
    attacker.modifySpecialDefense(2);
    std::cout << attacker.name << "'s Special Defense rose sharply!"
              << std::endl;
  } else if (move.name == "barrier") {
    attacker.modifyDefense(2);
    std::cout << attacker.name << "'s Defense rose sharply!" << std::endl;
  } else if (move.name == "sharpen") {
    attacker.modifyAttack(1);
    std::cout << attacker.name << "'s Attack rose!" << std::endl;
  } else if (move.name == "meditate") {
    attacker.modifyAttack(1);
    std::cout << attacker.name << "'s Attack rose!" << std::endl;
  } else if (move.name == "dragon-dance") {
    attacker.modifyAttack(1);
    attacker.modifySpeed(1);
    std::cout << attacker.name << "'s Attack and Speed rose!" << std::endl;
  } else if (move.name == "nasty-plot") {
    attacker.modifySpecialAttack(2);
    std::cout << attacker.name << "'s Special Attack rose sharply!"
              << std::endl;
  } else {
    std::cout << attacker.name << " used " << move.name
              << ", but it had no stat effect!" << std::endl;
  }
}

void Battle::processWeather() {
  if (currentWeather == WeatherCondition::NONE) {
    return;
  }

  // Display weather effect
  std::cout << "Weather: " << Weather::getWeatherName(currentWeather);
  if (weatherTurnsRemaining > 0) {
    std::cout << " (" << weatherTurnsRemaining << " turns left)";
  }

  // Show current weather boosts
  switch (currentWeather) {
  case WeatherCondition::RAIN:
    std::cout << " [Water +50%, Fire -50%]";
    break;
  case WeatherCondition::SUN:
    std::cout << " [Fire +50%, Water -50%]";
    break;
  case WeatherCondition::SANDSTORM:
    std::cout << " [Sandstorm damage]";
    break;
  case WeatherCondition::HAIL:
    std::cout << " [Hail damage]";
    break;
  default:
    break;
  }
  std::cout << std::endl;

  // Apply weather damage to Pokemon
  if (selectedPokemon && selectedPokemon->isAlive()) {
    if (!Weather::isImmuneToWeatherDamage(currentWeather,
                                          selectedPokemon->types)) {
      int damage =
          Weather::getWeatherDamage(currentWeather, selectedPokemon->hp);
      if (damage > 0) {
        selectedPokemon->takeDamage(damage);
        std::cout << selectedPokemon->name << " is hurt by "
                  << Weather::getWeatherName(currentWeather) << "! (-" << damage
                  << " HP)" << std::endl;
      }
    }
  }

  if (opponentSelectedPokemon && opponentSelectedPokemon->isAlive()) {
    if (!Weather::isImmuneToWeatherDamage(currentWeather,
                                          opponentSelectedPokemon->types)) {
      int damage = Weather::getWeatherDamage(currentWeather,
                                             opponentSelectedPokemon->hp);
      if (damage > 0) {
        opponentSelectedPokemon->takeDamage(damage);
        std::cout << opponentSelectedPokemon->name << " is hurt by "
                  << Weather::getWeatherName(currentWeather) << "! (-" << damage
                  << " HP)" << std::endl;
      }
    }
  }

  // Countdown weather turns
  if (weatherTurnsRemaining > 0) {
    weatherTurnsRemaining--;
    if (weatherTurnsRemaining == 0) {
      std::cout << "The " << Weather::getWeatherName(currentWeather)
                << " stopped." << std::endl;
      currentWeather = WeatherCondition::NONE;
    }
  }
}

void Battle::setWeather(WeatherCondition weather, int turns) {
  currentWeather = weather;
  weatherTurnsRemaining = turns;
  if (weather != WeatherCondition::NONE) {
    std::cout << Weather::getWeatherName(weather) << " started!";

    // Show what boost the weather provides
    switch (weather) {
    case WeatherCondition::RAIN:
      std::cout << " (Water moves boosted 1.5x, Fire moves weakened 0.5x)";
      break;
    case WeatherCondition::SUN:
      std::cout << " (Fire moves boosted 1.5x, Water moves weakened 0.5x)";
      break;
    case WeatherCondition::SANDSTORM:
      std::cout << " (Non Rock/Ground/Steel types take damage each turn)";
      break;
    case WeatherCondition::HAIL:
      std::cout << " (Non Ice types take damage each turn)";
      break;
    default:
      break;
    }
    std::cout << std::endl;
  }
}

void Battle::displayWeather() const {
  if (currentWeather != WeatherCondition::NONE) {
    std::cout << "Current weather: " << Weather::getWeatherName(currentWeather);
    if (weatherTurnsRemaining > 0) {
      std::cout << " (" << weatherTurnsRemaining << " turns remaining)";
    }
    std::cout << std::endl;
  }
}