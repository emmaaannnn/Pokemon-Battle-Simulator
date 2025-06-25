#pragma once

#include "move.h"
#include "pokemon.h"
#include "team.h"
#include <iostream>
#include <memory>

class Battle {
public:
  // Constructor
  Battle(const Team &playerTeam, const Team &opponentTeam);

  // Main battle interface
  void startBattle();
  bool isBattleOver() const;

  // Battle state queries
  enum class BattleResult { ONGOING, PLAYER_WINS, OPPONENT_WINS, DRAW };

  BattleResult getBattleResult() const;

private:
  Team playerTeam;
  Team opponentTeam;
  Pokemon *selectedPokemon;
  Pokemon *opponentSelectedPokemon;

  // Battle flow methods
  void selectPokemon();
  void selectOpponentPokemon();
  void executeTurn();
  void executeMove(Pokemon &attacker, Pokemon &defender, const Move &move);
  void handlePokemonFainted();

  // Utility methods
  void displayHealth(const Pokemon &pokemon) const;
  void displayBattleStatus() const;
  bool playerFirst(const Move &playerMove, const Move &opponentMove) const;
  int calculateDamage(const Pokemon &attacker, const Pokemon &defender,
                      const Move &move) const;

  // Input handling
  int getMoveChoice() const;
  int getPokemonChoice() const;
};