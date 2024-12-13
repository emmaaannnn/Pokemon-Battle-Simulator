#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <string> 

#include "json.hpp"
#include "Pokemon.cpp" 

using json = nlohmann::json;
using namespace std;

class Team {
public:
    std::unordered_map <int, Pokemon> PokemonTeam;
    // Loading BOTH opponent and player team
    void loadTeams(const vector<string> selectedTeam) {
        int PokemonCount = 0;
        for (const string& PokemonName : selectedTeam){
            Pokemon PokeObj = Pokemon(PokemonName); 
            PokeObj.loadMoves();
            PokemonTeam[PokemonCount] = PokeObj;
            PokemonCount++;
        };
    };
};