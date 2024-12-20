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
        for (const auto& [index, pokemon] : playerTeam.PokemonTeam) {
            cout << "[" << index + 1 << "] - " << pokemon.name << endl;
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

        // Get selected Pokémon
        const Pokemon& selectedPokemon = playerTeam.PokemonTeam.at(chosenPokemonNum - 1);
        cout << "\nYou have selected " << selectedPokemon.name << " to send out first!" << endl;

        // Display stats
        displayHealth(selectedPokemon);
    }

    void startBattle() {
        cout << "\n====================== BATTLE START ======================" << endl;

        // For now, only selecting the first Pokémon
        selectPokemon();

        // show moves
        // for (const auto& move : pokemon.moves) {
        //     std::cout << "    * " << move.name << " (Power: " << move.power << ", Accuracy: " << move.accuracy 
        //             << ", Class: " << move.damage_class << ")\n";
        // }

        cout << "\nBattle logic can be implemented here." << endl;

        cout << "\n====================== BATTLE END ======================" << endl;
    }
};