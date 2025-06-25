#pragma once

#include <map>
#include <string>

class MoveTypeMapping {
public:
  // Get the type of a move by its name
  static std::string getMoveType(const std::string &moveName);

private:
  // Map of move names to their types
  static std::map<std::string, std::string> moveTypeMap;

  // Initialise the move type mapping
  static void initialiseMoveTypes();

  // Ensure mapping is initialised
  static void ensureInitialised();

  static bool initialised;
};