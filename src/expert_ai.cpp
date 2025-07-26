#include "expert_ai.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "weather.h"

struct PredictionResult {
  int mostLikelyMoveIndex;
  double confidence;
  std::string reasoning;
  std::vector<double> moveProbabilities;
};

MoveEvaluation ExpertAI::chooseBestMove(const BattleState& battleState) {
  updateOpponentModel(battleState);

  std::vector<Move*> usableMoves = getUsableMoves(*battleState.aiPokemon);
  if (usableMoves.empty()) {
    return {0, -100.0, "No PP remaining on any moves"};
  }

  // Generate multi-turn plans
  std::vector<TurnPlan> plans = generateTurnPlans(battleState, 2);

  MoveEvaluation bestMove{-1, -1000.0, ""};

  for (size_t i = 0; i < battleState.aiPokemon->moves.size(); ++i) {
    if (!battleState.aiPokemon->moves[i].canUse()) continue;

    const Move& move = battleState.aiPokemon->moves[i];
    double score = 0.0;

    // Base evaluation with all advanced factors
    score += calculateExpectedValue(move, battleState, 2);
    score += analyzeWinConditions(battleState);
    score += evaluateLongTermAdvantage(battleState);
    score += assessPositionalAdvantage(battleState);

    // Counter-strategy considerations
    if (detectSetupAttempt(battleState) && shouldDisrupt(battleState)) {
      if (move.ailment_name != "none" || move.power > 80) {
        score += 40.0;  // Bonus for disrupting setup
      }
    }

    // Risk modeling
    RiskAssessment risk = modelOutcome(move, battleState);
    score += risk.expectedUtility;

    // Endgame analysis
    if (isEndgameScenario(battleState)) {
      score += analyzeEndgamePosition(battleState);
    }

    // Resource management
    score += evaluateResourceManagement(battleState);

    if (score > bestMove.score) {
      bestMove.moveIndex = static_cast<int>(i);
      bestMove.score = score;
      bestMove.reasoning =
          "Expert AI: Multi-turn planning with predictive analysis";
    }
  }

  return bestMove;
}

SwitchEvaluation ExpertAI::chooseBestSwitch(const BattleState& battleState) {
  SwitchEvaluation bestSwitch{-1, -1000.0, ""};

  // Analyze team synergy and identify optimal switch
  double currentTeamSynergy = calculateTeamSynergy(*battleState.aiTeam);
  std::vector<int> keyThreats = identifyKeyThreats(battleState);

  for (int i = 0; i < static_cast<int>(battleState.aiTeam->size()); ++i) {
    Pokemon* pokemon = battleState.aiTeam->getPokemon(i);
    if (!pokemon || !pokemon->isAlive() || pokemon == battleState.aiPokemon) {
      continue;
    }

    double score = 0.0;

    // Role analysis
    TeamRole role = analyzePokemonRole(*pokemon);

    // Match role to current battle situation
    if (role.role == TeamRole::SETUP_SWEEPER &&
        detectSetupAttempt(battleState)) {
      score += 50.0;
    } else if (role.role == TeamRole::WALL &&
               predictOpponentAction(battleState).confidence > 0.7) {
      score += 35.0;
    } else if (role.role == TeamRole::REVENGE_KILLER) {
      double opponentHealthRatio =
          calculateHealthRatio(*battleState.opponentPokemon);
      if (opponentHealthRatio < 0.6) score += 45.0;
    }

    // Advanced matchup analysis
    score += role.effectiveness * 30.0;

    // Health and condition
    double healthScore = calculateHealthRatio(*pokemon) * 25.0;
    score += healthScore;

    // Key threat coverage
    int threatsHandled = 0;
    for (int threatIndex : keyThreats) {
      Pokemon* threat = battleState.opponentTeam->getPokemon(threatIndex);
      if (!threat) continue;

      for (const auto& move : pokemon->moves) {
        if (!move.canUse()) continue;
        double effectiveness =
            calculateTypeEffectiveness(move.type, threat->types);
        if (effectiveness >= 2.0) {
          threatsHandled++;
          break;
        }
      }
    }
    score += threatsHandled * 20.0;

    // Long-term positioning
    double longTermValue = evaluateLongTermAdvantage(battleState);
    score += longTermValue * 0.3;

    if (score > bestSwitch.score) {
      bestSwitch.pokemonIndex = i;
      bestSwitch.score = score;
      bestSwitch.reasoning =
          "Expert AI: Strategic positioning with team role analysis";
    }
  }

  return bestSwitch;
}

