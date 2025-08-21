#pragma once

#include <iostream>
#include <memory>
#include <random>
#include <unordered_map>

#include "move.h"
#include "pokemon.h"
#include "team.h"
#include "type_effectiveness.h"
#include "weather.h"
#include "battle_events.h"
#include "health_bar_animator.h"
#include "health_bar_event_listener.h"

class Battle {
 public:
  // AI Difficulty Levels
  enum class AIDifficulty {
    EASY,   // Random moves, no switching
    MEDIUM, // Basic type effectiveness, limited switching
    HARD,   // Smart type effectiveness, strategic switching
    EXPERT  // Advanced strategy with prediction
  };

  // Constructor
  Battle(const Team &playerTeam, const Team &opponentTeam,
         AIDifficulty aiDifficulty = AIDifficulty::EASY);

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

  // AI Configuration
  AIDifficulty aiDifficulty;

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

  // AI move selection based on difficulty
  int getAIMoveChoice() const;
  int getAIPokemonChoice() const;
  bool shouldAISwitch() const;

  // AI difficulty-specific move selection
  int getAIMoveEasy() const;
  int getAIMoveMedium() const;
  int getAIMoveHard() const;
  int getAIMoveExpert() const;

  // AI strategy helpers
  int evaluateMoveScore(const Move &move, const Pokemon &attacker,
                        const Pokemon &defender) const;
  double
  calculateTypeAdvantage(const std::string &moveType,
                         const std::vector<std::string> &defenderTypes) const;

  // Random number generation for critical hits
  mutable std::mt19937 rng;
  mutable std::uniform_real_distribution<double> criticalDistribution;
  
  // Battle event system
  BattleEvents::BattleEventManager eventManager;
  
  // Health bar animation system
  std::shared_ptr<HealthBarAnimator> healthBarAnimator;
  std::shared_ptr<HealthBarEventListener> healthBarListener;
  
  // Health state tracking for smooth transitions
  mutable std::unordered_map<Pokemon*, int> previousHealthState;
  
public:
  // Event system access
  BattleEvents::BattleEventManager& getEventManager() { return eventManager; }
  
  // Health bar animation configuration
  void configureHealthBarAnimation(HealthBarAnimator::AnimationSpeed speed = HealthBarAnimator::AnimationSpeed::NORMAL,
                                  HealthBarAnimator::ColorTheme theme = HealthBarAnimator::ColorTheme::BASIC);
};