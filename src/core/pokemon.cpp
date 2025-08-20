#include "pokemon.h"

#include <random>
#include <set>
#include "input_validator.h"

using json = nlohmann::json;

Pokemon::Pokemon()
    : name(""),
      id(0),
      hp(0),
      attack(0),
      defense(0),
      special_attack(0),
      special_defense(0),
      speed(0),
      fainted(false),
      status(StatusCondition::NONE),
      status_turns_remaining(0),
      is_charging(false),
      must_recharge(false),
      charging_move_index(-1),
      charging_move_name(""),
      attack_stage(0),
      defense_stage(0),
      special_attack_stage(0),
      special_defense_stage(0),
      speed_stage(0) {}

Pokemon::Pokemon(const std::string& pokemonName)
    : fainted(false),
      status(StatusCondition::NONE),
      status_turns_remaining(0),
      is_charging(false),
      must_recharge(false),
      charging_move_index(-1),
      charging_move_name(""),
      attack_stage(0),
      defense_stage(0),
      special_attack_stage(0),
      special_defense_stage(0),
      speed_stage(0) {
  
  // Secure file path validation and construction
  auto pathResult = InputValidator::validateDataFilePath(pokemonName, "pokemon", ".json");
  if (!pathResult.isValid()) {
    std::cerr << "Pokemon loading failed - " << pathResult.errorMessage << std::endl;
    return;
  }
  
  loadFromJson(pathResult.value);
  // loadMoves(); // Removed - moves are loaded by Team::loadTeams()
}