bool ExpertAI::shouldSwitch(const BattleState& battleState) {
  // Multi-factor switching decision

  // Immediate danger assessment
  PredictionResult prediction = predictOpponentAction(battleState);
  if (prediction.confidence > 0.8) {
    // We're confident about opponent's next move
    double predictedDamage = 0.0;
    if (prediction.mostLikelyMoveIndex >= 0) {
      const Move& predictedMove =
          battleState.opponentPokemon->moves[prediction.mostLikelyMoveIndex];
      predictedDamage =
          estimateDamage(*battleState.opponentPokemon, *battleState.aiPokemon,
                         predictedMove, battleState.currentWeather);
    }

    if (predictedDamage >= battleState.aiPokemon->current_hp * 0.85) {
      SwitchEvaluation bestAlternative = chooseBestSwitch(battleState);
      if (bestAlternative.score > 40.0) return true;
    }
  }

  // Strategic positioning
  double positionalAdvantage = assessPositionalAdvantage(battleState);
  if (positionalAdvantage < -30.0) {
    // Poor position, consider switching
    return true;
  }

  // Win condition analysis
  double winConditionScore = analyzeWinConditions(battleState);
  if (winConditionScore < -40.0) {
    // Current Pokemon doesn't contribute to win conditions
    SwitchEvaluation alternative = chooseBestSwitch(battleState);
    if (alternative.score > winConditionScore + 25.0) return true;
  }

  // Counter-play opportunity
  double counterValue = evaluateCounterPlay(battleState);
  if (counterValue > 50.0) return true;

  // Team synergy optimization
  double currentSynergy = calculateTeamSynergy(*battleState.aiTeam);
  // Would need to simulate team synergy after switch - simplified for now

  return false;
}

PredictionResult ExpertAI::predictOpponentAction(const BattleState& battleState,
                                                 int turnsAhead) const {
  PredictionResult result;
  result.mostLikelyMoveIndex = -1;
  result.confidence = 0.0;
  result.reasoning = "Expert AI opponent prediction";
  result.moveProbabilities.resize(battleState.opponentPokemon->moves.size(),
                                  0.0);

  // Analyze opponent's likely moves based on battle state
  double totalScore = 0.0;
  std::vector<double> moveScores(battleState.opponentPokemon->moves.size(),
                                 0.0);

  for (size_t i = 0; i < battleState.opponentPokemon->moves.size(); ++i) {
    const Move& move = battleState.opponentPokemon->moves[i];
    if (!move.canUse()) continue;

    double score = 0.0;

    // Damage moves likely if they can KO
    if (move.power > 0) {
      double damage =
          estimateDamage(*battleState.opponentPokemon, *battleState.aiPokemon,
                         move, battleState.currentWeather);

      if (damage >= battleState.aiPokemon->current_hp) {
        score += 100.0;  // Very likely to go for KO
      } else {
        score += damage * 0.5;  // Scale with damage potential
      }

      // Type effectiveness consideration
      double effectiveness =
          calculateTypeEffectiveness(move.type, battleState.aiPokemon->types);
      score += effectiveness * 25.0;
    }

    // Status moves more likely early game or against healthy targets
    if (move.power == 0 &&
        battleState.aiPokemon->status == StatusCondition::NONE) {
      double ourHealthRatio = calculateHealthRatio(*battleState.aiPokemon);
      score += ourHealthRatio * 30.0;
    }

    // Use opponent model if available
    auto it = opponentModel.moveProbabilities.find(static_cast<int>(i));
    if (it != opponentModel.moveProbabilities.end()) {
      score += it->second * 20.0;  // Historical preference
    }

    moveScores[i] = std::max(0.0, score);
    totalScore += moveScores[i];
  }

  // Convert to probabilities
  if (totalScore > 0) {
    double maxScore = 0.0;
    int bestIndex = -1;

    for (size_t i = 0; i < moveScores.size(); ++i) {
      result.moveProbabilities[i] = moveScores[i] / totalScore;

      if (moveScores[i] > maxScore) {
        maxScore = moveScores[i];
        bestIndex = static_cast<int>(i);
      }
    }

    result.mostLikelyMoveIndex = bestIndex;
    result.confidence =
        maxScore / totalScore;  // How confident we are in top choice
  }

  return result;
}

