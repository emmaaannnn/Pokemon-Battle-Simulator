#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include "json.hpp"

#include "Pokemon.cpp"  // Assumes your Pokemon class is in a separate file
#include "Move.cpp"     // Assumes you have Move class set up with JSON parsing

using json = nlohmann::json;
using namespace std;
class Team {
public:
    public:
    std::vector<Pokemon> playerTeam1;
    std::vector<Pokemon> playerTeam2;
    std::vector<Pokemon> playerTeam3;
    std::vector<Pokemon> opponentTeam;

    // Method to load the player's teams and opponent's team
    void loadTeams(const std::unordered_map<std::string, std::vector<std::string>>& selectedTeams,
                   const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>>& selectedMoves) {
        loadPlayerTeam(selectedTeams.at("Team 1"), selectedMoves.at("Team 1"), playerTeam1);
        loadPlayerTeam(selectedTeams.at("Team 2"), selectedMoves.at("Team 2"), playerTeam2);
        loadPlayerTeam(selectedTeams.at("Team 3"), selectedMoves.at("Team 3"), playerTeam3);
        loadOpponentTeam(selectedTeams.at("opp"), selectedMoves.at("opp"));
    }

    private:
    // General function to load a team (player or opponent) with custom moves
    void loadPlayerTeam(const std::vector<std::string>& selectedPokemons, 
                        const std::unordered_map<std::string, std::vector<std::string>>& selectedMoves,
                        std::vector<Pokemon>& team) {
        for (const auto& pokemonName : selectedPokemons) {
            // Load Pokémon from JSON
            std::string filePath = "Pokemon2/" + pokemonName + ".json";
            Pokemon newPokemon(filePath);  // Assuming the Pokemon constructor loads data from file

            // Add custom moves for the Pokémon
            if (selectedMoves.find(pokemonName) != selectedMoves.end()) {
                loadMoves(newPokemon, selectedMoves.at(pokemonName));
            }

            // Add the loaded Pokémon to the team
            team.push_back(newPokemon);
        }
    }

    // Load the opponent team with custom moves
    void loadOpponentTeam(const std::vector<std::string>& selectedOpponent,
                          const std::unordered_map<std::string, std::vector<std::string>>& selectedOpponentMoves) {
        for (const auto& pokemonName : selectedOpponent) {
            // Load opponent's Pokémon from JSON
            std::string filePath = "Pokemon2/" + pokemonName + ".json";
            Pokemon newPokemon(filePath);

            // Add custom moves for the opponent Pokémon
            if (selectedOpponentMoves.find(pokemonName) != selectedOpponentMoves.end()) {
                loadMoves(newPokemon, selectedOpponentMoves.at(pokemonName));
            }

            // Add the opponent Pokémon to the team
            opponentTeam.push_back(newPokemon);
        }
    }
};