void Pokemon::loadFromJson(const std::string& file_path) {
  // Additional security validation for the file path
  auto accessValidation = InputValidator::validateFileAccessibility(file_path);
  if (!accessValidation.isValid()) {
    std::cerr << "Pokemon file accessibility check failed: " << accessValidation.errorMessage << std::endl;
    return;
  }

  auto file = std::ifstream(file_path);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << file_path << std::endl;
    return;
  }

  auto pokemon_json = json{};
  try {
    file >> pokemon_json;
  } catch (const json::parse_error& e) {
    std::cerr << "JSON parse error in " << file_path << ": " << e.what() << std::endl;
    return;
  }

  // Define valid Pokemon types for validation
  static const std::set<std::string> validTypes = {
    "bug", "dragon", "electric", "fairy", "fighting", "fire", 
    "flying", "ghost", "grass", "ground", "ice", "normal", 
    "poison", "psychic", "rock", "water"
  };

  // Validate and extract name (1-50 characters, alphanumeric and common symbols)
  auto nameResult = InputValidator::getJsonString(pokemon_json, "name", 1, 50);
  if (!nameResult.isValid()) {
    std::cerr << "Pokemon name validation failed: " << nameResult.errorMessage << std::endl;
    return;
  }
  name = nameResult.value;

  // Validate and extract ID (1-999 range for Pokemon IDs)
  auto idResult = InputValidator::getJsonInt(pokemon_json, "id", 1, 999);
  if (!idResult.isValid()) {
    std::cerr << "Pokemon ID validation failed: " << idResult.errorMessage << std::endl;
    return;
  }
  id = idResult.value;

  // Validate types array (must exist and be an array)
  if (pokemon_json.find("types") == pokemon_json.end() || !pokemon_json["types"].is_array()) {
    std::cerr << "Pokemon types field missing or invalid in " << file_path << std::endl;
    return;
  }

  // Clear existing types and validate each type
  types.clear();
  const auto& typesArray = pokemon_json["types"];
  if (typesArray.empty() || typesArray.size() > 2) {
    std::cerr << "Pokemon must have 1-2 types in " << file_path << std::endl;
    return;
  }

  for (const auto& typeElement : typesArray) {
    if (!typeElement.is_string()) {
      std::cerr << "Invalid type format in " << file_path << std::endl;
      return;
    }
    
    std::string typeStr = typeElement.get<std::string>();
    if (validTypes.find(typeStr) == validTypes.end()) {
      std::cerr << "Invalid Pokemon type '" << typeStr << "' in " << file_path << std::endl;
      return;
    }
    types.push_back(typeStr);
  }

  // Validate base_stats object exists
  if (pokemon_json.find("base_stats") == pokemon_json.end() || !pokemon_json["base_stats"].is_object()) {
    std::cerr << "Pokemon base_stats field missing or invalid in " << file_path << std::endl;
    return;
  }

  const auto& base_stats = pokemon_json["base_stats"];

  // Validate and extract HP (1-255 typical range for Pokemon stats)
  auto hpResult = InputValidator::getJsonInt(base_stats, "hp", 1, 255);
  if (!hpResult.isValid()) {
    std::cerr << "Pokemon HP validation failed: " << hpResult.errorMessage << std::endl;
    return;
  }
  hp = hpResult.value;
  current_hp = hp;

  // Validate and extract Attack (1-255 range)
  auto attackResult = InputValidator::getJsonInt(base_stats, "attack", 1, 255);
  if (!attackResult.isValid()) {
    std::cerr << "Pokemon Attack validation failed: " << attackResult.errorMessage << std::endl;
    return;
  }
  attack = attackResult.value;

  // Validate and extract Defense (1-255 range)
  auto defenseResult = InputValidator::getJsonInt(base_stats, "defense", 1, 255);
  if (!defenseResult.isValid()) {
    std::cerr << "Pokemon Defense validation failed: " << defenseResult.errorMessage << std::endl;
    return;
  }
  defense = defenseResult.value;

  // Validate and extract Special Attack (1-255 range)
  auto specialAttackResult = InputValidator::getJsonInt(base_stats, "special-attack", 1, 255);
  if (!specialAttackResult.isValid()) {
    std::cerr << "Pokemon Special Attack validation failed: " << specialAttackResult.errorMessage << std::endl;
    return;
  }
  special_attack = specialAttackResult.value;

  // Validate and extract Special Defense (1-255 range)
  auto specialDefenseResult = InputValidator::getJsonInt(base_stats, "special-defense", 1, 255);
  if (!specialDefenseResult.isValid()) {
    std::cerr << "Pokemon Special Defense validation failed: " << specialDefenseResult.errorMessage << std::endl;
    return;
  }
  special_defense = specialDefenseResult.value;

  // Validate and extract Speed (1-255 range)
  auto speedResult = InputValidator::getJsonInt(base_stats, "speed", 1, 255);
  if (!speedResult.isValid()) {
    std::cerr << "Pokemon Speed validation failed: " << speedResult.errorMessage << std::endl;
    return;
  }
  speed = speedResult.value;

  fainted = false;
}

void Pokemon::loadMoves() {
  auto file = std::ifstream("data/moves/" + name + ".json");
  if (!file.is_open()) {
    std::cerr << "Error opening file: data/moves/" + name + ".json"
              << std::endl;
    return;
  }
  auto move_json = json{};
  file >> move_json;
  for (const auto& move : move_json) {
    auto moveName = move["move"]["name"].get<std::string>();
    auto moveObj = Move(moveName);
    moves.push_back(moveObj);
  }
}

double Pokemon::getHealthPercentage() const {
  return (static_cast<double>(current_hp) / hp) * 100.0;
}

void Pokemon::takeDamage(int damage) {
  current_hp = std::max(0, current_hp - damage);
  if (current_hp == 0) {
    fainted = true;
  }
}

void Pokemon::heal(int amount) {
  current_hp = std::min(hp, current_hp + amount);
  if (current_hp > 0) {
    fainted = false;
  }
}

