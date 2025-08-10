#pragma once

#include <random>
#include <vector>

#include "pokemon.h"
#include "team.h"
#include "weather.h"

// AI difficulty levels
enum class AIDifficulty {
  EASY,    // Basic type awareness, prefers higher power moves
  MEDIUM,  // Adds status consideration, weather awareness
  HARD,    // Strategic switching, stat modifications
  EXPERT   // Predictive analysis, multi-turn planning
};

// Move evaluation result
struct MoveEvaluation {
  int moveIndex;
  double score;
  std::string reasoning;  // For debugging and analysis
};

// Pokemon switch evaluation result
struct SwitchEvaluation {
  int pokemonIndex;
  double score;
  std::string reasoning;
};

// Battle state information for AI decision making
struct BattleState {
  Pokemon* aiPokemon;               // Current AI Pokemon
  Pokemon* opponentPokemon;         // Current opponent Pokemon
  Team* aiTeam;                     // Full AI team
  Team* opponentTeam;               // Full opponent team
  WeatherCondition currentWeather;  // Current weather condition
  int weatherTurnsRemaining;        // Weather duration left
  int turnNumber;                   // Current turn count
  
  // Deterministic RNG seed for Expert AI paralysis checks
  // Used to ensure consistent behavior during minimax search
  mutable std::mt19937 deterministicRng{0};
};

// Abstract base class for AI strategies
class AIStrategy {
 public:
  explicit AIStrategy(AIDifficulty difficulty) : difficulty_(difficulty) {}
  virtual ~AIStrategy() = default;

  // Pure virtual methods that must be implemented by each AI difficulty
  virtual MoveEvaluation chooseBestMove(const BattleState& battleState) = 0;
  virtual SwitchEvaluation chooseBestSwitch(const BattleState& battleState) = 0;

  // Get AI difficulty level
  AIDifficulty getDifficulty() const { return difficulty_; }

  // Check if AI should switch (called when Pokemon can switch)
  virtual bool shouldSwitch(const BattleState& battleState) = 0;

 protected:
  AIDifficulty difficulty_;

  // Utility methods available to all AI implementations
  double calculateTypeEffectiveness(
      const std::string& moveType,
      const std::vector<std::string>& defenderTypes) const;

  double estimateDamage(const Pokemon& attacker, const Pokemon& defender,
                        const Move& move, WeatherCondition weather) const;

  bool isPokemonThreatened(const Pokemon& pokemon,
                           const Pokemon& opponent) const;

  double calculateHealthRatio(const Pokemon& pokemon) const;

  std::vector<Move*> getUsableMoves(Pokemon& pokemon) const;
};