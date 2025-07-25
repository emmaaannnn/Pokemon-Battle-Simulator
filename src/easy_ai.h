#pragma once

#include "ai_strategy.h"

class EasyAI : public AIStrategy {
 public:
  EasyAI() : AIStrategy(AIDifficulty::EASY) {}

  MoveEvaluation chooseBestMove(const BattleState& battleState) override;
  SwitchEvaluation chooseBestSwitch(const BattleState& battleState) override;
  bool shouldSwitch(const BattleState& battleState) override;

 private:
  // Simple scoring methods for EasyAI
  double scoreMoveBasic(const Move& move, const BattleState& battleState) const;
  double scoreTypeEffectiveness(const Move& move,
                                const Pokemon& defender) const;
};