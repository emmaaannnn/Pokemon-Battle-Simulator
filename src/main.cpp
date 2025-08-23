#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>
#include <csignal>
#include <functional>
#include <memory>
#include <chrono>
#include <algorithm>

#include "battle.h"
#include "input_validator.h"
#include "team_builder.h"
#include "pokemon_data.h"
#include "tournament_manager.h"

// Signal handler for graceful exit
void signalHandler(int /* signal */) {
  std::cout << "\n\n🛑 Game interrupted. Thanks for playing!\n" << std::endl;
  exit(0);
}

// Helper function to display template information
void displayTemplateInfo(const TeamBuilder::TeamTemplate& template_data) {
  std::cout << "\n═══════════════════════════════════════════════════════════════════════════════\n";
  std::cout << " " << template_data.name << "\n";
  std::cout << "═══════════════════════════════════════════════════════════════════════════════\n";
  std::cout << "Description: " << template_data.description << "\n";
  std::cout << "Difficulty: " << template_data.difficulty << "\n";
  std::cout << "Strategy: " << template_data.strategy << "\n";
  std::cout << "───────────────────────────────────────────────────────────────────────────────\n";
  
  std::cout << "\nPokemon Team:\n";
  for (const auto& pokemon : template_data.pokemon) {
    std::cout << "  • " << pokemon.name << " (" << pokemon.role << ")\n";
    std::cout << "    Strategy: " << pokemon.strategy << "\n";
    std::cout << "    Moves: ";
    for (size_t i = 0; i < pokemon.moves.size(); ++i) {
      std::cout << pokemon.moves[i];
      if (i < pokemon.moves.size() - 1) std::cout << ", ";
    }
    std::cout << "\n    Tips: " << pokemon.tips << "\n\n";
  }
  
  if (!template_data.usage_notes.empty()) {
    std::cout << "Usage Notes: " << template_data.usage_notes << "\n\n";
  }
}

// Helper function to create a team from template for battle
Team createBattleTeamFromTemplate(const TeamBuilder::Team& builderTeam, std::shared_ptr<TeamBuilder> teamBuilder) {
  // Export the team data to the legacy format
  auto exported = teamBuilder->exportTeamForBattle(builderTeam);
  
  // Create a new Team object and load it with the exported data
  Team battleTeam;
  battleTeam.loadTeams(exported.first, exported.second, builderTeam.name);
  
  return battleTeam;
}

// Helper function to show template categories and get user selection
int showTemplateCategories(const std::vector<std::string>& categories) {
  std::cout << "\n🎯 Template Categories:\n\n";
  for (size_t i = 0; i < categories.size(); ++i) {
    std::string display_name = categories[i];
    // Convert underscores to spaces and capitalize
    for (char& c : display_name) {
      if (c == '_') c = ' ';
    }
    display_name[0] = std::toupper(display_name[0]);
    for (size_t j = 1; j < display_name.length(); ++j) {
      if (display_name[j-1] == ' ') {
        display_name[j] = std::toupper(display_name[j]);
      }
    }
    
    std::cout << "  [" << (i + 1) << "] " << display_name << "\n";
  }
  std::cout << "  [" << (categories.size() + 1) << "] Build Custom Team\n";
  std::cout << "  [" << (categories.size() + 2) << "] Generate Random Team\n";
  std::cout << "  [" << (categories.size() + 3) << "] Tournament Draft Mode\n";
  std::cout << "  [" << (categories.size() + 4) << "] 🏆 Tournament Mode\n";
  std::cout << "  [" << (categories.size() + 5) << "] Import Team from Share Code\n";
  std::cout << "  [" << (categories.size() + 6) << "] Load Custom Team\n";
  std::cout << "  [" << (categories.size() + 7) << "] Team Comparison Tool\n\n";
  
  auto categoryValidator = [categories](std::istream& input) -> InputValidator::ValidationResult<int> {
    return InputValidator::getValidatedInt(input, 1, static_cast<int>(categories.size() + 7));
  };
  
  auto categoryResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "📝 Select a category (1-" + std::to_string(categories.size() + 7) + ")",
    2, categoryValidator
  );
  
  if (!categoryResult.isValid()) {
    std::cout << "Defaulting to first category.\n";
    return 1;
  }
  
  return categoryResult.value;
}

// Helper function to show templates in a category and get user selection
int showTemplatesInCategory(const std::vector<std::string>& templates, const std::string& category) {
  std::cout << "\n🎯 Available " << category << " Templates:\n\n";
  for (size_t i = 0; i < templates.size(); ++i) {
    std::cout << "  [" << (i + 1) << "] " << templates[i] << "\n";
  }
  std::cout << "\n";
  
  auto templateValidator = [templates](std::istream& input) -> InputValidator::ValidationResult<int> {
    return InputValidator::getValidatedInt(input, 1, static_cast<int>(templates.size()));
  };
  
  auto templateResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "📝 Select a template (1-" + std::to_string(templates.size()) + ")",
    2, templateValidator
  );
  
  if (!templateResult.isValid()) {
    std::cout << "Defaulting to first template.\n";
    return 1;
  }
  
  return templateResult.value;
}

// Helper function to handle tournament draft mode
TeamBuilder::Team handleTournamentDraft(std::shared_ptr<TeamBuilder> teamBuilder, const std::string& playerName) {
  std::cout << "\n🏆 Tournament Draft Mode\n";
  std::cout << "═══════════════════════════\n\n";
  
  // Configure draft settings
  TeamBuilder::DraftSettings settings;
  settings.player_count = 2; // Player vs AI for now
  settings.team_size = 6;
  settings.ban_phase_picks_per_player = 2;
  settings.max_legendaries_per_team = 1;
  settings.max_same_type_per_team = 2;
  
  std::cout << "Draft Configuration:\n";
  std::cout << "- Players: " << settings.player_count << "\n";
  std::cout << "- Team Size: " << settings.team_size << "\n";
  std::cout << "- Bans per player: " << settings.ban_phase_picks_per_player << "\n";
  std::cout << "- Max legendaries: " << settings.max_legendaries_per_team << "\n";
  std::cout << "- Max same type: " << settings.max_same_type_per_team << "\n\n";
  
  // Create draft session
  std::vector<std::string> players = {playerName, "AI Opponent"};
  auto session = teamBuilder->createDraftSession(settings, players);
  
  if (!session.is_active) {
    std::cout << "❌ Failed to create draft session. Using random team.\n";
    return teamBuilder->generateRandomTeam(playerName + "'s Team");
  }
  
  std::cout << "🎯 Draft started! Session ID: " << session.session_id << "\n\n";
  
  // Simulate draft process (simplified for single player vs AI)
  while (session.is_active) {
    std::cout << "═══ Turn " << session.current_turn << " ═══\n";
    
    if (session.current_phase == 0) {
      std::cout << "📛 Ban Phase\n";
    } else {
      std::cout << "🎯 Pick Phase\n";
    }
    
    std::cout << "Current Player: " << session.player_names[session.current_player] << "\n";
    
    if (session.current_player == 0) { // Human player
      if (session.current_phase == 0) {
        // Ban phase
        std::cout << "\nAvailable Pokemon to ban (showing first 10):\n";
        for (size_t i = 0; i < std::min(session.available_pokemon.size(), size_t(10)); ++i) {
          std::cout << "  [" << (i + 1) << "] " << session.available_pokemon[i] << "\n";
        }
        
        auto banValidator = [&session](std::istream& input) -> InputValidator::ValidationResult<int> {
          return InputValidator::getValidatedInt(input, 1, static_cast<int>(std::min(session.available_pokemon.size(), size_t(10))));
        };
        
        auto banResult = InputValidator::promptWithRetry<int>(
          std::cin, std::cout,
          "Select Pokemon to ban (1-" + std::to_string(std::min(session.available_pokemon.size(), size_t(10))) + ")",
          2, banValidator
        );
        
        if (banResult.isValid() && banResult.value > 0 && banResult.value <= static_cast<int>(std::min(session.available_pokemon.size(), size_t(10)))) {
          std::string pokemon_to_ban = session.available_pokemon[banResult.value - 1];
          if (teamBuilder->executeDraftBan(session, 0, pokemon_to_ban)) {
            std::cout << "🚫 Banned: " << pokemon_to_ban << "\n";
          } else {
            std::cout << "❌ Failed to ban " << pokemon_to_ban << "\n";
          }
        }
      } else {
        // Pick phase
        std::cout << "\nYour current team:\n";
        for (const auto& pokemon : session.player_teams[0]) {
          std::cout << "  - " << pokemon << "\n";
        }
        
        auto suggestions = teamBuilder->getDraftSuggestions(session, 5);
        std::cout << "\n💡 Suggested picks:\n";
        for (const auto& [pokemon, reasoning] : suggestions) {
          std::cout << "  - " << pokemon << " (" << reasoning << ")\n";
        }
        
        std::cout << "\nAvailable Pokemon to pick (showing first 10):\n";
        for (size_t i = 0; i < std::min(session.available_pokemon.size(), size_t(10)); ++i) {
          std::cout << "  [" << (i + 1) << "] " << session.available_pokemon[i] << "\n";
        }
        
        auto pickValidator = [&session](std::istream& input) -> InputValidator::ValidationResult<int> {
          return InputValidator::getValidatedInt(input, 1, static_cast<int>(std::min(session.available_pokemon.size(), size_t(10))));
        };
        
        auto pickResult = InputValidator::promptWithRetry<int>(
          std::cin, std::cout,
          "Select Pokemon to pick (1-" + std::to_string(std::min(session.available_pokemon.size(), size_t(10))) + ")",
          2, pickValidator
        );
        
        if (pickResult.isValid() && pickResult.value > 0 && pickResult.value <= static_cast<int>(std::min(session.available_pokemon.size(), size_t(10)))) {
          std::string pokemon_to_pick = session.available_pokemon[pickResult.value - 1];
          if (teamBuilder->executeDraftPick(session, 0, pokemon_to_pick)) {
            std::cout << "✅ Picked: " << pokemon_to_pick << "\n";
          } else {
            std::cout << "❌ Failed to pick " << pokemon_to_pick << "\n";
          }
        }
      }
    } else {
      // AI player
      std::cout << "🤖 AI is making a decision...\n";
      
      if (session.current_phase == 0 && !session.available_pokemon.empty()) {
        // AI bans a random Pokemon
        std::string ai_ban = session.available_pokemon[rand() % session.available_pokemon.size()];
        if (teamBuilder->executeDraftBan(session, 1, ai_ban)) {
          std::cout << "🤖 AI banned: " << ai_ban << "\n";
        }
      } else if (!session.available_pokemon.empty()) {
        // AI picks a Pokemon (simple strategy)
        auto suggestions = teamBuilder->getDraftSuggestions(session, 3);
        std::string ai_pick;
        if (!suggestions.empty()) {
          ai_pick = suggestions[0].first;
        } else {
          ai_pick = session.available_pokemon[rand() % session.available_pokemon.size()];
        }
        
        if (teamBuilder->executeDraftPick(session, 1, ai_pick)) {
          std::cout << "🤖 AI picked: " << ai_pick << "\n";
        }
      }
    }
    
    teamBuilder->advanceDraftTurn(session);
    std::cout << "\n";
  }
  
  std::cout << "🏁 Draft completed!\n";
  
  // Display draft analysis
  auto analysis = teamBuilder->analyzeDraftStrategy(session);
  std::cout << "\n📊 Draft Analysis:\n";
  for (const auto& [player_id, strategies] : analysis) {
    std::cout << "\n" << session.player_names[player_id] << ":\n";
    for (const auto& strategy : strategies) {
      std::cout << "  • " << strategy << "\n";
    }
  }
  
  // Finalize teams and return player team
  auto finalized_teams = teamBuilder->finalizeDraftTeams(session);
  if (!finalized_teams.empty()) {
    return finalized_teams[0]; // Return player's team
  }
  
  return teamBuilder->generateRandomTeam(playerName + "'s Team");
}

