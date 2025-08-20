# Multi-Turn Move System Developer Guide
# Pokemon Battle Simulator

**Guide Version:** 1.0  
**Date:** August 20, 2025  
**Target Audience:** Developers, Contributors, System Maintainers  

## Introduction

This guide provides comprehensive information for developers working with the Multi-Turn Move System in the Pokemon Battle Simulator. Whether you're adding new multi-turn moves, extending existing functionality, or maintaining the system, this guide covers all technical aspects with practical examples and best practices.

## Architecture Deep Dive

### System Components Overview

The Multi-Turn Move System is built on a modular architecture that integrates with existing battle mechanics:

```cpp
// Core Components Hierarchy
MultiTurnBehavior (enum) 
    ↓
Move Class (move.h/cpp)
    ↓ 
Pokemon State Management (pokemon.h/cpp)
    ↓
Battle System Integration (battle.h/cpp)
    ↓
AI Integration (all AI classes)
```

### Key Design Patterns

#### 1. State Pattern Implementation
The system uses a state pattern to manage multi-turn sequences:

```cpp
// Pokemon state representation
class Pokemon {
    // State variables
    bool is_charging;           // In charging phase
    bool must_recharge;         // In recharge phase  
    int charging_move_index;    // Move being charged
    std::string charging_move_name; // For UI display
    
    // State transition methods
    void startCharging(int moveIndex, const std::string& moveName);
    void finishCharging();
    void startRecharge();
    void finishRecharge();
    bool isInMultiTurnSequence() const;
};
```

#### 2. Strategy Pattern for Behaviors
Different multi-turn behaviors are handled through enumeration-based strategy:

```cpp
enum class MultiTurnBehavior {
    NONE,           // No multi-turn behavior
    RECHARGE,       // Post-move recharge required
    CHARGE,         // Pre-move charging required  
    CHARGE_BOOST    // Charging with stat modification
};
```

#### 3. Template-Based Configuration
Move properties are configured through JSON with type-safe loading:

```cpp
class Move {
    // Multi-turn properties loaded from JSON
    MultiTurnBehavior multi_turn_behavior;
    bool is_weather_dependent;
    bool boosts_defense_on_charge;
    
    // Utility methods for behavior checking
    bool isMultiTurnMove() const;
    bool requiresCharging() const;
    bool requiresRecharge() const;
};
```

### Data Flow Architecture

```
JSON Move Data → Move Constructor → Battle System → Pokemon State → AI Decision
      ↓                ↓               ↓             ↓              ↓
  Configuration   Type Safety    Turn Management  State Tracking  Constraint
   Validation                                                      Handling
```

## Adding New Multi-Turn Moves

### Step 1: Create Move Data File

Create a JSON configuration file in `/data/moves/` directory:

```json
{
    "name": "meteor-mash",
    "accuracy": 90,
    "effect_chance": null,
    "pp": 10,
    "priority": 0,
    "power": 100,
    "damage_class": {
        "name": "physical",
        "url": "https://pokeapi.co/api/v2/move-damage-class/2/"
    },
    "effect_entries": [
        {
            "effect": "Inflicts regular damage. User charges for one turn, then attacks with increased critical hit rate.",
            "language": {
                "name": "en", 
                "url": "https://pokeapi.co/api/v2/language/9/"
            },
            "short_effect": "Charges one turn, then attacks with high crit rate."
        }
    ],
    "Info": {
        "ailment": {
            "name": "none",
            "url": "https://pokeapi.co/api/v2/move-ailment/0/"
        },
        "ailment_chance": 0,
        "category": {
            "name": "damage",
            "url": "https://pokeapi.co/api/v2/move-category/0/"
        },
        "crit_rate": 2,
        "drain": 0,
        "flinch_chance": 0,
        "healing": 0,
        "max_hits": null,
        "max_turns": null,
        "min_hits": null,
        "min_turns": null,
        "stat_chance": 0,
        "multi_turn_behavior": "charge",
        "charge_required": true,
        "increased_crit_rate": true
    }
}
```

### Step 2: Extend Move Class (if needed)

For moves with unique properties, extend the Move class:

