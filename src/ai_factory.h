#pragma once

#include <memory>

#include "ai_strategy.h"

class AIFactory {
 public:
  static std::unique_ptr<AIStrategy> createAI(AIDifficulty difficulty);
};