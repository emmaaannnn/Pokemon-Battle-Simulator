#include "ai_factory.h"

#include "easy_ai.h"
#include "expert_ai.h"
#include "hard_ai.h"
#include "medium_ai.h"

std::unique_ptr<AIStrategy> AIFactory::createAI(AIDifficulty difficulty) {
  switch (difficulty) {
    case AIDifficulty::EASY:
      return std::make_unique<EasyAI>();
    case AIDifficulty::MEDIUM:
      return std::make_unique<MediumAI>();
    case AIDifficulty::HARD:
      return std::make_unique<HardAI>();
    case AIDifficulty::EXPERT:
      return std::make_unique<ExpertAI>();
    default:
      return std::make_unique<EasyAI>();  // Default fallback
  }
}