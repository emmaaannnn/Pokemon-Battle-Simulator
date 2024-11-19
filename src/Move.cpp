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


    Move(const std::string& moveName) {
        loadFromJson("Moves/" + moveName + ".json");
    }

    void loadFromJson(const std::string& file_path) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << file_path << std::endl;
            return;
        }

        json move_json;
        file >> move_json;

        // Basic move attributes
        name = move_json.value("name", "");
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
        
        // Damage class (extract only the name field)
        const auto& damage_class_data = move_json["damage_class"];
        damage_class = damage_class_data["name"];
       
        // Info section (ailment and category)
        const auto& info = move_json["Info"];
        ailment_chance = info["ailment_chance"];
        category = info["category"]["name"];
        crit_rate = info["crit_rate"];
        drain = info["drain"];
        flinch_chance = info["flinch_chance"];
        healing = info["healing"];

        // NULL CHECKS FOR HITS AND TURNS
        if (move_json["max_hits"].is_null()) {
            max_hits = 1;
        } else {
            max_hits = info["max_hits"]; // NULL - RELY ON ISNULL CHECK
        }

        if (move_json["max_turns"].is_null()) {
            max_turns = 1;
        } else {
            max_turns = info["max_turns"]; // NULL - RELY ON ISNULL CHECK
        }

        if (move_json["min_hits"].is_null()) {
            min_hits = 1;
        } else {
            min_hits = info["min_hits"]; // NULL - RELY ON ISNULL CHECK
        }


        if (move_json["min_turns"].is_null()) {
            min_turns = 1;
        } else {
            min_turns = info["min_turns"]; // NULL - RELY ON ISNULL CHECK
        }

        stat_chance = info["stat_chance"];

    }
};