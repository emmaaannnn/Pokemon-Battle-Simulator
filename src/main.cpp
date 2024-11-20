#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <chrono>

#include "Team.cpp"  

using namespace std;

int main() {
    // Get user's name
    string userName;
    cout << "Enter your name: ";
    cin >> userName;

    // Available teams
    unordered_map<string, vector<string>> selectedTeams = {
        // Player Pokemon
        {"Team 1", {"venusaur", "charmander", "geodude", "pikachu", "pidgey"}},
        {"Team 2", {"charizard"}},
        {"Team 3", {"blastoise"}},

        // Opponent Team Pokemon
        {"Opponent Team 1", {"venusaur"}},
        {"Opponent Team 2", {"charizard", "oddish", "kangaskhan", "jigglypuff", "dratini"}},
        {"Opponent Team 3", {"blastoise"}}
    };
    
    // Show available teams for player selection
    cout << "\nAvailable Teams:" << endl;
    cout << "[1] - Team 1 (Venusaur)" << endl;
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
    cout << "\nYou have selected " << chosenTeamKey << " with the Pokémon: ";
    for (const auto& pokemon : chosenTeam) {
        cout << pokemon << " ";
    }
    cout << "\n\n";

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
    std::cout << "========================================================== My Team ==========================================================" << std::endl;
    std::cout << "" << endl;
    //init Player Team and load Pokemon & Moves
    Team PlayerTeam;
    PlayerTeam.loadTeams(chosenTeam);

    std::cout << "" << endl;
    std::cout << "========================================================== Oppenent Team ==========================================================" << std::endl;
    std::cout << "" << endl;
    //init Opp Team and load Pokemon & Moves
    Team OppTeam;
    OppTeam.loadTeams(chosenOpponent);

   // Print out selected player's team
    cout << "\nYour selected team includes: ";
    for (const auto& pokemon : chosenTeam) {
        cout << pokemon << " ";
    }
    cout << endl;

    // Print out selected opponent's team
    cout << "\nOpponent's selected team includes: ";
    for (const auto& pokemon : chosenOpponent) {
        cout << pokemon << " ";
    }
    cout << endl;














    int gameState = 0;
    // Pokemon battle
    if (gameState == 0) {

    }

    // Defeated the opponent
    if (gameState == 1) {
        cout << "YOU WIN!" << endl;
    }

    // Lost to the opponent
    if (gameState == 2) {
        cout << "YOU LOSE!" << endl;
    }
}