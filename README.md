# 🎮 Pokémon Battle Simulator

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/5kng/Pokemon-Battle-Simulator)  
<sup>Click for interactive docs powered by DeepWiki</sup>

A **comprehensive C++ implementation** of a Pokémon battle system featuring authentic turn-based combat mechanics and a complete **Championship Tournament System**. Experience the full journey from Gym Leader battles to Elite Four and Champion challenges!

## ⚡ Latest Features

### 🏆 **Championship Tournament System** ⭐ *NEW*
- **Complete Tournament Mode**: 8 Gym Leaders → Elite Four → Champion progression
- **Type-Specialized Gym Leaders**: Each with thematic teams and strategies
- **Badge Collection System**: Track your tournament progress and achievements  
- **Elite Four Gauntlet**: Face four consecutive Champion-tier battles
- **Championship Battle**: Final showdown against the ultimate opponent
- **Tournament Statistics**: Detailed performance tracking and leaderboards
- **Custom Team Builder**: Create specialized teams for tournament challenges

### 🔥 **Advanced Battle Mechanics**
- **STAB (Same Type Attack Bonus)**: 1.5x damage when Pokémon type matches move type
- **Critical Hits**: 6.25% base chance (12.5% for high-crit moves) with 2x damage multiplier
- **Comprehensive Status Conditions**: Sleep, Poison, Burn, Paralysis, Freeze with authentic effects
- **Turn-based Status Processing**: Damage over time, action restrictions, and automatic recovery
- **Effective Stat Calculations**: Burn halves Attack, Paralysis halves Speed
- **🌤️ Weather System**: Rain, Sun, Sandstorm, and Hail with authentic type interactions and damage
- **🧠 4-Tier AI System**: Easy, Medium, Hard, and Expert AI with sophisticated strategies

### 💎 **Professional Code Quality**
- **Modern C++17**: Clean, object-oriented design with auto type deduction
- **Zero Namespace Pollution**: Removed `using namespace std;` throughout codebase
- **Type Safety**: Strategic use of `auto` for better maintainability
- **Enhanced Error Handling**: Robust JSON parsing and file operations
- **Comprehensive Testing**: Full test suite ensuring reliability

## 🎯 Core Features

- **🏆 Championship Tournament**: Complete progression system with 8 Gym Leaders, Elite Four, and Champion battles
- **🏟️ Advanced Battle System**: Turn-based combat with move priority, type effectiveness, and authentic damage calculation
- **📚 Extensive Database**: 300+ moves and 151 Generation I Pokémon with accurate stats
- **⚔️ Strategic Combat**: Status conditions, stat modifications, weather effects, and multi-turn mechanics
- **👥 Team Management**: Create and manage teams with sophisticated team builder and pre-built configurations
- **🤖 4-Tier AI System**: Easy, Medium, Hard, and Expert AI with sophisticated decision-making and opponent modeling
- **🎨 Enhanced UI**: Animated health bars, status condition indicators, and comprehensive battle feedback
- **📁 JSON Data Storage**: Modular, expandable data structure for easy customization

## 📁 Project Structure

```
Pokemon-Battle-Simulator/
├── src/                                    # Core implementation
│   ├── main.cpp                           # Tournament & battle menu system
│   ├── core/                              # Battle engine
│   │   ├── battle.cpp/h                   # Enhanced battle system with health bars
│   │   ├── tournament_manager.cpp/h       # Tournament progression & badge tracking
│   │   ├── championship_system.cpp/h      # Elite Four & Champion battles
│   │   ├── gym_leader.cpp/h               # Type-specialized gym battles
│   │   ├── pokemon.cpp/h & pokemon_data.cpp # Pokémon with full status support
│   │   ├── team.cpp/h & team_builder.cpp/h # Advanced team management
│   │   ├── weather.cpp/h                  # Weather system implementation
│   │   └── move.cpp/h                     # Move system with status effects
│   ├── ai/                                # 4-tier AI system
│   │   ├── expert_ai.cpp/h                # Advanced predictive AI
│   │   ├── hard_ai.cpp/h, medium_ai.cpp/h # Strategic AI variations
│   │   └── easy_ai.cpp/h, ai_factory.cpp/h # AI difficulty system
│   └── utils/                             # Support systems
│       ├── health_bar_animator.cpp/h      # Visual health bar system
│       ├── type_effectiveness.cpp/h       # Complete type chart
│       └── input_validator.cpp/h          # Robust input handling
├── include/                               # Headers & external libraries
│   ├── core/, ai/, utils/                 # System headers
│   └── json.hpp                           # nlohmann/json library
├── data/                                  # Game content
│   ├── pokemon/          # 151 Gen I Pokémon with authentic stats
│   ├── moves/            # 300+ moves with status effects & animations  
│   ├── tournaments/      # Tournament configs & gym strategies
│   ├── teams/            # Pre-built & custom team configurations
│   └── team_templates/   # Themed team archetypes
├── tests/                                 # Comprehensive test suite
│   ├── unit/             # Pokemon, AI, battle, weather system tests
│   ├── integration/      # Full battle & status interaction tests
│   └── utils/            # Testing utilities and helpers
├── build/                # CMake build artifacts
├── CMakeLists.txt        # Modern C++17 build configuration
└── README.md            # This comprehensive documentation
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

# Run the game (IMPORTANT: Run from project root directory!)
cd ..                           # Go back to project root
./build/bin/pokemon_battle      # Main tournament & battle system
# OR
./run_game.sh                   # Launcher script (recommended)
```

