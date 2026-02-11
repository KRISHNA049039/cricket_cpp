# Cricket Game Design Document

## Overview

This document outlines the technical design for a production-ready 3D cricket game built with C++, OpenGL 3.3+, and SDL2. The architecture follows a modular, component-based design with clear separation of concerns between rendering, physics, input handling, game logic, and audio systems.

The game will support single-player matches against AI opponents with configurable difficulty levels, realistic ball physics affected by environmental conditions, and smooth 60+ FPS performance on mid-range hardware.

## Architecture

### High-Level Architecture

The system follows a layered architecture pattern:

```
┌─────────────────────────────────────────────────────┐
│              Game Application Layer                  │
│  (Main Loop, State Management, Match Controller)    │
└─────────────────────────────────────────────────────┘
                        │
        ┌───────────────┼───────────────┐
        ▼               ▼               ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│   Gameplay   │ │   Rendering  │ │    Audio     │
│    Layer     │ │    Layer     │ │    Layer     │
└──────────────┘ └──────────────┘ └──────────────┘
        │               │               │
        ▼               ▼               ▼
┌──────────────────────────────────────────────────────┐
│              Core Systems Layer                       │
│  (Physics, Input, Resource Manager, Config)          │
└──────────────────────────────────────────────────────┘
        │
        ▼
┌──────────────────────────────────────────────────────┐
│           Platform Layer (SDL2, OpenGL)              │
└──────────────────────────────────────────────────────┘
```

### Core Design Principles

1. **Entity-Component Pattern**: Game objects (players, ball, field) use component-based design for flexibility
2. **Fixed Timestep Physics**: Physics updates at 120Hz independent of rendering framerate
3. **Resource Pooling**: Frequently created objects (particles, trails) use object pools
4. **Data-Driven Configuration**: Game parameters loaded from JSON/XML files
5. **State Machine Pattern**: Game states (menu, playing, paused) managed via state machine

## Components and Interfaces

### 1. Core Engine Components

#### Application Class
```cpp
class Application {
public:
    bool Initialize();
    void Run();
    void Shutdown();
    
private:
    void ProcessInput();
    void Update(float deltaTime);
    void Render();
    
    SDL_Window* window;
    SDL_GLContext glContext;
    bool isRunning;
    
    std::unique_ptr<GameStateManager> stateManager;
    std::unique_ptr<ResourceManager> resourceManager;
};
```

#### GameStateManager
```cpp
enum class GameState {
    MainMenu,
    MatchSetup,
    Playing,
    Paused,
    MatchEnd
};

class GameStateManager {
public:
    void PushState(std::unique_ptr<IGameState> state);
    void PopState();
    void ChangeState(std::unique_ptr<IGameState> state);
    
    void Update(float deltaTime);
    void Render();
    void HandleInput(const InputState& input);
    
private:
    std::vector<std::unique_ptr<IGameState>> stateStack;
};
```

### 2. Rendering System

#### Renderer Architecture
```cpp
class Renderer {
public:
    bool Initialize(int width, int height);
    void BeginFrame();
    void EndFrame();
    
    void RenderMesh(const Mesh& mesh, const Transform& transform, 
                    const Material& material);
    void RenderSkybox(const Texture& skybox);
    void RenderUI(const UIElement& element);
    
    void SetCamera(const Camera& camera);
    void SetLighting(const LightingParams& params);
    
private:
    ShaderManager shaderManager;
    Camera* activeCamera;
    
    // Render passes
    void GeometryPass();
    void LightingPass();
    void PostProcessPass();
};
```

#### Shader System
- **Vertex Shader**: Transform vertices, pass data to fragment shader
- **Fragment Shader**: Calculate lighting, apply textures, output color
- **Shader Programs**:
  - `player.shader`: Skeletal animation + Phong lighting
  - `field.shader`: Terrain rendering with texture blending
  - `ball.shader`: Simple Phong with motion blur support
  - `ui.shader`: 2D orthographic rendering with alpha blending

#### Camera System
```cpp
class Camera {
public:
    enum class Mode {
        BowlerView,      // Behind bowler
        BatsmanView,     // Behind batsman
        FollowBall,      // Dynamic ball tracking
        Cinematic        // Replay camera
    };
    
    void SetMode(Mode mode);
    void Update(float deltaTime, const Ball& ball);
    
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    
private:
    glm::vec3 position;
    glm::vec3 target;
    float fov;
    Mode currentMode;
};
```

### 3. Physics System

