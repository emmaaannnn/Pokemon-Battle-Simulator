#pragma once

#include <map>
#include <memory>

#include "ai_strategy.h"

// Forward declarations for advanced AI components
struct GameState;
struct PredictionResult;

class ExpertAI : public AIStrategy {
 public:
  ExpertAI() : AIStrategy(AIDifficulty::EXPERT) {}

  MoveEvaluation chooseBestMove(const BattleState& battleState) override;
  SwitchEvaluation chooseBestSwitch(const BattleState& battleState) override;
  bool shouldSwitch(const BattleState& battleState) override;

 private:
  // Advanced AI components

  // Multi-turn planning
  struct TurnPlan {
    int moveIndex;
    int switchIndex;
    double expectedValue;
    std::string strategy;
  };

  // Opponent modeling
  struct OpponentModel {
    std::map<int, double> moveProbabilities;  // Probability of using each move
    std::string playStyle;  // "aggressive", "defensive", "setup", "balanced"
    std::vector<int> preferredMoves;  // Moves opponent uses most
    double riskTolerance;             // How likely to use risky moves
  };

  // Predictive analysis methods
  PredictionResult predictOpponentAction(const BattleState& battleState,
                                         int turnsAhead = 1) const;
  std::vector<TurnPlan> generateTurnPlans(const BattleState& battleState,
                                          int depth = 2) const;
  double evaluateGameState(const BattleState& battleState,
                           int turnsFromNow) const;

  // Advanced strategic analysis
  double analyzeWinConditions(const BattleState& battleState) const;
  double evaluateLongTermAdvantage(const BattleState& battleState) const;
  bool detectOpponentPattern(const BattleState& battleState) const;

  // Team composition analysis
  struct TeamRole {
    enum Type { SWEEPER, WALL, SUPPORT, PIVOT, REVENGE_KILLER, SETUP_SWEEPER };
    Type role;
    double effectiveness;
  };

  TeamRole analyzePokemonRole(const Pokemon& pokemon) const;
  double calculateTeamSynergy(const Team& team) const;
  std::vector<int> identifyKeyThreats(const BattleState& battleState) const;

  // Counter-strategy systems
  double detectSetupAttempt(const BattleState& battleState) const;
  double evaluateCounterPlay(const BattleState& battleState) const;
  bool shouldDisrupt(const BattleState& battleState) const;

  // Advanced decision making
  double calculateExpectedValue(const Move& move,
                                const BattleState& battleState,
                                int turnsAhead) const;
  double assessPositionalAdvantage(const BattleState& battleState) const;
  double evaluateResourceManagement(const BattleState& battleState) const;

  // Probability and risk modeling
  struct RiskAssessment {
    double probability;
    double impact;
    double expectedUtility;
  };

  RiskAssessment modelOutcome(const Move& move,
                              const BattleState& battleState) const;
  double calculateRegret(const Move& move,
                         const BattleState& battleState) const;

  // Memory and adaptation (simplified for now)
  mutable OpponentModel opponentModel;
  void updateOpponentModel(const BattleState& battleState) const;

  // Endgame analysis
  double analyzeEndgamePosition(const BattleState& battleState) const;
  bool isEndgameScenario(const BattleState& battleState) const;

  // Utility methods
  double simulateBattleOutcome(const BattleState& initialState,
                               const TurnPlan& plan) const;
  std::vector<BattleState> generatePossibleStates(
      const BattleState& current) const;
};