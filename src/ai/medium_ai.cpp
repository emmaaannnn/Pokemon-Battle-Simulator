#include "medium_ai.h"

#include <algorithm>
#include <cmath>

#include "weather.h"

MoveEvaluation MediumAI::chooseBestMove(const BattleState& battleState) {
  std::vector<Move*> usableMoves = getUsableMoves(*battleState.aiPokemon);

  if (usableMoves.empty()) {
    return {0, -100.0, "No PP remaining on any moves"};
  }

  MoveEvaluation bestMove{-1, -1000.0, ""};

  for (size_t i = 0; i < battleState.aiPokemon->moves.size(); ++i) {
    if (!battleState.aiPokemon->moves[i].canUse()) continue;

    const Move& move = battleState.aiPokemon->moves[i];
    double score = scoreMoveAdvanced(move, battleState);

    if (score > bestMove.score) {
      bestMove.moveIndex = static_cast<int>(i);
      bestMove.score = score;
      bestMove.reasoning =
          "Medium AI: Advanced scoring with weather and status consideration";
    }
  }

  return bestMove;
}

SwitchEvaluation MediumAI::chooseBestSwitch(const BattleState& battleState) {
  SwitchEvaluation bestSwitch{-1, -1000.0, ""};

  for (int i = 0; i < static_cast<int>(battleState.aiTeam->size()); ++i) {
    Pokemon* pokemon = battleState.aiTeam->getPokemon(i);
    if (!pokemon || !pokemon->isAlive() || pokemon == battleState.aiPokemon) {
      continue;
    }

    double matchupScore =
        evaluatePokemonMatchup(*pokemon, *battleState.opponentPokemon);
    double healthScore =
        calculateHealthRatio(*pokemon) * 30.0;  // Prefer healthier Pokemon

    double totalScore = matchupScore + healthScore;

    if (totalScore > bestSwitch.score) {
      bestSwitch.pokemonIndex = i;
      bestSwitch.score = totalScore;
      bestSwitch.reasoning = "Medium AI: Better type matchup and health";
    }
  }

  return bestSwitch;
}

bool MediumAI::shouldSwitch(const BattleState& battleState) {
  // More strategic switching than EasyAI
  double healthRatio = calculateHealthRatio(*battleState.aiPokemon);

  // Switch if health is low
  if (healthRatio < 0.25) return true;

  // Switch if opponent has significant type advantage
  double currentMatchup = evaluatePokemonMatchup(*battleState.aiPokemon,
                                                 *battleState.opponentPokemon);
  if (currentMatchup < -50.0) return true;

  // Check if we have a much better Pokemon available
  SwitchEvaluation bestAlternative = chooseBestSwitch(battleState);
  if (bestAlternative.score > currentMatchup + 30.0) return true;

  return false;
}

double MediumAI::scoreMoveAdvanced(const Move& move,
                                   const BattleState& battleState) const {
  if (move.power > 0) {
    return scoreDamageMove(move, battleState);
  } else {
    return scoreStatusMove(move, battleState);
  }
}

double MediumAI::scoreStatusMove(const Move& move,
                                 const BattleState& battleState) const {
  double score = 10.0;  // Base value for status moves

  // Consider opponent's health - status moves better against healthy opponents
  double opponentHealthRatio =
      calculateHealthRatio(*battleState.opponentPokemon);
  score += opponentHealthRatio * 40.0;

  // Check if opponent already has a status condition
  if (battleState.opponentPokemon->status != StatusCondition::NONE) {
    score -= 30.0;  // Don't use status moves on already statused Pokemon
  }

  // Specific status move evaluation
  if (move.ailment_name == "poison" || move.ailment_name == "burn") {
    // Poison/burn better against high HP Pokemon
    score += battleState.opponentPokemon->current_hp * 0.3;
  } else if (move.ailment_name == "paralysis") {
    // Paralysis good against fast Pokemon
    if (battleState.opponentPokemon->speed > battleState.aiPokemon->speed) {
      score += 25.0;
    }
  } else if (move.ailment_name == "sleep") {
    // Sleep is generally powerful
    score += 35.0;
  }

  return score;
}

