#include <include/rapidjson/document.h>
#include <include/rapidjson/prettywriter.h>
#include <include/rapidjson/stringbuffer.h>
#include "include/rapidjson/filereadstream.h"

#include <cstdio>
#include <fstream> 
#include <iostream> 
#include "Pokemon.cpp"

using namespace std; 
using namespace rapidjson; 
int main(){
    FILE* file = fopen("1.json", "rb");
    char readBuffer[65536];
    rapidjson::FileReadStream data(file, readBuffer, 
                                 sizeof(readBuffer)); 

    rapidjson::Document doc;
    doc.ParseStream(data);
    fclose(file);
    std::cout << doc["name"].GetString() << std::endl;
    return -1;

selectTeamAndOpponent();

};

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