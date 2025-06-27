#pragma once

#include "json.hpp"
#include "move.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Status conditions enum
enum class StatusCondition { NONE, POISON, BURN, PARALYSIS, SLEEP, FREEZE };

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

  // Stat modification for status effects
  int getEffectiveAttack() const;
  int getEffectiveSpeed() const;

private:
  void loadFromJson(const std::string &file_path);
};