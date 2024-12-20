#include <string>
#include <vector>

#include "json.hpp" 

using json = nlohmann::json;
using namespace std;
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


    Move(const std::string& moveName) {
        loadFromJson("Moves_Data/" + moveName + ".json");
    }

    void loadFromJson(const std::string& file_path) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << file_path << std::endl;
            return;
        }
        json move_json;
        file >> move_json;


        std::string moveName = move_json["name"];
        // Basic move attributes
        name = move_json["name"];

        if (move_json["accuracy"].is_null()) {
            accuracy = -1;
        } else {
            accuracy = move_json["accuracy"];
        }
        

        if (move_json["effect_chance"].is_null()) {
            effect_chance = -1;
        } else {
            effect_chance = move_json["effect_chance"];
        }
        
        pp = move_json.value("pp", 0);
        priority = move_json.value("priority", 0);

        if (move_json["power"].is_null()) {
            power = -1;
        } else {
            power = move_json["power"];
        }

        damage_class = move_json["damage_class"]["name"];
        ailment_chance = move_json["Info"]["ailment_chance"];
        category = move_json["Info"]["category"]["name"];
        crit_rate = move_json["Info"]["crit_rate"];
        drain = move_json["Info"]["drain"];
        flinch_chance = move_json["Info"]["flinch_chance"];
        healing = move_json["Info"]["healing"];

        // NULL CHECKS FOR HITS AND TURNS
        if (move_json["Info"]["max_hits"].is_null()) {
            max_hits = 1;
        } else {
            max_hits = move_json["Info"]["max_hits"]; // NULL - RELY ON ISNULL CHECK
        }

        if (move_json["Info"]["max_turns"].is_null()) {
            max_turns = 1;
        } else {
            max_turns = move_json["Info"]["max_turns"]; // NULL - RELY ON ISNULL CHECK
        }
        
        if (move_json["Info"]["min_hits"].is_null()) {
            min_hits = 1;
        } else {
            min_hits = move_json["Info"]["min_hits"]; // NULL - RELY ON ISNULL CHECK
        }
    
        if (move_json["Info"]["min_turns"].is_null()) {
            min_turns = 1;
        } else {
            min_turns = move_json["Info"]["min_turns"]; // NULL - RELY ON ISNULL CHECK
        }


        stat_chance = move_json["Info"]["stat_chance"];
    };
};