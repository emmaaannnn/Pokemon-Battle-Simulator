# Pokémon Battle Simulator

## Overview
The Pokémon Battle Simulator is a command-line project that allows users to simulate battles between different Pokémon. Players can select their teams, choose moves for each Pokémon, and compete in turn-based battles similar to the classic Pokémon games.

## Features
- **Team Selection**: Choose from a variety of Pokémon to form your battle team.
- **Moves API Integration**: Each Pokémon has a unique set of moves fetched from an integrated API.
- **Turn-Based Battle System**: A simple battle mechanic where each Pokémon takes turns to use their moves.
- **Stats and Move Effects**: Includes move accuracy, power, and other effects that impact the battle outcome.

## Setup
To run this project locally, follow these steps:

### Prerequisites
- C++ Compiler (such as g++, part of the GCC suite)
- JSON library (for reading Pokémon and move data)
- Internet connection (optional, for fetching latest data)

### Installation Steps
1. Clone the repository:
   ```sh
   git clone https://github.com/emmaaannnn/Pokemon-Battle-Simulator.git
   ```
2. Navigate to the project directory:
   ```sh
   cd Pokemon-Battle-Simulator
   ```
3. Compile the project:
   ```sh
   g++ main.cpp -o pokemon_battle_simulator
   ```
4. Run the executable:
   ```sh
   ./pokemon_battle_simulator
   ```

## Usage
After starting the battle simulator, you will be prompted to:
1. **Enter Your Username**: Start by creating your profile.
2. **Select Your Team**: Choose a set of Pokémon from the available options.
3. **Select Moves**: Each Pokémon has predefined moves that can be used in battle.
4. **Battle**: Take turns attacking the opponent’s Pokémon until one trainer is victorious.

### Example
```sh
Welcome to the Pokémon Battle Simulator!
Enter your username: Ash
Select your team:
1. Pikachu
2. Charizard
...
Your opponent is Gym Leader Giovanni!

Charizard used Flamethrower!
Gym Leader Giovanni's Persian fainted!
```

## Project Structure
- **main.cpp**: Entry point for the application.
- **Move.cpp**: Contains the move definitions and logic.
- **Pokemon.cpp**: Manages individual Pokémon properties and actions.
- **API/**: Directory containing API interaction scripts for fetching Pokémon and move data.

## Contributing
1. Fork the repository.
2. Create your feature branch:
   ```sh
   git checkout -b feature/amazing_feature
   ```
3. Commit your changes:
   ```sh
   git commit -m "Add amazing feature"
   ```
4. Push to the branch:
   ```sh
   git push origin feature/amazing_feature
   ```
5. Open a pull request.

## License
This project is licensed under the MIT License. See `LICENSE` for more details.

## Acknowledgements
- [PokéAPI](https://pokeapi.co/) for providing data on Pokémon and moves.
- [Contributors](https://github.com/emmaaannnn/Pokemon-Battle-Simulator/graphs/contributors) who helped make this project possible.