```cpp
// In move.h - Add new property if needed
class Move {
public:
    // Existing properties...
    bool increases_crit_rate;    // New property for Meteor Mash
    
    // Add accessor method
    bool hasIncreasedCritRate() const { return increases_crit_rate; }
};

// In move.cpp - Load property from JSON
void Move::loadFromJson(const std::string &file_path) {
    // Existing loading code...
    
    // Load new property
    if (moveInfo.contains("increased_crit_rate")) {
        increases_crit_rate = moveInfo["increased_crit_rate"];
    } else {
        increases_crit_rate = false;
    }
}
```

### Step 3: Implement Battle Logic

Add handling in the Battle class for new move properties:

```cpp
// In battle.cpp - executeMove method
void Battle::executeMove(Pokemon &attacker, Pokemon &defender, int moveIndex) {
    Move &move = attacker.moves[moveIndex];
    
    // Existing multi-turn handling...
    
    // Handle new move-specific effects
    if (move.name == "meteor-mash" && attacker.is_charging) {
        std::cout << attacker.name << " is gathering cosmic energy!" << std::endl;
        // Apply any charging-phase effects
    }
    
    // Modify critical hit calculation for moves with increased crit rate
    if (move.hasIncreasedCritRate()) {
        // Enhanced critical hit logic in damage calculation
    }
}

// In damage calculation
bool Battle::isCriticalHit(const Move &move) const {
    int critLevel = move.crit_rate;
    
    // Enhanced crit rate for specific moves
    if (move.hasIncreasedCritRate()) {
        critLevel += 2;  // Significant crit rate increase
    }
    
    // Calculate critical hit probability based on level
    double critChance = calculateCriticalChance(critLevel);
    return (static_cast<double>(rand()) / RAND_MAX) < critChance;
}
```

### Step 4: Add to Pokemon Movesets

Include the new move in relevant Pokemon data files:

```json
{
    "name": "metagross",
    "id": 376,
    "moves": [
        "meteor-mash",
        "earthquake", 
        "zen-headbutt",
        "hammer-arm"
    ]
}
```

### Step 5: Create Unit Tests

Implement comprehensive tests for the new move:

```cpp
// In tests/unit/test_move.cpp
TEST(MoveTest, MeteorMashMultiTurnBehavior) {
    Move meteorMash("meteor-mash");
    
    // Test multi-turn properties
    EXPECT_TRUE(meteorMash.isMultiTurnMove());
    EXPECT_TRUE(meteorMash.requiresCharging());
    EXPECT_FALSE(meteorMash.requiresRecharge());
    EXPECT_EQ(meteorMash.getMultiTurnBehavior(), MultiTurnBehavior::CHARGE);
    
    // Test enhanced crit rate
    EXPECT_TRUE(meteorMash.hasIncreasedCritRate());
    EXPECT_EQ(meteorMash.crit_rate, 2);
}

TEST(BattleTest, MeteorMashChargingSequence) {
    Pokemon attacker("metagross");
    Pokemon defender("alakazam");
    Battle battle(/* test setup */);
    
    // Find Meteor Mash move index
    int moveIndex = -1;
    for (int i = 0; i < attacker.moves.size(); ++i) {
        if (attacker.moves[i].name == "meteor-mash") {
            moveIndex = i;
            break;
        }
    }
    ASSERT_NE(moveIndex, -1);
    
    // Test charging phase
    int initialHP = defender.current_hp;
    battle.executeMove(attacker, defender, moveIndex);
    
    EXPECT_TRUE(attacker.is_charging);
    EXPECT_EQ(attacker.charging_move_index, moveIndex);
    EXPECT_EQ(defender.current_hp, initialHP);  // No damage during charge
    
    // Test execution phase
    battle.executeMove(attacker, defender, moveIndex);
    
    EXPECT_FALSE(attacker.is_charging);
    EXPECT_LT(defender.current_hp, initialHP);  // Damage applied
}
```

## Extending Multi-Turn Behaviors

### Adding New Behavior Types

To add completely new multi-turn behavior patterns:

#### Step 1: Extend Enumeration

```cpp
// In move.h
enum class MultiTurnBehavior {
    NONE,
    RECHARGE,
    CHARGE,
    CHARGE_BOOST,
    MULTI_HIT,      // New: Multiple turn execution
    DELAYED_EFFECT  // New: Effect applies after delay
};
```

