# 🎮 Pokémon Battle Simulator

A **comprehensive C++ implementation** of a Pokémon battle system that simulates turn-based combat with **authentic game mechanics**. Experience strategic battles with advanced features like status conditions, STAB bonuses, critical hits, and full type effectiveness!

## ⚡ Latest Features

### 🔥 **Advanced Battle Mechanics**
- **STAB (Same Type Attack Bonus)**: 1.5x damage when Pokémon type matches move type
- **Critical Hits**: 6.25% base chance (12.5% for high-crit moves) with 2x damage multiplier
- **Comprehensive Status Conditions**: Sleep, Poison, Burn, Paralysis, Freeze with authentic effects
- **Turn-based Status Processing**: Damage over time, action restrictions, and automatic recovery
- **Effective Stat Calculations**: Burn halves Attack, Paralysis halves Speed
- **🌤️ Weather System**: Rain, Sun, Sandstorm, and Hail with authentic type interactions and damage

### 💎 **Professional Code Quality**
- **Modern C++17**: Clean, object-oriented design with auto type deduction
- **Zero Namespace Pollution**: Removed `using namespace std;` throughout codebase
- **Type Safety**: Strategic use of `auto` for better maintainability
- **Enhanced Error Handling**: Robust JSON parsing and file operations

## 🎯 Core Features

- **🏟️ Complete Battle System**: Turn-based combat with move priority, type effectiveness, and authentic damage calculation
- **📚 Extensive Database**: 300+ moves and 150+ Pokémon with accurate stats from Generation I
- **⚔️ Strategic Combat**: Status conditions, stat modifications, and multi-turn effects
- **👥 Team Management**: Create and manage teams of up to 6 Pokémon with pre-built configurations
- **🎨 Enhanced UI**: Visual health bars, status condition indicators, and detailed battle feedback
- **📁 JSON Data Storage**: Modular, expandable data structure for easy customization

## 📁 Project Structure

```
Pokemon-Battle-Simulator/
├── src/                    # Core battle system
│   ├── main.cpp           # Main application entry point
│   ├── battle.cpp/h       # Battle system with status conditions
│   ├── pokemon.cpp/h      # Pokémon class with status tracking
│   ├── move.cpp/h         # Move system with ailment support
│   ├── team.cpp/h         # Team management
│   ├── weather.cpp/h      # Weather system implementation
│   └── type_effectiveness.cpp/h  # Complete type chart implementation
├── include/               # External libraries
│   └── json.hpp          # nlohmann/json library
├── data/                  # Comprehensive game data
│   ├── moves/            # 300+ move definitions with status effects
│   ├── pokemon/          # 150+ Pokémon with authentic stats
│   └── teams/            # Pre-built team configurations
├── examples/             # Interactive demos
│   ├── game-demo.cpp     # Simplified battle interface
│   └── debug-test.cpp    # Development testing
├── build/                # Build artifacts (CMake generated)
├── CMakeLists.txt        # Build configuration
└── README.md            # This documentation
```

## 🚀 Quick Start

### Prerequisites

- **C++ Compiler**: GCC 7+ or Clang 5+ with C++17 support
- **CMake**: Version 3.16 or higher
- **OS**: Linux, macOS, or Windows

### Build & Run

```bash
# Clone and navigate
git clone <repository-url>
cd Pokemon-Battle-Simulator

# Build with CMake (mac/Linux)
mkdir build && cd build
cmake ..
make -j4

# Build with CMake (Windows)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Start battling!
./bin/pokemon_battle (mac)
./bin/Release/pokemon_battle.exe (windows)
```


### Quick Demo

```bash
# Interactive battle demo
./bin/game-demo
./bin/Release/game-demo.exe (windows)

# Test specific mechanics
./bin/debug-test
./bin/Release/debug-test.exe (windows)
```

## ⚔️ Battle System Deep Dive

### 🎲 **Status Conditions** (NEW!)
Our comprehensive status system includes:

