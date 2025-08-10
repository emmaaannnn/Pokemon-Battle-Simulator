#include "hard_ai.h"

#include <algorithm>
#include <cmath>

#include "weather.h"

MoveEvaluation HardAI::chooseBestMove(const BattleState& battleState) {
  std::vector<Move*> usableMoves = getUsableMoves(*battleState.aiPokemon);

  if (usableMoves.empty()) {
    return {0, -100.0, "No PP remaining on any moves"};
  }

  MoveEvaluation bestMove{-1, -1000.0, ""};

  // Check if this is a good setup opportunity
  double setupValue = evaluateSetupOpportunity(battleState);

  for (size_t i = 0; i < battleState.aiPokemon->moves.size(); ++i) {
    if (!battleState.aiPokemon->moves[i].canUse()) continue;

    const Move& move = battleState.aiPokemon->moves[i];
    double score = evaluateComplexMove(move, battleState);

    // Bonus for setup moves when opportunity is good
    if (move.power == 0 && setupValue > 0) {
      double statModValue = calculateStatModificationValue(move, battleState);
      score += statModValue + setupValue;
    }

    // Risk assessment
    double riskScore = assessRisk(move, battleState);
    score += riskScore;

    if (score > bestMove.score) {
      bestMove.moveIndex = static_cast<int>(i);
      bestMove.score = score;
      bestMove.reasoning =
          "Hard AI: Complex strategic analysis with risk assessment";
    }
  }

  return bestMove;
}

SwitchEvaluation HardAI::chooseBestSwitch(const BattleState& battleState) {
  SwitchEvaluation bestSwitch{-1, -1000.0, ""};

  for (int i = 0; i < static_cast<int>(battleState.aiTeam->size()); ++i) {
    Pokemon* pokemon = battleState.aiTeam->getPokemon(i);
    if (!pokemon || !pokemon->isAlive() || pokemon == battleState.aiPokemon) {
      continue;
    }

    double score = 0.0;

    // Analyze how well this Pokemon threatens opponent team
    double threatScore = analyzeTeamThreat(*pokemon, *battleState.opponentTeam);
    score += threatScore;

    // Health consideration
    double healthScore = calculateHealthRatio(*pokemon) * 40.0;
    score += healthScore;

    // Check if this Pokemon can potentially sweep
    if (canSweepTeam(*pokemon, *battleState.opponentTeam)) {
      score += 60.0;
    }

    // Consider immediate matchup against current opponent
    double immediateMatchup = 0.0;
    for (const auto& move : pokemon->moves) {
      if (!move.canUse()) continue;
      double effectiveness = calculateTypeEffectiveness(
          move.type, battleState.opponentPokemon->types);
      immediateMatchup += effectiveness * move.power * 0.1;
    }
    score += immediateMatchup;

    if (score > bestSwitch.score) {
      bestSwitch.pokemonIndex = i;
      bestSwitch.score = score;
      bestSwitch.reasoning =
          "Hard AI: Strategic team analysis and sweep potential";
    }
  }

  return bestSwitch;
}

bool HardAI::shouldSwitch(const BattleState& battleState) {
  double healthRatio = calculateHealthRatio(*battleState.aiPokemon);

  // Always switch if very low health
  if (healthRatio < 0.2) return true;

  // Predict incoming damage
  double predictedDamage = predictOpponentDamage(battleState);
  if (predictedDamage >= battleState.aiPokemon->current_hp * 0.8) {
    // Likely to be KO'd, consider switching
    SwitchEvaluation bestAlternative = chooseBestSwitch(battleState);
    if (bestAlternative.score > 30.0) return true;
  }

  // Check if we have a much better Pokemon for this matchup
  double currentThreatCount =
      countTeamThreats(*battleState.aiPokemon, *battleState.opponentTeam);

  for (int i = 0; i < static_cast<int>(battleState.aiTeam->size()); ++i) {
    Pokemon* alternative = battleState.aiTeam->getPokemon(i);
    if (!alternative || !alternative->isAlive() ||
        alternative == battleState.aiPokemon) {
      continue;
    }

    double alternativeThreatCount =
        countTeamThreats(*alternative, *battleState.opponentTeam);

    // Switch if alternative threatens significantly more opponents
    if (alternativeThreatCount >= currentThreatCount + 2) {
      return true;
    }

    // Switch if alternative can sweep and current can't
    if (canSweepTeam(*alternative, *battleState.opponentTeam) &&
        !canSweepTeam(*battleState.aiPokemon, *battleState.opponentTeam)) {
      return true;
    }
  }

  return false;
}

