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
        accuracy = move_json.value("accuracy", 0);
        effect_chance = move_json.value("effect_chance", 0);
        pp = move_json.value("pp", 0);
        priority = move_json.value("priority", 0);
        power = move_json.value("power", 0);
        
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
        max_hits = info.value("max_hits", 0);
        max_turns = info.value("max_turns", 0);
        min_hits = info.value("min_hits", 0);
        min_turns = info.value("min_turns", 0);
        stat_chance = info.value("stat_chance", 0);
    }
};