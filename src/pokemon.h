#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "json.hpp"
#include "move.h"

// Status conditions enum
enum class StatusCondition {
  NONE,
  POISON,
  BURN,
  PARALYSIS,
  SLEEP,
  FREEZE,
  FLINCH
};

class Pokemon {
 public:
  // Basic info
  std::string name;
  int id;
  std::vector<std::string> types;

  // Base stats
  int hp;
  int current_hp;
  int attack;
  int defense;
  int special_attack;
  int special_defense;
  int speed;
  bool fainted;

  // Status condition state
  StatusCondition status;
  int status_turns_remaining;

  // Stat modifications (stages: -6 to +6, like in real Pokemon)
  int attack_stage;
  int defense_stage;
  int special_attack_stage;
  int special_defense_stage;
  int speed_stage;

  // Array of Move objects
  std::vector<Move> moves;

  // Constructors
  Pokemon();
  explicit Pokemon(const std::string &pokemonName);

  // Utility methods
  void loadMoves();
  bool isAlive() const { return current_hp > 0; }
  double getHealthPercentage() const;
  void takeDamage(int damage);
  void heal(int amount);

  // Status condition methods
  void applyStatusCondition(StatusCondition newStatus);
  void processStatusCondition();
  bool canAct() const;
  std::string getStatusConditionName() const;
  bool hasStatusCondition() const { return status != StatusCondition::NONE; }
  void clearStatusCondition() {
    status = StatusCondition::NONE;
    status_turns_remaining = 0;
  }

  // Stat modification for status effects and stat stages
  int getEffectiveAttack() const;
  int getEffectiveDefense() const;
  int getEffectiveSpecialAttack() const;
  int getEffectiveSpecialDefense() const;
  int getEffectiveSpeed() const;

  // Stat stage modification methods
  void modifyAttack(int stages);
  void modifyDefense(int stages);
  void modifySpecialAttack(int stages);
  void modifySpecialDefense(int stages);
  void modifySpeed(int stages);
  void resetStatStages();

 private:
  void loadFromJson(const std::string &file_path);
};