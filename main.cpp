#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <chrono>

#include "Battle.cpp"

using namespace std;

int main() {
    // Get user's name
    string userName;
    cout << "Enter your name: ";
    cin >> userName;

    // Available teams
    unordered_map<string, vector<string>> selectedTeams = {
        // Player Pokemon
        {"Team 1", {"venusaur", "pikachu", "machamp", "arcanine", "lapras", "snorlax"}},
        {"Team 2", {"charizard"}},
        {"Team 3", {"blastoise"}},

        // Opponent Team Pokemon
        {"Opponent Team 1", {"venusaur"}},
        {"Opponent Team 2", {"charizard"}},
        {"Opponent Team 3", {"blastoise"}}
    };
    
    // Select moves
    unordered_map<string, vector<pair<string, vector<string>>>> selectedMoves = {
        {"Team 1", {
            {"venusaur", {"sludge-bomb", "mega-drain", "leech-seed", "amnesia"}},
            {"pikachu", {"thunderbolt", "brick-break", "iron-tail", "reflect"}},
            {"machamp", {"superpower", "fire-blast", "earthquake", "hyper-beam"}},
            {"arcanine", {"heat-wave", "crunch", "will-o-wisp", "roar"}},
            {"lapras", {"ice-shard", "waterfall", "body-slam", "megahorn"}},
            {"snorlax", {"toxic", "protect", "rest", "body-slam"}}
            }
        },
        {"Team 2", {
            {"charizard", {"flamethrower", "hyper-beam", "air-slash", "dragon-pulse"}}
            }
        },
        {"Team 3", {
            {"blastoise", {"hydro-pump", "flash-cannon", "ice-beam", "fake-out"}}
            }
        },
        {"Opponent Team 1", {
            {"venusaur", {"sludge-bomb", "mega-drain", "leech-seed", "amnesia"}}
            }
        },
        {"Opponent Team 2", {
            {"charizard", {"flamethrower", "hyper-beam", "air-slash", "dragon-pulse"}}
            }
        },
        {"Opponent Team 3", {
            {"blastoise", {"hydro-pump", "flash-cannon", "ice-beam", "fake-out"}}
            }
        }
    };

    // Show available teams for player selection
    cout << "\nAvailable Teams:" << endl;
    cout << "[1] - Team 1 (Venusaur, Pikachu, Machamp, Arcanine, Lapras, Snorlax)" << endl;
    cout << "[2] - Team 2 (Charizard)" << endl;
    cout << "[3] - Team 3 (Blastoise)" << endl;

    // Prompt for team selection
    int chosenTeamNum;
    cout << "\n Enter the number of the team you want to select: ";
    cin >> chosenTeamNum;

    // Validate input
    if (chosenTeamNum < 1 || chosenTeamNum > 3) {
        cout << "Invalid selection - try again." << endl;
        return 1;
    }

    string chosenTeamKey = "Team " + to_string(chosenTeamNum);
    const vector<string> chosenTeam = selectedTeams[chosenTeamKey];
    
    std::cout << "" << endl;
    std::cout << "========================================================== My Team ==========================================================" << std::endl;
    std::cout << "" << endl;
    
    //init Player Team and load Pokemon & Moves
    Team PlayerTeam;
    PlayerTeam.loadTeams(selectedTeams, selectedMoves, chosenTeamKey);

   // Print out Player's team with moves
    std::cout << "Your selected team includes:\n";
    for (const auto& [index, pokemon] : PlayerTeam.PokemonTeam) {
        std::cout << "- " << pokemon.name << "\n  Moves:\n";
        for (const auto& move : pokemon.moves) {
            std::cout << "    * " << move.name << " (Power: " << move.power << ", Accuracy: " << move.accuracy 
                    << ", Class: " << move.damage_class << ")\n";
        }
    }
    std::cout << std::endl;

    // Show available opponent teams
    cout << "Available Opponents:" << endl;
    cout << "[1] - Opponent Team 1 (Venusaur)" << endl;
    cout << "[2] - Opponent Team 2 (Charizard)" << endl;
    cout << "[3] - Opponent Team 3 (Blastoise)" << endl;

    // Prompt for opponent selection
    int chosenOpponentNum;
    cout << "\n Enter the number of your chosen opponent: ";
    cin >> chosenOpponentNum;

    // Validate input
    if (chosenOpponentNum < 1 || chosenOpponentNum > 3) {
        cout << "Invalid selection - try again." << endl;
        return 1;
    }

    string chosenOpponentKey = "Opponent Team " + to_string(chosenOpponentNum);
    const vector<string> chosenOpponent = selectedTeams[chosenOpponentKey];
    cout << "\nYou have selected " << chosenOpponentKey << " with the Pokémon: ";
    for (const auto& pokemon : chosenOpponent) {
        cout << pokemon << " ";
    }
    cout << "\n\n";

    std::cout << "" << endl;
    std::cout << "========================================================== Oppenent Team ==========================================================" << std::endl;
    std::cout << "" << endl;
    
    //init Opp Team and load Pokemon & Moves
    Team OppTeam;
    OppTeam.loadTeams(selectedTeams, selectedMoves, chosenOpponentKey);


    // Print out Opponent's team with moves
    std::cout << "Opponent's selected team includes:\n";
    for (const auto& [index, pokemon] : OppTeam.PokemonTeam) {
        std::cout << "- " << pokemon.name << "\n";
    }
    std::cout << std::endl;

    // BATTLE PART
    Battle battle(PlayerTeam, OppTeam);
    battle.startBattle();

    // GAMESTATE
    int gameState = 0;
    // Defeated the opponent
    if (gameState == 1) {
        cout << "YOU WIN!" << endl;
    }

    // Lost to the opponent
    if (gameState == 2) {
        cout << "YOU LOSE!" << endl;
    }
}