#pragma once

#include <string>
#include <vector>

// Weather conditions enum
enum class WeatherCondition {
  NONE,      // Clear weather
  RAIN,      // Boosts Water moves, weakens Fire moves
  SUN,       // Boosts Fire moves, weakens Water moves
  SANDSTORM, // Damages non Rock/Ground/Steel types
  HAIL       // Damages non Ice types
};

class Weather {
public:
  // Get weather name for display
  static std::string getWeatherName(WeatherCondition weather);

  // Get weather damage multiplier for move types
  static double getWeatherDamageMultiplier(WeatherCondition weather,
                                           const std::string &moveType);

  // Check if Pokemon type is immune to weather damage
  static bool
  isImmuneToWeatherDamage(WeatherCondition weather,
                          const std::vector<std::string> &pokemonTypes);

  // Get weather damage amount (1/16 HP for Sandstorm/Hail)
  static int getWeatherDamage(WeatherCondition weather, int maxHP);
};