#### Step 2: Update Move Class

```cpp
// Add new property accessors
class Move {
public:
    bool isMultiHitMove() const { 
        return multi_turn_behavior == MultiTurnBehavior::MULTI_HIT; 
    }
    bool hasDelayedEffect() const { 
        return multi_turn_behavior == MultiTurnBehavior::DELAYED_EFFECT; 
    }
};
```

#### Step 3: Extend Pokemon State

```cpp
// In pokemon.h - Add new state variables
class Pokemon {
public:
    // Existing state...
    int multi_hit_turns_remaining;   // For MULTI_HIT behavior
    int delayed_effect_countdown;    // For DELAYED_EFFECT behavior
    std::string delayed_effect_type; // Type of delayed effect
    
    // New state management methods
    void startMultiHit(int turns);
    void processMultiHit();
    void setDelayedEffect(const std::string& effectType, int delay);
    void processDelayedEffect();
};
```

#### Step 4: Implement Battle Logic

```cpp
// In battle.cpp
void Battle::executeMove(Pokemon &attacker, Pokemon &defender, int moveIndex) {
    Move &move = attacker.moves[moveIndex];
    
    // Handle new multi-turn behaviors
    switch (move.getMultiTurnBehavior()) {
        case MultiTurnBehavior::MULTI_HIT:
            handleMultiHitMove(attacker, defender, move);
            break;
            
        case MultiTurnBehavior::DELAYED_EFFECT:
            handleDelayedEffectMove(attacker, defender, move);
            break;
            
        // Existing cases...
    }
}

void Battle::handleMultiHitMove(Pokemon &attacker, Pokemon &defender, const Move &move) {
    if (attacker.multi_hit_turns_remaining == 0) {
        // Start multi-hit sequence
        attacker.startMultiHit(3);  // 3-turn attack sequence
        std::cout << attacker.name << " begins multi-hit attack!" << std::endl;
    }
    
    // Execute damage for this turn
    applyMoveDamage(attacker, defender, move);
    attacker.processMultiHit();
    
    if (attacker.multi_hit_turns_remaining > 0) {
        std::cout << "Attack continues! (" << attacker.multi_hit_turns_remaining 
                  << " turns remaining)" << std::endl;
    } else {
        std::cout << attacker.name << " completes multi-hit attack!" << std::endl;
    }
}
```

### Complex Multi-Turn Patterns

#### Example: Future Sight Pattern (Delayed Effect)

```cpp
// Move configuration for Future Sight
{
    "name": "future-sight",
    "power": 120,
    "pp": 10,
    "accuracy": 100,
    "Info": {
        "multi_turn_behavior": "delayed_effect",
        "effect_delay": 2,
        "delayed_damage": true,
        "cannot_miss_delayed": true
    }
}

// Implementation
void Battle::handleDelayedEffectMove(Pokemon &attacker, Pokemon &defender, const Move &move) {
    // Set up delayed effect
    defender.setDelayedEffect("future_sight_damage", 2);
    std::cout << attacker.name << " foresaw an attack!" << std::endl;
    
    // Store damage calculation for later
    int futureDamage = calculateDamage(attacker, defender, move);
    defender.delayed_effect_damage = futureDamage;
}

// In turn processing
void Battle::processTurnEnd() {
    // Process delayed effects for all Pokemon
    if (selectedPokemon->delayed_effect_countdown > 0) {
        selectedPokemon->processDelayedEffect();
    }
    if (opponentSelectedPokemon->delayed_effect_countdown > 0) {
        opponentSelectedPokemon->processDelayedEffect();
    }
}
```

## Performance Optimization

### Memory Management

#### Efficient State Storage

```cpp
// Optimize Pokemon state using bitfields for boolean flags
class Pokemon {
private:
    // Pack boolean flags into single byte
    struct MultiTurnState {
        bool is_charging : 1;
        bool must_recharge : 1;
        bool has_delayed_effect : 1;
        bool is_multi_hitting : 1;
        // 4 bits remaining for future flags
    } multi_turn_flags;
    
    // Use union for move index storage
    union {
        int charging_move_index;
        int multi_hit_move_index;
        int delayed_effect_move_index;
    } active_move_data;
};
```

