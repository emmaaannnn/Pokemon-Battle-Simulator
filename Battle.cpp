#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iomanip> // For std::setprecision
#include "Team.cpp"

using namespace std;

class Battle {
private:
    Team playerTeam;
    Team opponentTeam;
    Pokemon selectedPokemon; // To store the current player's Pokémon

    // Helper to display Pokémon stats
    void displayHealth(const Pokemon& pokemon) {
    double healthPercentage = (static_cast<double>(pokemon.current_hp) / pokemon.hp) * 100;
    std::cout << pokemon.name << " HP: " << healthPercentage << "%" << std::endl;
    }

public:
    Battle(const Team& playerTeam, const Team& opponentTeam)
        : playerTeam(playerTeam), opponentTeam(opponentTeam) {}

    void selectPokemon() {
        // Display player's team
        cout << "Select the Pokémon you want to send out first:" << endl;
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

    void startBattle() {
        cout << "\n====================== BATTLE START ======================" << endl;

        // For now, only selecting the first Pokémon
        selectPokemon();

        // Opponent selects their pokemon

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