// Helper function to handle team import from share code
TeamBuilder::Team handleTeamImport(std::shared_ptr<TeamBuilder> teamBuilder, const std::string& playerName) {
  std::cout << "\n📥 Import Team from Share Code\n";
  std::cout << "═══════════════════════════════\n\n";
  
  std::cout << "Enter the team share code (base64 encoded):\n";
  std::string shareCode;
  std::getline(std::cin, shareCode);
  
  if (shareCode.empty()) {
    std::cout << "❌ No share code provided. Using random team.\n";
    return teamBuilder->generateRandomTeam(playerName + "'s Team");
  }
  
  auto importedTeam = teamBuilder->importTeamFromShareCode(shareCode, true);
  
  if (importedTeam.name == "Import_Failed" || importedTeam.pokemon.empty()) {
    std::cout << "❌ Failed to import team from share code. Using random team.\n";
    return teamBuilder->generateRandomTeam(playerName + "'s Team");
  }
  
  std::cout << "✅ Successfully imported team: " << importedTeam.name << "\n";
  std::cout << "Team Pokemon:\n";
  for (const auto& pokemon : importedTeam.pokemon) {
    std::cout << "  - " << pokemon.name << "\n";
  }
  
  // Ask if user wants to save this as a custom team
  std::cout << "\nWould you like to save this team as a custom team? (y/n): ";
  char save_choice;
  std::cin >> save_choice;
  std::cin.ignore();
  
  if (save_choice == 'y' || save_choice == 'Y') {
    if (teamBuilder->saveCustomTeam(importedTeam)) {
      std::cout << "✅ Team saved to custom teams directory.\n";
    } else {
      std::cout << "⚠️ Failed to save team to custom directory.\n";
    }
  }
  
  return importedTeam;
}

// Helper function to load custom team
TeamBuilder::Team handleCustomTeamLoad(std::shared_ptr<TeamBuilder> teamBuilder, const std::string& playerName) {
  std::cout << "\n📂 Load Custom Team\n";
  std::cout << "═══════════════════\n\n";
  
  auto customTeams = teamBuilder->getCustomTeamsList();
  if (customTeams.empty()) {
    std::cout << "❌ No custom teams found. Using random team.\n";
    return teamBuilder->generateRandomTeam(playerName + "'s Team");
  }
  
  std::cout << "Available custom teams:\n";
  for (size_t i = 0; i < customTeams.size(); ++i) {
    std::cout << "  [" << (i + 1) << "] " << customTeams[i] << "\n";
  }
  
  auto teamValidator = [&customTeams](std::istream& input) -> InputValidator::ValidationResult<int> {
    return InputValidator::getValidatedInt(input, 1, static_cast<int>(customTeams.size()));
  };
  
  auto teamResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "Select team to load (1-" + std::to_string(customTeams.size()) + ")",
    2, teamValidator
  );
  
  if (!teamResult.isValid()) {
    std::cout << "❌ Invalid selection. Using random team.\n";
    return teamBuilder->generateRandomTeam(playerName + "'s Team");
  }
  
  std::string selectedTeam = customTeams[teamResult.value - 1];
  auto loadedTeam = teamBuilder->loadCustomTeam(selectedTeam);
  
  if (loadedTeam.pokemon.empty()) {
    std::cout << "❌ Failed to load team. Using random team.\n";
    return teamBuilder->generateRandomTeam(playerName + "'s Team");
  }
  
  std::cout << "✅ Loaded team: " << loadedTeam.name << "\n";
  return loadedTeam;
}

// Forward declarations for tournament functions
void enterTournamentMode(std::shared_ptr<TournamentManager> tournamentManager, 
                        std::shared_ptr<TeamBuilder> teamBuilder,
                        std::shared_ptr<PokemonData> pokemonData,
                        const std::string& playerName);
void handleGymLeaderChallenge(std::shared_ptr<TournamentManager> tournamentManager,
                             std::shared_ptr<TeamBuilder> teamBuilder,
                             std::shared_ptr<PokemonData> pokemonData,
                             const std::string& playerName);
void handleEliteFourChallenge(std::shared_ptr<TournamentManager> tournamentManager,
                             std::shared_ptr<TeamBuilder> teamBuilder,
                             std::shared_ptr<PokemonData> pokemonData,
                             const std::string& playerName);
void handleChampionshipBattle(std::shared_ptr<TournamentManager> tournamentManager,
                             std::shared_ptr<TeamBuilder> teamBuilder,
                             std::shared_ptr<PokemonData> pokemonData,
                             const std::string& playerName);
void displayTournamentStatistics(std::shared_ptr<TournamentManager> tournamentManager, 
                                const std::string& playerName);
void displayTournamentLeaderboard(std::shared_ptr<TournamentManager> tournamentManager);
void conductTournamentBattle(std::shared_ptr<TournamentManager> tournamentManager,
                            std::shared_ptr<TeamBuilder> teamBuilder,
                            std::shared_ptr<PokemonData> pokemonData,
                            const std::string& playerName,
                            const TournamentManager::ChallengeInfo& challenge);