#### Physics Engine
```cpp
class PhysicsEngine {
public:
    void Update(float fixedDeltaTime);  // Called at 120Hz
    
    void SetEnvironment(const EnvironmentParams& env);
    
    BallTrajectory CalculateBallTrajectory(
        const glm::vec3& initialPos,
        const glm::vec3& initialVelocity,
        const glm::vec3& spin,
        const EnvironmentParams& env
    );
    
    CollisionResult CheckBatBallCollision(
        const Ball& ball,
        const Bat& bat
    );
    
private:
    void IntegrateVelocity(Ball& ball, float dt);
    void ApplyAerodynamics(Ball& ball, const EnvironmentParams& env);
    void ApplyMagnusEffect(Ball& ball);  // Spin-induced curve
    
    const float GRAVITY = 9.8f;
    const float AIR_DENSITY = 1.225f;
};
```

#### Environmental Effects
```cpp
struct EnvironmentParams {
    // Weather
    float humidity;           // 0.0 to 1.0
    float windSpeed;          // km/h
    glm::vec3 windDirection;  // Normalized vector
    float temperature;        // Celsius
    
    // Pitch
    float pitchHardness;      // 0.0 (soft) to 1.0 (hard)
    float grassCoverage;      // 0.0 to 1.0
    float moisture;           // 0.0 (dry) to 1.0 (wet)
    
    // Calculated factors
    float GetSwingFactor() const;
    float GetBounceFactor() const;
    float GetSeamMovementFactor() const;
    float GetFrictionCoefficient() const;
};
```

#### Ball Physics Implementation
```cpp
class Ball {
public:
    void Update(float dt, const EnvironmentParams& env);
    
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 spin;  // Radians per second
    
    float mass = 0.156f;  // kg (standard cricket ball)
    float radius = 0.036f;  // meters
    float dragCoefficient = 0.47f;
    
private:
    void ApplySwing(const EnvironmentParams& env);
    void ApplySeamMovement(const EnvironmentParams& env);
    void HandleBounce(const EnvironmentParams& env);
};
```

### 4. Input System

#### Input Handler
```cpp
class InputHandler {
public:
    void Update();
    
    bool IsKeyPressed(SDL_Scancode key) const;
    bool IsKeyJustPressed(SDL_Scancode key) const;
    bool IsMouseButtonPressed(int button) const;
    
    glm::vec2 GetMousePosition() const;
    glm::vec2 GetMouseDelta() const;
    
    // Gamepad support
    float GetAxisValue(int axis) const;
    bool IsButtonPressed(int button) const;
    
private:
    std::array<bool, SDL_NUM_SCANCODES> currentKeys;
    std::array<bool, SDL_NUM_SCANCODES> previousKeys;
    
    SDL_GameController* gamepad;
};
```

#### Control Mapping
```cpp
struct ControlScheme {
    // Batting
    SDL_Scancode defensiveShot = SDL_SCANCODE_A;
    SDL_Scancode driveShot = SDL_SCANCODE_W;
    SDL_Scancode cutShot = SDL_SCANCODE_D;
    SDL_Scancode pullShot = SDL_SCANCODE_S;
    SDL_Scancode sweepShot = SDL_SCANCODE_Q;
    SDL_Scancode loftedShot = SDL_SCANCODE_E;
    
    // Bowling
    SDL_Scancode fastBall = SDL_SCANCODE_1;
    SDL_Scancode mediumBall = SDL_SCANCODE_2;
    SDL_Scancode spinBall = SDL_SCANCODE_3;
    SDL_Scancode yorker = SDL_SCANCODE_4;
    SDL_Scancode bouncer = SDL_SCANCODE_5;
    
    SDL_Scancode releaseBall = SDL_SCANCODE_SPACE;
};
```

### 5. AI System

#### AI Controller
```cpp
class AIController {
public:
    enum class Difficulty {
        Beginner,   // 300ms reaction, 60% accuracy
        Medium,     // 150ms reaction, 75% accuracy
        Hard,       // 75ms reaction, 85% accuracy
        Expert      // 50ms reaction, 95% accuracy
    };
    
    void SetDifficulty(Difficulty diff);
    
    // Batting AI
    ShotType DecideBattingShot(const BallTrajectory& trajectory,
                                const FieldPlacement& field);
    float CalculateShotTiming(const Ball& ball);
    
    // Bowling AI
    DeliveryType DecideBowlingDelivery(const BatsmanStats& stats,
                                        int ballsRemaining);
    glm::vec3 SelectBowlingTarget(const Batsman& batsman);
    
private:
    Difficulty difficulty;
    float reactionTime;
    float accuracy;
    
    std::mt19937 rng;  // Random number generator for variation
};
```

### 6. Game Logic Components

