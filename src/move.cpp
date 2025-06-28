#include "move.h"
#include "move_type_mapping.h"
#include "pokemon.h"
#include <algorithm>

using json = nlohmann::json;

Move::Move(const std::string &moveName) {
  loadFromJson("data/moves/" + moveName + ".json");
}

void Move::loadFromJson(const std::string &file_path) {
  auto file = std::ifstream(file_path);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << file_path << std::endl;
    return;
  }
  auto move_json = json{};
  file >> move_json;

  auto moveName = move_json["name"].get<std::string>();
  // Basic move attributes
  name = move_json["name"];

  if (move_json["accuracy"].is_null()) {
    accuracy = 0;
  } else {
    accuracy = move_json["accuracy"];
  }

  if (move_json["effect_chance"].is_null()) {
    effect_chance = -1;
  } else {
    effect_chance = move_json["effect_chance"];
  }

  pp = move_json.value("pp", 0);
  current_pp = pp; // Initialize current PP to maximum PP
  priority = move_json.value("priority", 0);

  if (move_json["power"].is_null()) {
    power = -1;
  } else {
    power = move_json["power"];
  }

  damage_class = move_json["damage_class"]["name"];

  // Get move type from mapping
  type = MoveTypeMapping::getMoveType(name);

  // Load ailment information
  ailment_name = move_json["Info"]["ailment"]["name"];
  ailment_chance = move_json["Info"]["ailment_chance"];
  category = move_json["Info"]["category"]["name"];
  crit_rate = move_json["Info"]["crit_rate"];
  drain = move_json["Info"]["drain"];
  flinch_chance = move_json["Info"]["flinch_chance"];
  healing = move_json["Info"]["healing"];

  // NULL CHECKS FOR HITS AND TURNS
  if (move_json["Info"]["max_hits"].is_null()) {
    max_hits = 1;
  } else {
    max_hits = move_json["Info"]["max_hits"]; // NULL - RELY ON ISNULL CHECK
  }

  if (move_json["Info"]["max_turns"].is_null()) {
    max_turns = 1;
  } else {
    max_turns = move_json["Info"]["max_turns"]; // NULL - RELY ON ISNULL CHECK
  }

  if (move_json["Info"]["min_hits"].is_null()) {
    min_hits = 1;
  } else {
    min_hits = move_json["Info"]["min_hits"]; // NULL - RELY ON ISNULL CHECK
  }

  if (move_json["Info"]["min_turns"].is_null()) {
    min_turns = 1;
  } else {
    min_turns = move_json["Info"]["min_turns"]; // NULL - RELY ON ISNULL CHECK
  }

  stat_chance = move_json["Info"]["stat_chance"];
}

// Helper function to convert ailment name to StatusCondition enum
StatusCondition Move::getStatusCondition() const {
  if (ailment_name == "poison")
    return StatusCondition::POISON;
  if (ailment_name == "burn")
    return StatusCondition::BURN;
  if (ailment_name == "paralysis")
    return StatusCondition::PARALYSIS;
  if (ailment_name == "sleep")
    return StatusCondition::SLEEP;
  if (ailment_name == "freeze")
    return StatusCondition::FREEZE;
  return StatusCondition::NONE;
}

// PP Management method implementations
bool Move::canUse() const { return current_pp > 0; }

bool Move::usePP() {
  if (current_pp > 0) {
    current_pp--;
    return true;
  }
  return false;
}

void Move::restorePP(int amount) {
  if (amount == -1) {
    // Restore to maximum PP
    current_pp = pp;
  } else {
    // Restore specified amount, but don't exceed maximum
    current_pp = std::min(pp, current_pp + amount);
  }
}

int Move::getRemainingPP() const { return current_pp; }

int Move::getMaxPP() const { return pp; }