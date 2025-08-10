#!/bin/bash

# Pokemon Battle Simulator Launcher
# This script ensures the game runs from the correct directory

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Change to the project root directory
cd "$SCRIPT_DIR"

# Check if the executable exists
if [ ! -f "./build/bin/pokemon_battle" ]; then
    echo "❌ Game executable not found! Please build the project first with:"
    echo "   cd build && make pokemon_battle"
    exit 1
fi

# Check if data files exist
if [ ! -d "./data" ]; then
    echo "❌ Data files not found! Please run from the project root directory."
    exit 1
fi

# Launch the game
echo "🎮 Launching Pokemon Battle Simulator..."
echo ""
./build/bin/pokemon_battle