#pragma once

#include "ai_strategy.h"

class MediumAI : public AIStrategy {
 public:
  MediumAI() : AIStrategy(AIDifficulty::MEDIUM) {}

  MoveEvaluation chooseBestMove(const BattleState& battleState) override;
  SwitchEvaluation chooseBestSwitch(const BattleState& battleState) override;
  bool shouldSwitch(const BattleState& battleState) override;

 private:
  // Enhanced scoring methods for MediumAI
  double scoreMoveAdvanced(const Move& move,
                           const BattleState& battleState) const;
  double scoreStatusMove(const Move& move,
                         const BattleState& battleState) const;
  double scoreDamageMove(const Move& move,
                         const BattleState& battleState) const;
  double scoreWeatherAdvantage(const Move& move,
                               WeatherCondition weather) const;
  double evaluatePokemonMatchup(const Pokemon& attacker,
                                const Pokemon& defender) const;
};