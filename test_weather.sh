#!/bin/bash

echo "🌦️ Testing Pokemon Battle Simulator Weather System!"
echo "=================================================="
echo ""
echo "Test scenario:"
echo "1. Select Team 1 (contains Lapras with Rain Dance and Arcanine with Sunny Day)"
echo "2. Choose Lapras first"
echo "3. Use Rain Dance to start rain"
echo "4. Use Water moves to see boosted damage"
echo ""
echo "Expected behavior:"
echo "- Rain Dance should start rain for 5 turns"
echo "- Water moves should do 1.5x damage"
echo "- Fire moves should do 0.5x damage"
echo ""
echo "Commands to send:"
echo "1        (select Team 1)"
echo "1        (select Opponent Team 1)" 
echo "5        (select Lapras)"
echo "3        (use Rain Dance - 3rd move)"
echo "2        (use Waterfall - 2nd move)"
echo ""

# Run the battle simulator with automated input
echo "1
1
5
3
2
1
1" | ./build/bin/pokemon_battle 