void Pokemon::applyStatusCondition(StatusCondition newStatus) {
  // Flinch can be applied even if Pokemon has another status condition
  if (newStatus == StatusCondition::FLINCH) {
    status = newStatus;
    status_turns_remaining = 1;  // Flinch only lasts 1 turn
    return;
  }

  // Can't apply other status if already has one (except replacing with same
  // type)
  if (hasStatusCondition() && status != newStatus) {
    return;
  }

  status = newStatus;

  // Set duration based on status type
  switch (newStatus) {
    case StatusCondition::SLEEP:
      // Sleep lasts 1-3 turns
      status_turns_remaining = 1 + (std::rand() % 3);
      break;
    case StatusCondition::POISON:
    case StatusCondition::BURN:
    case StatusCondition::PARALYSIS:
    case StatusCondition::FREEZE:
      // These last until cured or switched out
      status_turns_remaining = -1;  // Indefinite
      break;
    case StatusCondition::NONE:
      status_turns_remaining = 0;
      break;
    case StatusCondition::FLINCH:
      // Already handled above
      break;
  }
}

void Pokemon::processStatusCondition() {
  if (!hasStatusCondition()) return;

  switch (status) {
    case StatusCondition::POISON:
      // Poison deals 1/8 max HP damage each turn
      {
        int damage = std::max(1, hp / 8);
        takeDamage(damage);
        std::cout << name << " is hurt by poison! (-" << damage << " HP)"
                  << std::endl;
      }
      break;

    case StatusCondition::BURN:
      // Burn deals 1/16 max HP damage each turn
      {
        int damage = std::max(1, hp / 16);
        takeDamage(damage);
        std::cout << name << " is hurt by burn! (-" << damage << " HP)"
                  << std::endl;
      }
      break;

    case StatusCondition::SLEEP:
      // Sleep countdown
      if (status_turns_remaining > 0) {
        status_turns_remaining--;
        std::cout << name << " is fast asleep!" << std::endl;
        if (status_turns_remaining == 0) {
          clearStatusCondition();
          std::cout << name << " woke up!" << std::endl;
        }
      }
      break;

    case StatusCondition::FREEZE:
      // 20% chance to thaw out each turn
      {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0.0, 1.0);

        if (dis(gen) < 0.20) {
          clearStatusCondition();
          std::cout << name << " thawed out!" << std::endl;
        } else {
          std::cout << name << " is frozen solid!" << std::endl;
        }
      }
      break;

    case StatusCondition::PARALYSIS:
      // Paralysis persists until cured
      std::cout << name << " is paralyzed!" << std::endl;
      break;

    case StatusCondition::FLINCH:
      // Flinch automatically clears after 1 turn
      std::cout << name << " flinched and couldn't move!" << std::endl;
      clearStatusCondition();
      break;

    case StatusCondition::NONE:
      break;
  }
}

bool Pokemon::canAct() const {
  if (!isAlive()) return false;

  switch (status) {
    case StatusCondition::SLEEP:
    case StatusCondition::FREEZE:
    case StatusCondition::FLINCH:
      return false;

    case StatusCondition::PARALYSIS:
      // 25% chance to be fully paralyzed
      {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0.0, 1.0);
        return dis(gen) >= 0.25;
      }

    default:
      return true;
  }
}

bool Pokemon::canAct(std::mt19937& rng) const {
  if (!isAlive()) return false;

  switch (status) {
    case StatusCondition::SLEEP:
    case StatusCondition::FREEZE:
    case StatusCondition::FLINCH:
      return false;

    case StatusCondition::PARALYSIS:
      // 25% chance to be fully paralyzed using provided seeded RNG
      {
        std::uniform_real_distribution<> dis(0.0, 1.0);
        return dis(rng) >= 0.25;
      }

    default:
      return true;
  }
}

std::string Pokemon::getStatusConditionName() const {
  switch (status) {
    case StatusCondition::POISON:
      return "Poisoned";
    case StatusCondition::BURN:
      return "Burned";
    case StatusCondition::PARALYSIS:
      return "Paralyzed";
    case StatusCondition::SLEEP:
      return "Asleep";
    case StatusCondition::FREEZE:
      return "Frozen";
    case StatusCondition::FLINCH:
      return "Flinched";
    case StatusCondition::NONE:
      return "";
    default:
      return "";
  }
}

