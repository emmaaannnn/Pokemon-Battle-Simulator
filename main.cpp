#include <iostream>
#include <fstream>
#include "json.hpp"


using json = nlohmann::json;
#include <thread>  
#include <chrono>  
#include <vector>
#include <format> 
#include <cstdio>
#include <fstream> 
#include <iostream> 

#include "Pokemon.cpp"

#include <map>
#include <unordered_map>

using namespace std; 


std::unordered_map<string, vector<string>> selectTeamAndOpponent() {
    unordered_map<string, vector<string>> selection;
    string userName;
    cout << "Enter your name: ";
    cin >> userName;

    // Print available teams
    unordered_map<int, vector<string>> Available_teams;

    Available_teams[1] = {"Venusaur"};
    Available_teams[2] = {"Charizard"};
    Available_teams[3] = {"Blastoise"};

    // Available_teams[1] = {"Clefable", "Gengar", "Kangaskhan", "Victreebel", "Ninetales", "Blastoise"};
    // Available_teams[2] = {"Pikachu", "Machamp", "Arcanine", "Lapras", "Snorlax", "Venusaur"};
    // Available_teams[3] = {"Tauros", "Gyarados", "Aerodactyl", "Alakazam", "Exeggutor", "Charizard"};
    cout << "Available teams:" << endl;
    cout << "[1] - Venusaur" << endl;
    cout << "[2] - Charizard" << endl;
    cout << "[3] - Blastoise" << endl;
    // cout << "[1] - Clefable, Gengar, Kangaskhan, Victreebel, Ninetales, Blastoise" << endl;
    // cout << "[2] - Pikachu, Machamp, Arcanine, Lapras, Snorlax, Venusaur" << endl;
    // cout << "[3] - Tauros, Gyarados, Aerodactyl, Alakazam, Exeggutor, Charizard" << endl;
    cout << "" << endl;


    // Prompt user to select team
    int chosenTeamNum;
    cout << "Enter the number of the team you want to select: ";
    cin >> chosenTeamNum;
    if(chosenTeamNum > 3 || chosenTeamNum < 1){
        cout << "invalid selection - try again.";
        //Need to make another function to re-call the selection menu
    };
    vector<string> chosenTeam = Available_teams[chosenTeamNum];

    cout << "\nYou have selected Team " << chosenTeamNum << ".\n" << endl;
    for (const auto& pokemon : Available_teams[chosenTeamNum- 1]) {
        std::cout << "- " << pokemon << std::endl;
    }
    cout << "\n\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Prompt user to select opponent
    unordered_map<int,vector<string>> opponent;
    opponent[1] = {"Venasaur"};
    opponent[1] = {"Charizard"};
    opponent[1] = {"Blastoise"};
    // opponent[1] = {"Gym Leader Brock"};
    // opponent[2] = {"Gym Leader Misty"};
    // opponent[3] = {"Gym Leader Lt. Surge"};
    // opponent[4] = {"Gym Leader Erika"};
    // opponent[5] = {"Gym Leader Koga"};
    // opponent[6] = {"Gym Leader Sabrina"};
    // opponent[7] = {"Gym Leader Blaine"};
    // opponent[8] = {"Gym Leader Giovanni"};

    cout << "Select your opponent:" << endl;
    cout << "[1] - Venasaur" << endl;
    cout << "[2] - Charizard" << endl;
    cout << "[3] - Blastoise" << endl;
    // cout << "[1] - Gym Leader Brock" << endl;
    // cout << "[2] - Gym Leader Misty" << endl;
    // cout << "[3] - Gym Leader Lt. Surge" << endl;
    // cout << "[4] - Gym Leader Erika" << endl;
    // cout << "[5] - Gym Leader Koga" << endl;
    // cout << "[6] - Gym Leader Sabrina" << endl;
    // cout << "[7] - Gym Leader Blaine" << endl;
    // cout << "[8] - Gym Leader Giovanni" << endl;

    int chosenOpponentNum;
    cout << "Enter your chosen opponent: ";
    cin >> chosenOpponentNum;
    if(chosenOpponentNum > 8 || chosenOpponentNum < 1){
        cout << "invalid selection - try again.";
    };

    vector<string> chosenOpponent = opponent[chosenOpponentNum];
    cout << "\nYou have selected Opponent: " << chosenOpponentNum << ".\n" << endl;
    selection["Team"] = chosenTeam;
    selection["opp"] = chosenOpponent;
    cout << selection["opp"][0];
    cout << "\n\n\n";
    return selection;
}

// map<string, vector<string>> pokemonMoveMapping = {
//         {"Clefable", {"Sing", "Moonblast", "Metronome", "Cosmic Power"}}
// };
int main(){
    std::unordered_map<string, vector<string>> selections = selectTeamAndOpponent();
    vector<string> team = selections["Team"];
    vector<string> Opponent = selections["opp"];

    //  for (const auto& pokemon : team) {
    std::string filePath = ("Pokemon/" + team[0] + ".json");
    std::ifstream file(filePath);
    json jsonFile;
    file >> jsonFile;
    file.close();
    std::string name = jsonFile["name"];
    int id = jsonFile["id"];
    vector<string> types;
    for (const auto& typeInfo : jsonFile["types"]) {
        std::string typeName = typeInfo["type"]["name"];
        types.insert(types.end(),typeName);
    };
    //This needs to be of Move Type not string, but the Move class isn't ready yet
    // vector<string> move; 
    // Pokemon activeFriendlyPokemon(name, id, types, NULL);
    // cout << activeFriendlyPokemon.Name + "\n";
    // cout << activeFriendlyPokemon.id;
    // cout << "\n";
    // cout << activeFriendlyPokemon.Type[0];
    // cout << activeFriendlyPokemon.Type[1];



    //----------------------------------------------------------------------------------------
    // trying to use Team.cpp so we can set up pokemon moves but it isnt working

    // string userName;
    // cout << "Enter your name: ";
    // cin >> userName;

    // unordered_map<string, vector<string>> selectedTeams = {
    //     {"Team 1", {"venusaur"}},
    //     {"Team 2", {"charizard"}},
    //     {"Team 3", {"blastoise"}},
    //     {"opp 1", {"venusaur"}},
    //     {"opp 2", {"charizard"}},
    //     {"opp 3", {"blastoise"}},
    // };

    // unordered_map<string, unordered_map<string, vector<string>>> selectedMoves = {
    //     {"Team 1", {
    //         {"Venusaur", {"sludge-bomb", "mega-drain", "leech-seed", "amnesia"}},
    //     }},
    //     {"Team 2", {
    //         {"Charizard", {"flamethrower", "hyper-beam", "air-slash", "dragon-pulse"}},
    //     }},
    //     {"Team 3", {
    //         {"Blastoise", {"hydro-pump", "flash-cannon", "ice-beam", "fake-out"}},
    //     }},
    //     {"opp 1", {{"Venusaur", {"sludge-bomb", "mega-drain", "leech-seed", "amnesia"}},
    //     }},
    //     {"opp 2", {
    //         {"Charizard", {"flamethrower", "hyper-beam", "air-slash", "dragon-pulse"}},
    //     }},
    //     {"opp 3", {
    //         {"Blastoise", {"hydro-pump", "flash-cannon", "ice-beam", "fake-out"}},
    //     }}
    // };

    // // Create team and load data
    // Team team;
    // team.loadTeams(selectedTeams, selectedMoves);


    return 0;
};