double HardAI::evaluateComplexMove(const Move& move,
                                   const BattleState& battleState) const {
  double score = 0.0;

  if (move.power > 0) {
    // Damage move evaluation
    double estimatedDamage =
        estimateDamage(*battleState.aiPokemon, *battleState.opponentPokemon,
                       move, battleState.currentWeather);

    score += estimatedDamage * 2.5;

    // KO bonus
    if (estimatedDamage >= battleState.opponentPokemon->current_hp) {
      score += 120.0;
    }

    // Weather synergy
    double weatherMultiplier = Weather::getWeatherDamageMultiplier(
        battleState.currentWeather, move.type);
    score += (weatherMultiplier - 1.0) * 30.0;

    // Consider if this move helps against opponent's team
    int threatsHandled = 0;
    for (int i = 0; i < static_cast<int>(battleState.opponentTeam->size()); ++i) {
      Pokemon* opponent = battleState.opponentTeam->getPokemon(i);
      if (opponent && opponent->isAlive()) {
        double effectiveness =
            calculateTypeEffectiveness(move.type, opponent->types);
        if (effectiveness >= 2.0) threatsHandled++;
      }
    }
    score += threatsHandled * 10.0;

  } else {
    // Status move evaluation
    if (battleState.opponentPokemon->status == StatusCondition::NONE) {
      double opponentHealth =
          calculateHealthRatio(*battleState.opponentPokemon);
      score += 30.0 + (opponentHealth *
                       40.0);  // Status moves better vs healthy opponents

      // Specific status considerations
      if (move.ailment_name == "sleep" || move.ailment_name == "paralysis") {
        score += 25.0;  // These are very disruptive
      }
    }
  }

  return score;
}

double HardAI::analyzeTeamThreat(const Pokemon& pokemon,
                                 const Team& opponentTeam) const {
  double threatScore = 0.0;

  for (int i = 0; i < static_cast<int>(opponentTeam.size()); ++i) {
    const Pokemon* opponent = opponentTeam.getPokemon(i);
    if (!opponent || !opponent->isAlive()) continue;

    // Check how well this Pokemon matches up against each opponent
    double bestMoveScore = 0.0;
    for (const auto& move : pokemon.moves) {
      if (!move.canUse() || move.power <= 0) continue;

      double effectiveness =
          calculateTypeEffectiveness(move.type, opponent->types);
      double moveScore = move.power * effectiveness;
      bestMoveScore = std::max(bestMoveScore, moveScore);
    }

    threatScore += bestMoveScore * 0.01;  // Scale appropriately

    // Bonus for super effective coverage
    if (bestMoveScore >= 160.0) {  // 80 power * 2.0 effectiveness
      threatScore += 15.0;
    }
  }

  return threatScore;
}

double HardAI::calculateStatModificationValue(
    const Move& move, const BattleState& battleState) const {
  // Simplified stat mod evaluation - in reality would check specific moves
  if (move.power > 0) return 0.0;  // Not a stat modification move

  double value = 20.0;  // Base value for stat modifications

  // More valuable when we can survive to use the boost
  double healthRatio = calculateHealthRatio(*battleState.aiPokemon);
  value += healthRatio * 30.0;

  // More valuable when opponent is weakened
  double opponentHealthRatio =
      calculateHealthRatio(*battleState.opponentPokemon);
  if (opponentHealthRatio < 0.5) value += 20.0;

  // Check if we can potentially sweep after setup
  if (canSweepTeam(*battleState.aiPokemon, *battleState.opponentTeam)) {
    value += 40.0;
  }

  return value;
}

double HardAI::evaluateSetupOpportunity(const BattleState& battleState) const {
  double setupValue = 0.0;

  // Better setup opportunity when opponent is statused
  if (battleState.opponentPokemon->status == StatusCondition::SLEEP ||
      battleState.opponentPokemon->status == StatusCondition::PARALYSIS) {
    setupValue += 30.0;
  }

  // Better when opponent is low on damaging moves
  int opponentDamageMoves = 0;
  for (const auto& move : battleState.opponentPokemon->moves) {
    if (move.canUse() && move.power > 0) opponentDamageMoves++;
  }

  if (opponentDamageMoves <= 1) setupValue += 25.0;

  // Better when we're healthy
  double ourHealthRatio = calculateHealthRatio(*battleState.aiPokemon);
  setupValue += ourHealthRatio * 20.0;

  return setupValue;
}