**⚠️ Important**: The game must be run from the project root directory where the `data/` folder is located, not from the `build/` directory. The executable looks for data files relative to the current working directory.

### 🎮 **Tournament Mode Quick Start**
```bash
# Experience the full championship journey:
./build/bin/pokemon_battle
# Select: [7] 🏆 Tournament Mode
# Choose: [1] 🥇 Gym Leader Challenge  
# Build your team and start your championship run!
```

## ⚔️ Battle System Deep Dive

### 🎲 **Status Conditions**
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

### 🌤️ **Weather System**
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

### ⭐ **STAB & Critical Hits**
- **STAB Bonus**: 1.5x damage when Pokémon type matches move type
- **Critical Hits**: Base 6.25% chance, high-crit moves 12.5%
- **Enhanced Feedback**: "Charizard gets STAB!" and "A critical hit!" messages
- **Proper Stacking**: All multipliers combine correctly (Type × STAB × Critical × Weather)

### 🏆 **Tournament Championship System**
Experience the complete journey from rookie trainer to Champion:

| Stage | Requirement | Challenge Level | Rewards |
|-------|-------------|-----------------|---------|
| **🥇 Gym Leaders** | Win 8/8 Gym Battles | Type-specialized teams | Gym Badges |
| **👑 Elite Four** | All 8 badges earned | 4 consecutive battles | Elite Four completion |
| **🎖️ Champion Battle** | Elite Four defeated | Ultimate challenge | Championship title |

**Tournament Features**:
- ✅ **Progressive Difficulty**: Each gym leader has unique strategies and team themes
- ✅ **Badge Collection**: Visual progress tracking with badge requirements
- ✅ **Team Customization**: Build specialized teams for different gym types
- ✅ **Tournament Statistics**: Track wins, losses, and performance metrics
- ✅ **Championship Leaderboard**: Compare your achievements with others

### 🧠 **AI Difficulty System**
Choose your challenge level with progressively smarter opponents:

| Difficulty | Strategy | Switching | Decision Making | Used In |
|------------|----------|-----------|-----------------|---------|
| **🟢 Easy** | Random moves only | Never switches | Pure randomness | Practice battles |
| **🟡 Medium** | Basic type effectiveness | Switches when low HP | Considers move power + type | Early gym leaders |
| **🔴 Hard** | Advanced strategy | Strategic switching | Multi-factor analysis | Later gyms & Elite Four |
| **⭐ Expert** | Predictive analysis | Multi-turn planning | Opponent modeling | Champion battle |

**Expert AI Features**:
- ✅ **Multi-turn Planning**: Analyzes 2+ turns ahead with comprehensive strategy trees
- ✅ **Opponent Modeling**: Tracks patterns, predicts moves, adapts to player style
- ✅ **Predictive Analysis**: Forecasts opponent actions with confidence scoring
- ✅ **Setup Detection**: Recognizes and disrupts stat-boosting strategies
- ✅ **Win Condition Analysis**: Evaluates long-term positioning and victory paths
- ✅ **Risk Assessment**: Models outcome probabilities and expected utility
- ✅ **Counter-strategy Recognition**: Adapts tactics mid-battle based on player behavior

### 🎯 **Type Effectiveness System**
Complete implementation with authentic Pokémon type chart:
- **Super Effective** (2x): "It's super effective!"
- **Not Very Effective** (0.5x): "It's not very effective..."
- **No Effect** (0x): "It has no effect!"
- **Dual-type Support**: Proper multiplier stacking for dual-type Pokémon

