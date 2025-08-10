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
  // Note: currentTeamSynergy will be used in Phase 2 for synergy optimization
  // double currentTeamSynergy = calculateTeamSynergy(*battleState.aiTeam);
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
  // Note: currentSynergy analysis will be expanded in Phase 2
  // double currentSynergy = calculateTeamSynergy(*battleState.aiTeam);
  // Would need to simulate team synergy after switch - simplified for now

  return false;
}

PredictionResult ExpertAI::predictOpponentAction(const BattleState& battleState,
                                                 int turnsAhead) const {
  (void)turnsAhead;  // Will be used in Phase 2 for multi-turn prediction
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
  (void)battleState;  // Suppress unused parameter warning
  return 0.0;
}
double ExpertAI::detectSetupAttempt(const BattleState& battleState) const {
  (void)battleState;  // Suppress unused parameter warning
  return 0.0;
}
double ExpertAI::evaluateCounterPlay(const BattleState& battleState) const {
  (void)battleState;  // Suppress unused parameter warning
  return 0.0;
}
bool ExpertAI::shouldDisrupt(const BattleState& battleState) const {
  (void)battleState;  // Suppress unused parameter warning
  return false;
}
double ExpertAI::assessPositionalAdvantage(
    const BattleState& battleState) const {
  (void)battleState;  // Suppress unused parameter warning
  return 0.0;
}
double ExpertAI::evaluateResourceManagement(
    const BattleState& battleState) const {
  (void)battleState;  // Suppress unused parameter warning
  return 0.0;
}
double ExpertAI::calculateTeamSynergy(const Team& team) const { 
  (void)team;  // Suppress unused parameter warning
  return 50.0; 
}
std::vector<int> ExpertAI::identifyKeyThreats(
    const BattleState& battleState) const {
  (void)battleState;  // Suppress unused parameter warning
  return {};
}
double ExpertAI::analyzeEndgamePosition(const BattleState& battleState) const {
  (void)battleState;  // Suppress unused parameter warning
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

// ──────────────────────────────────────────────────────────────────
// Phase 1: Advanced Analysis & Prediction System Implementations
// ──────────────────────────────────────────────────────────────────

// Bayesian Opponent Modeling Implementation
void ExpertAI::updateBayesianModel(const BattleState& battle_state, int opponent_move) const {
  // Initialize model if first time
  if (bayesian_model_.total_observations_ == 0) {
    bayesian_model_.learning_rate_ = 0.1;
    bayesian_model_.exploration_bonus_ = 2.0;
    bayesian_model_.total_observations_ = 0;
    
    // Initialize priors for all moves (Beta distribution parameters)
    for (size_t i = 0; i < battle_state.opponentPokemon->moves.size(); ++i) {
      bayesian_model_.move_priors_[i] = {1.0, 1.0};  // Uniform prior
      bayesian_model_.move_usage_count_[i] = 0;
    }
    
    // Initialize play style probabilities
    bayesian_model_.playstyle_probabilities_["hyperoffensive"] = 0.2;
    bayesian_model_.playstyle_probabilities_["stall"] = 0.2;
    bayesian_model_.playstyle_probabilities_["balanced"] = 0.4;
    bayesian_model_.playstyle_probabilities_["setupsweep"] = 0.2;
  }
  
  // Update move frequency with Bayesian learning
  if (opponent_move >= 0 && opponent_move < static_cast<int>(battle_state.opponentPokemon->moves.size())) {
    auto& prior = bayesian_model_.move_priors_[opponent_move];
    prior.first += 1.0;  // Increment alpha (successes)
    bayesian_model_.move_usage_count_[opponent_move]++;
    bayesian_model_.total_observations_++;
    
    // Update all other moves' beta parameters (failures)
    for (auto& pair : bayesian_model_.move_priors_) {
      if (pair.first != opponent_move) {
        pair.second.second += bayesian_model_.learning_rate_;
      }
    }
  }
  
  // Update situational patterns
  std::string current_situation = "neutral";
  double ai_health_ratio = calculateHealthRatio(*battle_state.aiPokemon);
  double opp_health_ratio = calculateHealthRatio(*battle_state.opponentPokemon);
  
  if (ai_health_ratio < 0.3) current_situation = "ai_low_hp";
  else if (opp_health_ratio < 0.3) current_situation = "opp_low_hp";
  else if (battle_state.opponentPokemon->status != StatusCondition::NONE) current_situation = "opp_status";
  else if (battle_state.aiPokemon->status != StatusCondition::NONE) current_situation = "ai_status";
  
  // Find or create situational pattern
  bool found = false;
  for (auto& pattern : bayesian_model_.situational_patterns_) {
    if (pattern.situation == current_situation) {
      pattern.move_preferences[opponent_move] += bayesian_model_.learning_rate_;
      pattern.confidence = std::min(0.95, pattern.confidence + 0.05);
      found = true;
      break;
    }
  }
  
  if (!found) {
    BayesianOpponentModel::SituationalPattern new_pattern;
    new_pattern.situation = current_situation;
    new_pattern.move_preferences[opponent_move] = 1.0;
    new_pattern.confidence = 0.1;
    bayesian_model_.situational_patterns_.push_back(new_pattern);
  }
}

double ExpertAI::predictOpponentMoveProbability(const BattleState& battle_state, int move_index) const {
  if (bayesian_model_.total_observations_ == 0) {
    return 1.0 / battle_state.opponentPokemon->moves.size();  // Uniform prior
  }
  
  auto it = bayesian_model_.move_priors_.find(move_index);
  if (it == bayesian_model_.move_priors_.end()) {
    return 0.1;  // Low probability for unobserved moves
  }
  
  // Beta distribution mean: alpha / (alpha + beta)
  double alpha = it->second.first;
  double beta = it->second.second;
  double base_probability = alpha / (alpha + beta);
  
  // Apply situational adjustment
  std::string current_situation = "neutral";
  double ai_health_ratio = calculateHealthRatio(*battle_state.aiPokemon);
  double opp_health_ratio = calculateHealthRatio(*battle_state.opponentPokemon);
  
  if (ai_health_ratio < 0.3) current_situation = "ai_low_hp";
  else if (opp_health_ratio < 0.3) current_situation = "opp_low_hp";
  else if (battle_state.opponentPokemon->status != StatusCondition::NONE) current_situation = "opp_status";
  
  double situational_modifier = 1.0;
  for (const auto& pattern : bayesian_model_.situational_patterns_) {
    if (pattern.situation == current_situation) {
      auto move_it = pattern.move_preferences.find(move_index);
      if (move_it != pattern.move_preferences.end()) {
        situational_modifier = 1.0 + (move_it->second * pattern.confidence);
        break;
      }
    }
  }
  
  return base_probability * situational_modifier;
}

std::string ExpertAI::classifyOpponentPlayStyle(const BattleState& battle_state) const {
  if (bayesian_model_.total_observations_ < 5) {
    return "unknown";  // Not enough data for classification
  }
  
  // Analyze move usage patterns
  int damage_moves_used = 0;
  int status_moves_used = 0;
  int high_power_moves_used = 0;
  
  for (const auto& pair : bayesian_model_.move_usage_count_) {
    if (pair.first >= 0 && pair.first < static_cast<int>(battle_state.opponentPokemon->moves.size())) {
      const Move& move = battle_state.opponentPokemon->moves[pair.first];
      if (move.power > 0) {
        damage_moves_used += pair.second;
        if (move.power >= 100) {
          high_power_moves_used += pair.second;
        }
      } else {
        status_moves_used += pair.second;
      }
    }
  }
  
  double damage_ratio = static_cast<double>(damage_moves_used) / bayesian_model_.total_observations_;
  double status_ratio = static_cast<double>(status_moves_used) / bayesian_model_.total_observations_;
  double high_power_ratio = static_cast<double>(high_power_moves_used) / bayesian_model_.total_observations_;
  
  // Classification logic
  if (damage_ratio > 0.8 && high_power_ratio > 0.4) {
    return "hyperoffensive";
  } else if (status_ratio > 0.4) {
    return "stall";
  } else if (high_power_ratio > 0.2 && status_ratio > 0.2) {
    return "setupsweep";
  } else {
    return "balanced";
  }
}

// MiniMax Search Engine Implementation
double ExpertAI::miniMaxSearch(const BattleState& root_state, int depth, double alpha, double beta, 
                              bool maximizing_player, std::vector<int>& best_line) const {
  auto start_time = std::chrono::high_resolution_clock::now();
  search_engine_.nodes_evaluated_ = 0;
  search_engine_.alpha_beta_cutoffs_ = 0;
  
  if (depth <= 0 || isEndgamePosition(root_state)) {
    search_engine_.nodes_evaluated_++;
    return evaluatePosition(root_state);
  }
  
  std::vector<BattleState> legal_moves = generateLegalMoves(root_state, maximizing_player);
  if (legal_moves.empty()) {
    return evaluatePosition(root_state);
  }
  
  orderMoves(legal_moves, maximizing_player);
  
  double best_value = maximizing_player ? -1000.0 : 1000.0;
  std::vector<int> current_best_line;
  
  for (size_t i = 0; i < legal_moves.size() && i < static_cast<size_t>(MiniMaxSearchEngine::kMaxBranchingFactor); ++i) {
    std::vector<int> child_line;
    double value = miniMaxSearch(legal_moves[i], depth - 1, alpha, beta, !maximizing_player, child_line);
    
    if (maximizing_player) {
      if (value > best_value) {
        best_value = value;
        current_best_line = child_line;
      }
      alpha = std::max(alpha, value);
      if (beta <= alpha) {
        search_engine_.alpha_beta_cutoffs_++;
        break;  // Alpha-beta pruning
      }
    } else {
      if (value < best_value) {
        best_value = value;
        current_best_line = child_line;
      }
      beta = std::min(beta, value);
      if (beta <= alpha) {
        search_engine_.alpha_beta_cutoffs_++;
        break;  // Alpha-beta pruning
      }
    }
  }
  
  if (depth == MiniMaxSearchEngine::kMaxSearchDepth) {
    search_engine_.principal_variation_ = current_best_line;
    search_engine_.principal_variation_score_ = best_value;
    
    auto end_time = std::chrono::high_resolution_clock::now();
    search_engine_.search_time_ = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  }
  
  best_line = current_best_line;
  return best_value;
}

double ExpertAI::evaluatePosition(const BattleState& battle_state) const {
  double score = 0.0;
  
  // Material advantage (Pokemon count and health)
  int ai_alive = battle_state.aiTeam->getAlivePokemon().size();
  int opp_alive = battle_state.opponentTeam->getAlivePokemon().size();
  score += (ai_alive - opp_alive) * 30.0;
  
  // Health advantage
  double ai_health_total = 0.0, opp_health_total = 0.0;
  for (int i = 0; i < static_cast<int>(battle_state.aiTeam->size()); ++i) {
    Pokemon* pokemon = battle_state.aiTeam->getPokemon(i);
    if (pokemon && pokemon->isAlive()) {
      ai_health_total += calculateHealthRatio(*pokemon);
    }
  }
  for (int i = 0; i < static_cast<int>(battle_state.opponentTeam->size()); ++i) {
    Pokemon* pokemon = battle_state.opponentTeam->getPokemon(i);
    if (pokemon && pokemon->isAlive()) {
      opp_health_total += calculateHealthRatio(*pokemon);
    }
  }
  score += (ai_health_total - opp_health_total) * 20.0;
  
  // Positional factors
  if (battle_state.aiPokemon && battle_state.opponentPokemon) {
    // Type matchup advantage
    for (const auto& move : battle_state.aiPokemon->moves) {
      if (move.power > 0 && move.canUse()) {
        double effectiveness = calculateTypeEffectiveness(move.type, battle_state.opponentPokemon->types);
        if (effectiveness >= 2.0) score += 15.0;
        else if (effectiveness >= 1.5) score += 8.0;
        else if (effectiveness <= 0.5) score -= 10.0;
      }
    }
    
    // Speed advantage
    if (battle_state.aiPokemon->speed > battle_state.opponentPokemon->speed) {
      score += 10.0;
    } else if (battle_state.aiPokemon->speed < battle_state.opponentPokemon->speed) {
      score -= 5.0;
    }
    
    // Status condition factors
    if (battle_state.opponentPokemon->status != StatusCondition::NONE) score += 25.0;
    if (battle_state.aiPokemon->status != StatusCondition::NONE) score -= 20.0;
  }
  
  total_positions_analyzed_++;
  return score;
}

std::vector<BattleState> ExpertAI::generateLegalMoves(const BattleState& current_state, bool for_ai) const {
  std::vector<BattleState> legal_moves;
  
  Pokemon* active_pokemon = for_ai ? current_state.aiPokemon : current_state.opponentPokemon;
  Team* team = for_ai ? current_state.aiTeam : current_state.opponentTeam;
  
  if (!active_pokemon || !team) return legal_moves;
  
  // Generate move options
  for (size_t i = 0; i < active_pokemon->moves.size(); ++i) {
    if (active_pokemon->moves[i].canUse()) {
      BattleState new_state = current_state;
      // Simplified state transition - in practice this would simulate the move
      legal_moves.push_back(new_state);
      if (legal_moves.size() >= static_cast<size_t>(MiniMaxSearchEngine::kMaxBranchingFactor)) break;
    }
  }
  
  return legal_moves;
}

void ExpertAI::orderMoves(std::vector<BattleState>& states, bool maximizing_player) const {
  // Simple move ordering - prioritize high-damage moves for better alpha-beta pruning
  // In practice, this would use more sophisticated heuristics
  std::sort(states.begin(), states.end(), [this, maximizing_player](const BattleState& a, const BattleState& b) {
    double score_a = evaluatePosition(a);
    double score_b = evaluatePosition(b);
    return maximizing_player ? (score_a > score_b) : (score_a < score_b);
  });
}

// Meta-Game Analysis Implementation
ExpertAI::MetaGameAnalyzer::TeamArchetype ExpertAI::analyzeTeamArchetype(const Team& team) const {
  if (team.size() == 0) return MetaGameAnalyzer::TeamArchetype::UNKNOWN;
  
  int fast_attackers = 0;
  int defensive_walls = 0;
  int setup_sweepers = 0;
  int support_pokemon = 0;
  double avg_attack = 0.0;
  double avg_defense = 0.0;
  double avg_speed = 0.0;
  
  for (int i = 0; i < static_cast<int>(team.size()); ++i) {
    const Pokemon* pokemon = team.getPokemon(i);
    if (!pokemon) continue;
    
    double max_attack = std::max(pokemon->attack, pokemon->special_attack);
    double max_defense = std::max(pokemon->defense, pokemon->special_defense);
    
    avg_attack += max_attack;
    avg_defense += max_defense;
    avg_speed += pokemon->speed;
    
    // Classify individual Pokemon roles
    if (pokemon->speed >= 100 && max_attack >= 90) fast_attackers++;
    if (max_defense >= 100 && pokemon->hp >= 90) defensive_walls++;
    
    // Check for setup moves
    for (const auto& move : pokemon->moves) {
      if (move.power == 0 && (move.name.find("dance") != std::string::npos ||
                              move.name.find("growth") != std::string::npos ||
                              move.name.find("calm-mind") != std::string::npos)) {
        setup_sweepers++;
        break;
      }
    }
    
    // Count status/support moves
    int status_moves = 0;
    for (const auto& move : pokemon->moves) {
      if (move.power == 0) status_moves++;
    }
    if (status_moves >= 2) support_pokemon++;
  }
  
  avg_attack /= team.size();
  avg_defense /= team.size();
  avg_speed /= team.size();
  
  // Archetype classification
  if (fast_attackers >= static_cast<int>(team.size() * 0.7) && avg_speed >= 95) {
    return MetaGameAnalyzer::TeamArchetype::HYPER_OFFENSE;
  } else if (defensive_walls >= static_cast<int>(team.size() * 0.5) && support_pokemon >= 2) {
    return MetaGameAnalyzer::TeamArchetype::STALL;
  } else if (setup_sweepers >= 2 && fast_attackers >= 2) {
    return MetaGameAnalyzer::TeamArchetype::SETUP_SWEEP;
  } else if (avg_attack >= 85 && avg_defense >= 85) {
    return MetaGameAnalyzer::TeamArchetype::BULKY_OFFENSE;
  } else if (avg_attack >= 90 && avg_speed >= 80) {
    return MetaGameAnalyzer::TeamArchetype::BALANCED_OFFENSE;
  } else {
    return MetaGameAnalyzer::TeamArchetype::BALANCED;
  }
}

std::vector<ExpertAI::MetaGameAnalyzer::WinCondition> ExpertAI::identifyWinConditions(
    const BattleState& battle_state) const {
  std::vector<MetaGameAnalyzer::WinCondition> win_conditions;
  
  MetaGameAnalyzer::TeamArchetype our_archetype = analyzeTeamArchetype(*battle_state.aiTeam);
  // Note: opponent archetype analysis will be used in Phase 2 for counter-strategy selection
  // MetaGameAnalyzer::TeamArchetype opp_archetype = analyzeTeamArchetype(*battle_state.opponentTeam);
  
  // Primary win condition based on our team archetype
  MetaGameAnalyzer::WinCondition primary;
  switch (our_archetype) {
    case MetaGameAnalyzer::TeamArchetype::HYPER_OFFENSE:
      primary.name = "offensive_pressure";
      primary.probability_of_success = 0.75;
      primary.required_conditions = {"maintain_momentum", "avoid_walls"};
      primary.countering_strategies = {"defensive_pivoting", "priority_moves"};
      primary.priority = 1;
      break;
      
    case MetaGameAnalyzer::TeamArchetype::SETUP_SWEEP:
      primary.name = "setup_sweep";
      primary.probability_of_success = 0.8;
      primary.required_conditions = {"find_setup_opportunity", "avoid_priority"};
      primary.countering_strategies = {"status_moves", "phazing", "critical_hits"};
      primary.priority = 1;
      break;
      
    case MetaGameAnalyzer::TeamArchetype::STALL:
      primary.name = "wall_stall";
      primary.probability_of_success = 0.65;
      primary.required_conditions = {"inflict_status", "control_tempo"};
      primary.countering_strategies = {"wall_breakers", "setup_sweepers"};
      primary.priority = 1;
      break;
      
    default:
      primary.name = "balanced_play";
      primary.probability_of_success = 0.5;
      primary.required_conditions = {"maintain_type_advantage"};
      primary.countering_strategies = {"specialized_strategies"};
      primary.priority = 1;
  }
  
  win_conditions.push_back(primary);
  
  // Secondary win condition (endgame scenarios)
  MetaGameAnalyzer::WinCondition secondary;
  secondary.name = "endgame_advantage";
  secondary.probability_of_success = 0.4;
  secondary.required_conditions = {"preserve_key_pokemon", "maintain_health_advantage"};
  secondary.countering_strategies = {"sacrifice_plays", "momentum_shifts"};
  secondary.priority = 2;
  win_conditions.push_back(secondary);
  
  return win_conditions;
}

std::vector<std::string> ExpertAI::suggestCounterStrategies(
    const MetaGameAnalyzer::TeamArchetype& opponent_archetype) const {
  std::vector<std::string> strategies;
  
  switch (opponent_archetype) {
    case MetaGameAnalyzer::TeamArchetype::HYPER_OFFENSE:
      strategies = {"priority_moves", "defensive_walls", "speed_control", "residual_damage"};
      break;
      
    case MetaGameAnalyzer::TeamArchetype::STALL:
      strategies = {"wall_breakers", "setup_opportunities", "taunt", "offensive_pressure"};
      break;
      
    case MetaGameAnalyzer::TeamArchetype::SETUP_SWEEP:
      strategies = {"status_moves", "phazing", "priority_moves", "prevent_setup"};
      break;
      
    case MetaGameAnalyzer::TeamArchetype::BULKY_OFFENSE:
      strategies = {"type_advantage", "status_conditions", "setup_sweeping"};
      break;
      
    default:
      strategies = {"maintain_advantage", "exploit_weaknesses", "control_tempo"};
  }
  
  return strategies;
}

bool ExpertAI::isEndgamePosition(const BattleState& battle_state) const {
  int total_alive = battle_state.aiTeam->getAlivePokemon().size() + 
                    battle_state.opponentTeam->getAlivePokemon().size();
  return total_alive <= 4;
}

std::string ExpertAI::getEndgameEvaluation(const BattleState& battle_state) const {
  int ai_alive = battle_state.aiTeam->getAlivePokemon().size();
  int opp_alive = battle_state.opponentTeam->getAlivePokemon().size();
  
  if (ai_alive > opp_alive + 1) return "winning";
  else if (opp_alive > ai_alive + 1) return "losing";
  else if (ai_alive == 1 && opp_alive == 1) return "critical";
  else return "complex";
}