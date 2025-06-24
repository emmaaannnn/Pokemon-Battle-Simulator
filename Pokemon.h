#pragma once

#include "Move.h"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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

private:
  void loadFromJson(const std::string &file_path);
};