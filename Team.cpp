#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>

#include "json.hpp"
#include "Pokemon.cpp" 

using json = nlohmann::json;
using namespace std;

class Team {
public:
std::vector<Pokemon> playerTeam1;
std::vector<Pokemon> playerTeam2;
std::vector<Pokemon> playerTeam3;
std::vector<Pokemon> opponentTeam1;
std::vector<Pokemon> opponentTeam2;
std::vector<Pokemon> opponentTeam3;

// Loading BOTH opponent and player team
void loadTeams(const std::unordered_map<std::string, std::vector<std::string>>& selectedTeams,
                const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>>& selectedMoves) {
    // Load Player Team
    loadPlayerTeam(selectedTeams.at("Team 1"), selectedMoves.at("Team 1"), playerTeam1);
    loadPlayerTeam(selectedTeams.at("Team 2"), selectedMoves.at("Team 2"), playerTeam2);
    loadPlayerTeam(selectedTeams.at("Team 3"), selectedMoves.at("Team 3"), playerTeam3);
        
    // above will be replaced by bottom code once we are able to read files
    // for (int i = 1; i < 4; i++) {
    //     loadOpponentTeam(selectedTeams.at("Opponent Team" + {i} ), selectedMoves.at("Opponent Team" + {i} ), opponentTeam1);
    // }

    // Load Opponent team
    loadOpponentTeam(selectedTeams.at("Opponent Team 1"), selectedMoves.at("Opponent Team 1"), opponentTeam1);
    loadOpponentTeam(selectedTeams.at("Opponent Team 2"), selectedMoves.at("Opponent Team 2"), opponentTeam2);
    loadOpponentTeam(selectedTeams.at("Opponent Team 3"), selectedMoves.at("Opponent Team 3"), opponentTeam3);

    // above will be replaced by bottom code once we are able to read files
    // for (int i = 1; i < 4; i++) {
    //     loadOpponentTeam(selectedTeams.at("Opponent Team" + {i} ), selectedMoves.at("Opponent Team" + {i} ), opponentTeam1);
    // }
    }

private:
// Loading Player Team
void loadPlayerTeam(const std::vector<std::string>& selectedPokemons,
                    const std::unordered_map<std::string, std::vector<std::string>>& selectedMoves,
                    std::vector<Pokemon>& team) {
    for (const auto& pokemonName : selectedPokemons) {
        // Create Pokémon object
        Pokemon newPokemon(pokemonName);

        // Add custom moves for the Pokémon
        if (selectedMoves.find(pokemonName) != selectedMoves.end()) {
            loadMoves(newPokemon, selectedMoves.at(pokemonName));
        }

        // Add the loaded Pokémon to the team
        team.push_back(newPokemon);
        }
    }

// Loading Opponent Team
void loadOpponentTeam(const std::vector<std::string>& selectedOpponent,
                        const std::unordered_map<std::string, std::vector<std::string>>& selectedOpponentMoves,
                        std::vector<Pokemon>& opponentTeam) {
    for (const auto& pokemonName : selectedOpponent) {
        // Create Pokémon object
        Pokemon newPokemon(pokemonName);

        // Add custom moves for the opponent Pokémon
        if (selectedOpponentMoves.find(pokemonName) != selectedOpponentMoves.end()) {
            loadMoves(newPokemon, selectedOpponentMoves.at(pokemonName));
        }

        // Add the opponent Pokémon to the team
        opponentTeam.push_back(newPokemon);
        }
    }

// Load moves into a Pokémon
void loadMoves(Pokemon& pokemon, const std::vector<std::string>& moveNames) {
    // for (const auto& moveName : moveNames) {
    //     // Create a Move object add it to the Pokémon's moves
    //     Move move(moveName);
    //     pokemon.moves.push_back(move);
    //     }
    }
};