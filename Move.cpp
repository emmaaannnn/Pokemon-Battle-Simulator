#include <string>
#include <vector>
#include "json.hpp" // Include the JSON library

using json = nlohmann::json;

class Move{
    public:
        //move stats
        std::string name;
        int accuracy;
        int effect_chance;
        int pp;
        int priority;
        int power;
        
        //type of move
        std::string damage_class;
        std::string effect;
        std::string short_effect;
        
        //move effects
        int ailment_chance;
        std::string category;
        
        int crit_rate;
        int drain;
        int flinch_chance;
        int healing;
        int max_hits;
        int max_turns;
        int min_hits;
        int min_turns;
        int stat_chance;
};