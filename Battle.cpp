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

    void startBattle() {
        cout << "\n====================== BATTLE START ======================" << endl;

        // Opponent selects their pokemon
        selectOpponentPokemon();
        displayHealth(opponentSelectedPokemon);

        // For now, only selecting the first Pokémon
        selectPokemon();

        // Player Pokemon
        displayHealth(selectedPokemon);
        //show moves
        int moveIndex = 1; // To display move options starting from 1
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
            } else {
                cout << "Invalid choice. Please select a valid move.\n";
            }
        }


        cout << "\nBattle logic can be implemented here." << endl;

        cout << "\n====================== BATTLE END ======================" << endl;
    }
};