### 🎮 **Enhanced Battle Flow**
- **Turn Priority**: Move priority → Effective Speed → Random tiebreaker  
- **Status Processing**: Automatic status effects at turn start
- **Strategic AI**: Four difficulty levels (Easy/Medium/Hard/Expert) with sophisticated decision-making
- **Smart AI**: Opponent prioritises damage moves and switches strategically
- **Smart AI**: Multiple difficulty levels with strategic decision making
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

### Hard AI Strategic Thinking
```
Choose AI Difficulty Level:
[3] - Hard (Smart strategy with switching)

AI Difficulty set to: Hard (Smart strategy with switching)

venusaur used earthquake!
It dealt 156 damage! It's super effective!

// Hard AI recognises type disadvantage and switches
aerodactyl, come back!
Opponent sends out golem!

// Hard AI prioritises super-effective moves
golem used earthquake!
It dealt 189 damage! It's super effective!
```

### Strategic Depth
- **Status Timing**: When to inflict vs cure status conditions
- **Type Synergy**: STAB bonuses change move selection priorities  
- **Weather Strategy**: Choosing moves based on current weather conditions
- **Environmental Awareness**: Protecting vulnerable Pokémon from weather damage
- **Speed Control**: Paralysis and stat modifications affect turn order
- **Risk/Reward**: Status moves vs immediate damage trade-offs
- **AI Adaptation**: Different strategies required for Easy, Medium, and Hard opponents
- **Switching Timing**: Knowing when to switch vs when to attack based on AI difficulty

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

# Test AI difficulty levels
echo "1\n1\n1\n1\n1\n1\n1" | ./bin/pokemon_battle  # Easy AI
echo "1\n1\n2\n2\n2\n2\n2" | ./bin/pokemon_battle  # Medium AI  
echo "1\n1\n3\n3\n3\n3\n3" | ./bin/pokemon_battle  # Hard AI
```

### Code Quality Metrics
- ✅ **Zero compiler warnings** with `-Wall -Wextra`
- ✅ **Modern C++17** practices throughout
- ✅ **Type safety** with strategic `auto` usage
- ✅ **Memory safe** with RAII and smart pointers where needed

## 🎮 Gameplay Modes

### **🏆 Tournament Mode** (Main Feature)
Complete championship experience featuring:
- **8 Gym Leader Challenges**: Type-specialized battles (Fire, Water, Electric, etc.)
- **Elite Four Gauntlet**: 4 consecutive champion-tier battles
- **Champion Showdown**: Ultimate test against the strongest opponent
- **Custom Team Builder**: Create specialized teams for different challenges
- **Progress Tracking**: Badge collection, statistics, and leaderboards

### **⚔️ Quick Battle Mode**
Traditional battle system with:
- Team selection from pre-built configurations
- AI difficulty selection (Easy, Medium, Hard, Expert)
- Complete status condition and weather mechanics
- Enhanced visual feedback with animated health bars


## 🧪 Testing & Quality

### Comprehensive Testing Suite
Built with **Google Test (GTest)** following industry best practices. The test suite ensures code quality and validates all battle mechanics.

**Test Coverage:**
- ✅ **Unit Tests**: Pokemon, moves, teams, weather, type effectiveness, AI behavior
- ✅ **Integration Tests**: Full battles, status interactions, weather integration
- ✅ **138 Test Cases**: Comprehensive coverage across all components

### Running Tests
```bash
# Build and run all tests
cd build && make -j4
ctest -j4

# Run specific test categories
./test_pokemon
./test_battle
./test_ai
./test_weather
```

### Quality Metrics
- ✅ **Zero Compilation Warnings**: Clean builds with `-Wall -Wextra`
- ✅ **Memory Safety**: RAII and smart pointer usage
- ✅ **95%+ Test Pass Rate**: Comprehensive validation of all systems

## 🔮 Future Enhancements

### Planned Features
- **🎨 Battle Animations**: ASCII art and enhanced visual effects
- **🔄 Multi-turn Moves**: Hyper Beam recharge, Solar Beam charging mechanics
- **💎 Generation II**: Johto Pokémon, Steel/Dark types, breeding system
- **🌍 Battle Frontier**: Additional challenge modes beyond the Elite Four

### Technical Improvements  
- **💾 Enhanced Save System**: Tournament progress persistence across sessions
- **🎵 Audio Integration**: Sound effects and battle music
- **📊 Advanced Analytics**: Detailed battle statistics and AI performance metrics
- **🌐 Multiplayer Support**: Network battles and online tournaments
- **🎯 Battle Replay System**: Record and review championship battles

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

---

*Experience the complete Pokémon championship journey with tournament battles!* 🏆⚡

*Last updated: August 24, 2025*

 