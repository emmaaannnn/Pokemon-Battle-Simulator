#include "type_effectiveness.h"

// Static member initialisation
std::map<std::string, std::map<std::string, TypeEffectiveness::Effectiveness>>
    TypeEffectiveness::typeChart;
bool TypeEffectiveness::initialised = false;

void TypeEffectiveness::ensureInitialised() {
  if (!initialised) {
    initialiseTypeChart();
    initialised = true;
  }
}

double TypeEffectiveness::getEffectivenessMultiplier(
    const std::string &attackingType,
    const std::vector<std::string> &defendingTypes) {
  ensureInitialised();

  double multiplier = 1.0;

  // Calculate combined effectiveness against all defending types
  for (const std::string &defendingType : defendingTypes) {
    Effectiveness effectiveness =
        getEffectiveness(attackingType, defendingType);
    multiplier *= getMultiplier(effectiveness);
  }

  return multiplier;
}

TypeEffectiveness::Effectiveness
TypeEffectiveness::getEffectiveness(const std::string &attackingType,
                                    const std::string &defendingType) {
  ensureInitialised();

  // Check if attacking type exists in chart
  auto attackingTypeIt = typeChart.find(attackingType);
  if (attackingTypeIt == typeChart.end()) {
    return Effectiveness::NORMAL; // Default to normal effectiveness
  }

  // Check if defending type exists for this attacking type
  auto defendingTypeIt = attackingTypeIt->second.find(defendingType);
  if (defendingTypeIt == attackingTypeIt->second.end()) {
    return Effectiveness::NORMAL; // Default to normal effectiveness
  }

  return defendingTypeIt->second;
}

double TypeEffectiveness::getMultiplier(Effectiveness effectiveness) {
  switch (effectiveness) {
  case Effectiveness::NO_EFFECT:
    return 0.0;
  case Effectiveness::NOT_VERY_EFFECTIVE:
    return 0.5;
  case Effectiveness::NORMAL:
    return 1.0;
  case Effectiveness::SUPER_EFFECTIVE:
    return 2.0;
  default:
    return 1.0;
  }
}

std::vector<std::string> TypeEffectiveness::getAllTypes() {
  return {"normal",   "fire",   "water",  "electric", "grass",   "ice",
          "fighting", "poison", "ground", "flying",   "psychic", "bug",
          "rock",     "ghost",  "dragon", "dark",     "steel",   "fairy"};
}

