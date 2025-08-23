#include "battle.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <thread>
#include <functional>

#include "move_type_mapping.h"
#include "weather.h"
#include "input_validator.h"
#include "battle_events.h"

Battle::Battle(const Team &playerTeam, const Team &opponentTeam,
               AIDifficulty aiDifficulty)
    : playerTeam(playerTeam),
      opponentTeam(opponentTeam),
      selectedPokemon(nullptr),
      opponentSelectedPokemon(nullptr),
      aiDifficulty(aiDifficulty),
      currentWeather(WeatherCondition::NONE),
      weatherTurnsRemaining(0),
      rng(std::random_device{}()),
      criticalDistribution(0.0, 1.0) {
  srand(time(0));  // Seed random number generator once
  
  // Initialize health bar animation system with auto-detection
  auto config = HealthBarAnimator::detectOptimalConfig();
  healthBarAnimator = std::make_shared<HealthBarAnimator>(config);
  healthBarListener = std::make_shared<HealthBarEventListener>(healthBarAnimator);
  
  // Register the health bar listener with the event manager
  eventManager.subscribe(healthBarListener);
}

void Battle::displayHealth(const Pokemon &pokemon) const {
  if (!healthBarAnimator) {
    // Fallback to basic display if animator not initialized
    std::cout << pokemon.name << " HP: " << pokemon.current_hp << "/" << pokemon.hp;
    if (pokemon.hasStatusCondition()) {
      std::cout << " (" << pokemon.getStatusConditionName() << ")";
    }
    std::cout << std::endl;
    return;
  }

  // Get status condition string
  std::string statusCondition = pokemon.hasStatusCondition() ? pokemon.getStatusConditionName() : "";

  // Display static health bar (previous health tracking now handled by event system)
  healthBarAnimator->displayStaticHealth(pokemon.name, pokemon.current_hp, pokemon.hp, statusCondition);
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

  // Secure Pokemon selection with validation
  auto pokemonValidator = [this](std::istream& input) -> InputValidator::ValidationResult<int> {
    auto result = InputValidator::getValidatedInt(input, 1, static_cast<int>(playerTeam.size()));
    if (!result.isValid()) {
      return result;
    }
    
    // Validate that the selected Pokemon exists and is alive
    auto *pokemon = playerTeam.getPokemon(result.value - 1);
    if (!pokemon || !pokemon->isAlive()) {
      return InputValidator::ValidationResult<int>(
        InputValidator::ValidationError::INVALID_INPUT,
        "Selected Pokemon is not available or has fainted"
      );
    }
    
    return result;
  };
  
  auto pokemonResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "\nEnter the number of the Pokémon you want to send out",
    2, pokemonValidator
  );
  
  if (!pokemonResult.isValid()) {
    std::cout << "Failed to get valid Pokemon selection: " << pokemonResult.errorMessage << std::endl;
    // Auto-select first available Pokemon as fallback
    for (int i = 0; i < static_cast<int>(playerTeam.size()); ++i) {
      auto *pokemon = playerTeam.getPokemon(i);
      if (pokemon && pokemon->isAlive()) {
        selectedPokemon = pokemon;
        std::cout << "Auto-selecting " << selectedPokemon->name << " as fallback!" << std::endl;
        return;
      }
    }
    return; // No valid Pokemon available
  }
  
  int chosenPokemonNum = pokemonResult.value;
  selectedPokemon = playerTeam.getPokemon(chosenPokemonNum - 1);
  std::cout << "\nYou have selected " << selectedPokemon->name
            << " to send out!" << std::endl;
  
  // Register Pokemon with health bar system
  if (healthBarListener && selectedPokemon) {
    healthBarListener->registerPokemon(selectedPokemon, "Player");
  }
  
  std::cout << std::endl;
}

void Battle::selectOpponentPokemon() {
  auto alivePokemon = opponentTeam.getAlivePokemon();
  if (!alivePokemon.empty()) {
    auto randomIndex = rand() % alivePokemon.size();
    opponentSelectedPokemon = alivePokemon[randomIndex];
    std::cout << "\nThe opponent has selected " << opponentSelectedPokemon->name
              << " to send out!" << std::endl;
    
    // Register opponent Pokemon with health bar system
    if (healthBarListener && opponentSelectedPokemon) {
      healthBarListener->registerPokemon(opponentSelectedPokemon, "Opponent");
    }
  }
}

