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
            std :: cout << "stored accuracy" << std::endl;
        } else {
            accuracy = move_json["accuracy"];
            std :: cout << "stored accuracy" << std::endl;
        }
        if (move_json["effect_chance"].is_null()) {
            effect_chance = -1;
            std::cout << "stored effect chance" << std::endl;
        } else {
            effect_chance = move_json["effect_chance"];
            std::cout << "stored effect chance" << std::endl;
        }
        pp = move_json.value("pp", 0);
        priority = move_json.value("priority", 0);
        if (move_json["power"].is_null()) {
            power = -1;
            std::cout << "stored power" << std::endl;
        } else {
            power = move_json["power"];
            std::cout << "stored power" << std::endl;
        }
        
        // Damage class (extract only the name field)
        const auto& damage_class_data = move_json["damage_class"];
        damage_class = damage_class_data.value ("name", "");
       
        // Info section (ailment and category)
        const auto& info = move_json["Info"];
        ailment_chance = info.value("ailment_chance", 0);
        category = info["category"].value("name", "" );
        crit_rate = info.value("crit_rate`", 0);
        drain = info.value("drain", 0);
        flinch_chance = info.value("flinch_chance", 0);
        healing = info.value("healing", 0);

        // NULL CHECKS FOR HITS AND TURNS
        if (move_json["max_hits"].is_null()) {
            max_hits = 1;
            std::cout << "here" << std::endl;
        } else {
            max_hits = info["max_hits"]; // NULL - RELY ON ISNULL CHECK
        }

        if (move_json["max_turns"].is_null()) {
            max_turns = 1;
            std::cout << "here2" << std::endl;
        } else {
            max_turns = info["max_turns"]; // NULL - RELY ON ISNULL CHECK
        }

        if (move_json["min_hits"].is_null()) {
            min_hits = 1;
            std::cout << "here3" << std::endl;
        } else {
            min_hits = info["min_hits"]; // NULL - RELY ON ISNULL CHECK
        }


        if (move_json["min_turns"].is_null()) {
            min_turns = 1;
            std::cout << "here4" << std::endl;
        } else {
            min_turns = info["min_turns"]; // NULL - RELY ON ISNULL CHECK
        }

        stat_chance = info.value("stat_chance", 0);

    }
};