#### String Optimization

```cpp
// Use string_view for temporary string operations
#include <string_view>

class Move {
    // Store commonly used strings statically
    static const std::unordered_map<MultiTurnBehavior, std::string_view> BEHAVIOR_NAMES;
    
public:
    std::string_view getBehaviorName() const {
        auto it = BEHAVIOR_NAMES.find(multi_turn_behavior);
        return (it != BEHAVIOR_NAMES.end()) ? it->second : "unknown";
    }
};

// Reduce string allocations in battle messages
void Battle::displayChargingMessage(const Pokemon& pokemon, std::string_view moveName) {
    // Use string formatting to avoid temporary string creation
    std::cout << pokemon.name << " is charging " << moveName << "!" << std::endl;
}
```

### Execution Optimization

#### Fast Path for Single-Turn Moves

```cpp
// Optimize executeMove for common case (single-turn moves)
void Battle::executeMove(Pokemon &attacker, Pokemon &defender, int moveIndex) {
    Move &move = attacker.moves[moveIndex];
    
    // Fast path for single-turn moves (most common case)
    if (move.getMultiTurnBehavior() == MultiTurnBehavior::NONE) {
        executeSingleTurnMove(attacker, defender, move);
        return;
    }
    
    // Multi-turn move handling (less common path)
    executeMultiTurnMove(attacker, defender, moveIndex);
}

inline void Battle::executeSingleTurnMove(Pokemon &attacker, Pokemon &defender, const Move &move) {
    // Streamlined execution for single-turn moves
    if (checkMoveAccuracy(move)) {
        applyMoveDamage(attacker, defender, move);
        applyMoveEffects(attacker, defender, move);
    }
}
```

#### Batch State Updates

```cpp
// Update multiple state variables in single operation
class Pokemon {
public:
    struct StateTransition {
        bool new_is_charging;
        bool new_must_recharge;
        int new_charging_move_index;
        std::string new_charging_move_name;
    };
    
    void applyStateTransition(const StateTransition& transition) {
        // Atomic update of all related state
        is_charging = transition.new_is_charging;
        must_recharge = transition.new_must_recharge;
        charging_move_index = transition.new_charging_move_index;
        charging_move_name = transition.new_charging_move_name;
    }
};
```

### Caching Strategies

#### Move Property Caching

```cpp
// Cache frequently accessed move properties
class Move {
private:
    mutable std::optional<bool> cached_is_multi_turn;
    
public:
    bool isMultiTurnMove() const {
        if (!cached_is_multi_turn.has_value()) {
            cached_is_multi_turn = (multi_turn_behavior != MultiTurnBehavior::NONE);
        }
        return cached_is_multi_turn.value();
    }
    
    // Invalidate cache when properties change
    void invalidateCache() {
        cached_is_multi_turn.reset();
    }
};
```

## Testing Frameworks

### Unit Testing Structure

#### Test Organization

```cpp
// tests/unit/test_multi_turn_moves.cpp
class MultiTurnMoveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common test setup
        attacker = std::make_unique<Pokemon>("test-attacker");
        defender = std::make_unique<Pokemon>("test-defender");
        battle = std::make_unique<Battle>(playerTeam, opponentTeam);
    }
    
    void TearDown() override {
        // Cleanup
        attacker.reset();
        defender.reset();
        battle.reset();
    }
    
    std::unique_ptr<Pokemon> attacker, defender;
    std::unique_ptr<Battle> battle;
    Team playerTeam, opponentTeam;
};

// Group tests by behavior type
class ChargeMovesTest : public MultiTurnMoveTest {};
class RechargeMovesTest : public MultiTurnMoveTest {};
class ChargeBoostMovesTest : public MultiTurnMoveTest {};
```

#### Parameterized Testing