double MediumAI::scoreDamageMove(const Move& move,
                                 const BattleState& battleState) const {
  double score = 0.0;

  // Estimate actual damage this move would deal
  double estimatedDamage =
      estimateDamage(*battleState.aiPokemon, *battleState.opponentPokemon, move,
                     battleState.currentWeather);

  // Weight damage, but less heavily if this move can KO (focus on reliability instead)
  bool canKO = estimatedDamage >= battleState.opponentPokemon->current_hp;
  double damageWeight = canKO ? 1.0 : 2.0;  // Reduce damage importance for KO moves
  score += estimatedDamage * damageWeight;

  // Weather bonus
  score += scoreWeatherAdvantage(move, battleState.currentWeather);

  // Accuracy consideration
  score += (move.accuracy - 80) * 0.5;  // Penalty for low accuracy moves

  // Prefer moves that can KO, with very strong preference for reliable KOs  
  if (estimatedDamage >= battleState.opponentPokemon->current_hp) {
    // For KO moves, accuracy is critical - scale bonus heavily by reliability
    double accuracyFactor = move.accuracy / 100.0;
    double koBonus = 200.0 * accuracyFactor;  // Large bonus scaled by accuracy
    score += koBonus;
    
    // Massive additional bonus for guaranteed KOs (perfect accuracy)
    if (move.accuracy >= 100) {
      score += 100.0;  // Huge reliability bonus for guaranteed success
    }
  }

  // STAB consideration (already in damage estimate, but small extra bonus)
  bool hasSTAB = std::find(battleState.aiPokemon->types.begin(),
                           battleState.aiPokemon->types.end(),
                           move.type) != battleState.aiPokemon->types.end();
  if (hasSTAB) score += 5.0;

  return score;
}

double MediumAI::scoreWeatherAdvantage(const Move& move,
                                       WeatherCondition weather) const {
  double multiplier = Weather::getWeatherDamageMultiplier(weather, move.type);

  if (multiplier > 1.0) {
    // Weather boost - bonus proportional to power increase
    return (multiplier - 1.0) * move.power * 0.5;
  }
  if (multiplier < 1.0) {
    // Weather penalty - penalty proportional to power reduction
    return (multiplier - 1.0) * move.power * 2.0;  // Stronger penalty to account for damage weighting
  }
  return 0.0;  // No weather effect
}

double MediumAI::evaluatePokemonMatchup(const Pokemon& attacker,
                                        const Pokemon& defender) const {
  double score = 0.0;

  // Check type advantages for attacker's moves
  for (const auto& move : attacker.moves) {
    if (!move.canUse() || move.power <= 0) continue;

    double typeEffectiveness =
        calculateTypeEffectiveness(move.type, defender.types);
    if (typeEffectiveness >= 2.0)
      score += 40.0;  // Super effective
    else if (typeEffectiveness <= 0.5)
      score -= 20.0;  // Not very effective
    else if (typeEffectiveness == 0.0)
      score -= 30.0;  // No effect
  }

  // Consider speed advantage
  if (attacker.speed > defender.speed)
    score += 10.0;
  else
    score -= 5.0;

  // Consider defensive typing
  // This is simplified - in reality would check all of defender's potential
  // moves
  bool hasTypeResistance = false;
  for (const auto& defenderType : defender.types) {
    for (const auto& attackerType : attacker.types) {
      double resistance =
          calculateTypeEffectiveness(defenderType, {attackerType});
      if (resistance >= 2.0) {
        score -= 15.0;  // Defender can hit us super effectively
        break;
      } else if (resistance <= 0.5) {
        hasTypeResistance = true;
      }
    }
  }

  if (hasTypeResistance) score += 10.0;

  return score;
}