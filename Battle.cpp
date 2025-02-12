#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iomanip> // For std::setprecision
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time() to seed the random number generator
#include "Team.cpp"

using namespace std;

class Battle {
private:
    Team playerTeam;
    Team opponentTeam;
    Pokemon selectedPokemon; // To store the current player's Pokémon
    Pokemon opponentSelectedPokemon; // To store the opponent's Pokémon

    // Helper to display Pokémon stats
    void displayHealth(const Pokemon& pokemon) {
    double healthPercentage = (static_cast<double>(pokemon.current_hp) / pokemon.hp) * 100;
    std::cout << "\n" + pokemon.name << " HP: " << healthPercentage << "%" << std::endl;
    }

public:
    Battle(const Team& playerTeam, const Team& opponentTeam)
        : playerTeam(playerTeam), opponentTeam(opponentTeam) {}

    void selectPokemon() {
        // Display player's team
        cout << "\nSelect the Pokémon you want to send out first:" << endl;
        for (int i = 0; i < playerTeam.PokemonTeam.size(); ++i) {
            cout << "[" << i + 1 << "] - " << playerTeam.PokemonTeam[i].name << endl;
        }

        // Prompt for selection
        int chosenPokemonNum;
        cout << "\nEnter the number of the Pokémon you want to send out first: ";
        cin >> chosenPokemonNum;

        // Validate selection
        if (chosenPokemonNum < 1 || chosenPokemonNum > playerTeam.PokemonTeam.size()) {
            cout << "Invalid selection - try again." << endl;
            return;
        }

        // Get selected Pokémon and store it as the class member
        selectedPokemon = playerTeam.PokemonTeam.at(chosenPokemonNum - 1); // Assign selected Pokémon
        cout << "\nYou have selected " << selectedPokemon.name << " to send out first!" << endl;
    }

    void selectOpponentPokemon() {
        // Seed the random number generator with the current time
        srand(time(0));

        // Randomly select an opponent's Pokémon
        int randomIndex = rand() % opponentTeam.PokemonTeam.size(); // Get random index
        opponentSelectedPokemon = opponentTeam.PokemonTeam.at(randomIndex); // Select opponent Pokémon
        cout << "\nThe opponent has selected " << opponentSelectedPokemon.name << " to send out!" << endl;
    }

    void executeMove(Pokemon& attacker, Pokemon& defender, Move& move) {
        cout << attacker.name << " used " << move.name << "!\n";

        if (move.damage_class == "status") {
            cout << "The move applies a status effect!\n";
            return;
        }

        // Check for accuracy
        int hitChance = rand() % 100 + 1;
        if (hitChance > move.accuracy) {
            cout << "The move missed!\n";
            return;
        }

        int damage = 0;
        if (move.damage_class == "physical") {
            damage = (attacker.attack - defender.defense) + move.power;
        } else if (move.damage_class == "special") {
            damage = (attacker.special_attack - defender.special_defense) + move.power;
        }

        damage = max(1, damage); // Ensure minimum damage is 1
        defender.current_hp = max(0, defender.current_hp - damage);

        // ---------------------------------------------------------------------------------- DELETE LATER
        cout << "It dealt " << damage << " damage!\n";
    }   