```cpp
// Test all charging moves with same logic
class ChargeMoveParameterizedTest : public MultiTurnMoveTest,
                                   public ::testing::WithParamInterface<std::string> {};

TEST_P(ChargeMoveParameterizedTest, ChargingBehavior) {
    std::string moveName = GetParam();
    Move move(moveName);
    
    // Common charging behavior tests
    EXPECT_TRUE(move.requiresCharging());
    EXPECT_EQ(move.getMultiTurnBehavior(), MultiTurnBehavior::CHARGE);
    
    // Test charging sequence
    battle->executeMove(*attacker, *defender, 0);
    EXPECT_TRUE(attacker->is_charging);
    
    battle->executeMove(*attacker, *defender, 0);
    EXPECT_FALSE(attacker->is_charging);
}

INSTANTIATE_TEST_SUITE_P(
    AllChargeMoves,
    ChargeMoveParameterizedTest,
    ::testing::Values("solar-beam", "sky-attack", "skull-bash")
);
```

### Integration Testing

#### Battle Scenario Testing

```cpp
// tests/integration/test_multi_turn_battle_scenarios.cpp
class BattleScenarioTest : public ::testing::Test {
protected:
    void simulateFullBattle(const std::string& playerMoves, 
                           const std::string& opponentMoves) {
        // Simulate complete battle scenario
        // Return battle statistics for analysis
    }
    
    void verifyBattleState(const Battle& battle, 
                          const std::string& expectedState) {
        // Verify battle is in expected state
    }
};

TEST_F(BattleScenarioTest, SolarBeamWeatherCombo) {
    // Test Solar Beam + Sunny Day combination
    Battle battle = createTestBattle();
    
    // Turn 1: Set up sunny weather
    battle.executePlayerMove("sunny-day");
    EXPECT_EQ(battle.getCurrentWeather(), WeatherCondition::SUNNY);
    
    // Turn 2: Use Solar Beam (should skip charging)
    int initialHP = battle.getOpponentHP();
    battle.executePlayerMove("solar-beam");
    
    EXPECT_LT(battle.getOpponentHP(), initialHP);
    EXPECT_FALSE(battle.getPlayerPokemon().is_charging);
}
```

#### Performance Testing

```cpp
// tests/performance/test_multi_turn_performance.cpp
class MultiTurnPerformanceTest : public ::testing::Test {
protected:
    void measureExecutionTime(std::function<void()> operation, 
                             const std::string& operationName) {
        auto start = std::chrono::high_resolution_clock::now();
        operation();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << operationName << " took " << duration.count() << " microseconds" << std::endl;
    }
};

TEST_F(MultiTurnPerformanceTest, StateTransitionSpeed) {
    Pokemon pokemon("test-pokemon");
    
    measureExecutionTime([&]() {
        for (int i = 0; i < 10000; ++i) {
            pokemon.startCharging(0, "test-move");
            pokemon.finishCharging();
        }
    }, "10000 state transitions");
}

TEST_F(MultiTurnPerformanceTest, BattleTurnOverhead) {
    Battle battle = createTestBattle();
    
    measureExecutionTime([&]() {
        for (int i = 0; i < 1000; ++i) {
            battle.executeMove(/* single-turn move */);
        }
    }, "1000 single-turn moves");
    
    measureExecutionTime([&]() {
        for (int i = 0; i < 1000; ++i) {
            battle.executeMove(/* multi-turn move */);
        }
    }, "1000 multi-turn moves");
}
```

### Automated Testing Pipeline

#### Continuous Integration Setup

```yaml
# .github/workflows/multi_turn_tests.yml
name: Multi-Turn Move Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    
    - name: Setup Build Environment
      run: |
        sudo apt-get update
        sudo apt-get install build-essential cmake
    
    - name: Build Tests
      run: |
        mkdir build
        cd build
        cmake ..
        make multi_turn_tests
    
    - name: Run Unit Tests
      run: |
        cd build
        ./multi_turn_tests --gtest_filter="MultiTurnMove*"
    
    - name: Run Integration Tests
      run: |
        cd build
        ./multi_turn_tests --gtest_filter="BattleScenario*"
    
    - name: Run Performance Tests
      run: |
        cd build
        ./multi_turn_tests --gtest_filter="MultiTurnPerformance*"
    
    - name: Generate Coverage Report
      run: |
        cd build
        gcov -r ../src/core/move.cpp ../src/core/pokemon.cpp ../src/core/battle.cpp
        lcov --capture --directory . --output-file coverage.info
        genhtml coverage.info --output-directory coverage_html
```