// Helper function to handle team comparison
void handleTeamComparison(std::shared_ptr<TeamBuilder> teamBuilder) {
  std::cout << "\n⚔️ Team Comparison Tool\n";
  std::cout << "══════════════════════\n\n";
  
  // Generate two random teams for comparison
  auto team1 = teamBuilder->generateRandomTeam("Team Alpha");
  auto team2 = teamBuilder->generateRandomTeam("Team Beta");
  
  std::cout << "Comparing two randomly generated teams:\n\n";
  
  std::cout << "🔵 " << team1.name << ":\n";
  for (const auto& pokemon : team1.pokemon) {
    std::cout << "  - " << pokemon.name << "\n";
  }
  
  std::cout << "\n🔴 " << team2.name << ":\n";
  for (const auto& pokemon : team2.pokemon) {
    std::cout << "  - " << pokemon.name << "\n";
  }
  
  auto comparison = teamBuilder->compareTeams(team1, team2);
  
  std::cout << "\n📊 Comparison Results:\n";
  std::cout << "═══════════════════════\n";
  std::cout << "🔵 " << comparison.team1_name << " Balance Score: " << comparison.team1_balance_score << "/100\n";
  std::cout << "🔴 " << comparison.team2_name << " Balance Score: " << comparison.team2_balance_score << "/100\n";
  std::cout << "🎯 " << comparison.team1_name << " Win Probability: " << 
    static_cast<int>(comparison.team1_win_probability * 100) << "%\n\n";
  
  std::cout << "💭 Battle Prediction: " << comparison.battle_prediction_reasoning << "\n\n";
  
  if (!comparison.team1_coverage_advantages.empty()) {
    std::cout << "🔵 " << comparison.team1_name << " Advantages:\n";
    for (const auto& advantage : comparison.team1_coverage_advantages) {
      std::cout << "  • " << advantage << " type coverage\n";
    }
    std::cout << "\n";
  }
  
  if (!comparison.team2_coverage_advantages.empty()) {
    std::cout << "🔴 " << comparison.team2_name << " Advantages:\n";
    for (const auto& advantage : comparison.team2_coverage_advantages) {
      std::cout << "  • " << advantage << " type coverage\n";
    }
    std::cout << "\n";
  }
  
  if (!comparison.mutual_weaknesses.empty()) {
    std::cout << "⚠️ Mutual Weaknesses:\n";
    for (const auto& weakness : comparison.mutual_weaknesses) {
      std::cout << "  • Both teams lack " << weakness << " type coverage\n";
    }
    std::cout << "\n";
  }
  
  std::cout << "Press Enter to continue...";
  std::cin.get();
}

// Helper function to handle custom team building
TeamBuilder::Team handleCustomTeamBuilder(std::shared_ptr<TeamBuilder> teamBuilder, std::shared_ptr<PokemonData> pokemonData, const std::string& playerName) {
  std::cout << "\n🔨 Custom Team Builder\n";
  std::cout << "═══════════════════════\n\n";
  
  // Create a new team
  auto team = teamBuilder->createTeam(playerName + "'s Custom Team");
  
  std::cout << "Let's build your custom team! You can add up to 6 Pokemon.\n\n";
  
  // Get list of available Pokemon
  auto availablePokemon = pokemonData->getAvailablePokemon();
  auto availableMoves = pokemonData->getAvailableMoves();
  
  if (availablePokemon.empty()) {
    std::cout << "❌ No Pokemon data available. Using random team.\n";
    return teamBuilder->generateRandomTeam(playerName + "'s Team");
  }
  
  int pokemon_count = 0;
  while (pokemon_count < 6) {
    std::cout << "\n═══ Adding Pokemon " << (pokemon_count + 1) << "/6 ═══\n";
    
    // Show options
    std::cout << "  [1] Search Pokemon by name\n";
    std::cout << "  [2] Browse Pokemon by type\n";
    std::cout << "  [3] Select from random suggestions\n";
    if (pokemon_count > 0) {
      std::cout << "  [4] Finish team (current size: " << pokemon_count << ")\n";
    }
    std::cout << "\n";
    
    auto optionValidator = [pokemon_count](std::istream& input) -> InputValidator::ValidationResult<int> {
      int max_option = (pokemon_count > 0) ? 4 : 3;
      return InputValidator::getValidatedInt(input, 1, max_option);
    };
    
    auto optionResult = InputValidator::promptWithRetry<int>(
      std::cin, std::cout,
      "Choose option (1-" + std::to_string((pokemon_count > 0) ? 4 : 3) + ")",
      2, optionValidator
    );
    
    if (!optionResult.isValid()) {
      std::cout << "❌ Invalid option. Skipping this Pokemon.\n";
      continue;
    }
    
    std::string selectedPokemon;
    
    if (optionResult.value == 1) {
      // Search by name
      std::cout << "\n🔍 Search Pokemon by name:\n";
      std::cout << "Enter Pokemon name (or partial name): ";
      std::string searchTerm;
      std::getline(std::cin, searchTerm);
      
      if (searchTerm.empty()) {
        std::cout << "❌ No search term entered.\n";
        continue;
      }
      
      // Find matching Pokemon
      std::vector<std::string> matches;
      for (const auto& pokemon : availablePokemon) {
        std::string lowerPokemon = pokemon;
        std::string lowerSearch = searchTerm;
        std::transform(lowerPokemon.begin(), lowerPokemon.end(), lowerPokemon.begin(), ::tolower);
        std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);
        
        if (lowerPokemon.find(lowerSearch) != std::string::npos) {
          matches.push_back(pokemon);
        }
      }
      
      if (matches.empty()) {
        std::cout << "❌ No Pokemon found matching '" << searchTerm << "'.\n";
        continue;
      }
      
      std::cout << "\n🎯 Matching Pokemon:\n";
      for (size_t i = 0; i < std::min(matches.size(), size_t(10)); ++i) {
        std::cout << "  [" << (i + 1) << "] " << matches[i] << "\n";
      }
      
      if (matches.size() > 10) {
        std::cout << "  ... and " << (matches.size() - 10) << " more (showing first 10)\n";
      }
      
      auto matchValidator = [&matches](std::istream& input) -> InputValidator::ValidationResult<int> {
        return InputValidator::getValidatedInt(input, 1, static_cast<int>(std::min(matches.size(), size_t(10))));
      };
      
      auto matchResult = InputValidator::promptWithRetry<int>(
        std::cin, std::cout,
        "Select Pokemon (1-" + std::to_string(std::min(matches.size(), size_t(10))) + ")",
        2, matchValidator
      );
      
      if (matchResult.isValid()) {
        selectedPokemon = matches[matchResult.value - 1];
      }
      
    } else if (optionResult.value == 2) {
      // Browse by type
      std::vector<std::string> types = {"fire", "water", "grass", "electric", "psychic", "ice", "dragon", "dark", "fighting", "poison", "ground", "flying", "bug", "rock", "ghost", "steel", "fairy", "normal"};
      
      std::cout << "\n🔥 Select Pokemon type:\n";
      for (size_t i = 0; i < types.size(); ++i) {
        std::cout << "  [" << (i + 1) << "] " << types[i] << "\n";
      }
      
      auto typeValidator = [&types](std::istream& input) -> InputValidator::ValidationResult<int> {
        return InputValidator::getValidatedInt(input, 1, static_cast<int>(types.size()));
      };
      
      auto typeResult = InputValidator::promptWithRetry<int>(
        std::cin, std::cout,
        "Select type (1-" + std::to_string(types.size()) + ")",
        2, typeValidator
      );
      
      if (typeResult.isValid()) {
        std::string selectedType = types[typeResult.value - 1];
        auto pokemonOfType = pokemonData->getPokemonByType(selectedType);
        
        if (pokemonOfType.empty()) {
          std::cout << "❌ No Pokemon found of type " << selectedType << ".\n";
          continue;
        }
        
        std::cout << "\n🎯 " << selectedType << " type Pokemon:\n";
        for (size_t i = 0; i < std::min(pokemonOfType.size(), size_t(10)); ++i) {
          std::cout << "  [" << (i + 1) << "] " << pokemonOfType[i] << "\n";
        }
        
        auto pokemonValidator = [&pokemonOfType](std::istream& input) -> InputValidator::ValidationResult<int> {
          return InputValidator::getValidatedInt(input, 1, static_cast<int>(std::min(pokemonOfType.size(), size_t(10))));
        };
        
        auto pokemonResult = InputValidator::promptWithRetry<int>(
          std::cin, std::cout,
          "Select Pokemon (1-" + std::to_string(std::min(pokemonOfType.size(), size_t(10))) + ")",
          2, pokemonValidator
        );
        
        if (pokemonResult.isValid()) {
          selectedPokemon = pokemonOfType[pokemonResult.value - 1];
        }
      }
      
    } else if (optionResult.value == 3) {
      // Random suggestions
      std::cout << "\n🎲 Random Pokemon suggestions:\n";
      std::vector<std::string> suggestions;
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> dis(0, availablePokemon.size() - 1);
      
      for (int i = 0; i < 5; ++i) {
        std::string pokemon = availablePokemon[dis(gen)];
        if (std::find(suggestions.begin(), suggestions.end(), pokemon) == suggestions.end()) {
          suggestions.push_back(pokemon);
        }
      }
      
      for (size_t i = 0; i < suggestions.size(); ++i) {
        std::cout << "  [" << (i + 1) << "] " << suggestions[i] << "\n";
      }
      
      auto suggestionValidator = [&suggestions](std::istream& input) -> InputValidator::ValidationResult<int> {
        return InputValidator::getValidatedInt(input, 1, static_cast<int>(suggestions.size()));
      };
      
      auto suggestionResult = InputValidator::promptWithRetry<int>(
        std::cin, std::cout,
        "Select Pokemon (1-" + std::to_string(suggestions.size()) + ")",
        2, suggestionValidator
      );
      
      if (suggestionResult.isValid()) {
        selectedPokemon = suggestions[suggestionResult.value - 1];
      }
      
    } else if (optionResult.value == 4 && pokemon_count > 0) {
      // Finish team
      std::cout << "\n✅ Finishing team with " << pokemon_count << " Pokemon.\n";
      break;
    }
    
    if (selectedPokemon.empty()) {
      std::cout << "❌ No Pokemon selected. Try again.\n";
      continue;
    }
    
    // Check for duplicates
    bool duplicate = false;
    for (const auto& existing : team.pokemon) {
      if (existing.name == selectedPokemon) {
        std::cout << "⚠️ " << selectedPokemon << " is already on your team. Choose a different Pokemon.\n";
        duplicate = true;
        break;
      }
    }
    
    if (duplicate) {
      continue;
    }
    
    std::cout << "\n✅ Selected: " << selectedPokemon << "\n";
    
    // Now select moves for this Pokemon
    std::cout << "🎯 Select moves for " << selectedPokemon << " (up to 4 moves):\n";
    std::vector<std::string> selectedMoves;
    
    // Show random move suggestions
    std::cout << "\n💡 Suggested moves:\n";
    std::vector<std::string> movesSuggestions;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> moveDis(0, availableMoves.size() - 1);
    
    for (int i = 0; i < 8; ++i) {
      std::string move = availableMoves[moveDis(gen)];
      if (std::find(movesSuggestions.begin(), movesSuggestions.end(), move) == movesSuggestions.end()) {
        movesSuggestions.push_back(move);
      }
    }
    
    for (size_t i = 0; i < movesSuggestions.size(); ++i) {
      std::cout << "  [" << (i + 1) << "] " << movesSuggestions[i] << "\n";
    }
    
    std::cout << "  [" << (movesSuggestions.size() + 1) << "] Use random moves\n";
    std::cout << "  [" << (movesSuggestions.size() + 2) << "] Enter custom moves\n\n";
    
    auto moveOptionValidator = [&movesSuggestions](std::istream& input) -> InputValidator::ValidationResult<int> {
      return InputValidator::getValidatedInt(input, 1, static_cast<int>(movesSuggestions.size() + 2));
    };
    
    auto moveOptionResult = InputValidator::promptWithRetry<int>(
      std::cin, std::cout,
      "Select option (1-" + std::to_string(movesSuggestions.size() + 2) + ")",
      2, moveOptionValidator
    );
    
    if (moveOptionResult.isValid()) {
      if (moveOptionResult.value <= static_cast<int>(movesSuggestions.size())) {
        // Select from suggestions
        int numMoves = std::min(4, static_cast<int>(movesSuggestions.size()));
        for (int i = 0; i < numMoves; ++i) {
          selectedMoves.push_back(movesSuggestions[i]);
        }
      } else if (moveOptionResult.value == static_cast<int>(movesSuggestions.size() + 1)) {
        // Use random moves
        for (int i = 0; i < 4 && i < static_cast<int>(availableMoves.size()); ++i) {
          std::string move = availableMoves[moveDis(gen)];
          selectedMoves.push_back(move);
        }
      } else {
        // Enter custom moves
        std::cout << "\nEnter up to 4 moves (press Enter with empty line to finish):\n";
        for (int i = 0; i < 4; ++i) {
          std::cout << "Move " << (i + 1) << ": ";
          std::string moveName;
          std::getline(std::cin, moveName);
          
          if (moveName.empty()) {
            break;
          }
          
          if (pokemonData->hasMove(moveName)) {
            selectedMoves.push_back(moveName);
          } else {
            std::cout << "⚠️ Move '" << moveName << "' not found. Skipping.\n";
          }
        }
      }
    }
    
    // Add Pokemon to team
    if (teamBuilder->addPokemonToTeam(team, selectedPokemon, selectedMoves)) {
      std::cout << "✅ " << selectedPokemon << " added to your team!\n";
      pokemon_count++;
    } else {
      std::cout << "❌ Failed to add " << selectedPokemon << " to team.\n";
      if (!team.validation_errors.empty()) {
        std::cout << "Errors:\n";
        for (const auto& error : team.validation_errors) {
          std::cout << "  - " << error << "\n";
        }
        team.validation_errors.clear();
      }
    }
  }
  
  std::cout << "\n🎉 Custom team building complete!\n";
  std::cout << "Your team '" << team.name << "' has " << team.pokemon.size() << " Pokemon.\n";
  
  return team;
}

