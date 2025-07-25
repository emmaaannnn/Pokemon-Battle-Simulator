#include "pokemon.h"

#include <random>

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
      attack_stage(0),
      defense_stage(0),
      special_attack_stage(0),
      special_defense_stage(0),
      speed_stage(0) {}

Pokemon::Pokemon(const std::string& pokemonName)
    : fainted(false),
      status(StatusCondition::NONE),
      status_turns_remaining(0),
      attack_stage(0),
      defense_stage(0),
      special_attack_stage(0),
      special_defense_stage(0),
      speed_stage(0) {
  loadFromJson("data/pokemon/" + pokemonName + ".json");
  // loadMoves(); // Removed - moves are loaded by Team::loadTeams()
}

void Pokemon::loadFromJson(const std::string& file_path) {
  auto file = std::ifstream(file_path);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << file_path << std::endl;
    return;
  }

  auto pokemon_json = json{};
  file >> pokemon_json;

  // Map JSON data to class members
  name = pokemon_json["name"];
  id = pokemon_json["id"];

  // Accessing types which is an array of strings
  for (const auto& type : pokemon_json["types"]) {
    types.push_back(type);
  }

  // Accessing base_stats which is an object
  const auto& base_stats = pokemon_json["base_stats"];
  hp = base_stats["hp"];
  current_hp = hp;
  attack = base_stats["attack"];
  defense = base_stats["defense"];
  special_attack = base_stats["special-attack"];
  special_defense = base_stats["special-defense"];
  speed = base_stats["speed"];
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