## Best Practices

### Code Organization

#### File Structure
```
src/
├── core/
│   ├── move.cpp              # Multi-turn move logic
│   ├── pokemon.cpp           # State management
│   └── battle.cpp            # Execution logic
├── ai/
│   ├── expert_ai.cpp         # Advanced multi-turn strategy
│   └── hard_ai.cpp           # Strategic considerations
└── utils/
    └── multi_turn_helpers.cpp # Utility functions

include/
├── core/
│   ├── move.h                # Move interface
│   ├── pokemon.h             # State definitions
│   └── battle.h              # Battle interface
└── utils/
    └── multi_turn_helpers.h   # Utility declarations

tests/
├── unit/
│   ├── test_move.cpp         # Move behavior tests
│   ├── test_pokemon.cpp      # State management tests
│   └── test_battle.cpp       # Integration tests
├── integration/
│   └── test_scenarios.cpp    # Full battle scenarios
└── performance/
    └── test_benchmarks.cpp   # Performance validation
```

#### Naming Conventions

```cpp
// Use descriptive names for multi-turn related functions
void startChargingPhase(int moveIndex, const std::string& moveName);  // Good
void start(int i, const std::string& n);                             // Bad

// Use consistent prefixes for boolean checks
bool requiresCharging() const;    // Good
bool isChargingMove() const;      // Good  
bool charging() const;           // Bad

// Use clear enumeration values
enum class MultiTurnBehavior {
    NONE,           // Clear meaning
    CHARGE,         // Clear meaning
    RECHARGE,       // Clear meaning
    CHARGE_BOOST    // Clear meaning
};
```

### Error Handling

#### Defensive Programming

```cpp
// Always validate state before transitions
void Pokemon::startCharging(int moveIndex, const std::string& moveName) {
    // Validate inputs
    if (moveIndex < 0 || moveIndex >= static_cast<int>(moves.size())) {
        std::cerr << "Error: Invalid move index " << moveIndex << std::endl;
        return;
    }
    
    // Validate current state
    if (is_charging || must_recharge) {
        std::cerr << "Warning: Attempting to start charging while in multi-turn sequence" << std::endl;
        return;
    }
    
    // Safe state transition
    is_charging = true;
    charging_move_index = moveIndex;
    charging_move_name = moveName;
}
```

#### Exception Safety

```cpp
// Use RAII for state management
class ScopedMultiTurnState {
    Pokemon& pokemon_;
    bool previous_charging_state_;
    
public:
    ScopedMultiTurnState(Pokemon& pokemon) 
        : pokemon_(pokemon), previous_charging_state_(pokemon.is_charging) {}
    
    ~ScopedMultiTurnState() {
        // Restore previous state if exception occurred
        if (std::uncaught_exceptions() > 0) {
            pokemon_.is_charging = previous_charging_state_;
        }
    }
};

void Battle::executeMove(Pokemon &attacker, Pokemon &defender, int moveIndex) {
    ScopedMultiTurnState stateGuard(attacker);
    
    // Multi-turn move execution (may throw)
    // State automatically restored if exception occurs
}
```

### Documentation Standards

#### Code Comments

```cpp
/**
 * @brief Initiates charging phase for multi-turn moves
 * 
 * This method transitions the Pokemon into charging state for moves that
 * require a preparation turn before execution. The charging state prevents
 * the Pokemon from switching and locks in the move for next turn execution.
 * 
 * @param moveIndex Index of the move in the Pokemon's move array (0-3)
 * @param moveName Display name of the move for UI feedback
 * 
 * @pre Pokemon must not be in charging or recharge state
 * @pre moveIndex must be valid (0 <= moveIndex < moves.size())
 * @post Pokemon will be in charging state with locked move
 * 
 * @see finishCharging() to complete the charging sequence
 * @see isInMultiTurnSequence() to check if Pokemon can start charging
 */
void Pokemon::startCharging(int moveIndex, const std::string& moveName);
```

#### API Documentation

