#pragma once

#include "move.h"
#include "pokemon.h"
#include "team.h"
#include "type_effectiveness.h"
#include "weather.h"
#include <iostream>
#include <memory>
#include <random>

class Battle {
public:
  // Constructor
  Battle(const Team &playerTeam, const Team &opponentTeam);

  // Main battle interface
  void startBattle();
  bool isBattleOver() const;

  // Battle state queries
  enum class BattleResult { ONGOING, PLAYER_WINS, OPPONENT_WINS, DRAW };

  BattleResult getBattleResult() const;

private:
  Team playerTeam;
  Team opponentTeam;
  Pokemon *selectedPokemon;
  Pokemon *opponentSelectedPokemon;

  // Weather state
  WeatherCondition currentWeather;
  int weatherTurnsRemaining;

  // Battle flow methods
  void selectPokemon();
  void selectOpponentPokemon();
  void executeTurn();
  void executeMove(Pokemon &attacker, Pokemon &defender, int moveIndex);
  void handlePokemonFainted();

  // Utility methods
  void displayHealth(const Pokemon &pokemon) const;
  void displayBattleStatus() const;
  bool playerFirst(const Move &playerMove, const Move &opponentMove) const;
  struct DamageResult {
    int damage;
    bool wasCritical;
    bool hadSTAB;
  };

  DamageResult calculateDamageWithEffects(const Pokemon &attacker,
                                          const Pokemon &defender,
                                          const Move &move) const;
  int calculateDamage(const Pokemon &attacker, const Pokemon &defender,
                      const Move &move) const;

  // Enhanced damage calculation methods
  bool hasSTAB(const Pokemon &attacker, const Move &move) const;
  bool isCriticalHit(const Move &move) const;
  double calculateSTABMultiplier(const Pokemon &attacker,
                                 const Move &move) const;
  double calculateCriticalMultiplier(const Move &move) const;

  // Accuracy checking
  bool checkMoveAccuracy(const Move &move) const;

  // Stat modification handling
  void applyStatModification(Pokemon &attacker, Pokemon &defender,
                             const Move &move);

  // Weather handling
  void processWeather();
  void setWeather(WeatherCondition weather, int turns = 5);
  void displayWeather() const;

  // Input handling
  int getMoveChoice() const;
  int getPokemonChoice() const;

  // Random number generation for critical hits
  mutable std::mt19937 rng;
  mutable std::uniform_real_distribution<double> criticalDistribution;
};