// Helper function to calculate stat stage multiplier
double getStatStageMultiplier(int stage) {
  // Clamp stage to valid range (-6 to +6)
  stage = std::max(-6, std::min(6, stage));

  if (stage >= 0) {
    return 1.0 + (stage * 0.5);  // +1 stage = 1.5x, +2 = 2.0x, etc.
  } else {
    return 1.0 / (1.0 - (stage * 0.5));  // -1 stage = 0.66x, -2 = 0.5x, etc.
  }
}

int Pokemon::getEffectiveAttack() const {
  int base_attack = attack;

  // Apply status condition effects
  if (status == StatusCondition::BURN) {
    base_attack = base_attack / 2;
  }

  // Apply stat stage modifications
  double multiplier = getStatStageMultiplier(attack_stage);
  return static_cast<int>(base_attack * multiplier);
}

int Pokemon::getEffectiveDefense() const {
  double multiplier = getStatStageMultiplier(defense_stage);
  return static_cast<int>(defense * multiplier);
}

int Pokemon::getEffectiveSpecialAttack() const {
  double multiplier = getStatStageMultiplier(special_attack_stage);
  return static_cast<int>(special_attack * multiplier);
}

int Pokemon::getEffectiveSpecialDefense() const {
  double multiplier = getStatStageMultiplier(special_defense_stage);
  return static_cast<int>(special_defense * multiplier);
}

int Pokemon::getEffectiveSpeed() const {
  int base_speed = speed;

  // Apply status condition effects
  if (status == StatusCondition::PARALYSIS) {
    base_speed = base_speed / 2;
  }

  // Apply stat stage modifications
  double multiplier = getStatStageMultiplier(speed_stage);
  return static_cast<int>(base_speed * multiplier);
}

// Stat stage modification methods
void Pokemon::modifyAttack(int stages) {
  attack_stage = std::max(-6, std::min(6, attack_stage + stages));
}

void Pokemon::modifyDefense(int stages) {
  defense_stage = std::max(-6, std::min(6, defense_stage + stages));
}

void Pokemon::modifySpecialAttack(int stages) {
  special_attack_stage =
      std::max(-6, std::min(6, special_attack_stage + stages));
}

void Pokemon::modifySpecialDefense(int stages) {
  special_defense_stage =
      std::max(-6, std::min(6, special_defense_stage + stages));
}

void Pokemon::modifySpeed(int stages) {
  speed_stage = std::max(-6, std::min(6, speed_stage + stages));
}

void Pokemon::resetStatStages() {
  attack_stage = 0;
  defense_stage = 0;
  special_attack_stage = 0;
  special_defense_stage = 0;
  speed_stage = 0;
}

// Multi-turn move state management implementations
void Pokemon::startCharging(int moveIndex, const std::string& moveName) {
  is_charging = true;
  must_recharge = false;
  charging_move_index = moveIndex;
  charging_move_name = moveName;
  
  // Apply any charging effects (like Skull Bash defense boost)
  if (moveIndex >= 0 && moveIndex < static_cast<int>(moves.size())) {
    const Move& move = moves[moveIndex];
    if (move.boostsDefenseOnCharge()) {
      modifyDefense(1);
      std::cout << name << "'s Defense rose while charging " << moveName << "!" << std::endl;
    }
  }
}

void Pokemon::finishCharging() {
  is_charging = false;
  charging_move_index = -1;
  charging_move_name = "";
}

void Pokemon::startRecharge() {
  must_recharge = true;
  is_charging = false;
  charging_move_index = -1;
  charging_move_name = "";
}

void Pokemon::finishRecharge() {
  must_recharge = false;
}

bool Pokemon::canActThisTurn() const {
  // Check basic action ability (status conditions)
  if (!canAct()) {
    return false;
  }
  
  // Check multi-turn move restrictions
  if (must_recharge) {
    return false;  // Cannot act during recharge turn
  }
  
  // Pokemon can act normally (including during charging turn for move execution)
  return true;
}