void TypeEffectiveness::initialiseTypeChart() {
  // Initialise all combinations to normal effectiveness first
  auto types = getAllTypes();
  for (const auto &attackingType : types) {
    for (const auto &defendingType : types) {
      typeChart[attackingType][defendingType] = Effectiveness::NORMAL;
    }
  }

  // Normal type effectiveness
  typeChart["normal"]["rock"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["normal"]["ghost"] = Effectiveness::NO_EFFECT;
  typeChart["normal"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;

  // Fire type effectiveness
  typeChart["fire"]["fire"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["fire"]["water"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["fire"]["grass"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["fire"]["ice"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["fire"]["bug"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["fire"]["rock"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["fire"]["dragon"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["fire"]["steel"] = Effectiveness::SUPER_EFFECTIVE;

  // Water type effectiveness
  typeChart["water"]["fire"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["water"]["water"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["water"]["grass"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["water"]["ground"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["water"]["rock"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["water"]["dragon"] = Effectiveness::NOT_VERY_EFFECTIVE;

  // Electric type effectiveness
  typeChart["electric"]["water"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["electric"]["electric"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["electric"]["grass"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["electric"]["ground"] = Effectiveness::NO_EFFECT;
  typeChart["electric"]["flying"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["electric"]["dragon"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["electric"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;

  // Grass type effectiveness
  typeChart["grass"]["fire"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["grass"]["water"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["grass"]["grass"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["grass"]["poison"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["grass"]["ground"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["grass"]["flying"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["grass"]["bug"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["grass"]["rock"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["grass"]["dragon"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["grass"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;

  // Ice type effectiveness
  typeChart["ice"]["fire"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["ice"]["water"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["ice"]["grass"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["ice"]["ice"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["ice"]["ground"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["ice"]["flying"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["ice"]["dragon"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["ice"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;

  // Fighting type effectiveness
  typeChart["fighting"]["normal"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["fighting"]["ice"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["fighting"]["poison"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["fighting"]["flying"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["fighting"]["psychic"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["fighting"]["bug"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["fighting"]["rock"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["fighting"]["ghost"] = Effectiveness::NO_EFFECT;
  typeChart["fighting"]["dark"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["fighting"]["steel"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["fighting"]["fairy"] = Effectiveness::NOT_VERY_EFFECTIVE;

  // Poison type effectiveness
  typeChart["poison"]["grass"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["poison"]["poison"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["poison"]["ground"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["poison"]["rock"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["poison"]["ghost"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["poison"]["steel"] = Effectiveness::NO_EFFECT;
  typeChart["poison"]["fairy"] = Effectiveness::SUPER_EFFECTIVE;

  // Ground type effectiveness
  typeChart["ground"]["fire"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["ground"]["electric"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["ground"]["grass"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["ground"]["poison"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["ground"]["flying"] = Effectiveness::NO_EFFECT;
  typeChart["ground"]["bug"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["ground"]["rock"] = Effectiveness::SUPER_EFFECTIVE;
  // Ground vs Steel is neutral (1.0x) in Pokemon games

  // Flying type effectiveness
  typeChart["flying"]["electric"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["flying"]["grass"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["flying"]["fighting"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["flying"]["bug"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["flying"]["rock"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["flying"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;

  // Psychic type effectiveness
  typeChart["psychic"]["fighting"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["psychic"]["poison"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["psychic"]["psychic"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["psychic"]["dark"] = Effectiveness::NO_EFFECT;
  typeChart["psychic"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;

  // Bug type effectiveness
  typeChart["bug"]["fire"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["bug"]["grass"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["bug"]["fighting"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["bug"]["poison"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["bug"]["flying"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["bug"]["psychic"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["bug"]["ghost"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["bug"]["dark"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["bug"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["bug"]["fairy"] = Effectiveness::NOT_VERY_EFFECTIVE;

  // Rock type effectiveness
  typeChart["rock"]["fire"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["rock"]["ice"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["rock"]["fighting"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["rock"]["ground"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["rock"]["flying"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["rock"]["bug"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["rock"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;

  // Ghost type effectiveness
  typeChart["ghost"]["normal"] = Effectiveness::NO_EFFECT;
  typeChart["ghost"]["psychic"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["ghost"]["ghost"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["ghost"]["dark"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["ghost"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;

  // Dragon type effectiveness
  typeChart["dragon"]["dragon"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["dragon"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["dragon"]["fairy"] = Effectiveness::NO_EFFECT;

  // Dark type effectiveness
  typeChart["dark"]["fighting"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["dark"]["psychic"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["dark"]["ghost"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["dark"]["dark"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["dark"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["dark"]["fairy"] = Effectiveness::NOT_VERY_EFFECTIVE;

  // Steel type effectiveness
  typeChart["steel"]["fire"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["steel"]["water"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["steel"]["electric"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["steel"]["ice"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["steel"]["rock"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["steel"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["steel"]["fairy"] = Effectiveness::SUPER_EFFECTIVE;

  // Fairy type effectiveness
  typeChart["fairy"]["fire"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["fairy"]["fighting"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["fairy"]["poison"] = Effectiveness::NOT_VERY_EFFECTIVE;
  typeChart["fairy"]["dragon"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["fairy"]["dark"] = Effectiveness::SUPER_EFFECTIVE;
  typeChart["fairy"]["steel"] = Effectiveness::NOT_VERY_EFFECTIVE;
}