#pragma once

#include "json.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class Move {
public:
  // Move stats
  std::string name;
  int accuracy;
  int effect_chance;
  int pp;
  int priority;
  int power;

  // Type of move
  std::string damage_class;

  // Move effects
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

  // Constructor
  explicit Move(const std::string &moveName);

  // Default constructor
  Move() = default;

private:
  void loadFromJson(const std::string &file_path);
};