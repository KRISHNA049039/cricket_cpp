# Cricket Game

A cricket simulation game built with C++17. Includes a fully playable console match simulator and a 3D engine foundation (SDL2/OpenGL).

## Play Now (No Setup Needed)

```bash
# Windows - just run:
.\CricketSim.exe

# Or build from source (any C++17 compiler, zero dependencies):
g++ -std=c++17 -O2 -Isrc src/cricket/*.cpp -o CricketSim
```

## Features

### Cricket Simulation (Playable Now)
- Test Match and ODI formats with full rules
- Ball does more in first 40-50 overs in Tests (new ball swing, seam, bounce)
- ODI uses 2 balls from both ends that wear out independently after 40 overs
- Weather system: overcast boosts swing, sunny helps spin, humidity affects movement
- Pitch deterioration over Test match (helps spinners in 4th innings)
- Ball condition tracking: shine, seam integrity, reverse swing
- 2 teams: England and Australia with 11 skill-differentiated players each
- 8 stadiums (4 England, 4 Australia) with unique pitch/weather characteristics
- Player shots differ based on skill: technique, power, temperament
- Full scorecards with batting/bowling figures, fall of wickets, strike rates
- New ball at 80 overs in Tests, bowler rotation, maiden overs

### 3D Engine Foundation (In Progress)
- 3D graphics rendering with OpenGL
- Realistic ball physics with environmental effects
- Player batting and bowling controls
- AI opponents with difficulty levels
- Audio feedback system

## Requirements

### Dependencies

- CMake 3.15 or higher
- C++17 compatible compiler
- SDL2
- SDL2_mixer
- OpenGL 3.3+
- GLM (included in external/)
- GLAD (included in external/)

### Windows

Install dependencies using vcpkg:
```bash
vcpkg install sdl2 sdl2-mixer
```

### Linux

Install dependencies using package manager:
```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev libsdl2-mixer-dev libgl1-mesa-dev

# Fedora
sudo dnf install SDL2-devel SDL2_mixer-devel mesa-libGL-devel
```

### macOS

Install dependencies using Homebrew:
```bash
brew install sdl2 sdl2_mixer
```

## Building

### Windows (Visual Studio)

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### Linux/macOS

```bash
mkdir build
cd build
cmake ..
make -j4
```

## Running

After building, run the executable:

```bash
# Windows
.\build\bin\CricketGame.exe

# Linux/macOS
./build/bin/CricketGame
```

## Controls

### Batting
- A: Defensive shot
- W: Drive shot
- D: Cut shot
- S: Pull shot
- Q: Sweep shot
- E: Lofted shot

### Bowling
- 1: Fast ball
- 2: Medium ball
- 3: Spin ball
- 4: Yorker
- 5: Bouncer
- Space: Release ball

### General
- ESC: Quit game

## Project Structure

```
cricket-game/
├── src/
│   ├── cricket/        # ★ Match simulation engine (standalone, no dependencies)
│   │   ├── cricket_main.cpp    # Console UI entry point
│   │   ├── MatchEngine.cpp/h   # Core simulation (~960 lines)
│   │   ├── CricketTypes.cpp/h  # Data structures, enums, scoring
│   │   └── TeamData.cpp/h      # Teams, players, stadiums
│   ├── core/           # Application core, logging, config
│   ├── rendering/      # OpenGL rendering system
│   ├── physics/        # Physics engine
│   ├── input/          # Input handling
│   ├── gameplay/       # Game logic, AI, match management
│   └── audio/          # Audio system
├── docs/
│   ├── GAME_GUIDE.md              # ★ How to play, all mechanics explained
│   ├── CRICKET_ENGINE_ARCHITECTURE.md  # ★ Simulation engine technical docs
│   ├── ARCHITECTURE.md            # 3D engine architecture
│   ├── CONCEPTS.md                # Cricket and physics concepts
│   ├── CPP_CONCEPTS.md            # C++ patterns and best practices
│   └── SETUP_GUIDE.md             # 3D engine build setup
├── CricketSim.exe      # ★ Pre-built executable (Windows x64)
├── build_sim.bat        # Windows build script
├── build_sim.sh         # Linux/macOS build script
├── CMakeLists.txt       # Full build (3D engine + simulation)
└── CMakeLists_sim.txt   # Simulation-only CMake
```

## Development Status

- ✅ Cricket simulation engine (fully playable)
- ✅ Test and ODI match formats
- ✅ Weather, pitch, ball condition systems
- ✅ 2 teams with skill-differentiated players
- ✅ 8 stadiums (England + Australia)
- ✅ Project structure and build system
- ✅ SDL2 and OpenGL initialization
- ✅ Basic rendering pipeline
- ✅ Logging system
- ✅ Configuration management
- ✅ Input handling
- ⏳ 3D rendering of matches (in progress)
- ⏳ Interactive batting/bowling controls (in progress)
- ⏳ Audio system (in progress)

## Documentation

- [Game Guide](docs/GAME_GUIDE.md) - How to play, mechanics, teams, stadiums
- [Cricket Engine Architecture](docs/CRICKET_ENGINE_ARCHITECTURE.md) - Simulation engine internals
- [3D Engine Architecture](docs/ARCHITECTURE.md) - Rendering/physics system design
- [Cricket & Physics Concepts](docs/CONCEPTS.md) - Game rules and physics models
- [C++ Concepts](docs/CPP_CONCEPTS.md) - C++17 patterns used in the codebase
- [Setup Guide](docs/SETUP_GUIDE.md) - Building the 3D engine with SDL2/OpenGL

## License

Copyright © 2026. All rights reserved.

## Contributing

This is a spec-driven development project. See `.kiro/specs/cricket-game/` for requirements, design, and implementation tasks.