#### Match Manager
```cpp
class MatchManager {
public:
    void StartMatch(const MatchConfig& config);
    void Update(float deltaTime);
    
    void OnBallDelivered(const Ball& ball);
    void OnShotPlayed(ShotType shot, float timing);
    void OnRunsScored(int runs);
    void OnWicketFallen(WicketType type);
    void OnOverCompleted();
    
    const MatchState& GetMatchState() const;
    
private:
    MatchState state;
    Team battingTeam;
    Team bowlingTeam;
    
    int currentOver;
    int ballsInOver;
    
    void SwitchInnings();
    void UpdateBatsman();
    void UpdateBowler();
};
```

#### Score Tracking
```cpp
struct MatchState {
    int runs;
    int wickets;
    int overs;
    int balls;
    
    float runRate;
    float requiredRunRate;
    
    Player* striker;
    Player* nonStriker;
    Player* bowler;
    
    std::vector<int> overScores;
    std::vector<WicketInfo> wicketsFallen;
};
```

### 7. Audio System

#### Audio Manager
```cpp
class AudioManager {
public:
    bool Initialize();
    void Shutdown();
    
    void PlaySound(const std::string& soundName, float volume = 1.0f);
    void PlayMusic(const std::string& musicName, bool loop = true);
    
    void SetMasterVolume(float volume);
    void SetSFXVolume(float volume);
    void SetMusicVolume(float volume);
    
private:
    Mix_Music* backgroundMusic;
    std::unordered_map<std::string, Mix_Chunk*> soundEffects;
    
    float masterVolume = 1.0f;
    float sfxVolume = 0.8f;
    float musicVolume = 0.5f;
};
```

#### Sound Events
- `bat_hit_soft.wav`: Defensive shot
- `bat_hit_hard.wav`: Aggressive shot
- `stumps_break.wav`: Wicket fallen
- `crowd_cheer.wav`: Boundary scored
- `crowd_ambient.wav`: Background atmosphere
- `ball_bounce.wav`: Ball hitting pitch

### 8. Resource Management

#### Resource Manager
```cpp
class ResourceManager {
public:
    static ResourceManager& Instance();
    
    Texture* LoadTexture(const std::string& path);
    Mesh* LoadMesh(const std::string& path);
    Shader* LoadShader(const std::string& vertPath, 
                       const std::string& fragPath);
    
    void UnloadAll();
    
private:
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
};
```

## Data Models

### Player Model
```cpp
struct PlayerStats {
    std::string name;
    int battingSkill;    // 0-100
    int bowlingSkill;    // 0-100
    int fieldingSkill;   // 0-100
    int stamina;         // 0-100
    
    // Career stats
    int matchesPlayed;
    int runsScored;
    int wicketsTaken;
};

class Player {
public:
    PlayerStats stats;
    Transform transform;
    AnimationController animator;
    
    void UpdateAnimation(float deltaTime);
    void SetPose(PlayerPose pose);
};
```

### Field Configuration
```cpp
struct FieldPlacement {
    std::array<glm::vec3, 11> fielderPositions;
    
    static FieldPlacement GetPreset(const std::string& name);
    // Presets: "attacking", "defensive", "balanced"
};
```

### Match Configuration
```cpp
struct MatchConfig {
    int oversPerInnings;
    int playersPerTeam;
    
    Team team1;
    Team team2;
    
    EnvironmentParams environment;
    
    AIController::Difficulty aiDifficulty;
    
    std::string venue;
};
```

## Error Handling

### Error Categories

1. **Initialization Errors**
   - SDL initialization failure → Log error, show message box, exit
   - OpenGL context creation failure → Log error, show message box, exit
   - Shader compilation failure → Log error, load fallback shader, continue

2. **Runtime Errors**
   - Resource loading failure → Log warning, use placeholder resource
   - Audio playback failure → Log warning, continue without sound
   - Physics calculation overflow → Clamp values, log warning

3. **Configuration Errors**
   - Invalid config values → Use defaults, log warning
   - Missing config file → Create default config, continue

### Logging System
```cpp
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class Logger {
public:
    static void Log(LogLevel level, const std::string& message);
    static void SetLogFile(const std::string& path);
    
private:
    static std::ofstream logFile;
};
```

## Testing Strategy

### Unit Testing
- **Physics calculations**: Verify ball trajectory, collision detection, environmental effects
- **AI decision making**: Test shot selection, bowling variation
- **Score management**: Validate run counting, wicket tracking, over progression

### Integration Testing
- **Rendering pipeline**: Verify shader compilation, mesh loading, texture binding
- **Input handling**: Test keyboard, mouse, gamepad input processing
- **Audio system**: Verify sound playback, volume control

