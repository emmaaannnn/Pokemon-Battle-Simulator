#include <string>
#include <vector>

#include "json.hpp" 

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

    Move(const json& move) {
        // Basic move attributes
        name = move["name"];
        accuracy = move["accuracy"];
        effect_chance = move["effect_chance"];
        pp = move["pp"];
        priority = move["priority"];
        power = move["power"];
        
        // Damage class (extract only the name field)
        damage_class = move["damage_class"]["name"];

        // Info section (ailment and category)
        ailment_chance = move["Info"]["ailment_chance"];
        category = move["Info"]["category"]["name"];
        crit_rate = move["Info"]["crit_rate"];
        drain = move["Info"]["drain"];
        flinch_chance = move["Info"]["flinch_chance"];
        healing = move["Info"]["healing"];
        max_hits = move["Info"]["max_hits"];
        max_turns = move["Info"]["max_turns"];
        min_hits = move["Info"]["min_hits"];
        min_turns = move["Info"]["min_turns"];
        stat_chance = move["Info"]["stat_chance"];
    }
};