// Tournament mode handler
void enterTournamentMode(std::shared_ptr<TournamentManager> tournamentManager, 
                        std::shared_ptr<TeamBuilder> teamBuilder,
                        std::shared_ptr<PokemonData> pokemonData,
                        const std::string& playerName) {
  std::cout << "\n🏆═══════════════════════════════════════════════════════════════════════════════🏆\n";
  std::cout << "║                                Tournament Mode                                ║\n";
  std::cout << "🏆═══════════════════════════════════════════════════════════════════════════════🏆\n\n";

  // Initialize player progress if needed
  tournamentManager->initializePlayerProgress(playerName);
  
  // Get player progress
  auto progress = tournamentManager->getPlayerProgress(playerName);
  if (!progress) {
    std::cout << "❌ Failed to initialize tournament progress. Returning to main menu.\n";
    return;
  }

  while (true) {
    // Display tournament progress
    std::cout << "\n📊 Tournament Progress for " << playerName << ":\n";
    std::cout << "═════════════════════════════════════\n";
    std::cout << "🥇 Badges Earned: " << tournamentManager->getPlayerBadgeCount(playerName) << "/8\n";
    
    auto badges = tournamentManager->getPlayerBadges(playerName);
    if (!badges.empty()) {
      std::cout << "\n🏅 Your Badges:\n";
      for (const auto& badge : badges) {
        std::cout << "  ✅ " << badge.gym_name << " (" << badge.gym_type << " type) - " 
                  << badge.gym_leader_name << "\n";
      }
    }

    // Show Elite Four status
    bool eliteFourUnlocked = tournamentManager->isEliteFourUnlocked(playerName);
    bool championUnlocked = tournamentManager->isChampionshipUnlocked(playerName);
    
    std::cout << "\n🎖️ Elite Four Status: " << (eliteFourUnlocked ? "✅ Unlocked" : "🔒 Locked (need 8 badges)") << "\n";
    std::cout << "👑 Champion Status: " << (championUnlocked ? "✅ Unlocked" : "🔒 Locked (complete Elite Four)") << "\n";
    
    double completion = tournamentManager->getTournamentCompletionPercentage(playerName);
    std::cout << "📈 Tournament Completion: " << static_cast<int>(completion * 100) << "%\n\n";

    // Tournament menu options
    std::cout << "🎯 Tournament Options:\n";
    std::cout << "  [1] 🥊 Challenge Gym Leaders\n";
    std::cout << "  [2] 🎖️ Elite Four Challenge" << (eliteFourUnlocked ? "" : " (Locked)") << "\n";
    std::cout << "  [3] 👑 Championship Battle" << (championUnlocked ? "" : " (Locked)") << "\n";
    std::cout << "  [4] 📊 Tournament Statistics\n";
    std::cout << "  [5] 🏆 Leaderboard\n";
    std::cout << "  [6] ↩️ Return to Main Menu\n\n";

    auto optionValidator = [](std::istream& input) -> InputValidator::ValidationResult<int> {
      return InputValidator::getValidatedInt(input, 1, 6);
    };

    auto optionResult = InputValidator::promptWithRetry<int>(
      std::cin, std::cout,
      "Select an option (1-6)",
      2, optionValidator
    );

    if (!optionResult.isValid()) {
      std::cout << "Invalid input. Please try again.\n";
      continue;
    }

    switch (optionResult.value) {
      case 1:
        // Gym Leader challenges
        handleGymLeaderChallenge(tournamentManager, teamBuilder, pokemonData, playerName);
        break;
      case 2:
        if (eliteFourUnlocked) {
          handleEliteFourChallenge(tournamentManager, teamBuilder, pokemonData, playerName);
        } else {
          std::cout << "🔒 Elite Four is locked. You need all 8 badges to challenge the Elite Four.\n";
          std::cout << "Press Enter to continue...";
          std::cin.get();
        }
        break;
      case 3:
        if (championUnlocked) {
          handleChampionshipBattle(tournamentManager, teamBuilder, pokemonData, playerName);
        } else {
          std::cout << "🔒 Championship is locked. You must complete the Elite Four first.\n";
          std::cout << "Press Enter to continue...";
          std::cin.get();
        }
        break;
      case 4:
        displayTournamentStatistics(tournamentManager, playerName);
        break;
      case 5:
        displayTournamentLeaderboard(tournamentManager);
        break;
      case 6:
        return; // Exit tournament mode
      default:
        std::cout << "Invalid option. Please try again.\n";
        break;
    }
  }
}