```cpp
/**
 * @class MultiTurnMoveSystem
 * @brief Manages Pokemon moves requiring multiple turns to execute
 * 
 * The Multi-Turn Move System handles Pokemon moves that don't execute in a
 * single turn, including:
 * - Charging moves (Solar Beam, Sky Attack, Skull Bash)
 * - Recharge moves (Hyper Beam) 
 * - Weather-dependent moves (Solar Beam in sunny weather)
 * 
 * @section Usage
 * @code
 * // Basic usage for charging move
 * if (move.requiresCharging() && !pokemon.is_charging) {
 *     pokemon.startCharging(moveIndex, move.name);
 *     return; // End turn, move executes next turn
 * }
 * 
 * // Execute the charged move
 * if (pokemon.is_charging) {
 *     pokemon.finishCharging();
 *     executeMoveDamage(pokemon, target, move);
 * }
 * @endcode
 * 
 * @section Thread_Safety
 * Pokemon state management is not thread-safe. External synchronization
 * required for concurrent battle scenarios.
 * 
 * @section Performance
 * - State transitions: O(1) constant time
 * - Memory overhead: ~50 bytes per Pokemon
 * - No dynamic allocation during battles
 */
```

## Troubleshooting Guide

### Common Issues and Solutions

#### Issue 1: Move Not Executing After Charging

**Symptoms:**
- Pokemon charges but move doesn't execute next turn
- Pokemon stuck in charging state

**Diagnosis:**
```cpp
// Debug the charging state
void debugChargingState(const Pokemon& pokemon) {
    std::cout << "Pokemon: " << pokemon.name << std::endl;
    std::cout << "is_charging: " << pokemon.is_charging << std::endl;
    std::cout << "charging_move_index: " << pokemon.charging_move_index << std::endl;
    std::cout << "charging_move_name: " << pokemon.charging_move_name << std::endl;
    
    if (pokemon.charging_move_index >= 0 && 
        pokemon.charging_move_index < pokemon.moves.size()) {
        const Move& move = pokemon.moves[pokemon.charging_move_index];
        std::cout << "Move name: " << move.name << std::endl;
        std::cout << "Multi-turn behavior: " << static_cast<int>(move.getMultiTurnBehavior()) << std::endl;
    }
}
```

**Solutions:**
- Verify `finishCharging()` is called before move execution
- Check for status conditions preventing move execution
- Ensure move index remains valid during charging

#### Issue 2: AI Switching During Multi-Turn Sequences

**Symptoms:**
- AI switches Pokemon while charging or recharging
- Multi-turn constraints not enforced for AI

**Diagnosis:**
```cpp
// Verify AI constraint checking
bool Battle::shouldAISwitch() const {
    // Debug output
    std::cout << "AI switching check:" << std::endl;
    std::cout << "is_charging: " << opponentSelectedPokemon->is_charging << std::endl;
    std::cout << "must_recharge: " << opponentSelectedPokemon->must_recharge << std::endl;
    std::cout << "isInMultiTurnSequence: " << opponentSelectedPokemon->isInMultiTurnSequence() << std::endl;
    
    if (opponentSelectedPokemon->isInMultiTurnSequence()) {
        std::cout << "AI switching blocked due to multi-turn sequence" << std::endl;
        return false;
    }
    
    // Normal switching logic...
}
```

**Solutions:**
- Add multi-turn sequence checks to all AI switching logic
- Verify `isInMultiTurnSequence()` returns correct values
- Test all AI difficulty levels for constraint compliance

#### Issue 3: Weather Interactions Not Working

**Symptoms:**
- Solar Beam still charges in sunny weather
- Weather effects inconsistent

**Diagnosis:**
```cpp
// Debug weather state
void debugWeatherState(const Battle& battle) {
    std::cout << "Current weather: " << static_cast<int>(battle.getCurrentWeather()) << std::endl;
    std::cout << "Weather turns remaining: " << battle.getWeatherTurnsRemaining() << std::endl;
    
    // Check move weather properties
    Move solarBeam("solar-beam");
    std::cout << "Solar Beam weather dependent: " << solarBeam.is_weather_dependent << std::endl;
    std::cout << "Skips charge in sun: " << solarBeam.skipChargeInSunnyWeather() << std::endl;
}
```

**Solutions:**
- Verify weather state is correctly maintained
- Check weather condition enumeration values
- Ensure weather checks occur before charging logic

