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
    void loadTeams(const unordered_map<string, vector<string>>& selectedTeams, 
                const unordered_map<string, vector<pair<string, vector<string>>>>& selectedMoves, 
                const string& selectedTeamName) {
        int PokemonCount = 0;

        // Get the selected Pokémon names for the chosen team
        auto teamIt = selectedTeams.find(selectedTeamName);
        if (teamIt != selectedTeams.end()) {
            const vector<string>& teamPokemons = teamIt->second;

            // Loop over each Pokémon in the selected team
            for (const string& PokemonName : teamPokemons) {
                // Create a Pokémon object and load its details from JSON
                Pokemon PokeObj = Pokemon(PokemonName);

                // Check if moves for this Pokémon exist in selectedMoves
                auto movesIt = selectedMoves.find(selectedTeamName);
                if (movesIt != selectedMoves.end()) {
                    for (const auto& movePair : movesIt->second) {
                        if (movePair.first == PokemonName) {
                            for (const auto& move : movePair.second) {
                                // Load moves for this Pokémon
                                Move moveObj(move);
                                PokeObj.moves.push_back(moveObj);
                            };
                        };
                    };
                };

                // Add the Pokémon to the team
                PokemonTeam[PokemonCount] = PokeObj;
                PokemonCount++;
            };
        };
    };
};