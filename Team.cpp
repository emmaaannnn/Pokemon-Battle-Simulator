#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include "json.hpp"

#include "Pokemon.cpp"  // Assumes your Pokemon class is in a separate file
#include "Move.cpp"     // Assumes you have Move class set up with JSON parsing

using json = nlohmann::json;
using namespace std;