### Debugging Tools

#### State Inspection Utilities

```cpp
// Debug utility class for multi-turn state
class MultiTurnDebugger {
public:
    static void dumpPokemonState(const Pokemon& pokemon) {
        std::cout << "=== Pokemon Multi-Turn State ===" << std::endl;
        std::cout << "Name: " << pokemon.name << std::endl;
        std::cout << "is_charging: " << pokemon.is_charging << std::endl;
        std::cout << "must_recharge: " << pokemon.must_recharge << std::endl;
        std::cout << "charging_move_index: " << pokemon.charging_move_index << std::endl;
        std::cout << "charging_move_name: " << pokemon.charging_move_name << std::endl;
        std::cout << "isInMultiTurnSequence: " << pokemon.isInMultiTurnSequence() << std::endl;
        std::cout << "================================" << std::endl;
    }
    
    static void dumpMoveProperties(const Move& move) {
        std::cout << "=== Move Properties ===" << std::endl;
        std::cout << "Name: " << move.name << std::endl;
        std::cout << "Multi-turn behavior: " << static_cast<int>(move.multi_turn_behavior) << std::endl;
        std::cout << "Is multi-turn: " << move.isMultiTurnMove() << std::endl;
        std::cout << "Requires charging: " << move.requiresCharging() << std::endl;
        std::cout << "Requires recharge: " << move.requiresRecharge() << std::endl;
        std::cout << "Weather dependent: " << move.is_weather_dependent << std::endl;
        std::cout << "=======================" << std::endl;
    }
    
    static void traceBattleExecution(const Battle& battle) {
        // Log battle state changes for debugging
        std::cout << "Battle turn: " << battle.getCurrentTurn() << std::endl;
        std::cout << "Weather: " << static_cast<int>(battle.getCurrentWeather()) << std::endl;
        // Additional battle state information
    }
};
```

#### Automated Testing Helpers

```cpp
// Helper functions for test scenarios
class TestHelpers {
public:
    static Pokemon createTestPokemon(const std::string& name, 
                                   const std::vector<std::string>& moveNames) {
        Pokemon pokemon(name);
        pokemon.moves.clear();
        
        for (const auto& moveName : moveNames) {
            pokemon.moves.emplace_back(moveName);
        }
        
        return pokemon;
    }
    
    static void simulateChargingSequence(Battle& battle, Pokemon& attacker, 
                                       Pokemon& defender, int moveIndex) {
        // Turn 1: Start charging
        battle.executeMove(attacker, defender, moveIndex);
        
        // Turn 2: Execute charged move
        battle.executeMove(attacker, defender, moveIndex);
    }
    
    static bool verifyStateTransition(const Pokemon& pokemon, 
                                    bool expectedCharging, 
                                    bool expectedRecharge) {
        return (pokemon.is_charging == expectedCharging && 
                pokemon.must_recharge == expectedRecharge);
    }
};
```

## Conclusion

The Multi-Turn Move System provides a robust framework for implementing complex Pokemon battle mechanics while maintaining performance and extensibility. This developer guide covers all aspects of working with the system, from adding new moves to optimizing performance and debugging issues.

### Key Development Principles

1. **Modularity:** Each component has clear responsibilities and interfaces
2. **Extensibility:** New behaviors can be added without modifying existing code
3. **Performance:** System is optimized for real-time battle scenarios
4. **Testability:** Comprehensive testing frameworks ensure reliability
5. **Maintainability:** Clear documentation and debugging tools support long-term maintenance

### Next Steps for Developers

1. **Study Existing Implementation:** Review the current multi-turn moves to understand patterns
2. **Practice with Extensions:** Add new multi-turn moves to gain experience
3. **Contribute Tests:** Expand test coverage for edge cases and new scenarios
4. **Optimize Performance:** Profile and optimize bottlenecks in your use cases
5. **Improve Documentation:** Update guides based on your development experience

The Multi-Turn Move System is designed to grow with the Pokemon Battle Simulator, providing a solid foundation for future enhancements while maintaining the authentic Pokemon battle experience.

---

**Guide Status:** Complete - Ready for Development Teams  
**Last Updated:** August 20, 2025  
**Next Review:** After major system updates or developer feedback