| Status | Effect | Duration | Damage |
|--------|--------|----------|---------|
| **💤 Sleep** | Can't act | 1-3 turns | None |
| **☠️ Poison** | Damage over time | Until cured | 1/8 max HP per turn |
| **🔥 Burn** | Damage + Attack halved | Until cured | 1/16 max HP per turn |
| **⚡ Paralysis** | 25% miss chance + Speed halved | Until cured | None |
| **🧊 Freeze** | Can't act | 20% thaw chance per turn | None |

**Features**:
- ✅ Status immunity (can't stack multiple conditions)
- ✅ Visual status indicators in health bars
- ✅ Authentic damage calculations and restrictions
- ✅ Automatic recovery and turn countdowns

### 🌤️ **Weather System** (NEW!)
Dynamic weather conditions that affect battle strategy:

| Weather | Move Effects | Environmental Damage | Immunity |
|---------|-------------|---------------------|----------|
| **🌧️ Rain** | Water moves: 1.5x<br/>Fire moves: 0.5x | None | All types |
| **☀️ Sun** | Fire moves: 1.5x<br/>Water moves: 0.5x | None | All types |
| **🌪️ Sandstorm** | No move effects | 1/16 max HP/turn | Rock, Ground, Steel |
| **❄️ Hail** | No move effects | 1/16 max HP/turn | Ice |

**Features**:
- ✅ Authentic damage multipliers for weather-affected moves
- ✅ Environmental damage with type-based immunity
- ✅ Weather persistence throughout battle
- ✅ Visual weather indicators and damage notifications

### ⭐ **STAB & Critical Hits** (NEW!)
- **STAB Bonus**: 1.5x damage when Pokémon type matches move type
- **Critical Hits**: Base 6.25% chance, high-crit moves 12.5%
- **Enhanced Feedback**: "Charizard gets STAB!" and "A critical hit!" messages
- **Proper Stacking**: All multipliers combine correctly (Type × STAB × Critical × Weather)

### 🎯 **Type Effectiveness System**
Complete implementation with authentic Pokémon type chart:
- **Super Effective** (2x): "It's super effective!"
- **Not Very Effective** (0.5x): "It's not very effective..."
- **No Effect** (0x): "It has no effect!"
- **Dual-type Support**: Proper multiplier stacking for dual-type Pokémon

### 🎮 **Enhanced Battle Flow**
- **Turn Priority**: Move priority → Effective Speed → Random tiebreaker  
- **Status Processing**: Automatic status effects at turn start
- **Smart AI**: Opponent prioritizes damage moves and switches strategically
- **Visual Polish**: Health bars, status indicators, and detailed battle log

## 📊 Data Examples

### Pokémon with Status Effects
```json
{
  "name": "venusaur",
  "types": ["grass", "poison"],
  "hp": 80, "attack": 82, "defense": 83,
  "special_attack": 100, "special_defense": 100, "speed": 80,
  "moves": ["sludge-bomb", "sleep-powder", "leech-seed", "toxic"]
}
```

### Status-Inflicting Moves
```json
{
  "name": "will-o-wisp",
  "type": "fire",
  "power": null,
  "accuracy": 85,
  "damage_class": "status",
  "ailment": {"name": "burn"},
  "ailment_chance": 0
}
```

## 🎪 Battle Examples

### Status Warfare in Action
```
venusaur used sleep-powder!
pikachu is now Asleep!
pikachu is fast asleep!

arcanine used will-o-wisp!
venusaur is now Burned!
venusaur is hurt by burn! (-5 HP)

machamp used body-slam!
It dealt 95 damage! machamp gets STAB!
raichu is now Paralyzed!
raichu is paralyzed and can't move!
```

### Weather Effects in Battle
```
The battle begins under Rain!

blastoise used hydro-pump!
Rain boosted the attack! (1.5x damage)
It dealt 142 damage!

charizard used flamethrower!
Rain weakened the attack! (0.5x damage)
It dealt 31 damage.

A Sandstorm is raging!
pikachu is buffeted by the sandstorm! (-12 HP)
golem is unaffected by the sandstorm.
```

### Strategic Depth
- **Status Timing**: When to inflict vs cure status conditions
- **Type Synergy**: STAB bonuses change move selection priorities  
- **Weather Strategy**: Choosing moves based on current weather conditions
- **Environmental Awareness**: Protecting vulnerable Pokémon from weather damage
- **Speed Control**: Paralysis and stat modifications affect turn order
- **Risk/Reward**: Status moves vs immediate damage trade-offs

## 🔧 Advanced Usage

### Custom Battle Setup
```cpp
#include "team.h"
#include "battle.h"

// Load teams with status-capable Pokémon
Team playerTeam("data/teams/Team-1.json");
Team opponentTeam("data/teams/Team-2.json"); 

// Experience advanced battle mechanics
Battle battle(playerTeam, opponentTeam);
battle.startBattle();
```

### Status Condition Programming
```cpp
// Check if Pokémon can act (not asleep/frozen/paralyzed)
if (pokemon.canAct()) {
    // Execute move
}

// Apply status condition
pokemon.applyStatusCondition(StatusCondition::BURN);

// Process status effects each turn
pokemon.processStatusCondition();
```

### Weather System Programming
```cpp
#include "weather.h"

// Set battle weather
WeatherCondition currentWeather = WeatherCondition::RAIN;

// Calculate weather-modified damage
double weatherMultiplier = Weather::getWeatherDamageMultiplier(
    currentWeather, move.getType());
int finalDamage = baseDamage * weatherMultiplier;

// Check for weather immunity and apply environmental damage
if (!Weather::isImmuneToWeatherDamage(currentWeather, pokemon.getTypes())) {
    int weatherDamage = Weather::getWeatherDamage(currentWeather, pokemon.getMaxHP());
    pokemon.takeDamage(weatherDamage);
}
```

## 🧪 Testing & Quality

### Comprehensive Testing
```bash
# Build and test all mechanics
cd build && make -j4

# Test status conditions
echo "1\n4\n1\n3\n3\n3" | ./bin/pokemon_battle

# Test STAB and critical hits  
echo "2\n3\n2\n1\n1\n1" | ./bin/pokemon_battle
```

### Code Quality Metrics
- ✅ **Zero compiler warnings** with `-Wall -Wextra`
- ✅ **Modern C++17** practices throughout
- ✅ **Type safety** with strategic `auto` usage
- ✅ **Memory safe** with RAII and smart pointers where needed

## 🎮 Gameplay Modes

### 1. **Main Battle Mode** (`pokemon_battle`)
Full-featured battle system with:
- Team selection from 3 pre-built teams
- 8 Gym Leader opponents with themed teams
- Complete status condition mechanics
- Enhanced visual feedback

### 2. **Quick Demo** (`game-demo`)
Streamlined experience with:
- Simplified team selection
- Direct opponent matchups  
- Focus on core battle mechanics

## 🧪 Testing Infrastructure

### **Professional Testing Suite**
Our project features a comprehensive testing framework built with **Google Test (GTest)**, following industry best practices for C++ testing. The test suite ensures code quality, prevents regressions, and validates all battle mechanics.

### **Testing Architecture**

#### **🔬 Unit Tests** (7 test files)
Individual component testing with isolated test scenarios:

```cpp
// Example: Testing Pokemon stat modifications
TEST_F(PokemonTest, StatModifications) {
    Pokemon testPokemon = TestUtils::createTestPokemon("test", 100, 80, 70, 90, 85, 75, {"normal"});
    
    // Test attack modification
    testPokemon.modifyAttack(2);
    EXPECT_EQ(testPokemon.attack_stage, 2);
    EXPECT_GT(testPokemon.getEffectiveAttack(), testPokemon.attack);
}
```

**Coverage:**
- ✅ **test_team.cpp**: Team management, Pokemon access, alive Pokemon tracking
- ✅ **test_battle.cpp**: Battle state management, AI difficulties, result transitions
- ✅ **test_weather.cpp**: Weather effects, damage multipliers, immunity systems
- ✅ **test_ai.cpp**: AI decision making across Easy/Medium/Hard difficulties
- ✅ **test_pokemon.cpp**: Pokemon stats, status conditions, health management
- ✅ **test_move.cpp**: Move mechanics, PP system, status effects
- ✅ **test_type_effectiveness.cpp**: Complete type chart coverage

#### **🔄 Integration Tests** (3 test files)
End-to-end testing of complex interactions:

```cpp
// Example: Testing weather + battle integration
TEST_F(WeatherIntegrationTest, RainBoostsWaterMoves) {
    // Weather affects move damage in battle context
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "water"), 1.5);
    EXPECT_DOUBLE_EQ(Weather::getWeatherDamageMultiplier(WeatherCondition::RAIN, "fire"), 0.5);
}
```

**Coverage:**
- ✅ **test_full_battle.cpp**: Complete battle scenarios with type advantages, multi-hit moves, priority moves
- ✅ **test_status_integration.cpp**: Status conditions in battle context (poison, burn, paralysis, sleep, freeze)
- ✅ **test_weather_integration.cpp**: Weather system integration with battles and AI

#### **🛠️ Testing Infrastructure**
- **TestUtils Framework**: Comprehensive utilities for creating test Pokemon, moves, and teams
- **Test Fixtures**: Organized setup/teardown with `PokemonTestFixture` and `BattleTestFixture`
- **Mock Data**: Isolated test environments with controlled Pokemon and move data
- **CMake Integration**: Automated test discovery and execution

### **Technologies Used**

#### **Google Test Framework**
- **Assertions**: `EXPECT_EQ`, `EXPECT_TRUE`, `EXPECT_DOUBLE_EQ`, `ASSERT_NE`
- **Test Fixtures**: Class-based test organization with `SetUp()` and `TearDown()`
- **Parameterized Tests**: Data-driven testing for comprehensive coverage
- **Test Discovery**: Automatic test registration with `gtest_discover_tests()`

#### **CMake Test Configuration**
```cmake
# Professional test setup
include(GoogleTest)

# Automated test creation function
function(create_test test_name test_source)
    add_executable(${test_name} ${test_source} ${CORE_SOURCES})
    target_link_libraries(${test_name} gtest gtest_main test_utils)
    gtest_discover_tests(${test_name})
endfunction()
```

### **Running Tests**

#### **🚀 Quick Start**
```bash
# Build and run all tests
cd Pokemon-Battle-Simulator
mkdir -p build && cd build
cmake ..
make -j4
make run_all_tests
```

#### **📋 Common Test Commands**

##### **Build Tests**
```bash
# Build all tests
make

# Build specific test
make test_pokemon
make test_battle
make test_weather
```

##### **Run All Tests**
```bash
# Run all tests with summary
make run_all_tests

# Run with CTest (detailed output)
ctest --verbose

# Run with parallel execution
ctest -j4

# Run only failing tests
ctest --rerun-failed
```

##### **Run Specific Tests**
```bash
# Run individual test executables
./test_pokemon
./test_battle
./test_weather
./test_ai
./test_full_battle
./test_status_integration
./test_weather_integration
```

##### **Advanced Test Filtering**
```bash
# Run specific test cases
./test_pokemon --gtest_filter="PokemonTest.BasicProperties"
./test_battle --gtest_filter="*BattleState*"

# Run tests matching pattern
./test_weather --gtest_filter="WeatherTest.Rain*"

# Exclude specific tests
./test_ai --gtest_filter="*:-AITest.EdgeCases"
```

##### **Test Output Control**
```bash
# Verbose output
ctest --verbose

# Show only failures
ctest --output-on-failure

# Generate XML output
ctest --output-junit results.xml

# Run with specific verbosity
./test_pokemon --gtest_brief
```

##### **Performance and Debugging**
```bash
# Run tests with timing
ctest --verbose --parallel 4

# Run specific test with detailed output
./test_battle --gtest_filter="BattleTest.AIBehavior" --gtest_repeat=10

# Memory testing (if valgrind available)
valgrind --tool=memcheck ./test_pokemon
```

### **Test Coverage Analysis**

#### **✅ Fully Tested Components**
- **Weather System**: Rain, sun, sandstorm, hail with damage multipliers
- **Status Conditions**: All 5 major status effects with battle integration
- **Type Effectiveness**: Complete 18x18 type chart with dual-type support
- **Team Management**: Pokemon loading, alive checks, switching logic
- **Battle Flow**: State transitions, result determination, turn processing
- **AI Behavior**: Easy, Medium, Hard difficulty decision making

#### **🔄 Planned Test Expansion**
- **Pokemon Switching Integration**: Real-time switching mechanics in battle
- **STAB & Critical Hit Calculations**: Damage multiplier validation
- **Move Priority System**: Turn order with priority moves
- **Move Accuracy System**: Hit/miss mechanics testing
- **Stat Modification Integration**: Stat stage effects in battle
- **AI Behavioral Integration**: AI decision making in complex scenarios

### **Testing Best Practices**

#### **Google Test Patterns**
```cpp
// Test fixture for organized testing
class WeatherTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test environment
    }
};

// Parameterized testing for comprehensive coverage
class TypeEffectivenessTest : public ::testing::TestWithParam<TypeMatchup> {
    // Test all type combinations
};

// Custom assertions for domain-specific testing
#define EXPECT_POKEMON_ALIVE(pokemon) \
    EXPECT_TRUE(pokemon.isAlive()) << #pokemon << " should be alive"
```

#### **Test Organization**
- **Descriptive Test Names**: `TEST_F(PokemonTest, StatusConditionsPersistThroughBattle)`
- **Arrange-Act-Assert Pattern**: Clear test structure
- **Test Independence**: Each test runs in isolation
- **Edge Case Coverage**: Boundary conditions and error scenarios

### **Quality Metrics**
- ✅ **Zero Compilation Warnings**: Clean builds with `-Wall -Wextra`
- ✅ **Memory Safety**: RAII and smart pointer usage
- ✅ **Thread Safety**: Proper resource management
- ✅ **Regression Prevention**: Comprehensive test coverage prevents bugs

## 🔮 Future Enhancements

### Planned Features
- **🎯 Priority Moves**: Quick Attack, Mach Punch always go first
- **🔄 Multi-turn Moves**: Hyper Beam recharge, Solar Beam charging
- **🎨 Battle Animations**: ASCII art and enhanced visual effects
- **🏆 Tournament Mode**: Elite Four progression and championships

### Technical Improvements
- **📊 Battle Statistics**: Damage dealt, accuracy rates, status success
- **💾 Save System**: Team persistence and battle history
- **🤖 Advanced AI**: Difficulty levels and strategic decision making
- **🎵 Audio**: Sound effects and battle music integration

## 🤝 Contributing

We welcome contributions! Here's how to get involved:

### Development Workflow
1. **Fork** the repository
2. **Create** feature branch: `git checkout -b feature/amazing-feature`
3. **Implement** with modern C++ practices
4. **Test** thoroughly with provided examples
5. **Submit** pull request with detailed description

### Coding Standards
- **C++17** standard compliance
- **Const-correctness** throughout
- **RAII** for resource management
- **Auto** for type deduction where beneficial
- **No** `using namespace std;`

## 📈 Performance

### Optimized for Speed
- **Fast Compilation**: Parallel builds with CMake
- **Efficient Memory**: Stack-based objects, minimal heap allocation
- **Quick Battles**: Optimized damage calculations and RNG
- **Responsive UI**: Immediate feedback for all actions

## 📜 License

This project is open source and available under the [MIT License](LICENSE).

## 🙏 Acknowledgments

- **Pokémon Company**: For the incredible game mechanics that inspired this simulator
- **nlohmann/json**: For the excellent JSON library
- **PokeAPI**: For comprehensive Pokémon data structure inspiration

---

**Ready to become a Pokémon Master? Clone the repo and start your journey!** ⚡🔥💧

```bash
git clone <repository-url>
cd Pokemon-Battle-Simulator
mkdir build && cd build && cmake .. && make -j4
./bin/pokemon_battle
```

*Experience the thrill of strategic Pokémon battles with authentic mechanics!* 🎮✨ 