void Battle::executeMove(Pokemon &attacker, Pokemon &defender, int moveIndex) {
  // Check if attacker must recharge this turn
  if (attacker.mustRecharge()) {
    std::cout << attacker.name << " must recharge and cannot move!" << std::endl;
    attacker.finishRecharge();
    return;
  }
  
  // Check if attacker can act (not asleep, frozen, or fully paralyzed)
  if (!attacker.canActThisTurn()) {
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

  // Handle multi-turn move state transitions
  if (attacker.isCharging() && attacker.getChargingMoveIndex() == moveIndex) {
    // Pokemon is finishing a charging move
    std::cout << attacker.name << " unleashed " << move.name << "!" << std::endl;
    attacker.finishCharging();
    
    // Notify event system
    auto event = eventManager.createMultiTurnMoveEvent(
      &attacker, &move, BattleEvents::MultiTurnMoveEvent::Phase::EXECUTING,
      attacker.name + " unleashed " + move.name + "!"
    );
    eventManager.notifyMultiTurnMove(event);
    
    // Consume PP when actually executing the move
    move.usePP();
  } else if (move.requiresCharging()) {
    // Pokemon is starting to charge a move
    bool skipCharge = false;
    
    // Check for Solar Beam sunny weather skip
    if (move.skipChargeInSunnyWeather() && currentWeather == WeatherCondition::SUN) {
      std::cout << attacker.name << " used " << move.name << "!" << std::endl;
      std::cout << "The sunlight is strong! " << attacker.name << " doesn't need to charge!" << std::endl;
      
      // Notify event system for weather skip
      auto event = eventManager.createMultiTurnMoveEvent(
        &attacker, &move, BattleEvents::MultiTurnMoveEvent::Phase::EXECUTING,
        "The sunlight is strong! " + attacker.name + " doesn't need to charge!"
      );
      eventManager.notifyMultiTurnMove(event);
      
      skipCharge = true;
      move.usePP();
    } else {
      std::cout << attacker.name << " began charging " << move.name << "!" << std::endl;
      attacker.startCharging(moveIndex, move.name);
      
      // Notify event system
      auto event = eventManager.createMultiTurnMoveEvent(
        &attacker, &move, BattleEvents::MultiTurnMoveEvent::Phase::CHARGING,
        attacker.name + " began charging " + move.name + "!"
      );
      eventManager.notifyMultiTurnMove(event);
      
      move.usePP();
      return; // Charging turn, no damage dealt
    }
    
    if (!skipCharge) {
      return;
    }
  } else {
    // Regular move execution
    std::cout << attacker.name << " used " << move.name << "!" << std::endl;
    move.usePP();
    
    // Handle recharge moves
    if (move.requiresRecharge()) {
      attacker.startRecharge();
      
      // Notify event system
      auto event = eventManager.createMultiTurnMoveEvent(
        &attacker, &move, BattleEvents::MultiTurnMoveEvent::Phase::RECHARGING,
        attacker.name + " must recharge next turn!"
      );
      eventManager.notifyMultiTurnMove(event);
    }
  }

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
    int previousHealth = defender.current_hp;
    defender.takeDamage(defender.current_hp);  // Deal enough damage to KO
    
    // Emit health change event for OHKO
    auto healthEvent = eventManager.createHealthChangeEvent(
      &defender, previousHealth, defender.current_hp, attacker.name + "'s " + move.name + " (OHKO)"
    );
    eventManager.notifyHealthChanged(healthEvent);
    return;  // OHKO moves don't have other effects
  }

  // Handle healing moves (Recover, Soft-Boiled, etc.)
  if (move.healing > 0) {
    int healAmount = (attacker.hp * move.healing) / 100;
    int actualHeal = std::min(healAmount, attacker.hp - attacker.current_hp);

    if (actualHeal > 0) {
      int previousHealth = attacker.current_hp;
      attacker.heal(actualHeal);
      std::cout << attacker.name << " restored " << actualHeal << " HP! ("
                << healAmount << "% heal)" << std::endl;
      
      // Emit health change event for healing
      auto healthEvent = eventManager.createHealthChangeEvent(
        &attacker, previousHealth, attacker.current_hp, move.name + " (heal)"
      );
      eventManager.notifyHealthChanged(healthEvent);
    } else {
      std::cout << attacker.name << "'s HP is already full!" << std::endl;
    }
    return;  // Healing moves don't do damage or apply other effects
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
      if (damageResult.hadSTAB) hadSTAB = true;
      if (damageResult.wasCritical) wasCritical = true;

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
      
      // Store previous health for event
      int previousHealth = defender.current_hp;
      defender.takeDamage(damageResult.damage);
      
      // Emit health change event
      auto healthEvent = eventManager.createHealthChangeEvent(
        &defender, previousHealth, defender.current_hp, attacker.name + "'s " + move.name
      );
      eventManager.notifyHealthChanged(healthEvent);
    }

    // Show multi-hit summary
    if (numHits > 1) {
      std::cout << "Hit " << numHits << " time(s) for " << totalDamage
                << " total damage!";
      if (hadSTAB) {
        std::cout << " " << attacker.name << " gets STAB!";
      }
      if (wasCritical) {
        std::cout << " At least one critical hit!";
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
        int previousHealth = attacker.current_hp;
        attacker.heal(actualHeal);
        std::cout << attacker.name << " absorbed " << actualHeal << " HP! ("
                  << move.drain << "% of damage dealt)" << std::endl;
        
        // Emit health change event for drain healing
        auto healthEvent = eventManager.createHealthChangeEvent(
          &attacker, previousHealth, attacker.current_hp, move.name + " (drain)"
        );
        eventManager.notifyHealthChanged(healthEvent);
      }
    }

    // Handle recoil moves (Double Edge, Take Down, etc.)
    if (move.drain < 0 && totalDamage > 0) {
      int recoilPercent =
          -move.drain;  // Convert negative drain to positive percentage
      int recoilDamage = (totalDamage * recoilPercent) / 100;

      if (recoilDamage > 0) {
        int previousHealth = attacker.current_hp;
        attacker.takeDamage(recoilDamage);
        std::cout << attacker.name << " is hit with recoil! (" << recoilPercent
                  << "% of damage dealt = " << recoilDamage << " HP)"
                  << std::endl;
        
        // Emit health change event for recoil damage
        auto healthEvent = eventManager.createHealthChangeEvent(
          &attacker, previousHealth, attacker.current_hp, move.name + " (recoil)"
        );
        eventManager.notifyHealthChanged(healthEvent);
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

  // Use proper Pokemon damage formula for base damage calculation
  int baseDamage = calculateDamage(attacker, defender, move);

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
  int level = 50;  // Assuming level 50
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

  // Pokemon damage formula (simplified but balanced)
  // Base calculation: ((2*Level/5+2)*Power*Attack/Defense)/50 + 2
  double damage = (((2.0 * level / 5.0 + 2.0) * move.power * attackStat / defenseStat) / 50.0) + 2.0;
  
  // Add some randomness (85-100% of calculated damage)
  double randomFactor = 0.85 + (rand() % 16) / 100.0;  // 0.85 to 1.00
  damage *= randomFactor;

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
  return rand() % 2;  // Randomize if speeds are equal
}

int Battle::getMoveChoice() const {
  std::cout << "\nChoose an action:\n";

  // Check if Pokemon must recharge
  if (selectedPokemon->mustRecharge()) {
    std::cout << "\n" << selectedPokemon->name << " must recharge this turn and cannot act!\n";
    return -2; // Special value to indicate forced recharge
  }
  
  // Check if Pokemon is charging a move
  if (selectedPokemon->isCharging()) {
    int chargingMoveIndex = selectedPokemon->getChargingMoveIndex();
    std::string chargingMoveName = selectedPokemon->getChargingMoveName();
    std::cout << "\n" << selectedPokemon->name << " is charging " << chargingMoveName << " and must execute it!\n";
    return chargingMoveIndex; // Must execute the charging move
  }

  // Show moves
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
    
    // Show multi-turn information
    if (move.requiresCharging()) {
      std::cout << " [Charging move - takes 2 turns]";
      if (move.skipChargeInSunnyWeather() && currentWeather == WeatherCondition::SUN) {
        std::cout << " [Sunny weather: no charge needed!]";
      }
    } else if (move.requiresRecharge()) {
      std::cout << " [Recharge move - requires rest turn after use]";
    }
    std::cout << "\n";
  }

  // Show switch option if other Pokemon are available
  bool canSwitch = false;
  for (int i = 0; i < static_cast<int>(playerTeam.size()); ++i) {
    const auto *pokemon = playerTeam.getPokemon(i);
    if (pokemon && pokemon->isAlive() && pokemon != selectedPokemon) {
      canSwitch = true;
      break;
    }
  }

  if (canSwitch) {
    std::cout << "    " << (selectedPokemon->moves.size() + 1)
              << ". Switch Pokémon\n";
  }

  // Secure action selection with validation
  int maxChoice = static_cast<int>(selectedPokemon->moves.size() + (canSwitch ? 1 : 0));
  
  auto actionValidator = [this, canSwitch, maxChoice](std::istream& input) -> InputValidator::ValidationResult<int> {
    auto result = InputValidator::getValidatedInt(input, 1, maxChoice);
    if (!result.isValid()) {
      return result;
    }
    
    int choice = result.value;
    
    // Check if it's a move choice and validate PP and multi-turn constraints
    if (choice >= 1 && choice <= static_cast<int>(selectedPokemon->moves.size())) {
      const Move &selectedMove = selectedPokemon->moves[choice - 1];
      
      // Validate PP
      if (!selectedMove.canUse()) {
        return InputValidator::ValidationResult<int>(
          InputValidator::ValidationError::INVALID_INPUT,
          selectedMove.name + " has no PP left! Choose another action"
        );
      }
      
      // Validate multi-turn move constraints
      if (selectedPokemon->isCharging()) {
        int chargingMoveIndex = selectedPokemon->getChargingMoveIndex();
        if (choice - 1 != chargingMoveIndex) {
          return InputValidator::ValidationResult<int>(
            InputValidator::ValidationError::INVALID_INPUT,
            selectedPokemon->name + " is charging " + selectedPokemon->getChargingMoveName() + " and must execute it!"
          );
        }
      }
      
      if (selectedPokemon->mustRecharge()) {
        return InputValidator::ValidationResult<int>(
          InputValidator::ValidationError::INVALID_INPUT,
          selectedPokemon->name + " must recharge this turn and cannot use moves!"
        );
      }
    }
    
    // Validate switch choice
    if (choice == static_cast<int>(selectedPokemon->moves.size() + 1) && !canSwitch) {
      return InputValidator::ValidationResult<int>(
        InputValidator::ValidationError::INVALID_INPUT,
        "No Pokemon available to switch to"
      );
    }
    
    // Multi-turn moves prevent switching
    if (choice == static_cast<int>(selectedPokemon->moves.size() + 1)) {
      if (selectedPokemon->isCharging()) {
        return InputValidator::ValidationResult<int>(
          InputValidator::ValidationError::INVALID_INPUT,
          selectedPokemon->name + " is charging a move and cannot switch!"
        );
      }
      if (selectedPokemon->mustRecharge()) {
        return InputValidator::ValidationResult<int>(
          InputValidator::ValidationError::INVALID_INPUT,
          selectedPokemon->name + " must recharge and cannot switch!"
        );
      }
    }
    
    return result;
  };
  
  auto actionResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "\nSelect an action (1-" + std::to_string(maxChoice) + ")",
    2, actionValidator
  );
  
  if (!actionResult.isValid()) {
    std::cout << "Failed to get valid action selection: " << actionResult.errorMessage << std::endl;
    
    // Handle multi-turn move constraints in fallback
    if (selectedPokemon->mustRecharge()) {
      std::cout << "Pokemon must recharge - returning recharge indicator.\n";
      return -2; // Special value for forced recharge
    }
    
    if (selectedPokemon->isCharging()) {
      std::cout << "Pokemon is charging - returning charging move index.\n";
      return selectedPokemon->getChargingMoveIndex();
    }
    
    std::cout << "Auto-selecting first available move as fallback.\n";
    
    // Find first usable move as fallback
    for (size_t i = 0; i < selectedPokemon->moves.size(); ++i) {
      if (selectedPokemon->moves[i].canUse()) {
        return static_cast<int>(i);  // Return 0-based move index
      }
    }
    
    // If no moves are usable, return first move anyway (battle system should handle this)
    return 0;
  }
  
  int choice = actionResult.value;
  
  // Check if it's a move choice
  if (choice >= 1 && choice <= static_cast<int>(selectedPokemon->moves.size())) {
    return choice - 1;  // Return 0-based move index
  }

  // Check if it's a switch choice
  if (canSwitch && choice == static_cast<int>(selectedPokemon->moves.size() + 1)) {
    return -1;  // Special value to indicate switching
  }

  // This should never happen due to validation, but safety fallback
  return 0;
}