std::vector<ExpertAI::TurnPlan> ExpertAI::generateTurnPlans(
    const BattleState& battleState, int depth) const {
  std::vector<TurnPlan> plans;

  if (depth <= 0) return plans;

  // Generate immediate move options
  for (size_t i = 0; i < battleState.aiPokemon->moves.size(); ++i) {
    if (!battleState.aiPokemon->moves[i].canUse()) continue;

    TurnPlan plan;
    plan.moveIndex = static_cast<int>(i);
    plan.switchIndex = -1;

    // Evaluate this plan
    const Move& move = battleState.aiPokemon->moves[i];
    plan.expectedValue = calculateExpectedValue(move, battleState, depth);

    if (move.power > 0) {
      plan.strategy = "Aggressive damage dealing";
    } else {
      plan.strategy = "Status/setup play";
    }

    plans.push_back(plan);
  }

  // Generate switch options
  for (int i = 0; i < static_cast<int>(battleState.aiTeam->size()); ++i) {
    Pokemon* pokemon = battleState.aiTeam->getPokemon(i);
    if (!pokemon || !pokemon->isAlive() || pokemon == battleState.aiPokemon) {
      continue;
    }

    TurnPlan plan;
    plan.moveIndex = -1;
    plan.switchIndex = i;
    plan.strategy = "Strategic positioning";

    // Simplified switch evaluation
    TeamRole role = analyzePokemonRole(*pokemon);
    plan.expectedValue =
        role.effectiveness * 20.0 + calculateHealthRatio(*pokemon) * 15.0;

    plans.push_back(plan);
  }

  // Sort plans by expected value
  std::sort(plans.begin(), plans.end(),
            [](const TurnPlan& a, const TurnPlan& b) {
              return a.expectedValue > b.expectedValue;
            });

  return plans;
}

double ExpertAI::calculateExpectedValue(const Move& move,
                                        const BattleState& battleState,
                                        int turnsAhead) const {
  double expectedValue = 0.0;

  if (move.power > 0) {
    // Damage move evaluation
    double baseDamage =
        estimateDamage(*battleState.aiPokemon, *battleState.opponentPokemon,
                       move, battleState.currentWeather);

    // Account for accuracy
    double hitChance = move.accuracy / 100.0;
    double expectedDamage = baseDamage * hitChance;

    expectedValue += expectedDamage * 1.5;

    // KO value
    if (expectedDamage >= battleState.opponentPokemon->current_hp) {
      expectedValue += 80.0 * hitChance;
    }

    // Long-term damage value (less immediate pressure on us)
    if (turnsAhead > 1) {
      double healthReduction = expectedDamage / battleState.opponentPokemon->hp;
      expectedValue += healthReduction * 30.0;
    }

  } else {
    // Status move evaluation
    if (battleState.opponentPokemon->status == StatusCondition::NONE) {
      double statusValue = 25.0;

      // Status moves more valuable early and against healthy opponents
      double opponentHealthRatio =
          calculateHealthRatio(*battleState.opponentPokemon);
      statusValue += opponentHealthRatio * 35.0;

      // Factor in turns ahead - status effects compound
      statusValue += turnsAhead * 15.0;

      expectedValue += statusValue;
    }
  }

  return expectedValue;
}

double ExpertAI::analyzeWinConditions(const BattleState& battleState) const {
  double winScore = 0.0;

  // Count alive Pokemon advantage
  int ourAlive = battleState.aiTeam->getAlivePokemon().size();
  int oppAlive = battleState.opponentTeam->getAlivePokemon().size();
  winScore += (ourAlive - oppAlive) * 25.0;

  // Health advantage
  double ourTotalHealth = 0.0, oppTotalHealth = 0.0;

  for (int i = 0; i < static_cast<int>(battleState.aiTeam->size()); ++i) {
    Pokemon* pokemon = battleState.aiTeam->getPokemon(i);
    if (pokemon && pokemon->isAlive()) {
      ourTotalHealth += calculateHealthRatio(*pokemon);
    }
  }

  for (int i = 0; i < static_cast<int>(battleState.opponentTeam->size()); ++i) {
    Pokemon* pokemon = battleState.opponentTeam->getPokemon(i);
    if (pokemon && pokemon->isAlive()) {
      oppTotalHealth += calculateHealthRatio(*pokemon);
    }
  }

  winScore += (ourTotalHealth - oppTotalHealth) * 20.0;

  // Type advantage across teams
  int typeAdvantages = 0;
  for (int i = 0; i < static_cast<int>(battleState.aiTeam->size()); ++i) {
    Pokemon* ourPokemon = battleState.aiTeam->getPokemon(i);
    if (!ourPokemon || !ourPokemon->isAlive()) continue;

    for (int j = 0; j < static_cast<int>(battleState.opponentTeam->size()); ++j) {
      Pokemon* oppPokemon = battleState.opponentTeam->getPokemon(j);
      if (!oppPokemon || !oppPokemon->isAlive()) continue;

      for (const auto& move : ourPokemon->moves) {
        if (move.power > 0) {
          double effectiveness =
              calculateTypeEffectiveness(move.type, oppPokemon->types);
          if (effectiveness >= 2.0) typeAdvantages++;
        }
      }
    }
  }

  winScore += typeAdvantages * 3.0;

  return winScore;
}

