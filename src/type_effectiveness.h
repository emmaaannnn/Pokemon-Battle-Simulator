#pragma once

#include <map>
#include <string>
#include <vector>

class TypeEffectiveness {
public:
  // Type effectiveness multipliers
  enum class Effectiveness {
    NO_EFFECT = 0,      // 0x damage
    NOT_VERY_EFFECTIVE, // 0.5x damage
    NORMAL,             // 1x damage
    SUPER_EFFECTIVE     // 2x damage
  };

  // Get effectiveness multiplier for attacking type vs defending types
  static double
  getEffectivenessMultiplier(const std::string &attackingType,
                             const std::vector<std::string> &defendingTypes);

  // Get effectiveness enum for a specific type matchup
  static Effectiveness getEffectiveness(const std::string &attackingType,
                                        const std::string &defendingType);

  // Convert effectiveness enum to damage multiplier
  static double getMultiplier(Effectiveness effectiveness);

  // Get all valid Pokemon types
  static std::vector<std::string> getAllTypes();

private:
  // Type effectiveness chart - maps attacking type to defending type
  // effectiveness
  static std::map<std::string, std::map<std::string, Effectiveness>> typeChart;

  // Initialise the type effectiveness chart
  static void initialiseTypeChart();

  // Ensure type chart is initialised
  static void ensureInitialised();

  static bool initialised;
};