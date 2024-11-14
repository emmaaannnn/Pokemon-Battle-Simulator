#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include "json.hpp"

#include "Pokemon.cpp"  // Assumes your Pokemon class is in a separate file

using json = nlohmann::json;
using namespace std;

class Team {
public:
    public:
    std::vector<Pokemon> playerTeam1;
    std::vector<Pokemon> playerTeam2;
    std::vector<Pokemon> playerTeam3;
    std::vector<Pokemon> opponentTeam1;
    std::vector<Pokemon> opponentTeam2;
    std::vector<Pokemon> opponentTeam3;

    // Load Player team
    void loadTeams(const std::unordered_map<std::string, std::vector<std::string>>& selectedTeams,
                   const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>>& selectedMoves) {
        loadPlayerTeam(selectedTeams.at("Team 1"), selectedMoves.at("Team 1"), playerTeam1);
        loadPlayerTeam(selectedTeams.at("Team 2"), selectedMoves.at("Team 2"), playerTeam2);
        loadPlayerTeam(selectedTeams.at("Team 3"), selectedMoves.at("Team 3"), playerTeam3);
        
         // Load Opponent team
        loadOpponentTeam(selectedTeams.at("Opponent Team 1"), selectedMoves.at("Opponent Team 1"), opponentTeam1);
        loadOpponentTeam(selectedTeams.at("Opponent Team 2"), selectedMoves.at("Opponent Team 2"), opponentTeam2);
        loadOpponentTeam(selectedTeams.at("Opponent Team 3"), selectedMoves.at("Opponent Team 3"), opponentTeam3);
    }

    private:
    // Loading Player Team (reading Pokemon.json file)
    void loadPlayerTeam(const std::vector<std::string>& selectedPokemons,
                        const std::unordered_map<std::string, std::vector<std::string>>& selectedMoves,
                        std::vector<Pokemon>& team) {
        for (const auto& pokemonName : selectedPokemons) {
            // Load Pokémon from the json folder location
            std::string filePath = "pokemon2/" + pokemonName + ".json";  
            std::ifstream file(filePath);
            if (file.is_open()) {
                json pokemonJson;
                file >> pokemonJson;
                file.close();

                // Create Pokémon object from JSON data
                Pokemon newPokemon(pokemonJson);

                // Add custom moves for the Pokémon
                if (selectedMoves.find(pokemonName) != selectedMoves.end()) {
                    loadMoves(newPokemon, selectedMoves.at(pokemonName));
                }

                // Add the loaded Pokémon to the team
                team.push_back(newPokemon);
            } else {
                std::cerr << "Error opening Pokémon file: " << filePath << std::endl;
            }
        }
    }

    // Loading Opponent Team (reading Pokemon.json file)
    void loadOpponentTeam(const std::vector<std::string>& selectedOpponent,
                          const std::unordered_map<std::string, std::vector<std::string>>& selectedOpponentMoves,
                          std::vector<Pokemon>& opponentTeam) {
        for (const auto& pokemonName : selectedOpponent) {
            // Load Pokémon from the json folder location
            std::string filePath = "pokemon2/" + pokemonName + ".json";  // Adjust path to pokemon2 folder
            std::ifstream file(filePath);
            if (file.is_open()) {
                json pokemonJson;
                file >> pokemonJson;
                file.close();

                // Create Pokémon object from JSON data
                Pokemon newPokemon(pokemonJson);

                // Add custom moves for the opponent Pokémon
                if (selectedOpponentMoves.find(pokemonName) != selectedOpponentMoves.end()) {
                    loadMoves(newPokemon, selectedOpponentMoves.at(pokemonName));
                }

                // Add the opponent Pokémon to the team
                opponentTeam.push_back(newPokemon);
            } else {
                std::cerr << "Error opening opponent Pokémon file: " << filePath << std::endl;
            }
        }
    }

    // Load moves into a Pokémon (reading Move.json file)
    void loadMoves(Pokemon& pokemon, const std::vector<std::string>& moveNames) {
        for (const auto& moveName : moveNames) {
            std::string filePath = "Moves/" + moveName + ".json";
            std::ifstream file(filePath);
            if (file.is_open()) {
                json moveJson;
                file >> moveJson;
                file.close();

                // Create a Move object from JSON data and add it to the Pokémon's moves
                Move move(moveJson);
                pokemon.moves.push_back(move);
            } else {
                std::cerr << "Error opening move file: " << filePath << std::endl;
            }
        }
    }
};