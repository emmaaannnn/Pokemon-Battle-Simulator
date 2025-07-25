#include "ai_strategy.h"

#include <algorithm>
#include <cmath>

#include "type_effectiveness.h"
#include "weather.h"

double AIStrategy::calculateTypeEffectiveness(
    const std::string& moveType,
    const std::vector<std::string>& defenderTypes) const {
  return TypeEffectiveness::getEffectivenessMultiplier(moveType, defenderTypes);
}

double AIStrategy::estimateDamage(const Pokemon& attacker,
                                  const Pokemon& defender, const Move& move,
                                  WeatherCondition weather) const {
  // Simplified damage calculation for AI estimation
  // Based on the actual damage formula but streamlined for AI decision making

  if (move.power <= 0) {
    return 0.0;  // Status moves do no direct damage
  }

  // Determine if move is physical or special
  int attackStat = attacker.attack;
  int defenseStat = defender.defense;

  if (move.damage_class == "special") {
    attackStat = attacker.special_attack;
    defenseStat = defender.special_defense;
  }

  // Apply stat stage modifiers (simplified)
  double attackMultiplier = 1.0 + (attacker.attack_stage * 0.5);
  double defenseMultiplier = 1.0 + (defender.defense_stage * 0.5);

  attackStat = static_cast<int>(attackStat * attackMultiplier);
  defenseStat = static_cast<int>(defenseStat * defenseMultiplier);

  // Base damage calculation (simplified Pokemon formula)
  double baseDamage = ((2.0 * 50 + 10) / 250.0) *
                          (static_cast<double>(attackStat) / defenseStat) *
                          move.power +
                      2;

  // Type effectiveness
  double typeMultiplier = calculateTypeEffectiveness(move.type, defender.types);
  baseDamage *= typeMultiplier;

  // STAB (Same Type Attack Bonus)
  bool hasSTAB = std::find(attacker.types.begin(), attacker.types.end(),
                           move.type) != attacker.types.end();
  if (hasSTAB) {
    baseDamage *= 1.5;
  }

  // Weather effects
  double weatherMultiplier =
      Weather::getWeatherDamageMultiplier(weather, move.type);
  baseDamage *= weatherMultiplier;

  // Critical hit average (1/16 chance for 2x damage = ~1.06x average)
  baseDamage *= 1.06;

  return std::max(1.0, baseDamage);
}

bool AIStrategy::isPokemonThreatened(const Pokemon& pokemon,
                                     const Pokemon& opponent) const {
  // Check if the Pokemon is in danger from the opponent
  double healthRatio = calculateHealthRatio(pokemon);

  // If below 25% health, consider threatened
  if (healthRatio < 0.25) {
    return true;
  }

  // Check if opponent has super effective moves
  for (const auto& move : opponent.moves) {
    if (!move.canUse()) continue;

    double effectiveness = calculateTypeEffectiveness(move.type, pokemon.types);
    if (effectiveness >= 2.0) {  // Super effective
      // Estimate if this move could potentially KO
      double estimatedDamage =
          estimateDamage(opponent, pokemon, move, WeatherCondition::NONE);
      if (estimatedDamage >= pokemon.current_hp * 0.8) {
        return true;
      }
    }
  }

  return false;
}

double AIStrategy::calculateHealthRatio(const Pokemon& pokemon) const {
  if (pokemon.hp <= 0) return 0.0;
  return static_cast<double>(pokemon.current_hp) / pokemon.hp;
}

std::vector<Move*> AIStrategy::getUsableMoves(Pokemon& pokemon) const {
  std::vector<Move*> usableMoves;

  for (auto& move : pokemon.moves) {
    if (move.canUse()) {
      usableMoves.push_back(&move);
    }
  }

  return usableMoves;
}