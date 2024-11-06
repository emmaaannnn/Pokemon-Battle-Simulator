#include <iostream>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

#include <cstdio>
#include <fstream> 
#include <iostream> 
#include "Pokemon.cpp"
#include <map>

using namespace std; 


void selectTeamAndOpponent() {

    string userName;
    cout << "Enter your name: ";
    cin >> userName;

    // Print available teams
    cout << "Available teams:" << endl;
    cout << "[1] - Clefable, Gengar, Kangaskhan, Victreebel, Ninetales, Blastoise" << endl;
    cout << "[2] - Pikachu, Machamp, Arcanine, Lapras, Snorlax, Venusaur" << endl;
    cout << "[3] - Tauros, Gyarados, Aerodactyl, Alakazam, Exeggutor, Charizard" << endl;
    cout << "" << endl;

    // Prompt user to select team
    int chosenTeam;
    cout << "Enter the number of the team you want to select: ";
    cin >> chosenTeam;

    cout << "\nYou have selected Team " << chosenTeam << ".\n" << endl;

    // Prompt user to select opponent
    cout << "Select your opponent:" << endl;
    cout << "[1] - Gym Leader Brock" << endl;
    cout << "[2] - Gym Leader Misty" << endl;
    cout << "[3] - Gym Leader Lt. Surge" << endl;
    cout << "[4] - Gym Leader Erika" << endl;
    cout << "[5] - Gym Leader Koga" << endl;
    cout << "[6] - Gym Leader Sabrina" << endl;
    cout << "[7] - Gym Leader Blaine" << endl;
    cout << "[8] - Gym Leader Giovanni" << endl;

    int chosenOpponent;
    cout << "Enter your chosen opponent: ";
    cin >> chosenOpponent;

    cout << "\nYou have selected: " << chosenOpponent << ".\n" << endl;
}

map<string, vector<string>> pokemonMoveMapping = {
        {"Clefable", {"Sing", "Moonblast", "Metronome", "Cosmic Power"}}
};
int main(){
    std::string filePath = "Pokemon/1.json";
    std::ifstream file(filePath);
    json jsonData;
    file >> jsonData;
    file.close();
    std::string name = jsonData["name"];
    cout << name;
    for (const auto& type : jsonData["types"]){
        std::cout << type.get<std::string>();
    };
    selectTeamAndOpponent();
  
  
  
string pokemonName = "Clefable";
    cout << pokemonName << " can use the following moves:" << endl;
    for (const auto& move : pokemonMoveMapping[pokemonName]) {
        cout << "- " << move << endl;
    }
    return 0;
};