int Battle::getPokemonChoice() const {
  std::cout << "\nChoose a Pokémon to send out:\n";

  // Show available Pokemon (exclude currently selected one)
  std::vector<int> availableIndices;
  for (int i = 0; i < static_cast<int>(playerTeam.size()); ++i) {
    const auto *pokemon = playerTeam.getPokemon(i);
    if (pokemon && pokemon->isAlive() && pokemon != selectedPokemon) {
      availableIndices.push_back(i);
      std::cout << "    [" << availableIndices.size() << "] - "
                << pokemon->name;

      // Show health percentage
      double healthPercent = pokemon->getHealthPercentage();
      std::cout << " (HP: " << static_cast<int>(healthPercent) << "%)";

      // Show status condition if any
      if (pokemon->hasStatusCondition()) {
        std::cout << " (" << pokemon->getStatusConditionName() << ")";
      }

      std::cout << "\n";
    }
  }

  if (availableIndices.empty()) {
    std::cout << "No other Pokémon available!\n";
    return -1;  // No Pokemon to switch to
  }

  // Secure Pokemon switching selection with validation
  auto switchValidator = [this, &availableIndices](std::istream& input) -> InputValidator::ValidationResult<int> {
    auto result = InputValidator::getValidatedInt(input, 1, static_cast<int>(availableIndices.size()));
    if (!result.isValid()) {
      return result;
    }
    
    int choice = result.value;
    int pokemonIndex = availableIndices[choice - 1];
    const auto *pokemon = playerTeam.getPokemon(pokemonIndex);
    
    // Double-check that the Pokemon is still alive and valid
    if (!pokemon || !pokemon->isAlive() || pokemon == selectedPokemon) {
      return InputValidator::ValidationResult<int>(
        InputValidator::ValidationError::INVALID_INPUT,
        "Selected Pokemon is not available for switching"
      );
    }
    
    return InputValidator::ValidationResult<int>(pokemonIndex);
  };
  
  auto switchResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "\nSelect a Pokémon (1-" + std::to_string(availableIndices.size()) + ")",
    2, switchValidator
  );
  
  if (!switchResult.isValid()) {
    std::cout << "Failed to get valid Pokemon switch selection: " << switchResult.errorMessage << std::endl;
    std::cout << "Auto-selecting first available Pokemon as fallback.\n";
    
    // Return first available Pokemon as fallback
    if (!availableIndices.empty()) {
      return availableIndices[0];
    }
    
    return -1; // No Pokemon available
  }
  
  return switchResult.value;  // Return actual team index
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
  std::cout << "\n======================================================== "
               "BATTLE START "
            << "========================================================="
            << std::endl;

  // Initial Pokemon selection
  selectOpponentPokemon();
  selectPokemon();

  // Main battle loop
  while (!isBattleOver()) {
    std::cout
        << "==============================================================="
        << "==============================================================="
        << std::endl;
    std::cout << std::endl;

    // Process status conditions at start of turn
    if (selectedPokemon->hasStatusCondition()) {
      processStatusConditionWithEvents(*selectedPokemon);
    }
    if (opponentSelectedPokemon->hasStatusCondition()) {
      processStatusConditionWithEvents(*opponentSelectedPokemon);
    }

    // Process weather conditions
    processWeather();

    // Display current health status for both Pokemon at start of turn
    if (selectedPokemon && selectedPokemon->isAlive()) {
      displayHealth(*selectedPokemon);
    }
    if (opponentSelectedPokemon && opponentSelectedPokemon->isAlive()) {
      displayHealth(*opponentSelectedPokemon);
    }

    // Check if either Pokemon fainted from status damage
    if (!selectedPokemon->isAlive() || !opponentSelectedPokemon->isAlive()) {
      // Handle fainted Pokemon below...
    } else {
      // Player chooses action (move or switch)
      int playerChoice = getMoveChoice();

      if (playerChoice == -2) {
        // Pokemon must recharge - skip turn
        selectedPokemon->finishRecharge();
        std::cout << selectedPokemon->name << " is recharging and cannot move!" << std::endl;
        
        // Opponent still gets to attack
        int opponentMoveIndex = getAIMoveChoice();
        
        // Handle opponent recharge state
        if (opponentMoveIndex == -2) {
          opponentSelectedPokemon->finishRecharge();
          std::cout << opponentSelectedPokemon->name << " is recharging and cannot move!" << std::endl;
        } else {
          executeMove(*opponentSelectedPokemon, *selectedPokemon, opponentMoveIndex);
        }
        
        std::cout << std::endl;
      } else if (playerChoice == -1) {
        // Player wants to switch Pokemon
        int chosenIndex = getPokemonChoice();
        if (chosenIndex >= 0) {
          std::cout << "\n"
                    << selectedPokemon->name << ", come back!" << std::endl;
          selectedPokemon = playerTeam.getPokemon(chosenIndex);
          std::cout << "Go, " << selectedPokemon->name << "!" << std::endl;
          
          // Register new Pokemon with health bar system
          if (healthBarListener && selectedPokemon) {
            healthBarListener->registerPokemon(selectedPokemon, "Player");
          }
          
          // Health bar updated through event system registration

          // Opponent still gets to attack (switching takes a turn)
          int opponentMoveIndex = getAIMoveChoice();
          
          // Handle opponent recharge state
          if (opponentMoveIndex == -2) {
            opponentSelectedPokemon->finishRecharge();
            std::cout << opponentSelectedPokemon->name << " is recharging and cannot move!" << std::endl;
          } else {
            executeMove(*opponentSelectedPokemon, *selectedPokemon,
                        opponentMoveIndex);
          }

          std::cout << std::endl;
        }
      } else {
        // Player chose a move
        Move playerMove = selectedPokemon->moves[playerChoice];

        // Opponent chooses move based on AI difficulty
        int opponentMoveIndex = getAIMoveChoice();
        
        // Handle special AI states
        if (opponentMoveIndex == -2) {
          // AI must recharge
          opponentSelectedPokemon->finishRecharge();
          std::cout << opponentSelectedPokemon->name << " is recharging and cannot move!" << std::endl;
          
          // Only player moves
          executeMove(*selectedPokemon, *opponentSelectedPokemon, playerChoice);
        } else {
          // Normal move execution
          Move opponentMove = opponentSelectedPokemon->moves[opponentMoveIndex];

          // Determine turn order and execute moves
          if (playerFirst(playerMove, opponentMove)) {
            executeMove(*selectedPokemon, *opponentSelectedPokemon, playerChoice);
            if (opponentSelectedPokemon->isAlive()) {
              executeMove(*opponentSelectedPokemon, *selectedPokemon,
                          opponentMoveIndex);
            }
          } else {
            executeMove(*opponentSelectedPokemon, *selectedPokemon,
                        opponentMoveIndex);
            if (selectedPokemon->isAlive()) {
              executeMove(*selectedPokemon, *opponentSelectedPokemon,
                          playerChoice);
            }
          }
        }

        // Wait a moment to simulate turn processing
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Health bars updated through event system
        std::cout << std::endl;

        // Wait a moment to simulate turn processing
        std::this_thread::sleep_for(std::chrono::seconds(1));
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
        
        // Register new opponent Pokemon with health bar system
        if (healthBarListener && opponentSelectedPokemon) {
          healthBarListener->registerPokemon(opponentSelectedPokemon, "Opponent");
        }
      }
    }
  }

  // Display battle result
  BattleResult result = getBattleResult();
  switch (result) {
    case BattleResult::PLAYER_WINS:
      std::cout
          << "\nAll opponent's Pokémon have fainted! You won the battle!\n";
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
    processStatusConditionWithEvents(*selectedPokemon);
  }
  if (opponentSelectedPokemon->hasStatusCondition()) {
    processStatusConditionWithEvents(*opponentSelectedPokemon);
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
  // Handle player Pokemon fainting
  if (!selectedPokemon->isAlive()) {
    std::cout << "\n" << selectedPokemon->name << " has fainted!" << std::endl;

    // Check if player has any Pokemon left
    if (!playerTeam.hasAlivePokemon()) {
      return;  // Battle will end
    }

    // Let player choose replacement Pokemon
    int chosenIndex = getPokemonChoice();
    if (chosenIndex >= 0) {
      selectedPokemon = playerTeam.getPokemon(chosenIndex);
      std::cout << "\nYou send out " << selectedPokemon->name << "!\n";
      
      // Register new Pokemon with health bar system
      if (healthBarListener && selectedPokemon) {
        healthBarListener->registerPokemon(selectedPokemon, "Player");
      }
    }
  }

  // Handle opponent Pokemon fainting
  if (!opponentSelectedPokemon->isAlive()) {
    std::cout << "\nOpponent's " << opponentSelectedPokemon->name
              << " has fainted!" << std::endl;

    // Check if opponent has any Pokemon left
    if (!opponentTeam.hasAlivePokemon()) {
      return;  // Battle will end
    }

    // Opponent automatically sends out next Pokemon (AI behavior)
    auto *newPokemon = opponentTeam.getFirstAlivePokemon();
    if (newPokemon) {
      opponentSelectedPokemon = newPokemon;
      std::cout << "\nOpponent sends out " << opponentSelectedPokemon->name
                << "!\n";
      
      // Register new opponent Pokemon with health bar system
      if (healthBarListener && opponentSelectedPokemon) {
        healthBarListener->registerPokemon(opponentSelectedPokemon, "Opponent");
      }
    }
  }
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
        int previousHealth = selectedPokemon->current_hp;
        selectedPokemon->takeDamage(damage);
        std::cout << selectedPokemon->name << " is hurt by "
                  << Weather::getWeatherName(currentWeather) << "! (-" << damage
                  << " HP)" << std::endl;
        
        // Emit health change event for weather damage
        auto healthEvent = eventManager.createHealthChangeEvent(
          selectedPokemon, previousHealth, selectedPokemon->current_hp, 
          Weather::getWeatherName(currentWeather) + " damage"
        );
        eventManager.notifyHealthChanged(healthEvent);
      }
    }
  }

  if (opponentSelectedPokemon && opponentSelectedPokemon->isAlive()) {
    if (!Weather::isImmuneToWeatherDamage(currentWeather,
                                          opponentSelectedPokemon->types)) {
      int damage = Weather::getWeatherDamage(currentWeather,
                                             opponentSelectedPokemon->hp);
      if (damage > 0) {
        int previousHealth = opponentSelectedPokemon->current_hp;
        opponentSelectedPokemon->takeDamage(damage);
        std::cout << opponentSelectedPokemon->name << " is hurt by "
                  << Weather::getWeatherName(currentWeather) << "! (-" << damage
                  << " HP)" << std::endl;
        
        // Emit health change event for weather damage
        auto healthEvent = eventManager.createHealthChangeEvent(
          opponentSelectedPokemon, previousHealth, opponentSelectedPokemon->current_hp,
          Weather::getWeatherName(currentWeather) + " damage"
        );
        eventManager.notifyHealthChanged(healthEvent);
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

// AI Move Selection Implementation
int Battle::getAIMoveChoice() const {
  // Check if AI Pokemon must recharge
  if (opponentSelectedPokemon->mustRecharge()) {
    return -2; // Special value for forced recharge
  }
  
  // Check if AI Pokemon is charging a move
  if (opponentSelectedPokemon->isCharging()) {
    return opponentSelectedPokemon->getChargingMoveIndex(); // Must execute charging move
  }
  
  switch (aiDifficulty) {
  case AIDifficulty::EASY:
    return getAIMoveEasy();
  case AIDifficulty::MEDIUM:
    return getAIMoveMedium();
  case AIDifficulty::HARD:
    return getAIMoveHard();
  case AIDifficulty::EXPERT:
    return getAIMoveExpert();
  default:
    return getAIMoveEasy();
  }
}

// Easy AI: Random move selection (current behavior)
int Battle::getAIMoveEasy() const {
  std::vector<int> usableMoves;

  // Find moves with PP
  for (int i = 0; i < static_cast<int>(opponentSelectedPokemon->moves.size());
       ++i) {
    if (opponentSelectedPokemon->moves[i].canUse()) {
      usableMoves.push_back(i);
    }
  }

  // If no moves have PP, use first move anyway
  if (usableMoves.empty()) {
    return 0;
  }

  // Choose random move from usable moves
  return usableMoves[rand() % usableMoves.size()];
}

// Medium AI: Basic type effectiveness consideration
int Battle::getAIMoveMedium() const {
  std::vector<int> usableMoves;
  std::vector<double> moveScores;

  // Find moves with PP and calculate their type effectiveness
  for (int i = 0; i < static_cast<int>(opponentSelectedPokemon->moves.size());
       ++i) {
    if (opponentSelectedPokemon->moves[i].canUse()) {
      usableMoves.push_back(i);

      // Calculate type effectiveness score
      const Move &move = opponentSelectedPokemon->moves[i];
      double typeMultiplier =
          calculateTypeAdvantage(move.type, selectedPokemon->types);

      // Score based on type effectiveness and move power
      double score = (move.power > 0 ? move.power : 50) * typeMultiplier;
      moveScores.push_back(score);
    }
  }

  // If no moves have PP, use first move anyway
  if (usableMoves.empty()) {
    return 0;
  }

  // Find moves with the highest score (super effective moves)
  double maxScore = *std::max_element(moveScores.begin(), moveScores.end());
  std::vector<int> bestMoves;

  for (size_t i = 0; i < moveScores.size(); ++i) {
    if (moveScores[i] == maxScore) {
      bestMoves.push_back(usableMoves[i]);
    }
  }

  // Randomly select from the best moves
  return bestMoves[rand() % bestMoves.size()];
}

// Hard AI: Smart type effectiveness, strategic switching, status moves
int Battle::getAIMoveHard() const {
  std::vector<int> usableMoves;
  std::vector<double> moveScores;

  // Find moves with PP and calculate comprehensive scores
  for (int i = 0; i < static_cast<int>(opponentSelectedPokemon->moves.size());
       ++i) {
    if (opponentSelectedPokemon->moves[i].canUse()) {
      usableMoves.push_back(i);

      const Move &move = opponentSelectedPokemon->moves[i];
      double score = evaluateMoveScore(move, *opponentSelectedPokemon, *selectedPokemon);
      
      // Hard AI considerations:
      
      // 1. Prioritize OHKO moves against low health targets
      if (move.category == "ohko" && selectedPokemon->getHealthPercentage() < 30) {
        score += 200;
      }
      
      // 2. Prefer status moves early in battle if opponent is healthy
      if (move.power <= 0 && selectedPokemon->getHealthPercentage() > 70) {
        if (!selectedPokemon->hasStatusCondition()) {
          if (move.name == "toxic" || move.name == "will-o-wisp" || 
              move.name == "sleep-powder" || move.name == "thunder-wave") {
            score += 60;
          }
        }
      }
      
      // 3. Heavily favor super effective moves
      double typeMultiplier = calculateTypeAdvantage(move.type, selectedPokemon->types);
      if (typeMultiplier >= 2.0) {
        score *= 1.8; // Extra boost for super effective
      } else if (typeMultiplier <= 0.5) {
        score *= 0.3; // Heavy penalty for not very effective
      }
      
      // 4. Consider opponent's health for finishing moves
      if (selectedPokemon->getHealthPercentage() < 25) {
        if (move.power > 0) {
          score *= 1.5; // Prioritize damage when opponent is low
        }
      }
      
      // 5. Weather synergy
      double weatherMultiplier = Weather::getWeatherDamageMultiplier(currentWeather, move.type);
      if (weatherMultiplier > 1.0) {
        score *= 1.3; // Boost weather-synergistic moves
      }
      
      // 6. Stat modification strategy
      if (move.category == "net-good-stats") {
        // Prefer setup moves if we have health and no stat boosts yet
        if (opponentSelectedPokemon->getHealthPercentage() > 60) {
          score += 45;
        }
      }

      moveScores.push_back(score);
    }
  }

  // If no moves have PP, use first move anyway
  if (usableMoves.empty()) {
    return 0;
  }

  // Hard AI uses weighted selection instead of always picking the best
  // This makes it less predictable while still being smart
  
  // Sort moves by score (highest first)
  std::vector<std::pair<double, int>> scoredMoves;
  for (size_t i = 0; i < moveScores.size(); ++i) {
    scoredMoves.push_back({moveScores[i], usableMoves[i]});
  }
  std::sort(scoredMoves.begin(), scoredMoves.end(), std::greater<>());

  // Weighted selection: 50% chance for best move, 30% for second best, 20% for others
  int randomValue = rand() % 100;
  
  if (randomValue < 50 || scoredMoves.size() == 1) {
    return scoredMoves[0].second; // Best move
  } else if (randomValue < 80 && scoredMoves.size() > 1) {
    return scoredMoves[1].second; // Second best move
  } else {
    // Random selection from remaining moves
    int randomIndex = 2 + (rand() % std::max(1, static_cast<int>(scoredMoves.size()) - 2));
    if (randomIndex >= static_cast<int>(scoredMoves.size())) {
      randomIndex = static_cast<int>(scoredMoves.size()) - 1;
    }
    return scoredMoves[randomIndex].second;
  }
}

// Expert AI: TODO - Implement expert-level AI with prediction
int Battle::getAIMoveExpert() const {
  // For now, fallback to Easy AI until we implement this
  return getAIMoveEasy();
}

// Evaluate move effectiveness for AI decision making
int Battle::evaluateMoveScore(const Move &move, const Pokemon &attacker,
                              const Pokemon &defender) const {
  // Base score from move power
  int baseScore = move.power > 0 ? move.power : 50;

  // Type effectiveness multiplier
  double typeMultiplier = calculateTypeAdvantage(move.type, defender.types);

  // STAB (Same Type Attack Bonus)
  double stabMultiplier = 1.0;
  for (const auto &type : attacker.types) {
    if (type == move.type) {
      stabMultiplier = 1.5;
      break;
    }
  }

  // Weather bonus
  double weatherMultiplier =
      Weather::getWeatherDamageMultiplier(currentWeather, move.type);

  // Calculate final score
  double finalScore =
      baseScore * typeMultiplier * stabMultiplier * weatherMultiplier;

  // Bonus for status moves that could be beneficial
  if (move.power <= 0) {
    if (move.stat_chance > 0) {
      finalScore += 30; // Buff/debuff moves get bonus points
    }
    if (move.name == "toxic" || move.name == "will-o-wisp" ||
        move.name == "sleep-powder") {
      finalScore += 40; // Status condition moves get bonus points
    }
  }

  return static_cast<int>(finalScore);
}

// Calculate type advantage multiplier (placeholder for future AI levels)
double Battle::calculateTypeAdvantage(
    const std::string &moveType,
    const std::vector<std::string> &defenderTypes) const {
  // TODO: Use this for Medium/Hard/Expert AI levels
  return TypeEffectiveness::getEffectivenessMultiplier(moveType, defenderTypes);
}

// AI Pokemon switching (placeholder for future implementation)
int Battle::getAIPokemonChoice() const {
  std::vector<int> availablePokemon;
  std::vector<double> pokemonScores;

  // Find all alive Pokemon except current one
  for (int i = 0; i < static_cast<int>(opponentTeam.size()); ++i) {
    const auto *pokemon = opponentTeam.getPokemon(i);
    if (pokemon && pokemon->isAlive() && pokemon != opponentSelectedPokemon) {
      availablePokemon.push_back(i);
      
      double score = 0.0;
      
      if (aiDifficulty == AIDifficulty::HARD || aiDifficulty == AIDifficulty::EXPERT) {
        // Smart switching for Hard/Expert AI
        
        // 1. Health consideration (prefer healthier Pokemon)
        score += pokemon->getHealthPercentage() * 0.5;
        
        // 2. Type advantage assessment
        for (const auto& move : pokemon->moves) {
          if (move.canUse() && move.power > 0) {
            double typeMultiplier = calculateTypeAdvantage(move.type, selectedPokemon->types);
            if (typeMultiplier >= 2.0) {
              score += 40; // Big bonus for super effective moves
            } else if (typeMultiplier >= 1.0) {
              score += 10; // Small bonus for neutral/effective moves
            }
          }
        }
        
        // 3. Defensive typing (resistance to opponent's moves)
        int resistanceCount = 0;
        for (const auto& move : selectedPokemon->moves) {
          if (move.canUse() && move.power > 0) {
            double typeMultiplier = calculateTypeAdvantage(move.type, pokemon->types);
            if (typeMultiplier <= 0.5) {
              resistanceCount++;
            }
          }
        }
        score += resistanceCount * 15;
        
        // 4. Status condition consideration
        if (!pokemon->hasStatusCondition()) {
          score += 20; // Prefer healthy, non-statused Pokemon
        }
        
        // 5. Move PP availability
        int usableMoves = 0;
        for (const auto& move : pokemon->moves) {
          if (move.canUse()) {
            usableMoves++;
          }
        }
        score += usableMoves * 5;
        
      } else {
        // Simple logic for Easy/Medium AI - just prefer healthier Pokemon
        score = pokemon->getHealthPercentage();
      }
      
      pokemonScores.push_back(score);
    }
  }

  if (availablePokemon.empty()) {
    return -1;
  }

  // For Hard AI, pick the best option most of the time, but add some randomness
  if (aiDifficulty == AIDifficulty::HARD) {
    // Find the highest scoring Pokemon
    auto maxIt = std::max_element(pokemonScores.begin(), pokemonScores.end());
    int bestIndex = std::distance(pokemonScores.begin(), maxIt);
    
    // 80% chance to pick the best, 20% chance for random
    if (rand() % 100 < 80) {
      return availablePokemon[bestIndex];
    } else {
      return availablePokemon[rand() % availablePokemon.size()];
    }
  } else {
    // Easy/Medium AI: just return first available or best for Medium
    if (aiDifficulty == AIDifficulty::MEDIUM && !pokemonScores.empty()) {
      auto maxIt = std::max_element(pokemonScores.begin(), pokemonScores.end());
      int bestIndex = std::distance(pokemonScores.begin(), maxIt);
      return availablePokemon[bestIndex];
    } else {
      return availablePokemon[0];
    }
  }
}

// AI switching decision (Easy AI never switches voluntarily)
bool Battle::shouldAISwitch() const {
  switch (aiDifficulty) {
  case AIDifficulty::EASY:
    return false; // Easy AI never switches
  case AIDifficulty::MEDIUM:
    // Medium AI has basic switching logic
    if (opponentSelectedPokemon->getHealthPercentage() < 20) {
      return opponentTeam.hasAlivePokemon() && getAIPokemonChoice() != -1;
    }
    return false;
  case AIDifficulty::HARD:
    // Hard AI uses strategic switching logic
    if (!opponentSelectedPokemon || !selectedPokemon) {
      return false;
    }
    
    // Switch if current Pokemon is in serious danger (low health + bad matchup)
    if (opponentSelectedPokemon->getHealthPercentage() < 30) {
      // Check if player has super effective moves
      for (const auto& move : selectedPokemon->moves) {
        if (move.canUse() && move.power > 0) {
          double typeMultiplier = calculateTypeAdvantage(move.type, opponentSelectedPokemon->types);
          if (typeMultiplier >= 2.0) {
            return opponentTeam.hasAlivePokemon() && getAIPokemonChoice() != -1;
          }
        }
      }
    }
    
    // Switch if heavily disadvantaged type-wise (even with good health)
    if (opponentSelectedPokemon->getHealthPercentage() > 50) {
      int disadvantageCount = 0;
      int advantageCount = 0;
      
      for (const auto& move : selectedPokemon->moves) {
        if (move.canUse() && move.power > 0) {
          double typeMultiplier = calculateTypeAdvantage(move.type, opponentSelectedPokemon->types);
          if (typeMultiplier >= 2.0) {
            disadvantageCount++;
          }
        }
      }
      
      for (const auto& move : opponentSelectedPokemon->moves) {
        if (move.canUse() && move.power > 0) {
          double typeMultiplier = calculateTypeAdvantage(move.type, selectedPokemon->types);
          if (typeMultiplier >= 2.0) {
            advantageCount++;
          }
        }
      }
      
      // Switch if we're heavily disadvantaged and have another option
      if (disadvantageCount >= 2 && advantageCount == 0) {
        return opponentTeam.hasAlivePokemon() && getAIPokemonChoice() != -1;
      }
    }
    
    return false;
  case AIDifficulty::EXPERT:
    // TODO: Advanced switching logic
    return false;
  default:
    return false;
  }
}

void Battle::configureHealthBarAnimation(HealthBarAnimator::AnimationSpeed speed, HealthBarAnimator::ColorTheme theme) {
  if (!healthBarAnimator || !healthBarListener) {
    // Initialize if not already done
    HealthBarAnimator::Config config;
    config.speed = speed;
    config.colorTheme = theme;
    healthBarAnimator = std::make_shared<HealthBarAnimator>(config);
    healthBarListener = std::make_shared<HealthBarEventListener>(healthBarAnimator);
    eventManager.subscribe(healthBarListener);
  } else {
    // Update existing configuration
    HealthBarAnimator::Config config;
    config.speed = speed;
    config.colorTheme = theme;
    
    // Create new animator with updated config
    healthBarAnimator = std::make_shared<HealthBarAnimator>(config);
    
    // Update listener's animator
    healthBarListener = std::make_shared<HealthBarEventListener>(healthBarAnimator);
    
    // Re-register with event manager
    eventManager.subscribe(healthBarListener);
    
    // Re-register all currently selected Pokemon
    if (selectedPokemon && healthBarListener) {
      healthBarListener->registerPokemon(selectedPokemon, "Player");
    }
    if (opponentSelectedPokemon && healthBarListener) {
      healthBarListener->registerPokemon(opponentSelectedPokemon, "Opponent");
    }
  }
}

void Battle::processStatusConditionWithEvents(Pokemon& pokemon) {
  if (!pokemon.hasStatusCondition()) return;
  
  int previousHealth = pokemon.current_hp;
  pokemon.processStatusCondition();
  
  // Only emit event if health actually changed
  if (pokemon.current_hp != previousHealth) {
    auto healthEvent = eventManager.createHealthChangeEvent(
      &pokemon, previousHealth, pokemon.current_hp, 
      pokemon.getStatusConditionName() + " damage"
    );
    eventManager.notifyHealthChanged(healthEvent);
  }
}