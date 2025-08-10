#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <vector>

#include "ai_strategy.h"

// Forward declarations for advanced AI components
struct GameState;
struct PredictionResult;

class ExpertAI : public AIStrategy {
 public:
  ExpertAI() : AIStrategy(AIDifficulty::EXPERT) {}

  // Friend declaration for testing
  friend class ExpertAITest;

  MoveEvaluation chooseBestMove(const BattleState& battleState) override;
  SwitchEvaluation chooseBestSwitch(const BattleState& battleState) override;
  bool shouldSwitch(const BattleState& battleState) override;

  // Meta-Game Analysis System (Public for testing access)
  struct MetaGameAnalyzer {
    // Team archetype classification
    enum class TeamArchetype {
      HYPER_OFFENSE,     // Fast, powerful attackers with minimal bulk
      BALANCED_OFFENSE,  // Mix of power and utility
      STALL,            // Defensive with status/residual damage
      BULKY_OFFENSE,    // Tanky Pokemon that can take hits while dealing damage
      SETUP_SWEEP,      // Focus on stat boosts then sweeping
      TRICK_ROOM,       // Speed control strategy
      WEATHER_TEAM,     // Built around weather conditions
      BALANCED,         // No clear specialization
      UNKNOWN           // Cannot classify with confidence
    };
    
    struct TeamAnalysis {
      TeamArchetype primary_archetype;
      TeamArchetype secondary_archetype;  // For hybrid teams
      double archetype_confidence;
      std::vector<std::string> key_strategies;  // "setup_sweeping", "wall_breaking", etc.
      std::vector<int> core_pokemon_indices;    // Most important team members
      double offensive_pressure;  // Team's ability to threaten opponents
      double defensive_stability; // Team's ability to take hits
    };
    
    // Win condition identification
    struct WinCondition {
      std::string name;  // "setup_sweep", "wall_stall", "offensive_pressure", etc.
      double probability_of_success;
      std::vector<std::string> required_conditions;  // Prerequisites for this win path
      std::vector<std::string> countering_strategies;  // How opponent might stop this
      int priority;  // 1 = primary, 2 = secondary, etc.
    };
    
    // Counter-strategy database
    std::map<TeamArchetype, std::vector<std::string>> counter_strategies_;
    std::map<std::string, std::vector<std::string>> move_counters_;  // Specific move -> counter moves
    std::map<std::string, double> strategy_effectiveness_;  // Historical success rates
    
    // Endgame tablebase (simplified)
    struct EndgamePosition {
      int ai_pokemon_remaining;
      int opponent_pokemon_remaining;
      std::string position_type;  // "winning", "losing", "drawn", "complex"
      std::vector<std::string> optimal_moves;
      double evaluation;
    };
    std::map<std::string, EndgamePosition> endgame_tablebase_;
  };

  // Phase 1 Advanced Analysis Methods (Public for testing and extensibility)
  void updateBayesianModel(const BattleState& battle_state, int opponent_move) const;
  double predictOpponentMoveProbability(const BattleState& battle_state, int move_index) const;
  std::string classifyOpponentPlayStyle(const BattleState& battle_state) const;
  
  // MiniMax search methods
  double miniMaxSearch(const BattleState& root_state, int depth, double alpha, double beta, 
                      bool maximizing_player, std::vector<int>& best_line) const;
  double evaluatePosition(const BattleState& battle_state) const;
  std::vector<BattleState> generateLegalMoves(const BattleState& current_state, bool for_ai) const;
  void orderMoves(std::vector<BattleState>& states, bool maximizing_player) const;
  
  // Meta-game analysis methods
  MetaGameAnalyzer::TeamArchetype analyzeTeamArchetype(const Team& team) const;
  std::vector<MetaGameAnalyzer::WinCondition> identifyWinConditions(const BattleState& battle_state) const;
  std::vector<std::string> suggestCounterStrategies(const MetaGameAnalyzer::TeamArchetype& opponent_archetype) const;
  bool isEndgamePosition(const BattleState& battle_state) const;
  std::string getEndgameEvaluation(const BattleState& battle_state) const;
  
  // Advanced evaluation methods (Public for testing)
  double evaluateLongTermAdvantage(const BattleState& battleState) const;
  double detectSetupAttempt(const BattleState& battleState) const;
  double evaluateCounterPlay(const BattleState& battleState) const;
  double assessPositionalAdvantage(const BattleState& battleState) const;
  double evaluateResourceManagement(const BattleState& battleState) const;

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
  bool shouldDisrupt(const BattleState& battleState) const;

  // Advanced decision making
  double calculateExpectedValue(const Move& move,
                                const BattleState& battleState,
                                int turnsAhead) const;

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

  // ──────────────────────────────────────────────────────────────────
  // Phase 1: Advanced Analysis & Prediction Systems
  // ──────────────────────────────────────────────────────────────────

  // Advanced Bayesian Opponent Modeling
  struct BayesianOpponentModel {
    // Move frequency tracking with Bayesian updating
    std::map<int, std::pair<double, double>> move_priors_;  // moveIndex -> (alpha, beta) parameters
    std::map<int, int> move_usage_count_;  // Historical usage tracking
    
    // Play style classification with confidence intervals
    std::map<std::string, double> playstyle_probabilities_;  // "hyperoffensive", "stall", etc.
    std::vector<std::string> observed_behaviors_;  // Behavior history
    
    // Situational pattern learning
    struct SituationalPattern {
      std::string situation;  // "low_hp", "setup_opportunity", "type_disadvantage", etc.
      std::map<int, double> move_preferences;  // Conditional move probabilities
      double confidence;  // Statistical confidence in pattern
    };
    std::vector<SituationalPattern> situational_patterns_;
    
    // Adaptive learning parameters
    double learning_rate_;  // How quickly to adapt to new information
    int total_observations_;  // Total number of opponent actions observed
    double exploration_bonus_;  // Bonus for less-explored moves in prediction
  };

  // MiniMax Search Engine with Alpha-Beta Pruning
  struct MiniMaxSearchEngine {
    struct GameTreeNode {
      BattleState state;
      double evaluation_score;
      int move_taken;  // -1 for switch, >= 0 for move index
      std::vector<std::unique_ptr<GameTreeNode>> children;
      bool is_maximizing_player;  // true for AI turn, false for opponent
      int depth_remaining;
    };
    
    // Search configuration
    static constexpr int kMaxSearchDepth = 4;
    static constexpr int kMaxBranchingFactor = 8;  // Limit moves considered per position
    static constexpr double kAlphaBetaThreshold = 0.1;  // Pruning sensitivity
    
    // Search statistics for performance analysis
    mutable int nodes_evaluated_;
    mutable int alpha_beta_cutoffs_;
    mutable std::chrono::milliseconds search_time_;
    
    // Principal Variation (best line found)
    mutable std::vector<int> principal_variation_;
    mutable double principal_variation_score_;
  };



  // Phase 1 member variables
  mutable BayesianOpponentModel bayesian_model_;
  mutable MiniMaxSearchEngine search_engine_;
  mutable MetaGameAnalyzer meta_analyzer_;
  
  // Performance tracking
  mutable std::map<std::string, std::chrono::milliseconds> method_timings_;
  mutable int total_positions_analyzed_;
};