ExpertAI::TeamRole ExpertAI::analyzePokemonRole(const Pokemon& pokemon) const {
  TeamRole role;
  role.effectiveness = 50.0;  // Base effectiveness

  // Analyze stats to determine role
  double attackPower = std::max(pokemon.attack, pokemon.special_attack);
  double defensePower = std::max(pokemon.defense, pokemon.special_defense);
  double speedTier = pokemon.speed;

  // Count move types
  int damageMoves = 0, statusMoves = 0, setupMoves = 0;
  double avgPower = 0.0;

  for (const auto& move : pokemon.moves) {
    if (move.power > 0) {
      damageMoves++;
      avgPower += move.power;
    } else {
      statusMoves++;
      // Simple setup move detection (would be more sophisticated in practice)
      if (move.name.find("dance") != std::string::npos ||
          move.name.find("growth") != std::string::npos) {
        setupMoves++;
      }
    }
  }

  if (damageMoves > 0) avgPower /= damageMoves;

  // Role classification
  if (setupMoves > 0 && attackPower >= 90) {
    role.role = TeamRole::SETUP_SWEEPER;
    role.effectiveness += 15.0;
  } else if (attackPower >= 100 && speedTier >= 90) {
    role.role = TeamRole::SWEEPER;
    role.effectiveness += 12.0;
  } else if (defensePower >= 90 && pokemon.hp >= 90) {
    role.role = TeamRole::WALL;
    role.effectiveness += 10.0;
  } else if (speedTier >= 100 && avgPower >= 70) {
    role.role = TeamRole::REVENGE_KILLER;
    role.effectiveness += 8.0;
  } else if (statusMoves >= 2) {
    role.role = TeamRole::SUPPORT;
    role.effectiveness += 5.0;
  } else {
    role.role = TeamRole::PIVOT;
  }

  return role;
}

void ExpertAI::updateOpponentModel(const BattleState& battleState) const {
  // Simple learning - track which moves opponent uses
  // In a real implementation, this would be more sophisticated

  // For now, just initialize some basic tendencies
  if (opponentModel.moveProbabilities.empty()) {
    for (size_t i = 0; i < battleState.opponentPokemon->moves.size(); ++i) {
      opponentModel.moveProbabilities[static_cast<int>(i)] =
          0.25;  // Equal probability initially
    }
    opponentModel.playStyle = "balanced";
    opponentModel.riskTolerance = 0.5;
  }
}

bool ExpertAI::isEndgameScenario(const BattleState& battleState) const {
  int ourAlive = battleState.aiTeam->getAlivePokemon().size();
  int oppAlive = battleState.opponentTeam->getAlivePokemon().size();

  return (ourAlive <= 2 && oppAlive <= 2) || (ourAlive + oppAlive <= 3);
}

// Simplified implementations for remaining methods
double ExpertAI::evaluateLongTermAdvantage(
    const BattleState& battleState) const {
  return 0.0;
}
double ExpertAI::detectSetupAttempt(const BattleState& battleState) const {
  return 0.0;
}
double ExpertAI::evaluateCounterPlay(const BattleState& battleState) const {
  return 0.0;
}
bool ExpertAI::shouldDisrupt(const BattleState& battleState) const {
  return false;
}
double ExpertAI::assessPositionalAdvantage(
    const BattleState& battleState) const {
  return 0.0;
}
double ExpertAI::evaluateResourceManagement(
    const BattleState& battleState) const {
  return 0.0;
}
double ExpertAI::calculateTeamSynergy(const Team& team) const { return 50.0; }
std::vector<int> ExpertAI::identifyKeyThreats(
    const BattleState& battleState) const {
  return {};
}
double ExpertAI::analyzeEndgamePosition(const BattleState& battleState) const {
  return 0.0;
}

ExpertAI::RiskAssessment ExpertAI::modelOutcome(
    const Move& move, const BattleState& battleState) const {
  RiskAssessment assessment;

  double hitChance = move.accuracy / 100.0;
  assessment.probability = hitChance;

  if (move.power > 0) {
    double damage =
        estimateDamage(*battleState.aiPokemon, *battleState.opponentPokemon,
                       move, battleState.currentWeather);
    assessment.impact = damage;
    assessment.expectedUtility =
        damage * hitChance - (1.0 - hitChance) * 10.0;  // Miss penalty
  } else {
    assessment.impact = 30.0;  // Status effect value
    assessment.expectedUtility = assessment.impact * hitChance;
  }

  return assessment;
}