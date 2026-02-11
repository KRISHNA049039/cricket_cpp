# Cricket Game

A production-ready 3D cricket game built with C++, OpenGL 3.3+, and SDL2.

## Features

- 3D graphics rendering with OpenGL
- Realistic ball physics with environmental effects
- Player batting and bowling controls
- AI opponents with difficulty levels
- Match management and score tracking
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
│   ├── core/           # Application core, logging, config
│   ├── rendering/      # OpenGL rendering system
│   ├── physics/        # Physics engine
│   ├── input/          # Input handling
│   ├── gameplay/       # Game logic, AI, match management
│   └── audio/          # Audio system
├── assets/
│   ├── shaders/        # GLSL shaders
│   ├── models/         # 3D models
│   ├── textures/       # Textures
│   └── audio/          # Sound effects
├── external/           # Third-party libraries
├── docs/               # Documentation
└── CMakeLists.txt      # Build configuration
```

## Development Status

This is an MVP (Minimum Viable Product) implementation with core systems in place:
- ✅ Project structure and build system
- ✅ SDL2 and OpenGL initialization
- ✅ Basic rendering pipeline
- ✅ Logging system
- ✅ Configuration management
- ✅ Input handling
- ⏳ Physics engine (in progress)
- ⏳ Gameplay mechanics (in progress)
- ⏳ AI system (in progress)
- ⏳ Audio system (in progress)

## License

Copyright © 2026. All rights reserved.

## Contributing

This is a spec-driven development project. See `.kiro/specs/cricket-game/` for requirements, design, and implementation tasks.
