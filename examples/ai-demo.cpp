#include "battle.h"
#include "ai_strategy.h"
#include <iostream>
#include <vector>
#include <unordered_map>

int main() {
    std::cout << "🤖 Pokemon Battle Simulator - Expert AI Demonstration 🤖\n" << std::endl;
    
    // Create a simple player team
    std::vector<std::string> playerPokemon = {"pikachu", "charizard", "blastoise"};
    
    // Define moves for each Pokemon (simplified for demo)
    const auto pokemonMoves = std::unordered_map<
        std::string, std::vector<std::pair<std::string, std::vector<std::string>>>>{
        {"Demo Player Team",
         {{"pikachu", {"thunderbolt", "iron-tail", "quick-attack", "thunder-wave"}},
          {"charizard", {"flamethrower", "air-slash", "dragon-pulse", "roar"}},
          {"blastoise", {"hydro-pump", "ice-beam", "earthquake", "withdraw"}}}},
        {"Demo AI Team", 
         {{"venusaur", {"solar-beam", "sludge-bomb", "synthesis", "sleep-powder"}},
          {"alakazam", {"psychic", "shadow-ball", "calm-mind", "recover"}},
          {"machamp", {"dynamic-punch", "earthquake", "stone-edge", "bulk-up"}}}}
    };
    
    try {
        // Create teams
        Team playerTeam;
        playerTeam.loadTeams(
            {{"Demo Player Team", playerPokemon}},
            pokemonMoves,
            "Demo Player Team"
        );
        
        std::vector<std::string> aiPokemon = {"venusaur", "alakazam", "machamp"};
        Team aiTeam;
        aiTeam.loadTeams(
            {{"Demo AI Team", aiPokemon}},
            pokemonMoves, 
            "Demo AI Team"
        );
        
        std::cout << "Choose AI difficulty level:" << std::endl;
        std::cout << "1. Easy AI (Basic type effectiveness)" << std::endl;
        std::cout << "2. Medium AI (Weather & status awareness)" << std::endl; 
        std::cout << "3. Hard AI (Strategic switching & risk assessment)" << std::endl;
        std::cout << "4. Expert AI (Predictive analysis & multi-turn planning)" << std::endl;
        std::cout << "Enter choice (1-4): ";
        
        int choice;
        std::cin >> choice;
        
        AIDifficulty difficulty;
        std::string difficultyName;
        
        switch(choice) {
            case 1:
                difficulty = AIDifficulty::EASY;
                difficultyName = "Easy AI";
                break;
            case 2:
                difficulty = AIDifficulty::MEDIUM;
                difficultyName = "Medium AI";
                break;
            case 3:
                difficulty = AIDifficulty::HARD;
                difficultyName = "Hard AI";
                break;
            case 4:
                difficulty = AIDifficulty::EXPERT;
                difficultyName = "Expert AI";
                break;
            default:
                difficulty = AIDifficulty::EXPERT;
                difficultyName = "Expert AI (default)";
                break;
        }
        
        std::cout << "\n🎮 Starting battle with " << difficultyName << "!\n" << std::endl;
        
        // Create battle with selected AI difficulty
        Battle battle(playerTeam, aiTeam, difficulty);
        
        std::cout << "Battle features:" << std::endl;
        std::cout << "• Advanced AI decision-making" << std::endl;
        std::cout << "• Type effectiveness calculations" << std::endl;
        std::cout << "• Weather and status condition integration" << std::endl;
        std::cout << "• Strategic move selection" << std::endl;
        
        if (difficulty == AIDifficulty::EXPERT) {
            std::cout << "• 🧠 Predictive opponent modeling" << std::endl;
            std::cout << "• 📊 Multi-turn planning analysis" << std::endl;
            std::cout << "• 🎯 Counter-strategy recognition" << std::endl;
            std::cout << "• 💭 AI reasoning display" << std::endl;
        }
        
        std::cout << "\nPress Enter to start the battle..." << std::endl;
        std::cin.ignore();
        std::cin.get();
        
        // Start the battle
        battle.startBattle();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}