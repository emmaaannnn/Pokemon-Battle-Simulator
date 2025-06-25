# Pokémon Battle Simulator

A comprehensive C++ implementation of a Pokémon battle system that simulates turn-based combat with authentic game mechanics.

## 🎮 Features

- **Complete Battle System**: Turn-based combat with move priority, type effectiveness, and damage calculation
- **Extensive Move Database**: 300+ moves with authentic stats and effects from the Pokémon games
- **Full Pokémon Roster**: 150+ Pokémon from Generation I with accurate stats and movesets
- **Team Management**: Create and manage teams of up to 6 Pokémon
- **JSON Data Storage**: Modular data structure for easy expansion and modification
- **Modern C++**: Written in C++17 with clean, object-oriented design

## 📁 Project Structure

```
Pokemon-Battle-Sim/
├── src/                    # Source code
│   ├── main.cpp           # Main application entry point
│   ├── battle.cpp/h       # Battle system implementation
│   ├── pokemon.cpp/h      # Pokémon class and logic
│   ├── move.cpp/h         # Move system and effects
│   └── team.cpp/h         # Team management
├── include/               # External libraries
│   └── json.hpp          # nlohmann/json library
├── data/                  # Game data files
│   ├── moves/            # Move definitions (JSON)
│   ├── pokemon/          # Pokémon stats and data (JSON)
│   └── teams/            # Pre-built team configurations
├── examples/             # Example implementations
│   ├── game-demo.cpp     # Interactive battle demo
│   ├── debug-test.cpp    # Testing and debugging
│   └── working-battle.cpp # Alternative battle implementation
├── tests/                # Unit tests (future implementation)
├── docs/                 # Documentation
│   └── IMPROVEMENTS.md   # Development roadmap
├── build/                # Build artifacts (generated)
├── CMakeLists.txt        # Build configuration
└── README.md            # This file
```

## 🚀 Getting Started

### Prerequisites

- **C++ Compiler**: GCC 7+ or Clang 5+ with C++17 support
- **CMake**: Version 3.16 or higher
- **Operating System**: Linux, macOS, or Windows

### Building the Project

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd Pokemon-Battle-Sim
   ```

2. **Create build directory**:
   ```bash
   mkdir build && cd build
   ```

3. **Generate build files**:
   ```bash
   cmake ..
   ```

4. **Compile the project**:
   ```bash
   make
   ```

5. **Run the battle simulator**:
   ```bash
   ./bin/pokemon_battle
   ```

### Quick Start Examples

Run the interactive demo:
```bash
# From the build directory
./bin/game-demo
```

Run debug tests:
```bash
./bin/debug-test
```

## 🎯 Usage

### Basic Battle Setup

```cpp
#include "team.h"
#include "battle.h"

// Create teams from JSON files
Team playerTeam("data/teams/Team-1.json");
Team opponentTeam("data/teams/Team-2.json");

// Initialize and start battle
Battle battle(playerTeam, opponentTeam);
battle.startBattle();
```

### Creating Custom Pokémon

```cpp
#include "pokemon.h"

// Load Pokémon from data file
Pokemon pikachu("data/pokemon/pikachu.json");

// Or create programmatically
Pokemon customPokemon("CustomMon", 100, 95, 85, 90, 80, 75);
customPokemon.learnMove(Move("thunderbolt"));
```

## 🔧 Game Mechanics

### Battle System
- **Turn Order**: Determined by Pokémon speed and move priority
- **Damage Calculation**: Includes attack/defense stats, move power, and type effectiveness
- **Status Effects**: Poison, paralysis, sleep, and other conditions
- **Critical Hits**: Random chance for increased damage

### Move System
- **Physical/Special**: Different damage calculation methods
- **Status Moves**: Non-damaging moves that affect stats or conditions
- **Priority System**: Some moves always go first regardless of speed
- **PP (Power Points)**: Limited uses per move

### Type Effectiveness
Full type chart implementation with:
- Super effective (2x damage)
- Not very effective (0.5x damage)
- No effect (0x damage)

## 📊 Data Format

### Pokémon JSON Structure
```json
{
  "name": "Pikachu",
  "type1": "Electric",
  "type2": "",
  "hp": 35,
  "attack": 55,
  "defense": 40,
  "special_attack": 50,
  "special_defense": 50,
  "speed": 90,
  "moves": ["thunderbolt", "quick-attack", "thunder-wave"]
}
```

### Move JSON Structure
```json
{
  "name": "Thunderbolt",
  "type": "Electric",
  "category": "Special",
  "power": 90,
  "accuracy": 100,
  "pp": 15,
  "priority": 0,
  "description": "A strong electric blast that may paralyze the target."
}
```

## 🧪 Testing

Run the included test suite:
```bash
cd build
make test
```

For manual testing, use the debug example:
```bash
./bin/debug-test
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/new-mechanic`)
3. Commit your changes (`git commit -am 'Add new battle mechanic'`)
4. Push to the branch (`git push origin feature/new-mechanic`)
5. Create a Pull Request

### Development Guidelines
- Follow C++17 standards
- Maintain const-correctness
- Use RAII for resource management
- Write descriptive commit messages
- Include tests for new features

## 📈 Roadmap

See `docs/IMPROVEMENTS.md` for planned features and enhancements:
- [ ] AI opponent implementation
- [ ] Online multiplayer support
- [ ] Generation II+ Pokémon and moves
- [ ] Advanced battle mechanics (abilities, held items)
- [ ] GUI interface
- [ ] Save/load game states

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- **Game Freak & Nintendo**: For creating the original Pokémon games
- **nlohmann/json**: For the excellent JSON library
- **Pokémon Database**: For move and Pokémon data reference

## 📞 Support

If you encounter any issues or have questions:
1. Check the `docs/` directory for additional documentation
2. Review existing issues in the GitHub repository
3. Create a new issue with detailed information about your problem

---

**Note**: This is a fan-made educational project and is not affiliated with or endorsed by Nintendo, Game Freak, or The Pokémon Company. 