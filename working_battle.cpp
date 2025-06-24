#include "Team.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

class SimpleBattle {
private:
  Team &playerTeam;
  Team &opponentTeam;
  Pokemon *playerPokemon;
  Pokemon *opponentPokemon;
  std::string playerName;

public:
  SimpleBattle(Team &pTeam, Team &oTeam, const std::string &name)
      : playerTeam(pTeam), opponentTeam(oTeam), playerName(name) {
    srand(time(0));
  }

  void displayHealth(const Pokemon *pokemon) {
    if (!pokemon)
      return;
    double hp_percent = pokemon->getHealthPercentage();
    std::cout << pokemon->name << " HP: " << pokemon->current_hp << "/"
              << pokemon->hp << " (" << hp_percent << "%)" << std::endl;
  }

  int calculateDamage(const Pokemon *attacker, const Pokemon *defender,
                      const Move &move) {
    if (move.power <= 0)
      return 0; // Status moves

    int damage;
    if (move.damage_class == "physical") {
      damage = (attacker->attack - defender->defense / 2) + move.power / 2;
    } else {
      damage = (attacker->special_attack - defender->special_defense / 2) +
               move.power / 2;
    }
    return std::max(1, damage / 3); // Scale down for longer battles
  }

  void executeTurn(Pokemon *attacker, Pokemon *defender, const Move &move,
                   const std::string &attackerName) {
    std::cout << "\n"
              << attackerName << "'s " << attacker->name << " used "
              << move.name << "!" << std::endl;

    if (move.power <= 0) {
      std::cout << "It's a status move! " << attacker->name
                << " feels stronger!" << std::endl;
      return;
    }

    // Check accuracy
    if (rand() % 100 + 1 > move.accuracy) {
      std::cout << "The attack missed!" << std::endl;
      return;
    }

    int damage = calculateDamage(attacker, defender, move);
    defender->takeDamage(damage);

    std::cout << "It dealt " << damage << " damage!" << std::endl;
    displayHealth(defender);

    if (!defender->isAlive()) {
      std::cout << "💀 " << defender->name << " fainted!" << std::endl;
    }
  }

  void startBattle() {
    std::cout << "\n⚔️  POKEMON BATTLE!" << std::endl;
    std::cout << "==================" << std::endl;

    // Player selects Pokemon
    std::cout << "\nChoose your Pokemon:" << std::endl;
    for (int i = 0; i < static_cast<int>(playerTeam.size()); ++i) {
      const Pokemon *p = playerTeam.getPokemon(i);
      if (p && p->isAlive()) {
        std::cout << "[" << (i + 1) << "] " << p->name << std::endl;
      }
    }

    int choice;
    std::cout << "Enter number: ";
    std::cin >> choice;
    playerPokemon = playerTeam.getPokemon(choice - 1);

    // Opponent selects first Pokemon
    opponentPokemon = opponentTeam.getFirstAlivePokemon();

    std::cout << "\n"
              << playerName << " sends out " << playerPokemon->name << "!"
              << std::endl;
    std::cout << "Brock sends out " << opponentPokemon->name << "!"
              << std::endl;

    // Battle loop
    int turn = 1;
    while (playerTeam.hasAlivePokemon() && opponentTeam.hasAlivePokemon()) {
      std::cout << "\n🎯 Turn " << turn << std::endl;
      std::cout << "========" << std::endl;

      displayHealth(playerPokemon);
      displayHealth(opponentPokemon);

      // Player chooses move
      std::cout << "\nChoose your move:" << std::endl;
      for (size_t i = 0; i < playerPokemon->moves.size(); ++i) {
        const Move &move = playerPokemon->moves[i];
        std::cout << "[" << (i + 1) << "] " << move.name;
        if (move.power > 0)
          std::cout << " (Power: " << move.power << ")";
        std::cout << std::endl;
      }

      int moveChoice;
      std::cout << "Enter move number: ";
      std::cin >> moveChoice;
      Move playerMove = playerPokemon->moves[moveChoice - 1];

      // Opponent chooses random move
      Move opponentMove =
          opponentPokemon->moves[rand() % opponentPokemon->moves.size()];

      // Determine turn order (speed)
      bool playerFirst = playerPokemon->speed >= opponentPokemon->speed;

      if (playerFirst) {
        executeTurn(playerPokemon, opponentPokemon, playerMove, playerName);
        if (opponentPokemon->isAlive()) {
          executeTurn(opponentPokemon, playerPokemon, opponentMove, "Brock");
        }
      } else {
        executeTurn(opponentPokemon, playerPokemon, opponentMove, "Brock");
        if (playerPokemon->isAlive()) {
          executeTurn(playerPokemon, opponentPokemon, playerMove, playerName);
        }
      }

      // Handle fainted Pokemon
      if (!playerPokemon->isAlive()) {
        Pokemon *newPokemon = playerTeam.getFirstAlivePokemon();
        if (newPokemon && newPokemon != playerPokemon) {
          playerPokemon = newPokemon;
          std::cout << "\n"
                    << playerName << " sends out " << playerPokemon->name << "!"
                    << std::endl;
        }
      }

      if (!opponentPokemon->isAlive()) {
        Pokemon *newPokemon = opponentTeam.getFirstAlivePokemon();
        if (newPokemon && newPokemon != opponentPokemon) {
          opponentPokemon = newPokemon;
          std::cout << "\nBrock sends out " << opponentPokemon->name << "!"
                    << std::endl;
        }
      }

      turn++;
      if (turn > 10)
        break; // Safety limit for demo
    }

    // Battle result
    std::cout << "\n🏆 BATTLE RESULT" << std::endl;
    std::cout << "================" << std::endl;
    if (opponentTeam.hasAlivePokemon()) {
      std::cout << "💔 You lost the battle!" << std::endl;
    } else {
      std::cout << "🎉 You won the battle!" << std::endl;
    }
  }
};

int main() {
  std::cout << "🎮 Pokemon Battle Simulator - Full Demo!" << std::endl;
  std::cout << "=======================================" << std::endl;

  std::string name;
  std::cout << "Enter your name: ";
  std::getline(std::cin, name);

  // Setup teams
  std::unordered_map<std::string, std::vector<std::string>> teams = {
      {"Player", {"pikachu", "machamp"}}, {"Brock", {"golem", "onix"}}};

  std::unordered_map<
      std::string,
      std::vector<std::pair<std::string, std::vector<std::string>>>>
      moves = {
          {"Player",
           {{"pikachu", {"thunderbolt", "iron-tail", "tackle", "reflect"}},
            {"machamp", {"superpower", "earthquake", "fire-blast", "tackle"}}}},
          {"Brock",
           {{"golem", {"earthquake", "stone-edge", "tackle", "protect"}},
            {"onix", {"tackle", "bind", "rock-throw", "protect"}}}}};

  Team playerTeam, opponentTeam;
  playerTeam.loadTeams(teams, moves, "Player");
  opponentTeam.loadTeams(teams, moves, "Brock");

  SimpleBattle battle(playerTeam, opponentTeam, name);
  battle.startBattle();

  return 0;
}