bool HardAI::canSweepTeam(const Pokemon& sweeper,
                          const Team& opponentTeam) const {
  int threatenedOpponents = 0;

  for (int i = 0; i < static_cast<int>(opponentTeam.size()); ++i) {
    const Pokemon* opponent = opponentTeam.getPokemon(i);
    if (!opponent || !opponent->isAlive()) continue;

    // Check if we have a move that can deal significant damage to this opponent
    bool canThreaten = false;
    for (const auto& move : sweeper.moves) {
      if (!move.canUse() || move.power <= 0) continue;

      double effectiveness =
          calculateTypeEffectiveness(move.type, opponent->types);
      if (effectiveness >= 1.0 && move.power >= 60) {
        canThreaten = true;
        break;
      }
    }

    if (canThreaten) threatenedOpponents++;
  }

  // Can sweep if we threaten at least 2/3 of opponent's team
  // Count alive opponents manually since getAlivePokemon() is not const
  int aliveOpponents = 0;
  for (int i = 0; i < static_cast<int>(opponentTeam.size()); ++i) {
    const Pokemon* pokemon = opponentTeam.getPokemon(i);
    if (pokemon && pokemon->isAlive()) {
      aliveOpponents++;
    }
  }
  
  return threatenedOpponents >=
         std::max(2, static_cast<int>(aliveOpponents * 2 / 3));
}

double HardAI::predictOpponentDamage(const BattleState& battleState) const {
  double maxDamage = 0.0;

  // Look at opponent's moves and estimate best damage they can do
  for (const auto& move : battleState.opponentPokemon->moves) {
    if (!move.canUse() || move.power <= 0) continue;

    double estimatedDamage =
        estimateDamage(*battleState.opponentPokemon, *battleState.aiPokemon,
                       move, battleState.currentWeather);

    maxDamage = std::max(maxDamage, estimatedDamage);
  }

  return maxDamage;
}

int HardAI::countTeamThreats(const Pokemon& pokemon,
                             const Team& opponentTeam) const {
  int threatCount = 0;

  for (int i = 0; i < static_cast<int>(opponentTeam.size()); ++i) {
    const Pokemon* opponent = opponentTeam.getPokemon(i);
    if (!opponent || !opponent->isAlive()) continue;

    // Check if we have a super effective move against this opponent
    for (const auto& move : pokemon.moves) {
      if (!move.canUse() || move.power <= 0) continue;

      double effectiveness =
          calculateTypeEffectiveness(move.type, opponent->types);
      if (effectiveness >= 2.0) {
        threatCount++;
        break;  // Found one super effective move, that's enough
      }
    }
  }

  return threatCount;
}

double HardAI::assessRisk(const Move& move,
                          const BattleState& battleState) const {
  double riskScore = 0.0;

  // Accuracy risk
  if (move.accuracy < 100) {
    double missChance = (100 - move.accuracy) / 100.0;
    riskScore -= missChance * 15.0;  // Penalty for miss chance

    // But bonus if it's high power and could KO
    if (move.power >= 100 &&
        estimateDamage(*battleState.aiPokemon, *battleState.opponentPokemon,
                       move, battleState.currentWeather) >=
            battleState.opponentPokemon->current_hp) {
      riskScore += 20.0;  // High risk, high reward
    }
  }

  // Risk of retaliation
  double predictedCounterDamage = predictOpponentDamage(battleState);
  double ourHealthRatio = calculateHealthRatio(*battleState.aiPokemon);

  if (predictedCounterDamage >= battleState.aiPokemon->current_hp &&
      ourHealthRatio < 0.4) {
    riskScore -= 25.0;  // Risky if we might get KO'd back
  }

  return riskScore;
}

bool HardAI::isLowRiskHighReward(const Move& move,
                                 const BattleState& battleState) const {
  // High reward: can KO or deal massive damage
  double damage =
      estimateDamage(*battleState.aiPokemon, *battleState.opponentPokemon, move,
                     battleState.currentWeather);
  bool highReward = (damage >= battleState.opponentPokemon->current_hp) ||
                    (damage >= battleState.opponentPokemon->hp * 0.6);

  // Low risk: high accuracy and we won't get KO'd back
  bool lowRisk =
      (move.accuracy >= 90) && (predictOpponentDamage(battleState) <
                                battleState.aiPokemon->current_hp * 0.7);

  return highReward && lowRisk;
}