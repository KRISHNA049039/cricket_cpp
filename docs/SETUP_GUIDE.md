# Cricket Game - Complete Setup Guide

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Platform-Specific Setup](#platform-specific-setup)
3. [Building the Project](#building-the-project)
4. [Troubleshooting](#troubleshooting)
5. [Verifying Installation](#verifying-installation)

---

## Prerequisites

### Required Software

- **CMake**: Version 3.15 or higher
- **C++ Compiler**: Supporting C++17 standard
  - Windows: Visual Studio 2019+ or MinGW-w64
  - Linux: GCC 9+ or Clang 10+
  - macOS: Xcode 12+ or Clang 10+
- **Git**: For cloning the repository

### Required Libraries

- **SDL2**: Version 2.0.10 or higher
- **SDL2_mixer**: Version 2.0.4 or higher
- **OpenGL**: Version 3.3 or higher (usually comes with graphics drivers)

### Included Libraries (No Installation Needed)

These are bundled in the `external/` directory:
- **GLM**: OpenGL Mathematics library
- **GLAD**: OpenGL loader
- **stb_image**: Image loading library
- **RapidJSON**: JSON parsing library

---

## Platform-Specific Setup

### Windows Setup

#### Option 1: Using vcpkg (Recommended)

1. **Install vcpkg**:
```powershell
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# Add to PATH (optional)
setx PATH "%PATH%;%CD%"
```

2. **Install Dependencies**:
```powershell
# Install SDL2 and SDL2_mixer
vcpkg install sdl2:x64-windows
vcpkg install sdl2-mixer:x64-windows

# Integrate with Visual Studio
vcpkg integrate install
```

3. **Set CMake Toolchain** (if not integrated):
```powershell
# When running cmake, add:
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
```

#### Option 2: Manual Installation

1. **Download SDL2**:
   - Visit: https://www.libsdl.org/download-2.0.php
   - Download: SDL2-devel-2.x.x-VC.zip (Visual C++)
   - Extract to: `C:\SDL2`

2. **Download SDL2_mixer**:
   - Visit: https://www.libsdl.org/projects/SDL_mixer/
   - Download: SDL2_mixer-devel-2.x.x-VC.zip
   - Extract to: `C:\SDL2_mixer`

3. **Set Environment Variables**:
```powershell
# Add to System PATH
setx SDL2_DIR "C:\SDL2"
setx SDL2_MIXER_DIR "C:\SDL2_mixer"
```

4. **Update CMakeLists.txt**:
```cmake
# Add these lines if SDL2 is not found automatically
set(SDL2_DIR "C:/SDL2/cmake")
set(SDL2_MIXER_DIR "C:/SDL2_mixer/cmake")
```

---

### Linux Setup

#### Ubuntu/Debian

```bash
# Update package list
sudo apt-get update

# Install build tools
sudo apt-get install build-essential cmake git

# Install SDL2 and dependencies
sudo apt-get install libsdl2-dev libsdl2-mixer-dev

# Install OpenGL development files
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev

# Verify installation
sdl2-config --version
```

#### Fedora/RHEL/CentOS

```bash
# Install build tools
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git

# Install SDL2 and dependencies
sudo dnf install SDL2-devel SDL2_mixer-devel

# Install OpenGL development files
sudo dnf install mesa-libGL-devel mesa-libGLU-devel

# Verify installation
sdl2-config --version
```

#### Arch Linux

```bash
# Install build tools
sudo pacman -S base-devel cmake git

# Install SDL2 and dependencies
sudo pacman -S sdl2 sdl2_mixer

# Install OpenGL development files
sudo pacman -S mesa

# Verify installation
sdl2-config --version
```

---

### macOS Setup

#### Using Homebrew (Recommended)

1. **Install Homebrew** (if not installed):
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

2. **Install Dependencies**:
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install CMake
brew install cmake

# Install SDL2 and SDL2_mixer
brew install sdl2 sdl2_mixer

# Verify installation
sdl2-config --version
cmake --version
```

#### Using MacPorts

```bash
# Install dependencies
sudo port install cmake
sudo port install libsdl2 libsdl2_mixer

# Verify installation
sdl2-config --version
```

---

## Building the Project

### Step 1: Clone the Repository

```bash
git clone <repository-url>
cd cricket-game
```

### Step 2: Download External Libraries

Create the `external/` directory structure and download required libraries:

```bash
# Create external directory
mkdir -p external

# Download GLM
cd external
git clone https://github.com/g-truc/glm.git

# Download GLAD
# Visit: https://glad.dav1d.de/
# Select: OpenGL 3.3, Core profile, Generate loader
# Extract to: external/glad/

# Download stb_image
mkdir -p stb
cd stb
curl -O https://raw.githubusercontent.com/nothings/stb/master/stb_image.h

# Download RapidJSON
cd ..
git clone https://github.com/Tencent/rapidjson.git

cd ../..
```

### Step 3: Build with CMake

#### Windows (Visual Studio)

```powershell
# Create build directory
mkdir build
cd build

# Generate Visual Studio project
cmake .. -G "Visual Studio 16 2019" -A x64

# Build the project
cmake --build . --config Release

# Or open CricketGame.sln in Visual Studio and build
```

#### Windows (MinGW)

```powershell
# Create build directory
mkdir build
cd build

# Generate Makefiles
cmake .. -G "MinGW Makefiles"

# Build the project
mingw32-make -j4
```

#### Linux/macOS

```bash
# Create build directory
mkdir build
cd build

# Generate Makefiles
cmake ..

# Build the project (use -j for parallel compilation)
make -j$(nproc)  # Linux
make -j$(sysctl -n hw.ncpu)  # macOS

# Or simply
make -j4
```

### Step 4: Run the Game

```bash
# Windows
.\build\bin\Release\CricketGame.exe

# Linux/macOS
./build/bin/CricketGame
```

---

## Advanced Build Options

### Debug Build

```bash
# Configure for debug
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build
make -j4

# Run with debugger
gdb ./build/bin/CricketGame
```

### Release Build with Optimizations

```bash
# Configure for release
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j4
```

### Custom Installation Prefix

```bash
# Configure with custom install location
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/cricket-game

# Build and install
make -j4
sudo make install
```

### Specify Compiler

```bash
# Use specific compiler
cmake .. -DCMAKE_C_COMPILER=gcc-11 -DCMAKE_CXX_COMPILER=g++-11

# Or use Clang
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
```

---

## Troubleshooting

### SDL2 Not Found

**Problem**: CMake cannot find SDL2

**Solution**:
```bash
# Linux: Install development packages
sudo apt-get install libsdl2-dev

# Windows: Set SDL2_DIR environment variable
set SDL2_DIR=C:\path\to\SDL2\cmake

# macOS: Ensure Homebrew path is in CMAKE_PREFIX_PATH
export CMAKE_PREFIX_PATH=/usr/local
```

### OpenGL Version Too Old

**Problem**: OpenGL 3.3 not supported

**Solution**:
- Update graphics drivers
- Check OpenGL version:
```bash
# Linux
glxinfo | grep "OpenGL version"

# Windows: Use GPU-Z or similar tool
```

### Missing GLAD Files

**Problem**: GLAD headers not found

**Solution**:
1. Visit https://glad.dav1d.de/
2. Select: Language=C/C++, API gl=Version 3.3, Profile=Core
3. Click "Generate"
4. Download and extract to `external/glad/`

### Linking Errors on Linux

**Problem**: Undefined references to SDL or OpenGL functions

**Solution**:
```bash
# Install missing development packages
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev

# Verify pkg-config can find SDL2
pkg-config --cflags --libs sdl2
```

### Runtime DLL Errors on Windows

**Problem**: Missing SDL2.dll or SDL2_mixer.dll

**Solution**:
1. Copy DLLs from SDL2 installation to executable directory
2. Or add SDL2 bin directory to PATH:
```powershell
setx PATH "%PATH%;C:\SDL2\lib\x64"
```

### macOS Code Signing Issues

**Problem**: "App is damaged and can't be opened"

**Solution**:
```bash
# Remove quarantine attribute
xattr -cr /path/to/CricketGame.app

# Or disable Gatekeeper temporarily
sudo spctl --master-disable
```

---

## Verifying Installation

### Check Dependencies

```bash
# Check CMake version
cmake --version

# Check compiler version
g++ --version      # Linux
clang++ --version  # macOS
cl                 # Windows (Visual Studio)

# Check SDL2
sdl2-config --version  # Linux/macOS
```

### Test Build

Create a simple test program:

```cpp
// test_sdl.cpp
#include <SDL2/SDL.h>
#include <iostream>

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "SDL initialized successfully!" << std::endl;
    SDL_Quit();
    return 0;
}
```

Compile and run:
```bash
# Linux/macOS
g++ test_sdl.cpp -o test_sdl `sdl2-config --cflags --libs`
./test_sdl

# Windows (with vcpkg)
cl test_sdl.cpp /I"C:\vcpkg\installed\x64-windows\include" /link SDL2.lib
```

### Verify OpenGL

```cpp
// test_opengl.cpp
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    
    SDL_Window* window = SDL_CreateWindow("Test", 0, 0, 800, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
```

---

## Next Steps

After successful setup:

1. Read [ARCHITECTURE.md](ARCHITECTURE.md) to understand the codebase
2. Read [CONCEPTS.md](CONCEPTS.md) to learn about game systems
3. Check [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines
4. Review `.kiro/specs/cricket-game/` for implementation tasks

---

## Getting Help

- **Build Issues**: Check CMake output for specific errors
- **Runtime Issues**: Check `logs/game.log` for error messages
- **Graphics Issues**: Verify OpenGL version and driver updates
- **Audio Issues**: Ensure SDL2_mixer is properly installed

For more help, consult the project documentation or open an issue.
