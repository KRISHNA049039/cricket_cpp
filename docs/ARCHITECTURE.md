# Cricket Game - Architecture Documentation

## Table of Contents
1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Core Systems](#core-systems)
4. [Rendering Pipeline](#rendering-pipeline)
5. [Physics System](#physics-system)
6. [Gameplay Systems](#gameplay-systems)
7. [Data Flow](#data-flow)
8. [Design Patterns](#design-patterns)

---

## Overview

The Cricket Game is built using a modular, component-based architecture that separates concerns into distinct subsystems. The architecture follows modern C++ practices and game engine design principles.

### Key Architectural Principles

1. **Separation of Concerns**: Each system has a single, well-defined responsibility
2. **Loose Coupling**: Systems communicate through interfaces and events
3. **High Cohesion**: Related functionality is grouped together
4. **Data-Driven Design**: Game parameters are configurable through external files
5. **Performance First**: Fixed timestep physics, object pooling, and efficient rendering

---

## System Architecture

### High-Level Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                        │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ Main Loop    │  │ State Manager│  │ Event System │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        ▼                   ▼                   ▼
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│   Gameplay   │    │  Rendering   │    │    Audio     │
│    Layer     │    │    Layer     │    │    Layer     │
│              │    │              │    │              │
│ • Match Mgr  │    │ • Renderer   │    │ • Audio Mgr  │
│ • AI Control │    │ • Camera     │    │ • Sound FX   │
│ • Players    │    │ • Shaders    │    │ • Music      │
└──────────────┘    └──────────────┘    └──────────────┘
        │                   │                   │
        └───────────────────┼───────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    Core Systems Layer                        │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │ Physics  │  │  Input   │  │ Resource │  │  Config  │   │
│  │ Engine   │  │ Handler  │  │ Manager  │  │ Manager  │   │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                   Platform Layer                             │
│              SDL2, OpenGL, Operating System                  │
└─────────────────────────────────────────────────────────────┘
```

### Layer Responsibilities

#### Application Layer
- Main game loop execution
- Frame timing and delta time calculation
- System initialization and shutdown
- State management (menu, playing, paused)

#### Gameplay Layer
- Match logic and rules enforcement
- AI decision making
- Player state management
- Score tracking

#### Rendering Layer
- 3D graphics rendering
- Camera management
- Shader compilation and management
- UI rendering

#### Audio Layer
- Sound effect playback
- Music management
- Volume control

#### Core Systems Layer
- Physics simulation
- Input processing
- Resource loading and caching
- Configuration management

#### Platform Layer
- Window management (SDL2)
- OpenGL context
- File I/O
- Operating system integration

---

## Core Systems

### Application System

**File**: `src/core/Application.cpp`

The Application class is the entry point and orchestrates all subsystems.

```cpp
class Application {
    // Initialization
    bool Initialize();
    
    // Main loop
    void Run();
    
    // Cleanup
    void Shutdown();
    
private:
    // Loop components
    void ProcessInput();
    void Update(float deltaTime);
    void FixedUpdate(float fixedDeltaTime);
    void Render();
};
```

**Responsibilities**:
- SDL2 and OpenGL initialization
- Subsystem creation and management
- Main game loop with fixed timestep
- Frame rate management
- Graceful error handling

**Game Loop Structure**:
```
┌─────────────────────────────────────┐
│ While Running:                      │
│                                     │
│  1. Calculate Delta Time            │
│  2. Process Input Events            │
│  3. Fixed Physics Updates (120Hz)   │
│  4. Variable Game Logic Update      │
│  5. Render Frame                    │
│  6. Swap Buffers                    │
│                                     │
└─────────────────────────────────────┘
```

### Logger System

**File**: `src/core/Logger.cpp`

Thread-safe logging system with multiple output targets.

```cpp
enum class LogLevel {
    Debug, Info, Warning, Error, Fatal
};

class Logger {
    static void Log(LogLevel level, const std::string& message);
};
```

**Features**:
- Thread-safe logging with mutex
- File output with timestamps
- Console output with color coding
- Log level filtering
- Automatic log rotation

**Log Format**:
```
[2026-02-11 14:30:25.123] [INFO] Application initialized
[2026-02-11 14:30:25.456] [ERROR] Failed to load texture: ball.png
```

### Configuration System

**File**: `src/core/ConfigManager.cpp`

Manages game and graphics settings with JSON persistence.

```cpp
struct GraphicsConfig {
    int width, height;
    bool fullscreen, vsync;
    std::string quality;
    int fov;
};

struct GameConfig {
    std::string difficulty;
    int oversPerInnings;
    float humidity, windSpeed;
    float pitchHardness, grassCoverage;
};
```

**Configuration Files**:
- `config/graphics.json`: Display and rendering settings
- `config/game.json`: Gameplay parameters
- `config/controls.json`: Key bindings

---

## Rendering Pipeline

### Rendering Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Renderer                              │
│                                                          │
│  BeginFrame() ──► Clear Buffers                         │
│                                                          │
│  RenderScene() ──► ┌──────────────────────────┐        │
│                    │ 1. Geometry Pass         │        │
│                    │    • Field               │        │
│                    │    • Players             │        │
│                    │    • Ball                │        │
│                    │                          │        │
│                    │ 2. Lighting Pass         │        │
│                    │    • Directional Light   │        │
│                    │    • Ambient Light       │        │
│                    │                          │        │
│                    │ 3. UI Pass               │        │
│                    │    • HUD                 │        │
│                    │    • Menus               │        │
│                    └──────────────────────────┘        │
│                                                          │
│  EndFrame() ──► Swap Buffers                            │
└─────────────────────────────────────────────────────────┘
```

### Shader System

**File**: `src/rendering/Shader.cpp`

Manages GLSL shader compilation and uniform setting.

```cpp
class Shader {
    bool CompileFromSource(const std::string& vertexSrc, 
                          const std::string& fragmentSrc);
    void Use();
    void SetMat4(const std::string& name, const glm::mat4& value);
    void SetVec3(const std::string& name, const glm::vec3& value);
};
```

**Shader Pipeline**:
1. Load vertex and fragment shader source
2. Compile shaders individually
3. Link into shader program
4. Validate and check for errors
5. Cache uniform locations

**Standard Shaders**:
- `player.shader`: Character rendering with Phong lighting
- `field.shader`: Terrain with texture blending
- `ball.shader`: Sphere with specular highlights
- `ui.shader`: 2D orthographic rendering

### Camera System

**File**: `src/rendering/Camera.cpp`

Manages view and projection matrices with multiple camera modes.

```cpp
enum class CameraMode {
    BowlerView,    // Behind bowler
    BatsmanView,   // Behind batsman
    FollowBall,    // Dynamic tracking
    Cinematic      // Replay camera
};

class Camera {
    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix(float aspectRatio);
    void Update(float deltaTime);
};
```

**Camera Transitions**:
- Smooth interpolation between positions
- Look-at target tracking
- FOV adjustments for dramatic effect

### Mesh System

**File**: `src/rendering/Mesh.cpp`

Manages vertex data and OpenGL buffers.

```cpp
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Mesh {
    void SetupMesh(const std::vector<Vertex>& vertices,
                   const std::vector<unsigned int>& indices);
    void Draw();
};
```

**Buffer Layout**:
```
VAO (Vertex Array Object)
├── VBO (Vertex Buffer Object)
│   ├── Position (3 floats)
│   ├── Normal (3 floats)
│   └── TexCoords (2 floats)
└── EBO (Element Buffer Object)
    └── Indices (unsigned ints)
```

---

## Physics System

### Physics Architecture

```
┌─────────────────────────────────────────────────────────┐
│                  Physics Engine                          │
│                                                          │
│  Update(dt) ──► ┌──────────────────────────┐           │
│                 │ 1. Integrate Forces      │           │
│                 │    • Gravity             │           │
│                 │    • Drag                │           │
│                 │    • Magnus Effect       │           │
│                 │                          │           │
│                 │ 2. Environmental Effects │           │
│                 │    • Wind                │           │
│                 │    • Humidity (swing)    │           │
│                 │    • Pitch conditions    │           │
│                 │                          │           │
│                 │ 3. Collision Detection   │           │
│                 │    • Ball-Bat            │           │
│                 │    • Ball-Ground         │           │
│                 │    • Ball-Stumps         │           │
│                 │                          │           │
│                 │ 4. Collision Response    │           │
│                 │    • Impulse calculation │           │
│                 │    • Friction            │           │
│                 └──────────────────────────┘           │
└─────────────────────────────────────────────────────────┘
```

### Ball Physics

**File**: `src/physics/Ball.cpp`

Realistic cricket ball simulation with environmental effects.

```cpp
class Ball {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 spin;
    
    void Update(float dt);
    void ApplyForce(const glm::vec3& force);
};
```

**Forces Applied**:
1. **Gravity**: Constant downward acceleration (9.8 m/s²)
2. **Drag**: Air resistance proportional to velocity²
3. **Magnus Effect**: Spin-induced curve
4. **Wind**: Environmental lateral force
5. **Swing**: Humidity-based deviation

**Physics Integration**:
```cpp
// Euler integration
velocity += acceleration * dt;
position += velocity * dt;

// Apply drag
float dragMagnitude = 0.5f * airDensity * dragCoeff * 
                      area * velocitySquared;
velocity -= normalize(velocity) * dragMagnitude * dt;
```

### Environmental Effects

**File**: `src/physics/PhysicsEngine.cpp`

```cpp
struct EnvironmentParams {
    // Weather
    float humidity;           // 0.0 to 1.0
    float windSpeed;          // km/h
    glm::vec3 windDirection;
    
    // Pitch
    float pitchHardness;      // 0.0 (soft) to 1.0 (hard)
    float grassCoverage;      // 0.0 to 1.0
    float moisture;           // 0.0 (dry) to 1.0 (wet)
    
    float GetSwingFactor();
    float GetBounceFactor();
};
```

**Effect Calculations**:
- **Swing**: `swingMagnitude *= (1.0 + 0.3 * (humidity - 0.7) / 0.3)` when humidity > 70%
- **Bounce**: `bounceHeight *= (1.0 + 0.2 * pitchHardness)` for hard pitches
- **Seam Movement**: `seamDeviation *= (1.0 + 0.3 * grassCoverage)` for grassy pitches

### Collision Detection

**File**: `src/physics/CollisionDetector.cpp`

```cpp
class CollisionDetector {
    bool CheckSpherePlane(const Ball& ball, const Plane& ground);
    bool CheckSphereBox(const Ball& ball, const Bat& bat);
    bool CheckSphereCylinder(const Ball& ball, const Stumps& stumps);
};
```

**Collision Types**:
1. **Ball-Ground**: Sphere-plane intersection
2. **Ball-Bat**: Sphere-OBB (Oriented Bounding Box)
3. **Ball-Stumps**: Sphere-cylinder intersection

---

## Gameplay Systems

### Match Management

**File**: `src/gameplay/MatchManager.cpp`

Orchestrates match flow and enforces cricket rules.

```cpp
class MatchManager {
    void StartMatch(const MatchConfig& config);
    void OnBallDelivered();
    void OnRunsScored(int runs);
    void OnWicketFallen();
    void OnOverCompleted();
};
```

**Match State Machine**:
```
┌──────────┐
│  Setup   │
└────┬─────┘
     │
     ▼
┌──────────┐     ┌──────────┐
│ Bowling  │────►│ Batting  │
└────┬─────┘     └────┬─────┘
     │                │
     │    ┌──────┐    │
     └───►│ Over │◄───┘
          │ End  │
          └──┬───┘
             │
             ▼
        ┌─────────┐
        │ Innings │
        │   End   │
        └────┬────┘
             │
             ▼
        ┌─────────┐
        │  Match  │
        │   End   │
        └─────────┘
```

### AI Controller

**File**: `src/gameplay/AIController.cpp`

Intelligent opponent behavior with difficulty scaling.

```cpp
class AIController {
    enum class Difficulty {
        Beginner,  // 300ms reaction, 60% accuracy
        Medium,    // 150ms reaction, 75% accuracy
        Hard,      // 75ms reaction, 85% accuracy
        Expert     // 50ms reaction, 95% accuracy
    };
    
    ShotType DecideBattingShot(const BallTrajectory& ball);
    DeliveryType DecideBowlingDelivery(const BatsmanStats& stats);
};
```

**AI Decision Making**:
1. **Analyze ball trajectory**: Line, length, speed
2. **Evaluate field placement**: Gaps and fielder positions
3. **Consider match situation**: Required run rate, wickets
4. **Apply difficulty modifiers**: Reaction time, accuracy
5. **Add randomness**: Prevent predictability

### Player System

**File**: `src/gameplay/Player.cpp`

Represents individual players with stats and animations.

```cpp
struct PlayerStats {
    std::string name;
    int battingSkill;    // 0-100
    int bowlingSkill;    // 0-100
    int fieldingSkill;   // 0-100
    int stamina;         // 0-100
};

class Player {
    PlayerStats stats;
    Transform transform;
    AnimationController animator;
};
```

---

## Data Flow

### Input to Rendering Flow

```
User Input
    │
    ▼
┌─────────────┐
│ Input       │
│ Handler     │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ Game Logic  │
│ (Match Mgr) │
└──────┬──────┘
       │
       ├──────────────┐
       │              │
       ▼              ▼
┌─────────────┐  ┌─────────────┐
│ Physics     │  │ AI          │
│ Engine      │  │ Controller  │
└──────┬──────┘  └──────┬──────┘
       │              │
       └──────┬───────┘
              │
              ▼
       ┌─────────────┐
       │ Game State  │
       │ Update      │
       └──────┬──────┘
              │
              ▼
       ┌─────────────┐
       │ Renderer    │
       └──────┬──────┘
              │
              ▼
       Display Output
```

### Event Flow Example: Batting Shot

```
1. Player presses shot button
   └─► InputHandler detects key press
   
2. InputHandler notifies MatchManager
   └─► MatchManager validates shot timing
   
3. MatchManager calculates shot parameters
   └─► Shot type, timing quality, power
   
4. PhysicsEngine applies forces to ball
   └─► Ball trajectory calculation
   
5. CollisionDetector checks bat-ball contact
   └─► Collision response applied
   
6. MatchManager updates score
   └─► Runs calculated based on ball position
   
7. Renderer updates visuals
   └─► Ball animation, player animation, HUD update
   
8. AudioManager plays sound effects
   └─► Bat hit sound, crowd reaction
```

---

## Design Patterns

### Singleton Pattern

Used for global managers that should have only one instance.

```cpp
class ResourceManager {
public:
    static ResourceManager& Instance() {
        static ResourceManager instance;
        return instance;
    }
private:
    ResourceManager() = default;
};
```

**Used in**:
- ResourceManager
- Logger (static methods)

### Component Pattern

Game objects composed of reusable components.

```cpp
class Player {
    Transform transform;
    AnimationController animator;
    PhysicsBody physics;
    PlayerStats stats;
};
```

### State Pattern

Game states managed through state machine.

```cpp
class IGameState {
    virtual void Enter() = 0;
    virtual void Update(float dt) = 0;
    virtual void Exit() = 0;
};

class PlayingState : public IGameState { };
class MenuState : public IGameState { };
```

### Object Pool Pattern

Reuse frequently created objects for performance.

```cpp
class ParticlePool {
    std::vector<Particle> particles;
    std::queue<Particle*> available;
    
    Particle* Acquire();
    void Release(Particle* particle);
};
```

**Used for**:
- Particle effects
- Ball trails
- UI elements

### Observer Pattern

Event system for loose coupling.

```cpp
class IEventListener {
    virtual void OnEvent(const Event& event) = 0;
};

class EventManager {
    void Subscribe(EventType type, IEventListener* listener);
    void Publish(const Event& event);
};
```

---

## Performance Considerations

### Fixed Timestep Physics

Physics runs at constant 120Hz regardless of frame rate:

```cpp
const float FIXED_TIMESTEP = 1.0f / 120.0f;
float accumulator = 0.0f;

while (isRunning) {
    float deltaTime = CalculateDeltaTime();
    accumulator += deltaTime;
    
    while (accumulator >= FIXED_TIMESTEP) {
        FixedUpdate(FIXED_TIMESTEP);
        accumulator -= FIXED_TIMESTEP;
    }
    
    Update(deltaTime);
    Render();
}
```

### Memory Management

- **Smart Pointers**: Use `std::unique_ptr` and `std::shared_ptr`
- **RAII**: Resource acquisition is initialization
- **Object Pooling**: Reuse objects instead of allocation
- **Cache Locality**: Structure data for cache efficiency

### Rendering Optimizations

- **Frustum Culling**: Don't render off-screen objects
- **Instanced Rendering**: Batch similar objects
- **Texture Atlasing**: Reduce texture binds
- **LOD System**: Lower detail for distant objects

---

## Thread Safety

### Thread-Safe Components

- **Logger**: Mutex-protected logging
- **Resource Loading**: Background thread loading (future)

### Single-Threaded Components

Most systems run on main thread for simplicity:
- Rendering (OpenGL context requirement)
- Physics
- Game logic

---

## Extension Points

### Adding New Game Modes

1. Create new `IGameState` implementation
2. Register with `GameStateManager`
3. Implement state-specific logic

### Adding New Physics Effects

1. Extend `EnvironmentParams`
2. Implement effect calculation in `PhysicsEngine`
3. Update ball physics integration

### Adding New AI Behaviors

1. Extend `AIController` with new decision methods
2. Add behavior parameters to configuration
3. Integrate with match manager

---

## Conclusion

This architecture provides a solid foundation for a production-ready cricket game with:
- Clear separation of concerns
- Extensible design
- Performance optimization
- Maintainable codebase

For implementation details, see the source code and inline documentation.
