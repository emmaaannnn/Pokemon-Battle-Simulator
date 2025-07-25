#include "easy_ai.h"

#include <algorithm>
#include <random>

MoveEvaluation EasyAI::chooseBestMove(const BattleState& battleState) {
  std::vector<Move*> usableMoves = getUsableMoves(*battleState.aiPokemon);

  if (usableMoves.empty()) {
    // No usable moves, return first move with very low score
    return {0, -100.0, "No PP remaining on any moves"};
  }

  MoveEvaluation bestMove{-1, -1000.0, ""};

  for (size_t i = 0; i < battleState.aiPokemon->moves.size(); ++i) {
    if (!battleState.aiPokemon->moves[i].canUse()) continue;

    const Move& move = battleState.aiPokemon->moves[i];
    double score = scoreMoveBasic(move, battleState);

    if (score > bestMove.score) {
      bestMove.moveIndex = static_cast<int>(i);
      bestMove.score = score;
      bestMove.reasoning = "Easy AI: Power=" + std::to_string(move.power) +
                           ", Type effectiveness considered";
    }
  }

  return bestMove;
}

SwitchEvaluation EasyAI::chooseBestSwitch(const BattleState& battleState) {
  // EasyAI only switches to first alive Pokemon (very basic)
  for (int i = 0; i < battleState.aiTeam->getSize(); ++i) {
    Pokemon* pokemon = battleState.aiTeam->getPokemon(i);
    if (pokemon && pokemon->isAlive() && pokemon != battleState.aiPokemon) {
      return {i, 50.0, "Easy AI: Switch to first available Pokemon"};
    }
  }

  return {-1, -100.0, "No Pokemon available to switch"};
}

bool EasyAI::shouldSwitch(const BattleState& battleState) {
  // EasyAI is very conservative, only switches when current Pokemon is at very
  // low health
  double healthRatio = calculateHealthRatio(*battleState.aiPokemon);
  return healthRatio < 0.15;  // Only switch when below 15% health
}

double EasyAI::scoreMoveBasic(const Move& move,
                              const BattleState& battleState) const {
  double score = 0.0;

  // Base power contributes to score
  score += move.power * 0.8;

  // Type effectiveness is major factor
  double typeEffectiveness =
      scoreTypeEffectiveness(move, *battleState.opponentPokemon);
  score += typeEffectiveness * 30.0;  // Weight type effectiveness heavily

  // Prefer moves with higher accuracy
  score += move.accuracy * 0.1;

  // Small bonus for status moves if opponent health is high
  if (move.power == 0 &&
      calculateHealthRatio(*battleState.opponentPokemon) > 0.7) {
    score += 15.0;
  }

  return score;
}

double EasyAI::scoreTypeEffectiveness(const Move& move,
                                      const Pokemon& defender) const {
  double effectiveness = calculateTypeEffectiveness(move.type, defender.types);

  if (effectiveness >= 2.0)
    return 100.0;  // Super effective
  else if (effectiveness == 0.0)
    return -50.0;  // No effect
  else if (effectiveness <= 0.5)
    return -25.0;  // Not very effective
  else
    return 0.0;  // Normal effectiveness
}