// Helper function to handle gym leader challenge
void handleGymLeaderChallenge(std::shared_ptr<TournamentManager> tournamentManager,
                             std::shared_ptr<TeamBuilder> teamBuilder,
                             std::shared_ptr<PokemonData> pokemonData,
                             const std::string& playerName) {
  std::cout << "\n🥊 Gym Leader Challenge Selection\n";
  std::cout << "═════════════════════════════════\n\n";

  auto availableChallenges = tournamentManager->getAvailableChallenges(playerName);
  std::vector<TournamentManager::ChallengeInfo> gymChallenges;
  
  // Filter for gym challenges
  for (const auto& challenge : availableChallenges) {
    if (challenge.challenge_type == "gym" && !challenge.is_completed) {
      gymChallenges.push_back(challenge);
    }
  }

  if (gymChallenges.empty()) {
    std::cout << "🎉 Congratulations! You have defeated all gym leaders!\n";
    std::cout << "Press Enter to continue...";
    std::cin.get();
    return;
  }

  std::cout << "Available Gym Challenges:\n";
  for (size_t i = 0; i < gymChallenges.size(); ++i) {
    const auto& gym = gymChallenges[i];
    std::cout << "  [" << (i + 1) << "] " << gym.challenge_name << " (" << gym.difficulty_level << ")\n";
    std::cout << "      Type Specialization: " << gym.challenge_type << "\n";
    if (!gym.required_badges.empty()) {
      std::cout << "      Prerequisites: ";
      for (size_t j = 0; j < gym.required_badges.size(); ++j) {
        std::cout << gym.required_badges[j];
        if (j < gym.required_badges.size() - 1) std::cout << ", ";
      }
      std::cout << "\n";
    }
    std::cout << "      Reward: " << gym.reward_description << "\n\n";
  }

  std::cout << "  [" << (gymChallenges.size() + 1) << "] ↩️ Back to Tournament Menu\n\n";

  auto gymValidator = [&gymChallenges](std::istream& input) -> InputValidator::ValidationResult<int> {
    return InputValidator::getValidatedInt(input, 1, static_cast<int>(gymChallenges.size() + 1));
  };

  auto gymResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "Select a gym to challenge (1-" + std::to_string(gymChallenges.size() + 1) + ")",
    2, gymValidator
  );

  if (!gymResult.isValid() || gymResult.value > static_cast<int>(gymChallenges.size())) {
    return; // Back to tournament menu
  }

  const auto& selectedGym = gymChallenges[gymResult.value - 1];
  
  // Conduct tournament battle
  conductTournamentBattle(tournamentManager, teamBuilder, pokemonData, playerName, selectedGym);
}

// Helper function to handle Elite Four challenge
void handleEliteFourChallenge(std::shared_ptr<TournamentManager> tournamentManager,
                             std::shared_ptr<TeamBuilder> teamBuilder,
                             std::shared_ptr<PokemonData> pokemonData,
                             const std::string& playerName) {
  std::cout << "\n🎖️ Elite Four Challenge\n";
  std::cout << "══════════════════════\n\n";
  
  auto availableChallenges = tournamentManager->getAvailableChallenges(playerName);
  std::vector<TournamentManager::ChallengeInfo> eliteFourChallenges;
  
  // Filter for Elite Four challenges
  for (const auto& challenge : availableChallenges) {
    if (challenge.challenge_type == "elite_four" && !challenge.is_completed) {
      eliteFourChallenges.push_back(challenge);
    }
  }

  if (eliteFourChallenges.empty()) {
    std::cout << "🎉 Congratulations! You have defeated all Elite Four members!\n";
    std::cout << "The Championship awaits you!\n";
    std::cout << "Press Enter to continue...";
    std::cin.get();
    return;
  }

  std::cout << "Elite Four Members Available:\n";
  for (size_t i = 0; i < eliteFourChallenges.size(); ++i) {
    const auto& member = eliteFourChallenges[i];
    std::cout << "  [" << (i + 1) << "] " << member.challenge_name << " (" << member.difficulty_level << ")\n";
    std::cout << "      Specialization: " << member.challenge_type << "\n";
    std::cout << "      Challenge Level: Elite Four\n\n";
  }

  std::cout << "  [" << (eliteFourChallenges.size() + 1) << "] ↩️ Back to Tournament Menu\n\n";

  auto eliteValidator = [&eliteFourChallenges](std::istream& input) -> InputValidator::ValidationResult<int> {
    return InputValidator::getValidatedInt(input, 1, static_cast<int>(eliteFourChallenges.size() + 1));
  };

  auto eliteResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "Select Elite Four member to challenge (1-" + std::to_string(eliteFourChallenges.size() + 1) + ")",
    2, eliteValidator
  );

  if (!eliteResult.isValid() || eliteResult.value > static_cast<int>(eliteFourChallenges.size())) {
    return; // Back to tournament menu
  }

  const auto& selectedMember = eliteFourChallenges[eliteResult.value - 1];
  
  // Conduct tournament battle
  conductTournamentBattle(tournamentManager, teamBuilder, pokemonData, playerName, selectedMember);
}

// Helper function to handle championship battle
void handleChampionshipBattle(std::shared_ptr<TournamentManager> tournamentManager,
                             std::shared_ptr<TeamBuilder> teamBuilder,
                             std::shared_ptr<PokemonData> pokemonData,
                             const std::string& playerName) {
  std::cout << "\n👑 Championship Battle\n";
  std::cout << "════════════════════\n\n";
  
  std::cout << "You stand before the ultimate challenge - the Pokemon Champion!\n";
  std::cout << "This is the culmination of your tournament journey.\n\n";
  
  std::cout << "Are you ready to face the Champion? (y/n): ";
  char ready;
  std::cin >> ready;
  std::cin.ignore();
  
  if (ready != 'y' && ready != 'Y') {
    std::cout << "Return when you're ready for the ultimate challenge.\n";
    std::cout << "Press Enter to continue...";
    std::cin.get();
    return;
  }

  // Create championship challenge
  TournamentManager::ChallengeInfo championChallenge;
  championChallenge.challenge_name = "Pokemon Champion";
  championChallenge.challenge_type = "champion";
  championChallenge.difficulty_level = "Expert";
  championChallenge.reward_description = "Pokemon Tournament Champion Title";
  championChallenge.is_unlocked = true;
  championChallenge.is_completed = false;

  // Conduct championship battle
  conductTournamentBattle(tournamentManager, teamBuilder, pokemonData, playerName, championChallenge);
}

// Helper function to display tournament statistics
void displayTournamentStatistics(std::shared_ptr<TournamentManager> tournamentManager, 
                                const std::string& playerName) {
  std::cout << "\n📊 Tournament Statistics\n";
  std::cout << "═══════════════════════\n\n";

  auto stats = tournamentManager->getPlayerTournamentStats(playerName);
  auto battleHistory = tournamentManager->getPlayerBattleHistory(playerName);

  std::cout << "📈 Overall Performance:\n";
  for (const auto& [statName, value] : stats) {
    std::cout << "  " << statName << ": " << static_cast<int>(value) << "\n";
  }

  std::cout << "\n🏆 Battle History (Last 10 battles):\n";
  size_t displayCount = std::min(battleHistory.size(), size_t(10));
  for (size_t i = 0; i < displayCount; ++i) {
    const auto& battle = battleHistory[battleHistory.size() - 1 - i]; // Show most recent first
    std::cout << "  " << (battle.victory ? "✅" : "❌") << " " << battle.challenge_name 
              << " (" << battle.difficulty_level << ") - Score: " 
              << static_cast<int>(battle.performance_score) << "/100\n";
  }

  std::cout << "\nPress Enter to continue...";
  std::cin.get();
}

