#include "test_utils.h"
#include <fstream>
#include <sstream>

namespace TestUtils {

Pokemon createTestPokemon(const std::string& name, int hp, int attack, int defense, 
                         int special_attack, int special_defense, int speed,
                         const std::vector<std::string>& types,
                         const std::vector<std::string>& moveNames) {
    Pokemon pokemon;
    pokemon.name = name;
    pokemon.hp = hp;
    pokemon.current_hp = hp;
    pokemon.attack = attack;
    pokemon.defense = defense;
    pokemon.special_attack = special_attack;
    pokemon.special_defense = special_defense;
    pokemon.speed = speed;
    pokemon.types = types;
    
    // Reset all stat stages
    pokemon.attack_stage = 0;
    pokemon.defense_stage = 0;
    pokemon.special_attack_stage = 0;
    pokemon.special_defense_stage = 0;
    pokemon.speed_stage = 0;
    
    // Set status to none
    pokemon.status = StatusCondition::NONE;
    pokemon.status_turns_remaining = 0;
    
    // Create test moves
    for (const auto& moveName : moveNames) {
        Move move = createTestMove(moveName);
        pokemon.moves.push_back(move);
    }
    
    return pokemon;
}

Move createTestMove(const std::string& name, int power, int accuracy, int pp,
                   const std::string& type, const std::string& damageClass,
                   StatusCondition ailment, int ailmentChance) {
    Move move;
    move.name = name;
    move.power = power;
    move.accuracy = accuracy;
    move.pp = pp;
    move.current_pp = pp;
    move.type = type;
    move.damage_class = damageClass;
    move.priority = 0;
    move.crit_rate = 0;
    move.drain = 0;
    move.healing = 0;
    move.flinch_chance = 0;
    move.stat_chance = 0;
    move.ailment_chance = ailmentChance;
    move.min_hits = 0;
    move.max_hits = 0;
    move.category = (power > 0) ? "damage" : "ailment";
    
    // Set ailment based on status condition
    switch (ailment) {
        case StatusCondition::POISON:
            move.ailment_name = "poison";
            break;
        case StatusCondition::BURN:
            move.ailment_name = "burn";
            break;
        case StatusCondition::PARALYSIS:
            move.ailment_name = "paralysis";
            break;
        case StatusCondition::SLEEP:
            move.ailment_name = "sleep";
            break;
        case StatusCondition::FREEZE:
            move.ailment_name = "freeze";
            break;
        default:
            move.ailment_name = "none";
            break;
    }
    
    return move;
}

Team createTestTeam(const std::vector<Pokemon>& pokemon) {
    Team team;
    // Create team configuration for loadTeams method
    std::unordered_map<std::string, std::vector<std::string>> teams;
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>> moves;
    
    std::vector<std::string> pokemonNames;
    std::vector<std::pair<std::string, std::vector<std::string>>> moveConfig;
    
    for (const auto& p : pokemon) {
        pokemonNames.push_back(p.name);
        
        std::vector<std::string> moveNames;
        for (const auto& move : p.moves) {
            moveNames.push_back(move.name);
        }
        
        moveConfig.push_back({p.name, moveNames});
    }
    
    teams["TestTeam"] = pokemonNames;
    moves["TestTeam"] = moveConfig;
    
    // This approach requires that the Pokemon data exists in the data files
    // For now, we'll create a simpler approach
    team.loadTeams(teams, moves, "TestTeam");
    return team;
}

void writeTestJsonFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << content;
        file.close();
    }
}

std::string createTestPokemonJson(const std::string& name, int hp, int attack, int defense,
                                 int special_attack, int special_defense, int speed,
                                 const std::vector<std::string>& types) {
    std::stringstream ss;
    ss << R"({
    "name": ")" << name << R"(",
    "types": [)";
    
    for (size_t i = 0; i < types.size(); ++i) {
        ss << "\"" << types[i] << "\"";
        if (i < types.size() - 1) ss << ", ";
    }
    
    ss << R"(],
    "base_stats": {
        "hp": )" << hp << R"(,
        "attack": )" << attack << R"(,
        "defense": )" << defense << R"(,
        "special-attack": )" << special_attack << R"(,
        "special-defense": )" << special_defense << R"(,
        "speed": )" << speed << R"(
    }
})";
    
    return ss.str();
}

std::string createTestMoveJson(const std::string& name, int power, int accuracy, int pp,
                              const std::string& type, const std::string& damageClass,
                              const std::string& ailment, int ailmentChance) {
    std::stringstream ss;
    ss << R"({
    "name": ")" << name << R"(",
    "power": )" << (power > 0 ? std::to_string(power) : "null") << R"(,
    "accuracy": )" << accuracy << R"(,
    "pp": )" << pp << R"(,
    "priority": 0,
    "damage_class": {"name": ")" << damageClass << R"("},
    "Info": {
        "ailment": {"name": ")" << ailment << R"("},
        "ailment_chance": )" << ailmentChance << R"(,
        "crit_rate": 0,
        "drain": 0,
        "flinch_chance": 0,
        "healing": 0,
        "stat_chance": 0,
        "category": {"name": ")" << (power > 0 ? "damage" : "ailment") << R"("}
    }
})";
    
    return ss.str();
}

bool compareFloats(double a, double b, double epsilon) {
    return std::abs(a - b) < epsilon;
}

void setupTestEnvironment() {
    // Create test data directory if it doesn't exist
    std::system("mkdir -p test_data/pokemon");
    std::system("mkdir -p test_data/moves");
    std::system("mkdir -p test_data/teams");
    
    // Create basic test Pokemon
    writeTestJsonFile("test_data/pokemon/testmon.json", 
        createTestPokemonJson("testmon", 100, 80, 70, 90, 85, 75, {"normal"}));
    
    writeTestJsonFile("test_data/pokemon/firetest.json",
        createTestPokemonJson("firetest", 90, 85, 65, 95, 80, 85, {"fire"}));
    
    writeTestJsonFile("test_data/pokemon/watertest.json",
        createTestPokemonJson("watertest", 110, 75, 80, 85, 90, 70, {"water"}));
    
    // Create basic test moves
    writeTestJsonFile("test_data/moves/testmove.json",
        createTestMoveJson("testmove", 80, 100, 15, "normal", "physical", "none", 0));
    
    writeTestJsonFile("test_data/moves/statusmove.json",
        createTestMoveJson("statusmove", 0, 85, 10, "normal", "status", "poison", 100));
    
    writeTestJsonFile("test_data/moves/fireblast.json",
        createTestMoveJson("fireblast", 110, 85, 5, "fire", "special", "burn", 10));
}

} // namespace TestUtils