    void startBattle() {
        cout << "\n====================== BATTLE START ======================" << endl;

        // Opponent selects their pokemon
        selectOpponentPokemon();
        displayHealth(opponentSelectedPokemon);

        // Player Selects their pokemon
        selectPokemon();
        displayHealth(selectedPokemon);

        while (true) {
            // Check if all of the player’s Pokémon have fainted
            bool playerHasAlivePokemon = false;
            for (const auto& pair : playerTeam.PokemonTeam) {  // Loop through the map
                if (pair.second.current_hp > 0) {  // Check if Pokémon still has HP
                    playerHasAlivePokemon = true;
                    break;
                }
            }

            // Check if all of the opponent’s Pokémon have fainted
            bool opponentHasAlivePokemon = false;
            for (const auto& pair : opponentTeam.PokemonTeam) {  
                if (pair.second.current_hp > 0) {
                    opponentHasAlivePokemon = true;
                    break;
                }
            }

            // End battle if no Pokémon remain
            if (!playerHasAlivePokemon) {
                cout << "\nAll your Pokémon have fainted! You lost the battle.\n";
                break;
            }
            if (!opponentHasAlivePokemon) {
                cout << "\nAll opponent’s Pokémon have fainted! You won the battle.\n";
                break;
            }

            // Select a move for the player's Pokémon
            cout << "\nChoose a move:\n";
            int moveIndex = 1;
            for (const auto& move : selectedPokemon.moves) {
                cout << "    " << moveIndex++ << ". " << move.name
                    << " (Power: " << move.power << ", Accuracy: " << move.accuracy
                    << ", Class: " << move.damage_class << ")\n";
            }

            int chosenMoveIndex = 0;
            while (true) {
                cout << "\nSelect a move (1-" << selectedPokemon.moves.size() << "): ";
                cin >> chosenMoveIndex;

                // Validate the input
                if (chosenMoveIndex >= 1 && chosenMoveIndex <= selectedPokemon.moves.size()) {
                    break; // Valid choice
                } 
                cout << "Invalid choice. Please select a valid move.\n";
            }

            // Select opponent's move randomly
            srand(time(0));
            int opponentMoveIndex = rand() % opponentSelectedPokemon.moves.size();
            
            Move playerMove = selectedPokemon.moves[chosenMoveIndex - 1];
            Move opponentMove = opponentSelectedPokemon.moves[opponentMoveIndex];

            // Determine attack order based on priority, then speed
            bool playerFirst;
            if (playerMove.priority > opponentMove.priority) {
                playerFirst = true;
            } else if (playerMove.priority < opponentMove.priority) {
                playerFirst = false;
            } else if (selectedPokemon.speed > opponentSelectedPokemon.speed) {
                playerFirst = true;
            } else if (selectedPokemon.speed < opponentSelectedPokemon.speed) {
                playerFirst = false;
            } else {
                playerFirst = rand() % 2; // Randomize if speeds are equal
            }

            // Execute moves in the determined order
            if (playerFirst) {
                executeMove(selectedPokemon, opponentSelectedPokemon, playerMove);
                if (opponentSelectedPokemon.current_hp > 0) {
                    executeMove(opponentSelectedPokemon, selectedPokemon, opponentMove);
                }
            } else {
                executeMove(opponentSelectedPokemon, selectedPokemon, opponentMove);
                if (selectedPokemon.current_hp > 0) {
                    executeMove(selectedPokemon, opponentSelectedPokemon, playerMove);
                }
            }

        //     // Update Pokémon status
        //     if (selectedPokemon.current_hp <= 0) {
        //         selectedPokemon.feinted = true;
        //         cout << "\n" << selectedPokemon.name << " has fainted!" << endl;
        //         displayHealth(selectedPokemon);

        //         // Select a new Pokémon if available
        //         bool foundNewPokemon = false;
        //         for (auto& pair : playerTeam.PokemonTeam) {
        //             if (pair.second.current_hp > 0) {  // Find first non-fainted Pokémon
        //                 selectedPokemon = pair.second;
        //                 cout << "\nYou send out " << selectedPokemon.name << "!\n";
        //                 foundNewPokemon = true;
        //                 break;
        //             }
        //         }


        //         if (!foundNewPokemon) {
        //             cout << "\nAll your Pokémon have fainted! You lost the battle.\n";
        //             return;  // End the battle
        //         }
        //     }

        //     if (opponentSelectedPokemon.current_hp <= 0) {
        //         cout << "\nOpponent's " << opponentSelectedPokemon.name << " has fainted!" << endl;

        //         int remainingPokemon = 0;

        //         // Collect available Pokémon indices
        //         vector<int> availableIndices;
        //         for (const auto& pair : opponentTeam.PokemonTeam) {
        //             if (pair.second.current_hp > 0) {
        //                 availableIndices.push_back(pair.first);
        //                 remainingPokemon++;
        //             }
        //         }

        //         if (remainingPokemon == 0) {
        //             cout << "\nAll opponent’s Pokémon have fainted! You won the battle!\n";
        //             return;  // End the battle
        //         }

        //         // Show remaining Pokémon count
        //         cout << "Opponent has " << remainingPokemon << " Pokémon left.\n";

        //         // Seed random number generator to avoid the same selection
        //         int randomIndex = rand() % availableIndices.size();
        //         opponentSelectedPokemon = opponentTeam.PokemonTeam[availableIndices[randomIndex]];

        //         cout << "\nOpponent sends out " << opponentSelectedPokemon.name << "!\n";
        //     }

        //     // Display updated health
        //     displayHealth(selectedPokemon);
        //     displayHealth(opponentSelectedPokemon);

        }
    

        cout << "\nBattle logic can be implemented here." << endl;

        cout << "\n====================== BATTLE END ======================" << endl;
    }
};