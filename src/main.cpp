#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <chrono>

#include "Battle.cpp"
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
        {"Team 1", {"venusaur"}},
        {"Team 2", {"charizard"}},
        {"Team 3", {"blastoise"}},

        // Opponent Team Pokemon
        {"Opponent Team 1", {"venusaur"}},
        {"Opponent Team 2", {"charizard"}},
        {"Opponent Team 3", {"blastoise"}}
    };
    
    // Select moves
    unordered_map<string, vector<pair<string, vector<string>>>> selectedMoves = {
        {"Team 1", {{"venusaur", {"sludge-bomb", "mega-drain", "leech-seed", "amnesia"}}}},
        {"Team 2", {{"charizard", {"flamethrower", "air-slash", "dragon-pulse", "hyper-beam"}}}},
        {"Team 3", {{"blastoise", {"hydro-pump", "ice-beam", "surf", "protect"}}}},
        {"Opponent Team 1", {{"venusaur", {"solar-beam", "growth", "razor-leaf", "toxic"}}}},
        {"Opponent Team 2", {{"charizard", {"heat-wave", "fly", "flare-blitz", "roost"}}}},
        {"Opponent Team 3", {{"blastoise", {"waterfall", "iron-defense", "aqua-jet", "scald"}}}}
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
    PlayerTeam.loadTeams(selectedTeams, selectedMoves, chosenTeamKey);

    std::cout << "" << endl;
    std::cout << "========================================================== Oppenent Team ==========================================================" << std::endl;
    std::cout << "" << endl;
    
    //init Opp Team and load Pokemon & Moves
    Team OppTeam;
    OppTeam.loadTeams(selectedTeams, selectedMoves, chosenOpponentKey);

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


    // Battle battle(PlayerTeam, OppTeam);
    // battle.initiateBattle();







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