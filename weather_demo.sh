#!/bin/bash

echo "🌧️ WEATHER SYSTEM DEMO 🌧️"
echo "========================="
echo ""
echo "This demo will:"
echo "1. Select Team 1"
echo "2. Choose Lapras (has Rain Dance + Waterfall)"
echo "3. Use Rain Dance to start rain"
echo "4. Use Waterfall to see 1.5x water damage boost"
echo ""

# Team 1, Opponent 1, Lapras (5th pokemon), Rain Dance (3rd move), Waterfall (2nd move)
echo "1
1
5
3
2" | ./build/bin/pokemon_battle 