// Helper function to display tournament leaderboard
void displayTournamentLeaderboard(std::shared_ptr<TournamentManager> tournamentManager) {
  std::cout << "\n🏆 Tournament Leaderboard\n";
  std::cout << "════════════════════════\n\n";

  auto leaderboard = tournamentManager->getTournamentLeaderboard("completion", 10);

  std::cout << "Top Tournament Trainers:\n";
  for (size_t i = 0; i < leaderboard.size(); ++i) {
    const auto& [playerName, score] = leaderboard[i];
    std::cout << "  " << (i + 1) << ". " << playerName << " - " 
              << static_cast<int>(score * 100) << "% completion\n";
  }

  std::cout << "\nPress Enter to continue...";
  std::cin.get();
}

// Helper function to conduct tournament battles
void conductTournamentBattle(std::shared_ptr<TournamentManager> tournamentManager,
                            std::shared_ptr<TeamBuilder> teamBuilder,
                            std::shared_ptr<PokemonData> pokemonData,
                            const std::string& playerName,
                            const TournamentManager::ChallengeInfo& challenge) {
  std::cout << "\n⚔️ Preparing for battle against " << challenge.challenge_name << "!\n";
  std::cout << "Difficulty: " << challenge.difficulty_level << "\n\n";

  // Team selection for tournament battle
  std::cout << "📝 Tournament Battle Team Selection:\n";
  std::cout << "  [1] Use existing team\n";
  std::cout << "  [2] Build new team\n";
  std::cout << "  [3] Load custom team\n\n";

  auto teamValidator = [](std::istream& input) -> InputValidator::ValidationResult<int> {
    return InputValidator::getValidatedInt(input, 1, 3);
  };

  auto teamResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "Select team option (1-3)",
    2, teamValidator
  );

  TeamBuilder::Team playerTeam;
  
  switch (teamResult.isValid() ? teamResult.value : 1) {
    case 1:
      // Use a random team for now (in full implementation, would save last used team)
      playerTeam = teamBuilder->generateRandomTeam(playerName + "'s Tournament Team");
      break;
    case 2:
      playerTeam = handleCustomTeamBuilder(teamBuilder, pokemonData, playerName);
      break;
    case 3:
      playerTeam = handleCustomTeamLoad(teamBuilder, playerName);
      break;
    default:
      playerTeam = teamBuilder->generateRandomTeam(playerName + "'s Tournament Team");
      break;
  }

  // Generate opponent team based on challenge
  TeamBuilder::Team opponentTeam;
  std::string templateCategory = "competitive";
  std::string templateName = "balanced_meta";
  
  // Map challenge types to templates
  if (challenge.challenge_type == "gym") {
    templateCategory = "type_themed";
    if (challenge.challenge_name.find("Brock") != std::string::npos) templateName = "rock_team";
    else if (challenge.challenge_name.find("Misty") != std::string::npos) templateName = "water_team";
    else if (challenge.challenge_name.find("Surge") != std::string::npos) templateName = "electric_team";
    else if (challenge.challenge_name.find("Erika") != std::string::npos) templateName = "grass_team";
    else if (challenge.challenge_name.find("Koga") != std::string::npos) templateName = "psychic_team";
    else if (challenge.challenge_name.find("Sabrina") != std::string::npos) templateName = "psychic_team";
    else if (challenge.challenge_name.find("Blaine") != std::string::npos) templateName = "fire_team";
    else if (challenge.challenge_name.find("Giovanni") != std::string::npos) templateName = "balanced_meta";
  }

  opponentTeam = teamBuilder->generateTeamFromTemplate(templateCategory, templateName, challenge.challenge_name);
  if (opponentTeam.pokemon.empty()) {
    opponentTeam = teamBuilder->generateRandomTeam(challenge.challenge_name);
  }

  // Convert to battle format
  Team battlePlayerTeam = createBattleTeamFromTemplate(playerTeam, teamBuilder);
  Team battleOpponentTeam = createBattleTeamFromTemplate(opponentTeam, teamBuilder);

  // Determine AI difficulty based on challenge
  Battle::AIDifficulty aiDifficulty = Battle::AIDifficulty::MEDIUM;
  if (challenge.difficulty_level == "Easy") aiDifficulty = Battle::AIDifficulty::EASY;
  else if (challenge.difficulty_level == "Medium") aiDifficulty = Battle::AIDifficulty::MEDIUM;
  else if (challenge.difficulty_level == "Hard") aiDifficulty = Battle::AIDifficulty::HARD;
  else if (challenge.difficulty_level == "Expert") aiDifficulty = Battle::AIDifficulty::EXPERT;

  // Start the battle
  std::cout << "\n🎉 Tournament Battle Beginning!\n";
  std::cout << "Player: " << playerName << " vs " << challenge.challenge_name << "\n\n";

  auto battleStart = std::chrono::steady_clock::now();
  Battle battle(battlePlayerTeam, battleOpponentTeam, aiDifficulty);
  battle.startBattle();
  auto battleEnd = std::chrono::steady_clock::now();

  // Calculate battle results
  auto duration = std::chrono::duration_cast<std::chrono::seconds>(battleEnd - battleStart);
  int estimatedTurns = std::max(1, static_cast<int>(duration.count() / 3));
  
  Battle::BattleResult battleResult = battle.getBattleResult();
  bool victory = (battleResult == Battle::BattleResult::PLAYER_WINS);
  bool isDraw = (battleResult == Battle::BattleResult::DRAW);
  
  // Calculate performance score
  double performanceScore = 50.0;
  if (victory) {
    performanceScore += 40.0;
    if (estimatedTurns < 10) performanceScore += 10.0;
  } else if (isDraw) {
    performanceScore += 15.0;
  }
  performanceScore = std::max(0.0, std::min(100.0, performanceScore));

  // Create tournament battle result
  TournamentManager::TournamentBattleResult tournamentResult;
  tournamentResult.challenge_name = challenge.challenge_name;
  tournamentResult.challenge_type = challenge.challenge_type;
  tournamentResult.player_team_name = playerTeam.name;
  tournamentResult.opponent_name = challenge.challenge_name;
  tournamentResult.victory = victory;
  tournamentResult.turns_taken = estimatedTurns;
  tournamentResult.difficulty_level = challenge.difficulty_level;
  tournamentResult.performance_score = performanceScore;
  tournamentResult.battle_date = "2025-08-23"; // Current date

  // Update tournament progress
  if (victory) {
    tournamentManager->updatePlayerProgress(playerName, tournamentResult);
    
    if (challenge.challenge_type == "gym") {
      // Award badge
      TournamentManager::Badge badge(
        challenge.challenge_name,
        "unknown", // Would be extracted from challenge data in full implementation
        challenge.challenge_name,
        tournamentResult.battle_date,
        1,
        performanceScore
      );
      tournamentManager->awardBadge(playerName, badge);
      
      std::cout << "\n🏅 Congratulations! You earned a new badge!\n";
      std::cout << "Badge: " << challenge.challenge_name << "\n";
    }
    
    std::cout << "\n🎉 Victory! You defeated " << challenge.challenge_name << "!\n";
  } else if (isDraw) {
    std::cout << "\n🤝 Draw! Both teams fought valiantly!\n";
  } else {
    std::cout << "\n💪 Good effort! " << challenge.challenge_name << " proved to be a tough opponent.\n";
    std::cout << "Keep training and try again!\n";
  }
  
  std::cout << "Performance Score: " << static_cast<int>(performanceScore) << "/100\n";
  std::cout << "\nPress Enter to continue...";
  std::cin.get();
}

