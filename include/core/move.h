#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "json.hpp"

// Forward declaration
enum class StatusCondition;

// Multi-turn move behavior enumeration
enum class MultiTurnBehavior {
  NONE,           // Standard single-turn move
  RECHARGE,       // Requires recharge turn after use (Hyper Beam)
  CHARGE,         // Requires charging turn before use (Solar Beam, Sky Attack)
  CHARGE_BOOST    // Charging turn with stat boost (Skull Bash)
};

class Move {
 public:
  // Move stats
  std::string name;
  int accuracy;
  int effect_chance;
  int pp;          // Maximum PP
  int current_pp;  // Current remaining PP
  int priority;
  int power;

  // Type of move
  std::string damage_class;
  std::string type;

  // Move effects
  std::string ailment_name;
  int ailment_chance;
  std::string category;
  int crit_rate;
  int drain;
  int flinch_chance;
  int healing;
  int max_hits;
  int max_turns;
  int min_hits;
  int min_turns;
  int stat_chance;

  // Multi-turn move properties
  MultiTurnBehavior multi_turn_behavior;
  bool is_weather_dependent;      // For Solar Beam - skips charge in sunny weather
  bool boosts_defense_on_charge;  // For Skull Bash - defense boost during charge

  // Constructor
  explicit Move(const std::string &moveName);

  // Default constructor
  Move() = default;

  // PP Management methods
  bool canUse() const;              // Check if move has PP remaining
  bool usePP();                     // Use 1 PP, returns false if no PP left
  void restorePP(int amount = -1);  // Restore PP (default restores to max)
  int getRemainingPP() const;       // Get current PP
  int getMaxPP() const;             // Get maximum PP

  // Status condition utility
  StatusCondition getStatusCondition() const;

  // Multi-turn move utilities
  bool isMultiTurnMove() const;
  bool requiresCharging() const;
  bool requiresRecharge() const;
  bool skipChargeInSunnyWeather() const;
  bool boostsDefenseOnCharge() const;
  MultiTurnBehavior getMultiTurnBehavior() const;

 private:
  void loadFromJson(const std::string &file_path);
};