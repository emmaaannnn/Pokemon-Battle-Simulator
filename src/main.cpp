#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>
#include <csignal>
#include <functional>
#include <memory>

#include "battle.h"
#include "input_validator.h"
#include "team_builder.h"
#include "pokemon_data.h"

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
  std::cout << "  [" << (categories.size() + 2) << "] Generate Random Team\n\n";
  
  auto categoryValidator = [categories](std::istream& input) -> InputValidator::ValidationResult<int> {
    return InputValidator::getValidatedInt(input, 1, static_cast<int>(categories.size() + 2));
  };
  
  auto categoryResult = InputValidator::promptWithRetry<int>(
    std::cin, std::cout,
    "📝 Select a category (1-" + std::to_string(categories.size() + 2) + ")",
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
  std::shared_ptr<TeamBuilder> teamBuilder = std::make_shared<TeamBuilder>(pokemonData);
  
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
    std::cout << "\n🔨 Custom Team Builder\n";
    std::cout << "This feature is coming soon! Using random team for now.\n";
    playerTeam = teamBuilder->generateRandomTeam(userName + "'s Team");
  } else {
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
  battle.startBattle();
}