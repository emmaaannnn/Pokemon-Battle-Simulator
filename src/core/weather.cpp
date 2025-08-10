#include "weather.h"

#include <algorithm>

std::string Weather::getWeatherName(WeatherCondition weather) {
  switch (weather) {
    case WeatherCondition::RAIN:
      return "Rain";
    case WeatherCondition::SUN:
      return "Harsh Sunlight";
    case WeatherCondition::SANDSTORM:
      return "Sandstorm";
    case WeatherCondition::HAIL:
      return "Hail";
    case WeatherCondition::NONE:
    default:
      return "Clear";
  }
}

double Weather::getWeatherDamageMultiplier(WeatherCondition weather,
                                           const std::string &moveType) {
  switch (weather) {
    case WeatherCondition::RAIN:
      if (moveType == "water") return 1.5;  // Water moves boosted
      if (moveType == "fire") return 0.5;   // Fire moves weakened
      break;

    case WeatherCondition::SUN:
      if (moveType == "fire") return 1.5;   // Fire moves boosted
      if (moveType == "water") return 0.5;  // Water moves weakened
      break;

    case WeatherCondition::SANDSTORM:
    case WeatherCondition::HAIL:
    case WeatherCondition::NONE:
    default:
      break;
  }
  return 1.0;  // No modifier
}

bool Weather::isImmuneToWeatherDamage(
    WeatherCondition weather, const std::vector<std::string> &pokemonTypes) {
  switch (weather) {
    case WeatherCondition::SANDSTORM:
      // Rock, Ground, and Steel types are immune to Sandstorm
      return std::find(pokemonTypes.begin(), pokemonTypes.end(), "rock") !=
                 pokemonTypes.end() ||
             std::find(pokemonTypes.begin(), pokemonTypes.end(), "ground") !=
                 pokemonTypes.end() ||
             std::find(pokemonTypes.begin(), pokemonTypes.end(), "steel") !=
                 pokemonTypes.end();

    case WeatherCondition::HAIL:
      // Ice types are immune to Hail
      return std::find(pokemonTypes.begin(), pokemonTypes.end(), "ice") !=
             pokemonTypes.end();

    case WeatherCondition::RAIN:
    case WeatherCondition::SUN:
    case WeatherCondition::NONE:
    default:
      return true;  // No weather damage
  }
}

int Weather::getWeatherDamage(WeatherCondition weather, int maxHP) {
  switch (weather) {
    case WeatherCondition::SANDSTORM:
    case WeatherCondition::HAIL:
      // No damage if Pokemon has 0 HP (already fainted)
      if (maxHP <= 0) {
        return 0;
      }
      return std::max(1, maxHP / 16);  // 1/16 max HP damage, minimum 1
    case WeatherCondition::RAIN:
    case WeatherCondition::SUN:
    case WeatherCondition::NONE:
    default:
      return 0;  // No damage
  }
}