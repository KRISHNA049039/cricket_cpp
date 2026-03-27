#!/bin/bash
echo "Building Cricket Simulation..."
g++ -std=c++17 -O2 -Isrc \
    src/cricket/cricket_main.cpp \
    src/cricket/MatchEngine.cpp \
    src/cricket/CricketTypes.cpp \
    src/cricket/TeamData.cpp \
    -o CricketSim

if [ $? -eq 0 ]; then
    echo "Build successful! Run: ./CricketSim"
else
    echo "Build failed."
fi
