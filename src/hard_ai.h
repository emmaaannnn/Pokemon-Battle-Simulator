#pragma once

#include "ai_strategy.h"

class HardAI : public AIStrategy {
 public:
  HardAI() : AIStrategy(AIDifficulty::HARD) {}

  MoveEvaluation chooseBestMove(const BattleState& battleState) override;
  SwitchEvaluation chooseBestSwitch(const BattleState& battleState) override;
  bool shouldSwitch(const BattleState& battleState) override;

 private:
  // Strategic analysis methods
  double evaluateComplexMove(const Move& move,
                             const BattleState& battleState) const;
  double analyzeTeamThreat(const Pokemon& pokemon,
                           const Team& opponentTeam) const;
  double calculateStatModificationValue(const Move& move,
                                        const BattleState& battleState) const;
  double evaluateSetupOpportunity(const BattleState& battleState) const;

  // Advanced decision making
  bool canSweepTeam(const Pokemon& sweeper, const Team& opponentTeam) const;
  double predictOpponentDamage(const BattleState& battleState) const;
  int countTeamThreats(const Pokemon& pokemon, const Team& opponentTeam) const;

  // Risk assessment
  double assessRisk(const Move& move, const BattleState& battleState) const;
  bool isLowRiskHighReward(const Move& move,
                           const BattleState& battleState) const;
};