### Performance Testing
- **Frame rate**: Maintain 60+ FPS on target hardware
- **Memory usage**: Stay under 2GB RAM
- **Load times**: Initial load under 3 seconds

### Test Framework
```cpp
// Using Catch2 or Google Test
TEST_CASE("Ball physics with high humidity increases swing") {
    PhysicsEngine physics;
    EnvironmentParams env;
    env.humidity = 0.8f;
    
    Ball ball;
    ball.velocity = glm::vec3(30.0f, 0.0f, 0.0f);
    ball.spin = glm::vec3(0.0f, 10.0f, 0.0f);
    
    float initialSwing = CalculateSwing(ball, env);
    
    env.humidity = 0.4f;
    float lowHumiditySwing = CalculateSwing(ball, env);
    
    REQUIRE(initialSwing > lowHumiditySwing * 1.2f);
}
```

## Performance Optimization

### Rendering Optimizations
1. **Frustum Culling**: Don't render objects outside camera view
2. **Level of Detail (LOD)**: Use lower poly models for distant objects
3. **Instanced Rendering**: Render multiple fielders with single draw call
4. **Texture Atlasing**: Combine multiple textures to reduce state changes

### Physics Optimizations
1. **Spatial Partitioning**: Use octree for collision detection
2. **Predictive Collision**: Calculate collision time analytically when possible
3. **Sleep States**: Don't update stationary objects

### Memory Optimizations
1. **Object Pooling**: Reuse particle effects, ball trails
2. **Lazy Loading**: Load assets on demand for menus
3. **Texture Compression**: Use DXT/BC compression for textures

## Build and Deployment

### Project Structure
```
cricket-game/
├── src/
│   ├── core/           # Application, main loop
│   ├── rendering/      # Renderer, shaders, camera
│   ├── physics/        # Physics engine, collision
│   ├── gameplay/       # Match manager, AI, rules
│   ├── audio/          # Audio manager
│   ├── input/          # Input handler
│   └── utils/          # Logger, math helpers
├── assets/
│   ├── models/         # 3D meshes (.obj, .fbx)
│   ├── textures/       # Texture files (.png, .jpg)
│   ├── shaders/        # GLSL shader files
│   ├── audio/          # Sound effects and music
│   └── config/         # JSON configuration files
├── external/           # Third-party libraries
│   ├── SDL2/
│   ├── glm/
│   ├── glad/
│   └── stb_image/
├── docs/               # Documentation
├── tests/              # Unit and integration tests
└── CMakeLists.txt      # Build configuration
```

### Dependencies
- **SDL2**: Window management, input, audio
- **OpenGL 3.3+**: Graphics rendering
- **GLM**: Mathematics library
- **GLAD**: OpenGL loader
- **stb_image**: Image loading
- **SDL2_mixer**: Audio mixing
- **RapidJSON**: Configuration parsing
- **Catch2/Google Test**: Unit testing (optional)

### Build System (CMake)
```cmake
cmake_minimum_required(VERSION 3.15)
project(CricketGame)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(CricketGame
    src/main.cpp
    # ... other source files
)

target_link_libraries(CricketGame
    SDL2::SDL2
    SDL2::SDL2_mixer
    OpenGL::GL
)
```

### Platform Support
- **Windows**: Visual Studio 2019+, MinGW
- **Linux**: GCC 9+, Clang 10+
- **macOS**: Xcode 12+, Clang 10+

## Configuration Files

### Graphics Settings (graphics.json)
```json
{
  "resolution": {
    "width": 1920,
    "height": 1080
  },
  "fullscreen": false,
  "vsync": true,
  "quality": "high",
  "shadows": true,
  "antialiasing": "MSAA4x",
  "fov": 75.0
}
```

### Game Settings (game.json)
```json
{
  "difficulty": "medium",
  "oversPerInnings": 20,
  "environment": {
    "humidity": 0.6,
    "windSpeed": 5.0,
    "pitchHardness": 0.7,
    "grassCoverage": 0.4
  }
}
```

## Documentation Requirements

### Code Documentation
- **Header files**: Doxygen-style comments for all public APIs
- **Implementation files**: Comments for complex algorithms
- **README.md**: Build instructions, dependencies, quick start

### User Documentation
- **User Manual**: Controls, game modes, settings
- **Quick Start Guide**: Installation, first match
- **Troubleshooting**: Common issues and solutions

### Developer Documentation
- **Architecture Overview**: This design document
- **API Reference**: Generated from Doxygen
- **Contributing Guide**: Code style, pull request process
