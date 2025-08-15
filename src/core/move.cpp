#include "move.h"

#include <algorithm>
#include <set>

#include "move_type_mapping.h"
#include "pokemon.h"
#include "input_validator.h"

using json = nlohmann::json;

Move::Move(const std::string &moveName) {
  // Secure file path validation and construction
  auto pathResult = InputValidator::validateDataFilePath(moveName, "moves", ".json");
  if (!pathResult.isValid()) {
    std::cerr << "Move loading failed - " << pathResult.errorMessage << std::endl;
    return;
  }
  
  loadFromJson(pathResult.value);
}

void Move::loadFromJson(const std::string &file_path) {
  // Additional security validation for the file path
  auto accessValidation = InputValidator::validateFileAccessibility(file_path);
  if (!accessValidation.isValid()) {
    std::cerr << "Move file accessibility check failed: " << accessValidation.errorMessage << std::endl;
    return;
  }

  auto file = std::ifstream(file_path);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << file_path << std::endl;
    return;
  }

  auto move_json = json{};
  try {
    file >> move_json;
  } catch (const json::parse_error& e) {
    std::cerr << "JSON parse error in " << file_path << ": " << e.what() << std::endl;
    return;
  }

  // Define valid damage classes for validation
  static const std::set<std::string> validDamageClasses = {
    "physical", "special", "status"
  };

  // Define valid ailment names for validation
  static const std::set<std::string> validAilments = {
    "none", "poison", "burn", "paralysis", "sleep", "freeze", 
    "confusion", "heal", "disable", "yawn", "nightmare", "swagger", "trap"
  };

  // Validate and extract name (1-50 characters, allowing hyphens for move names)
  auto nameResult = InputValidator::getJsonString(move_json, "name", 1, 50);
  if (!nameResult.isValid()) {
    std::cerr << "Move name validation failed: " << nameResult.errorMessage << std::endl;
    return;
  }
  name = nameResult.value;

  // Handle accuracy field (can be null for moves that always hit)
  if (move_json["accuracy"].is_null()) {
    accuracy = 100;  // Default for moves that always hit
  } else {
    auto accuracyResult = InputValidator::getJsonInt(move_json, "accuracy", 0, 100);
    if (!accuracyResult.isValid()) {
      std::cerr << "Move accuracy validation failed: " << accuracyResult.errorMessage << std::endl;
      return;
    }
    accuracy = accuracyResult.value;
  }

  // Handle effect_chance field (can be null for moves with no effect chance)
  if (move_json["effect_chance"].is_null()) {
    effect_chance = -1;  // No effect chance
  } else {
    auto effectChanceResult = InputValidator::getJsonInt(move_json, "effect_chance", 0, 100);
    if (!effectChanceResult.isValid()) {
      std::cerr << "Move effect_chance validation failed: " << effectChanceResult.errorMessage << std::endl;
      return;
    }
    effect_chance = effectChanceResult.value;
  }

  // Validate and extract PP (1-40 typical range)
  auto ppResult = InputValidator::getJsonInt(move_json, "pp", 1, 40);
  if (!ppResult.isValid()) {
    std::cerr << "Move PP validation failed: " << ppResult.errorMessage << std::endl;
    return;
  }
  pp = ppResult.value;
  current_pp = pp;  // Initialize current PP to maximum PP

  // Validate and extract priority (-7 to +5 typical range for move priority)
  auto priorityResult = InputValidator::getJsonInt(move_json, "priority", -7, 5, 0);
  if (!priorityResult.isValid()) {
    std::cerr << "Move priority validation failed: " << priorityResult.errorMessage << std::endl;
    return;
  }
  priority = priorityResult.value;

  // Handle power field (can be null for status moves)
  if (move_json["power"].is_null()) {
    power = -1;  // Status moves have no power
  } else {
    auto powerResult = InputValidator::getJsonInt(move_json, "power", 0, 250);
    if (!powerResult.isValid()) {
      std::cerr << "Move power validation failed: " << powerResult.errorMessage << std::endl;
      return;
    }
    power = powerResult.value;
  }

  // Validate damage_class nested object
  if (move_json.find("damage_class") == move_json.end() || !move_json["damage_class"].is_object()) {
    std::cerr << "Move damage_class field missing or invalid in " << file_path << std::endl;
    return;
  }

  auto damageClassResult = InputValidator::getJsonString(move_json["damage_class"], "name", 1, 20);
  if (!damageClassResult.isValid()) {
    std::cerr << "Move damage_class name validation failed: " << damageClassResult.errorMessage << std::endl;
    return;
  }

  if (validDamageClasses.find(damageClassResult.value) == validDamageClasses.end()) {
    std::cerr << "Invalid damage class '" << damageClassResult.value << "' in " << file_path << std::endl;
    return;
  }
  damage_class = damageClassResult.value;

  // Get move type from mapping (this provides additional validation)
  type = MoveTypeMapping::getMoveType(name);

  // Validate Info object exists
  if (move_json.find("Info") == move_json.end() || !move_json["Info"].is_object()) {
    std::cerr << "Move Info field missing or invalid in " << file_path << std::endl;
    return;
  }

  const auto& info = move_json["Info"];

  // Validate ailment nested object
  if (info.find("ailment") == info.end() || !info["ailment"].is_object()) {
    std::cerr << "Move ailment field missing or invalid in " << file_path << std::endl;
    return;
  }

  auto ailmentResult = InputValidator::getJsonString(info["ailment"], "name", 1, 20);
  if (!ailmentResult.isValid()) {
    std::cerr << "Move ailment name validation failed: " << ailmentResult.errorMessage << std::endl;
    return;
  }

  if (validAilments.find(ailmentResult.value) == validAilments.end()) {
    std::cerr << "Invalid ailment '" << ailmentResult.value << "' in " << file_path << std::endl;
    return;
  }
  ailment_name = ailmentResult.value;

  // Validate and extract ailment_chance (0-100 range)
  auto ailmentChanceResult = InputValidator::getJsonInt(info, "ailment_chance", 0, 100, 0);
  if (!ailmentChanceResult.isValid()) {
    std::cerr << "Move ailment_chance validation failed: " << ailmentChanceResult.errorMessage << std::endl;
    return;
  }
  ailment_chance = ailmentChanceResult.value;

  // Validate category nested object
  if (info.find("category") == info.end() || !info["category"].is_object()) {
    std::cerr << "Move category field missing or invalid in " << file_path << std::endl;
    return;
  }

  auto categoryResult = InputValidator::getJsonString(info["category"], "name", 1, 30);
  if (!categoryResult.isValid()) {
    std::cerr << "Move category name validation failed: " << categoryResult.errorMessage << std::endl;
    return;
  }
  category = categoryResult.value;

  // Validate and extract various Info fields with appropriate ranges
  auto critRateResult = InputValidator::getJsonInt(info, "crit_rate", 0, 5, 0);
  if (!critRateResult.isValid()) {
    std::cerr << "Move crit_rate validation failed: " << critRateResult.errorMessage << std::endl;
    return;
  }
  crit_rate = critRateResult.value;

  auto drainResult = InputValidator::getJsonInt(info, "drain", -100, 100, 0);
  if (!drainResult.isValid()) {
    std::cerr << "Move drain validation failed: " << drainResult.errorMessage << std::endl;
    return;
  }
  drain = drainResult.value;

  auto flinchChanceResult = InputValidator::getJsonInt(info, "flinch_chance", 0, 100, 0);
  if (!flinchChanceResult.isValid()) {
    std::cerr << "Move flinch_chance validation failed: " << flinchChanceResult.errorMessage << std::endl;
    return;
  }
  flinch_chance = flinchChanceResult.value;

  auto healingResult = InputValidator::getJsonInt(info, "healing", -100, 100, 0);
  if (!healingResult.isValid()) {
    std::cerr << "Move healing validation failed: " << healingResult.errorMessage << std::endl;
    return;
  }
  healing = healingResult.value;

  // Handle hit/turn fields that can be null
  if (info["max_hits"].is_null()) {
    max_hits = 1;
  } else {
    auto maxHitsResult = InputValidator::getJsonInt(info, "max_hits", 1, 10);
    if (!maxHitsResult.isValid()) {
      std::cerr << "Move max_hits validation failed: " << maxHitsResult.errorMessage << std::endl;
      return;
    }
    max_hits = maxHitsResult.value;
  }

  if (info["max_turns"].is_null()) {
    max_turns = 1;
  } else {
    auto maxTurnsResult = InputValidator::getJsonInt(info, "max_turns", 1, 10);
    if (!maxTurnsResult.isValid()) {
      std::cerr << "Move max_turns validation failed: " << maxTurnsResult.errorMessage << std::endl;
      return;
    }
    max_turns = maxTurnsResult.value;
  }

  if (info["min_hits"].is_null()) {
    min_hits = 1;
  } else {
    auto minHitsResult = InputValidator::getJsonInt(info, "min_hits", 1, 10);
    if (!minHitsResult.isValid()) {
      std::cerr << "Move min_hits validation failed: " << minHitsResult.errorMessage << std::endl;
      return;
    }
    min_hits = minHitsResult.value;
  }

  if (info["min_turns"].is_null()) {
    min_turns = 1;
  } else {
    auto minTurnsResult = InputValidator::getJsonInt(info, "min_turns", 1, 10);
    if (!minTurnsResult.isValid()) {
      std::cerr << "Move min_turns validation failed: " << minTurnsResult.errorMessage << std::endl;
      return;
    }
    min_turns = minTurnsResult.value;
  }

  auto statChanceResult = InputValidator::getJsonInt(info, "stat_chance", 0, 100);
  if (!statChanceResult.isValid()) {
    std::cerr << "Move stat_chance validation failed: " << statChanceResult.errorMessage << std::endl;
    return;
  }
  stat_chance = statChanceResult.value;
}

// Helper function to convert ailment name to StatusCondition enum
StatusCondition Move::getStatusCondition() const {
  if (ailment_name == "poison") return StatusCondition::POISON;
  if (ailment_name == "burn") return StatusCondition::BURN;
  if (ailment_name == "paralysis") return StatusCondition::PARALYSIS;
  if (ailment_name == "sleep") return StatusCondition::SLEEP;
  if (ailment_name == "freeze") return StatusCondition::FREEZE;
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