int main() {
  // Set up signal handler for graceful interruption
  signal(SIGINT, signalHandler);
  
  // Get user's name with validation and sanitization
  std::string userName;
  auto nameResult = InputValidator::getValidatedString(
    std::cin, 0, 50, true, "Enter your name"
  );
  
  if (nameResult.isValid()) {
    userName = InputValidator::sanitizeString(nameResult.value);
    if (userName.empty()) {
      userName = "Trainer";
    }
  } else {
    std::cout << "Using default name due to input error: " << nameResult.errorMessage << std::endl;
    userName = "Trainer";
  }
  
  std::cout << "\nWelcome, " << userName << "!" << std::endl;

  // Initialize Pokemon data and team builder
  std::shared_ptr<PokemonData> pokemonData = std::make_shared<PokemonData>();
  auto initResult = pokemonData->initialize();
  if (!initResult.success) {
    std::cout << "Error: Failed to initialize Pokemon data: " << initResult.error_message << std::endl;
    return 1;
  }
  std::cout << "📊 Loaded " << initResult.loaded_count << " Pokemon and moves successfully!" << std::endl;
  
  std::shared_ptr<TeamBuilder> teamBuilder = std::make_shared<TeamBuilder>(pokemonData);
  
  // Initialize Tournament Manager
  std::shared_ptr<TournamentManager> tournamentManager = std::make_shared<TournamentManager>(pokemonData, teamBuilder);
  
  std::cout << "\n╔════════════════════════════════════════════════════════════════════════════════╗\n";
  std::cout << "║                              Pokemon Battle Simulator                          ║\n";
  std::cout << "║                               Team Builder System                              ║\n";
  std::cout << "╚════════════════════════════════════════════════════════════════════════════════╝\n";

  // Get template categories
  auto categories = teamBuilder->getTemplateCategories();
  
  if (categories.empty()) {
    std::cout << "⚠️  No templates found. Using legacy team selection.\n";
    // Fall back to original team selection...
    // [Original team selection code would go here]
    return 1;
  }

  // Show categories and get user selection
  int categoryChoice = showTemplateCategories(categories);
  
  TeamBuilder::Team playerTeam;
  
  if (categoryChoice <= static_cast<int>(categories.size())) {
    // User selected a template category
    std::string selectedCategory = categories[categoryChoice - 1];
    auto templatesInCategory = teamBuilder->getTemplatesInCategory(selectedCategory);
    
    if (templatesInCategory.empty()) {
      std::cout << "⚠️  No templates found in this category. Generating random team.\n";
      playerTeam = teamBuilder->generateRandomTeam(userName + "'s Team");
    } else {
      // Show templates in category
      int templateChoice = showTemplatesInCategory(templatesInCategory, selectedCategory);
      std::string selectedTemplate = templatesInCategory[templateChoice - 1];
      
      // Get template details and show to user
      auto templateData = teamBuilder->getTemplate(selectedCategory, selectedTemplate);
      if (templateData) {
        displayTemplateInfo(*templateData);
        
        // Ask for confirmation
        std::cout << "Would you like to use this template? (y/n): ";
        char confirm;
        std::cin >> confirm;
        std::cin.ignore();
        
        if (confirm == 'y' || confirm == 'Y') {
          playerTeam = teamBuilder->generateTeamFromTemplate(selectedCategory, selectedTemplate, userName + "'s Team");
        } else {
          std::cout << "Generating random team instead...\n";
          playerTeam = teamBuilder->generateRandomTeam(userName + "'s Team");
        }
      } else {
        std::cout << "⚠️  Template not found. Generating random team.\n";
        playerTeam = teamBuilder->generateRandomTeam(userName + "'s Team");
      }
    }
  } else if (categoryChoice == static_cast<int>(categories.size() + 1)) {
    // Build custom team
    playerTeam = handleCustomTeamBuilder(teamBuilder, pokemonData, userName);
  } else if (categoryChoice == static_cast<int>(categories.size() + 2)) {
    // Generate random team
    std::cout << "\n🎲 Random Team Generator\n";
    
    // Ask for team size
    auto sizeValidator = [](std::istream& input) -> InputValidator::ValidationResult<int> {
      return InputValidator::getValidatedInt(input, 1, 6);
    };
    
    auto sizeResult = InputValidator::promptWithRetry<int>(
      std::cin, std::cout,
      "How many Pokemon do you want? (1-6)",
      2, sizeValidator
    );
    
    int teamSize = sizeResult.isValid() ? sizeResult.value : 6;
    playerTeam = teamBuilder->generateRandomTeam(userName + "'s Team", teamSize);
  } else if (categoryChoice == static_cast<int>(categories.size() + 3)) {
    // Tournament Draft Mode
    playerTeam = handleTournamentDraft(teamBuilder, userName);
  } else if (categoryChoice == static_cast<int>(categories.size() + 4)) {
    // Tournament Mode
    enterTournamentMode(tournamentManager, teamBuilder, pokemonData, userName);
    // Tournament mode has its own battle system, so we can return after this
    return 0;
  } else if (categoryChoice == static_cast<int>(categories.size() + 5)) {
    // Import Team from Share Code
    playerTeam = handleTeamImport(teamBuilder, userName);
  } else if (categoryChoice == static_cast<int>(categories.size() + 6)) {
    // Load Custom Team
    playerTeam = handleCustomTeamLoad(teamBuilder, userName);
  } else if (categoryChoice == static_cast<int>(categories.size() + 7)) {
    // Team Comparison Tool
    handleTeamComparison(teamBuilder);
    // After comparison, still need a team for battle
    std::cout << "\nGenerating random team for battle...\n";
    playerTeam = teamBuilder->generateRandomTeam(userName + "'s Team");
  } else {
    // Default fallback
    playerTeam = teamBuilder->generateRandomTeam(userName + "'s Team");
  }

  // Validate the team
  if (!teamBuilder->validateTeam(playerTeam)) {
    std::cout << "⚠️  Team validation failed. Errors:\n";
    for (const auto& error : playerTeam.validation_errors) {
      std::cout << "  - " << error << "\n";
    }
    std::cout << "Generating a backup random team...\n";
    playerTeam = teamBuilder->generateRandomTeam(userName + "'s Team");
  }

  // Convert TeamBuilder team to Battle team format
  Team PlayerTeam = createBattleTeamFromTemplate(playerTeam, teamBuilder);

  // Display player's team
  std::cout << "\n========================================================== Your Team ==========================================================\n";
  std::cout << "\nYour team '" << playerTeam.name << "' includes:\n";
  for (const auto& pokemon : playerTeam.pokemon) {
    std::cout << "- " << pokemon.name << "\n  Moves: ";
    for (size_t i = 0; i < pokemon.moves.size(); ++i) {
      std::cout << pokemon.moves[i];
      if (i < pokemon.moves.size() - 1) std::cout << ", ";
    }
    std::cout << "\n";
  }
  
  // Team management options
  std::cout << "\n📋 Team Options:\n";
  std::cout << "  [1] 📤 Get Share Code\n";
  std::cout << "  [2] 💾 Save as Custom Team\n";
  std::cout << "  [3] 📊 View Team Statistics\n";
  std::cout << "  [4] ➡️  Continue to Battle\n\n";
  
  auto teamOptionValidator = [](std::istream& input) -> InputValidator::ValidationResult<int> {
    return InputValidator::getValidatedInt(input, 1, 4);
  };
  
  auto teamOptionResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "Select an option (1-4)",
    2, teamOptionValidator
  );
  
  if (teamOptionResult.isValid()) {
    switch (teamOptionResult.value) {
      case 1: {
        // Generate and display share code
        std::string shareCode = teamBuilder->exportTeamShareCode(playerTeam, userName, "Generated team");
        std::cout << "\n📤 Team Share Code:\n";
        std::cout << "════════════════════\n";
        std::cout << shareCode << "\n";
        std::cout << "════════════════════\n";
        std::cout << "💡 Share this code with others to let them import your team!\n\n";
        std::cout << "Press Enter to continue...";
        std::cin.get();
        break;
      }
      case 2: {
        // Save as custom team
        if (teamBuilder->saveCustomTeam(playerTeam)) {
          std::cout << "✅ Team saved successfully to custom teams directory!\n";
        } else {
          std::cout << "❌ Failed to save team to custom directory.\n";
        }
        std::cout << "Press Enter to continue...";
        std::cin.get();
        break;
      }
      case 3: {
        // View team statistics
        auto stats = teamBuilder->getTeamStatistics(playerTeam.name);
        if (stats) {
          std::cout << "\n📊 Team Statistics for '" << stats->team_name << "':\n";
          std::cout << "═══════════════════════════════════════\n";
          std::cout << "Total Battles: " << stats->total_battles << "\n";
          std::cout << "Victories: " << stats->victories << "\n";
          std::cout << "Defeats: " << stats->defeats << "\n";
          std::cout << "Win Rate: " << static_cast<int>(stats->win_rate) << "%\n";
          std::cout << "Average Battle Length: " << static_cast<int>(stats->average_battle_length) << " turns\n";
          std::cout << "Average Effectiveness: " << static_cast<int>(stats->average_effectiveness_score) << "/100\n";
        } else {
          std::cout << "\n📊 No battle statistics found for this team.\n";
          std::cout << "Statistics will be recorded after battles.\n";
        }
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        break;
      }
      case 4:
      default:
        // Continue to battle selection
        break;
    }
  }
  
  std::cout << std::endl;

  // Generate opponent teams from templates
  std::vector<TeamBuilder::Team> opponentTemplateTeams;
  std::vector<std::string> opponentNames = {
    "Rock Fortress", "Water Masters", "Electric Storm", "Grass Garden", 
    "Poison Shadows", "Psychic Mind", "Fire Blaze", "Ground Earthquake"
  };
  
  // Generate diverse opponent teams (with fallbacks to random teams if templates don't exist)
  auto tryTemplate = [&](const std::string& category, const std::string& template_name, const std::string& team_name) -> TeamBuilder::Team {
    auto team = teamBuilder->generateTeamFromTemplate(category, template_name, team_name);
    if (team.pokemon.empty()) {
      std::cout << "Template " << template_name << " not found, generating random team for " << team_name << "\n";
      return teamBuilder->generateRandomTeam(team_name);
    }
    return team;
  };
  
  opponentTemplateTeams.push_back(tryTemplate("type_themed", "rock_team", opponentNames[0]));
  opponentTemplateTeams.push_back(tryTemplate("type_themed", "water_team", opponentNames[1]));
  opponentTemplateTeams.push_back(tryTemplate("type_themed", "electric_team", opponentNames[2]));
  opponentTemplateTeams.push_back(tryTemplate("type_themed", "grass_team", opponentNames[3]));
  opponentTemplateTeams.push_back(tryTemplate("competitive", "fortress_stall", opponentNames[4]));
  opponentTemplateTeams.push_back(tryTemplate("type_themed", "psychic_team", opponentNames[5]));
  opponentTemplateTeams.push_back(tryTemplate("type_themed", "fire_team", opponentNames[6]));
  opponentTemplateTeams.push_back(tryTemplate("competitive", "balanced_meta", opponentNames[7]));

  // Available teams - using auto to reduce verbosity  
  const auto selectedTeams = std::unordered_map<std::string,
                                                std::vector<std::string>>{
      // Player Pokemon (fallback)
      {playerTeam.name, {}}, // Will be populated from template

      // Opponent Team Pokemon - Generated from templates
      {"Opponent Team 1", {}}, // Rock team
      {"Opponent Team 2", {}}, // Water team
      {"Opponent Team 3", {}}, // Electric team
      {"Opponent Team 4", {}}, // Grass team
      {"Opponent Team 5", {}}, // Poison/Stall team
      {"Opponent Team 6", {}}, // Psychic team
      {"Opponent Team 7", {}}, // Fire team
      {"Opponent Team 8", {}}, // Ground/Balanced team
  };

  // Template-based move generation is handled by TeamBuilder, so we don't need the large selectedMoves structure

  std::cout << "==============================================================="
               "==============================================================="
            << std::endl;
  std::cout << "" << std::endl;

  // Show available opponent teams
  std::cout << "🏆 Available Gym Leaders:\n" << std::endl;
  std::cout << "  [1] 🪨 Brock (Rock Specialist)" << std::endl;
  std::cout << "  [2] 💧 Misty (Water Specialist)" << std::endl;
  std::cout << "  [3] ⚡ Surge (Electric Specialist)" << std::endl;
  std::cout << "  [4] 🌿 Erika (Grass Specialist)" << std::endl;
  std::cout << "  [5] ☠️  Koga (Stall Specialist)" << std::endl;
  std::cout << "  [6] 🔮 Sabrina (Psychic Specialist)" << std::endl;
  std::cout << "  [7] 🔥 Blaine (Fire Specialist)" << std::endl;
  std::cout << "  [8] 🌍 Giovanni (Balanced Meta)\n" << std::endl;

  // Prompt for opponent selection with secure validation
  auto opponentValidator = [](std::istream& input) -> InputValidator::ValidationResult<int> {
    return InputValidator::getValidatedInt(input, 1, 8);
  };
  
  auto opponentResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "⚔️  Enter the number of your chosen opponent (1-8)",
    2, opponentValidator
  );
  
  if (!opponentResult.isValid()) {
    std::cout << "Failed to get valid opponent selection after multiple attempts: "
              << opponentResult.errorMessage << std::endl;
    std::cout << "Defaulting to Opponent 1 (Brock)." << std::endl;
    opponentResult = InputValidator::ValidationResult<int>(1);
  }
  
  int chosenOpponentNum = opponentResult.value;

  // Get the opponent team from templates
  TeamBuilder::Team opponentBuilderTeam = opponentTemplateTeams[chosenOpponentNum - 1];
  Team OppTeam = createBattleTeamFromTemplate(opponentBuilderTeam, teamBuilder);

  std::cout << "\nYou have selected " << opponentBuilderTeam.name << std::endl;

  std::cout << "" << std::endl;
  std::cout
      << "========================================================== Opponent "
         "Team =========================================================="
      << std::endl;
  std::cout << "" << std::endl;

  // Print out Opponent's team
  std::cout << "Opponent's team '" << opponentBuilderTeam.name << "' includes:\n";
  for (const auto& pokemon : opponentBuilderTeam.pokemon) {
    std::cout << "- " << pokemon.name << "\n";
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

  // Prompt for difficulty selection with secure validation
  auto difficultyValidator = [](std::istream& input) -> InputValidator::ValidationResult<int> {
    return InputValidator::getValidatedInt(input, 1, 4);
  };
  
  auto difficultyResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "🎮 Enter the difficulty level (1-4)",
    2, difficultyValidator
  );
  
  if (!difficultyResult.isValid()) {
    std::cout << "Failed to get valid difficulty selection after multiple attempts: "
              << difficultyResult.errorMessage << std::endl;
    std::cout << "Defaulting to Easy difficulty." << std::endl;
    difficultyResult = InputValidator::ValidationResult<int>(1);
  }
  
  int chosenDifficulty = difficultyResult.value;

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
  
  // Record pre-battle information
  std::string difficultyStr;
  switch (chosenDifficulty) {
    case 1: difficultyStr = "Easy"; break;
    case 2: difficultyStr = "Medium"; break;
    case 3: difficultyStr = "Hard"; break;
    case 4: difficultyStr = "Expert"; break;
    default: difficultyStr = "Easy"; break;
  }
  
  auto battle_start_time = std::chrono::steady_clock::now();
  battle.startBattle();
  auto battle_end_time = std::chrono::steady_clock::now();
  
  // Calculate battle duration (simplified turn estimation)
  auto duration = std::chrono::duration_cast<std::chrono::seconds>(battle_end_time - battle_start_time);
  int estimated_turns = std::max(1, static_cast<int>(duration.count() / 3)); // Rough estimate: 3 seconds per turn
  
  // Get actual battle result
  Battle::BattleResult battle_result = battle.getBattleResult();
  bool player_victory = (battle_result == Battle::BattleResult::PLAYER_WINS);
  bool is_draw = (battle_result == Battle::BattleResult::DRAW);
  
  // Calculate effectiveness score based on battle performance
  double effectiveness_score = 50.0; // Base score
  if (player_victory) {
    effectiveness_score += 30.0; // Victory bonus
    if (estimated_turns < 10) effectiveness_score += 10.0; // Quick victory bonus
    if (difficultyStr == "Expert") effectiveness_score += 10.0; // Difficulty bonus
  } else if (is_draw) {
    effectiveness_score += 10.0; // Draw bonus (shows competitive performance)
    if (difficultyStr == "Expert") effectiveness_score += 5.0; // Difficulty bonus for surviving
  } else {
    effectiveness_score -= 20.0; // Loss penalty
  }
  effectiveness_score = std::max(0.0, std::min(100.0, effectiveness_score));
  
  // Record battle result
  teamBuilder->recordBattleResult(
    playerTeam.name,
    opponentBuilderTeam.name,
    player_victory,
    estimated_turns,
    difficultyStr,
    effectiveness_score
  );
  
  // Post-battle summary
  std::cout << "\n" << std::string(80, '=') << "\n";
  std::cout << "🏁 BATTLE COMPLETE!\n";
  std::cout << std::string(80, '=') << "\n";
  
  if (player_victory) {
    std::cout << "🎉 Congratulations! You defeated " << opponentBuilderTeam.name << "!\n";
  } else if (is_draw) {
    std::cout << "🤝 It's a draw! Both teams fought valiantly to the very end!\n";
  } else {
    std::cout << "💪 Good effort! " << opponentBuilderTeam.name << " proved to be a worthy opponent.\n";
  }
  
  std::cout << "\n📊 Battle Summary:\n";
  std::cout << "Team: " << playerTeam.name << "\n";
  std::cout << "Opponent: " << opponentBuilderTeam.name << "\n";
  std::cout << "Difficulty: " << difficultyStr << "\n";
  std::cout << "Estimated Turns: " << estimated_turns << "\n";
  std::cout << "Effectiveness Score: " << static_cast<int>(effectiveness_score) << "/100\n";
  
  // Show updated team statistics
  auto updated_stats = teamBuilder->getTeamStatistics(playerTeam.name);
  if (updated_stats) {
    std::cout << "\n📈 Updated Team Statistics:\n";
    std::cout << "Total Battles: " << updated_stats->total_battles << "\n";
    std::cout << "Win Rate: " << static_cast<int>(updated_stats->win_rate) << "%\n";
    std::cout << "Average Effectiveness: " << static_cast<int>(updated_stats->average_effectiveness_score) << "/100\n";
  }
  
  std::cout << "\n💡 Want to try different teams or face other opponents?\n";
  std::cout << "Restart the application to access all the advanced team building features!\n\n";
  
  std::cout << "Thanks for playing the Pokemon Battle Simulator!\n";
  std::cout << "🎮 Advanced Team Builder - Phase 4 Implementation Complete! 🎮\n";
}