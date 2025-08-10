#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>
#include <csignal>

#include "battle.h"

// Signal handler for graceful exit
void signalHandler(int signal) {
  std::cout << "\n\n🛑 Game interrupted. Thanks for playing!\n" << std::endl;
  exit(0);
}

int main() {
  // Set up signal handler for graceful interruption
  signal(SIGINT, signalHandler);
  
  // Get user's name
  std::string userName;
  std::cout << "Enter your name: ";
  std::getline(std::cin, userName);
  
  // Handle empty input gracefully
  if (userName.empty()) {
    userName = "Trainer";
  }
  
  std::cout << "\nWelcome, " << userName << "!" << std::endl;

  // Available teams - using auto to reduce verbosity
  const auto selectedTeams = std::unordered_map<std::string,
                                                std::vector<std::string>>{
      // Player Pokemon
      {"Team 1",
       {"venusaur", "pikachu", "machamp", "arcanine", "lapras", "snorlax"}},
      {"Team 2",
       {"charizard", "starmie", "snorlax", "alakazam", "rhydon", "jolteon"}},
      {"Team 3",
       {"venusaur", "zapdos", "nidoking", "gengar", "lapras", "tauros"}},

      // Opponent Team Pokemon
      {"Opponent Team 1",
       {"aerodactyl", "kabutops", "golem", "onix", "omastar", "rhyhorn"}},
      {"Opponent Team 2",
       {"starmie", "gyarados", "lapras", "golduck", "vaporeon", "seaking"}},
      {"Opponent Team 3",
       {"raichu", "magneton", "electrode", "electabuzz", "jolteon", "pikachu"}},
      {"Opponent Team 4",
       {"victreebel", "exeggutor", "parasect", "tangela", "vileplume",
        "venusaur"}},
      {"Opponent Team 5",
       {"arbok", "tentacruel", "muk", "gengar", "weezing", "venomoth"}},
      {"Opponent Team 6",
       {"alakazam", "slowbro", "mr-mime", "jynx", "hypno", "exeggutor"}},
      {"Opponent Team 7",
       {"ninetales", "arcanine", "rapidash", "magmar", "flareon", "charizard"}},
      {"Opponent Team 8",
       {"nidoking", "nidoqueen", "dugtrio", "rhydon", "marowak", "sandslash"}},
  };

  // Select moves - using auto for complex type
  const auto selectedMoves = std::unordered_map<
      std::string,
      std::vector<std::pair<std::string, std::vector<std::string>>>>{
      {"Team 1",
       {{"venusaur", {"sludge-bomb", "mega-drain", "leech-seed", "amnesia"}},
        {"pikachu", {"thunderbolt", "brick-break", "iron-tail", "reflect"}},
        {"machamp", {"superpower", "fire-blast", "earthquake", "hyper-beam"}},
        {"arcanine", {"heat-wave", "sunny-day", "will-o-wisp", "roar"}},
        {"lapras", {"ice-shard", "waterfall", "rain-dance", "megahorn"}},
        {"snorlax", {"toxic", "protect", "rest", "body-slam"}}}},
      {"Team 2",
       {{"charizard", {"flamethrower", "slash", "earthquake", "fire-spin"}},
        {"starmie", {"hydro-pump", "psychic", "ice-beam", "recover"}},
        {"snorlax", {"body-slam", "hyper-beam", "earthquake", "rest"}},
        {"alakazam", {"psychic", "recover", "thunder-wave", "reflect"}},
        {"rhydon", {"earthquake", "rock-slide", "body-slam", "substitute"}},
        {"jolteon",
         {"thunderbolt", "thunder-wave", "pin-missile", "double-kick"}}}},
      {"Team 3",
       {{"venusaur", {"razor-leaf", "sleep-powder", "body-slam", "leech-seed"}},
        {"zapdos", {"thunderbolt", "drill-peck", "thunder-wave", "agility"}},
        {"nidoking", {"earthquake", "ice-beam", "thunderbolt", "rock-slide"}},
        {"gengar", {"psychic", "night-shade", "hypnosis", "explosion"}},
        {"lapras", {"hydro-pump", "blizzard", "psychic", "body-slam"}},
        {"tauros", {"body-slam", "hyper-beam", "blizzard", "earthquake"}}}},

      // Opponent Teams
      {"Opponent Team 1",
       {
           {"aerodactyl", {"tackle", "scratch", "protect", "amnesia"}},
           {"kabutops", {"tackle", "scratch", "protect", "amnesia"}},
           {"golem", {"tackle", "scratch", "protect", "amnesia"}},
           {"onix", {"tackle", "scratch", "protect", "amnesia"}},
           {"omastar", {"tackle", "scratch", "protect", "amnesia"}},
           {"rhyhorn", {"tackle", "scratch", "protect", "amnesia"}},
       }},
      {"Opponent Team 2",
       {
           {"starmie", {"psychic", "surf", "ice-beam", "recover"}},
           {"gyarados", {"hydro-pump", "bite", "thunder", "hyper-beam"}},
           {"lapras", {"surf", "ice-beam", "psychic", "body-slam"}},
           {"golduck", {"surf", "psychic", "ice-beam", "disable"}},
           {"vaporeon", {"surf", "ice-beam", "acid-armor", "haze"}},
           {"seaking", {"surf", "ice-beam", "double-edge", "agility"}},
       }},
      {"Opponent Team 3",
       {
           {"raichu",
            {"thunderbolt", "thunder", "double-kick", "seismic-toss"}},
           {"magneton", {"thunderbolt", "thunder-wave", "sonic-boom", "swift"}},
           {"electrode",
            {"thunderbolt", "thunder", "self-destruct", "light-screen"}},
           {"electabuzz",
            {"thunderbolt", "thunder-punch", "seismic-toss", "light-screen"}},
           {"jolteon",
            {"thunderbolt", "thunder", "double-kick", "sand-attack"}},
           {"pikachu",
            {"thunderbolt", "thunder", "seismic-toss", "double-team"}},
       }},
      {"Opponent Team 4",
       {
           {"victreebel",
            {"razor-leaf", "acid", "poison-powder", "sleep-powder"}},
           {"exeggutor", {"razor-leaf", "hypnosis", "psychic", "explosion"}},
           {"parasect", {"spore", "slash", "leech-life", "stun-spore"}},
           {"tangela",
            {"vine-whip", "poison-powder", "stun-spore", "sleep-powder"}},
           {"vileplume",
            {"petal-dance", "poison-powder", "acid", "sleep-powder"}},
           {"venusaur",
            {"razor-leaf", "leech-seed", "poison-powder", "sleep-powder"}},
       }},
      {"Opponent Team 5",
       {
           {"arbok", {"bite", "poison-sting", "acid", "glare"}},
           {"tentacruel",
            {"hydro-pump", "poison-sting", "constrict", "barrier"}},
           {"muk", {"poison-gas", "minimize", "sludge", "harden"}},
           {"gengar", {"night-shade", "hypnosis", "dream-eater", "psychic"}},
           {"weezing", {"sludge", "smokescreen", "explosion", "haze"}},
           {"venomoth",
            {"psychic", "poison-powder", "stun-spore", "sleep-powder"}},
       }},
      {"Opponent Team 6",
       {
           {"alakazam", {"psychic", "recover", "reflect", "kinesis"}},
           {"slowbro", {"psychic", "surf", "amnesia", "disable"}},
           {"mr-mime", {"psychic", "barrier", "light-screen", "meditate"}},
           {"jynx", {"psychic", "ice-beam", "lovely-kiss", "body-slam"}},
           {"hypno", {"psychic", "hypnosis", "dream-eater", "poison-gas"}},
           {"exeggutor", {"psychic", "hypnosis", "leech-seed", "explosion"}},
       }},
      {"Opponent Team 7",
       {
           {"ninetales",
            {"flamethrower", "fire-spin", "confuse-ray", "take-down"}},
           {"arcanine", {"flamethrower", "fire-blast", "take-down", "leer"}},
           {"rapidash", {"fire-blast", "stomp", "take-down", "growl"}},
           {"magmar", {"flamethrower", "fire-punch", "smokescreen", "leer"}},
           {"flareon", {"flamethrower", "fire-spin", "quick-attack", "leer"}},
           {"charizard", {"flamethrower", "fire-spin", "slash", "leer"}},
       }},
      {"Opponent Team 8",
       {
           {"nidoking", {"earthquake", "thrash", "focus-energy", "leer"}},
           {"nidoqueen",
            {"earthquake", "body-slam", "double-kick", "tail-whip"}},
           {"dugtrio", {"earthquake", "slash", "sand-attack", "growl"}},
           {"rhydon", {"earthquake", "horn-drill", "leer", "tail-whip"}},
           {"marowak", {"earthquake", "focus-energy", "leer", "growl"}},
           {"sandslash", {"earthquake", "slash", "sand-attack", "swift"}},
       }},

  };

  // Show available teams for player selection
  std::cout
      << "\n╔════════════════════════════════════════════════════════════════════════════════╗\n"
      << "║                              Pokemon Battle Simulator                          ║\n"
      << "╚════════════════════════════════════════════════════════════════════════════════╝\n"
      << std::endl;
  std::cout << "🎯 Choose your team:\n" << std::endl;
  std::cout << "  [1] 🌿 Balanced Team\n"
            << "      → Venusaur, Pikachu, Machamp, Arcanine, Lapras, Snorlax\n" << std::endl;
  std::cout << "  [2] ⚡ Competitive Team\n"
            << "      → Charizard, Starmie, Snorlax, Alakazam, Rhydon, Jolteon\n" << std::endl;
  std::cout << "  [3] 🔥 Mixed Team\n"
            << "      → Venusaur, Zapdos, Nidoking, Gengar, Lapras, Tauros\n" << std::endl;

  // Prompt for team selection
  int chosenTeamNum;
  while (true) {
    std::cout << "📝 Enter the number of the team you want to select (1-3): ";
    std::cin >> chosenTeamNum;
    
    // Check for input failure or EOF
    if (std::cin.fail() || std::cin.eof()) {
      std::cout << "Invalid input or end of input. Exiting game." << std::endl;
      return 1;
    }
    
    // Validate range
    if (chosenTeamNum >= 1 && chosenTeamNum <= 3) {
      break;
    }
    
    std::cout << "Invalid selection. Please enter 1, 2, or 3." << std::endl;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  const auto chosenTeamKey = "Team " + std::to_string(chosenTeamNum);

  std::cout << "" << std::endl;
  std::cout << "========================================================== My "
               "Team =========================================================="
            << std::endl;
  std::cout << "" << std::endl;

  // Initialize Player Team and load Pokemon & Moves
  Team PlayerTeam;
  PlayerTeam.loadTeams(selectedTeams, selectedMoves, chosenTeamKey);

  // Print out Player's team with moves
  std::cout << "Your selected team includes:\n";
  for (int i = 0; i < static_cast<int>(PlayerTeam.size()); ++i) {
    const auto *pokemon = PlayerTeam.getPokemon(i);
    if (pokemon) {
      std::cout << "- " << pokemon->name << "\n  Moves:\n";
      for (const auto &move : pokemon->moves) {
        std::cout << "    * " << move.name << " (Power: " << move.power
                  << ", Accuracy: " << move.accuracy
                  << ", Class: " << move.damage_class << ")\n";
      }
    }
  }
  std::cout << std::endl;

  std::cout << "==============================================================="
               "==============================================================="
            << std::endl;
  std::cout << "" << std::endl;

  // Show available opponent teams
  std::cout << "🏆 Available Gym Leaders:\n" << std::endl;
  std::cout << "  [1] 🪨 Brock (Rock Gym Leader)" << std::endl;
  std::cout << "  [2] 💧 Misty (Water Gym Leader)" << std::endl;
  std::cout << "  [3] ⚡ Surge (Electric Gym Leader)" << std::endl;
  std::cout << "  [4] 🌿 Erika (Grass Gym Leader)" << std::endl;
  std::cout << "  [5] ☠️  Koga (Poison Gym Leader)" << std::endl;
  std::cout << "  [6] 🔮 Sabrina (Psychic Gym Leader)" << std::endl;
  std::cout << "  [7] 🔥 Blaine (Fire Gym Leader)" << std::endl;
  std::cout << "  [8] 🌍 Giovanni (Ground Gym Leader)\n" << std::endl;

  // Prompt for opponent selection
  int chosenOpponentNum;
  while (true) {
    std::cout << "⚔️  Enter the number of your chosen opponent (1-8): ";
    std::cin >> chosenOpponentNum;
    
    // Check for input failure or EOF
    if (std::cin.fail() || std::cin.eof()) {
      std::cout << "Invalid input or end of input. Exiting game." << std::endl;
      return 1;
    }
    
    // Validate range
    if (chosenOpponentNum >= 1 && chosenOpponentNum <= 8) {
      break;
    }
    
    std::cout << "Invalid selection. Please enter a number between 1 and 8." << std::endl;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  const auto chosenOpponentKey =
      "Opponent Team " + std::to_string(chosenOpponentNum);
  const auto &chosenOpponent = selectedTeams.at(chosenOpponentKey);
  std::cout << "\nYou have selected " << chosenOpponentKey
            << " with the Pokémon: ";
  for (const auto &pokemon : chosenOpponent) {
    std::cout << pokemon << " ";
  }
  std::cout << "\n\n";

  std::cout << "" << std::endl;
  std::cout
      << "========================================================== Oppenent "
         "Team =========================================================="
      << std::endl;
  std::cout << "" << std::endl;

  // Initialize Opponent Team and load Pokemon & Moves
  Team OppTeam;
  OppTeam.loadTeams(selectedTeams, selectedMoves, chosenOpponentKey);

  // Print out Opponent's team with moves
  std::cout << "Opponent's selected team includes:\n";
  for (int i = 0; i < static_cast<int>(OppTeam.size()); ++i) {
    const auto *pokemon = OppTeam.getPokemon(i);
    if (pokemon) {
      std::cout << "- " << pokemon->name << "\n";
    }
  }
  std::cout << std::endl;

  // AI Difficulty Selection
  std::cout << "==============================================================="
               "==============================================================="
            << std::endl;
  std::cout << "" << std::endl;
  std::cout << "🤖 Choose AI Difficulty Level:\n" << std::endl;
  std::cout << "  [1] 😊 Easy - Random moves, no switching" << std::endl;
  std::cout << "  [2] 🎯 Medium - Basic type effectiveness" << std::endl;
  std::cout << "  [3] 🧠 Hard - Smart strategy with switching" << std::endl;
  std::cout << "  [4] 🚀 Expert - Advanced AI with prediction & analysis\n" << std::endl;

  int chosenDifficulty;
  while (true) {
    std::cout << "🎮 Enter the difficulty level (1-4): ";
    std::cin >> chosenDifficulty;
    
    // Check for input failure or EOF
    if (std::cin.fail() || std::cin.eof()) {
      std::cout << "Invalid input or end of input. Defaulting to Easy." << std::endl;
      chosenDifficulty = 1;
      break;
    }
    
    // Validate range
    if (chosenDifficulty >= 1 && chosenDifficulty <= 4) {
      break;
    }
    
    std::cout << "Invalid selection. Please enter a number between 1 and 4." << std::endl;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  // Convert to AI difficulty enum
  Battle::AIDifficulty aiDifficulty;
  switch (chosenDifficulty) {
  case 1:
    aiDifficulty = Battle::AIDifficulty::EASY;
    std::cout << "\nAI Difficulty set to: Easy" << std::endl;
    break;
  case 2:
    aiDifficulty = Battle::AIDifficulty::MEDIUM;
    std::cout << "\nAI Difficulty set to: Medium (Basic type effectiveness)"
              << std::endl;
    break;
  case 3:
    aiDifficulty = Battle::AIDifficulty::HARD;
    std::cout << "\nAI Difficulty set to: Hard (Smart strategy with switching)"
              << std::endl;
    break;
  case 4:
    aiDifficulty = Battle::AIDifficulty::EXPERT;
    std::cout << "\nAI Difficulty set to: Expert (Advanced AI with prediction and strategic analysis)"
              << std::endl;
    break;
  default:
    aiDifficulty = Battle::AIDifficulty::EASY;
    break;
  }

  std::cout << std::endl;

  // BATTLE PART
  Battle battle(PlayerTeam, OppTeam, aiDifficulty);
  battle.startBattle();
}