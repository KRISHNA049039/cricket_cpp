# Cricket Game - Core Concepts and Systems

## Table of Contents
1. [Game Concepts](#game-concepts)
2. [Physics Concepts](#physics-concepts)
3. [Rendering Concepts](#rendering-concepts)
4. [AI Concepts](#ai-concepts)
5. [Audio Concepts](#audio-concepts)
6. [Performance Concepts](#performance-concepts)

---

## Game Concepts

### Cricket Rules Implementation

#### Match Structure

A cricket match consists of:
- **Innings**: Each team bats once (or twice in test matches)
- **Overs**: Set of 6 consecutive deliveries
- **Balls**: Individual deliveries from bowler to batsman

```
Match
├── Innings 1 (Team A batting)
│   ├── Over 1 (6 balls)
│   ├── Over 2 (6 balls)
│   └── ...
└── Innings 2 (Team B batting)
    ├── Over 1 (6 balls)
    └── ...
```

#### Scoring System

**Runs**:
- **Singles/Twos/Threes**: Batsmen run between wickets
- **Boundary (4 runs)**: Ball reaches boundary after bouncing
- **Six (6 runs)**: Ball clears boundary without bouncing

**Extras**:
- **Wide**: Ball too far from batsman (1 run + re-bowl)
- **No-ball**: Illegal delivery (1 run + re-bowl + free hit)
- **Bye**: Runs scored without bat contact
- **Leg-bye**: Runs off batsman's body

**Dismissals (Wickets)**:
- **Bowled**: Ball hits stumps
- **Caught**: Fielder catches ball before it bounces
- **LBW (Leg Before Wicket)**: Ball would have hit stumps but hits leg
- **Run Out**: Batsman out of crease when stumps broken
- **Stumped**: Wicket-keeper breaks stumps while batsman out of crease

#### Match Progression

```cpp
struct MatchState {
    int runs;              // Total runs scored
    int wickets;           // Wickets fallen (max 10)
    int overs;             // Complete overs bowled
    int balls;             // Balls in current over (0-5)
    
    float runRate;         // Runs per over
    float requiredRunRate; // Target run rate (chase scenarios)
    
    Player* striker;       // Batsman facing delivery
    Player* nonStriker;    // Batsman at other end
    Player* bowler;        // Current bowler
};
```

### Game Modes

#### Quick Match
- Single innings per team
- Configurable overs (5, 10, 20, 50)
- Simplified rules

#### Tournament Mode
- Multiple matches
- Points table
- Knockout stages

#### Practice Mode
- Batting practice
- Bowling practice
- No match pressure

---

## Physics Concepts

### Ball Dynamics

#### Forces Acting on Cricket Ball

1. **Gravity (Fg)**
```
Fg = m × g
where:
  m = ball mass (0.156 kg)
  g = gravitational acceleration (9.8 m/s²)
```

2. **Drag Force (Fd)**
```
Fd = 0.5 × ρ × Cd × A × v²
where:
  ρ = air density (1.225 kg/m³)
  Cd = drag coefficient (0.47 for sphere)
  A = cross-sectional area (π × r²)
  v = velocity magnitude
```

3. **Magnus Force (Fm)** - Spin-induced curve
```
Fm = S × (ω × v)
where:
  S = spin parameter
  ω = angular velocity (spin vector)
  v = linear velocity
```

#### Ball Trajectory Calculation

```cpp
void Ball::Update(float dt) {
    // 1. Calculate forces
    glm::vec3 gravity(0.0f, -9.8f * mass, 0.0f);
    glm::vec3 drag = -0.5f * airDensity * dragCoeff * 
                     area * length(velocity) * velocity;
    glm::vec3 magnus = CalculateMagnusForce(spin, velocity);
    
    // 2. Apply environmental effects
    glm::vec3 wind = windSpeed * windDirection;
    glm::vec3 swing = CalculateSwing(humidity, velocity);
    
    // 3. Sum forces
    glm::vec3 totalForce = gravity + drag + magnus + wind + swing;
    
    // 4. Integrate (Euler method)
    glm::vec3 acceleration = totalForce / mass;
    velocity += acceleration * dt;
    position += velocity * dt;
    
    // 5. Update spin (decay over time)
    spin *= (1.0f - spinDecay * dt);
}
```

### Environmental Effects

#### Swing Bowling

Swing is the lateral movement of the ball through the air.

**Conventional Swing**:
- Ball moves towards the shiny side
- Enhanced by high humidity (>70%)
- Most effective at medium pace (120-135 km/h)

```cpp
float CalculateSwingMagnitude(float humidity, float speed) {
    float baseSwing = 0.1f;
    
    // Humidity effect
    if (humidity > 0.7f) {
        float humidityFactor = 1.0f + 0.3f * (humidity - 0.7f) / 0.3f;
        baseSwing *= humidityFactor;
    }
    
    // Speed effect (optimal at medium pace)
    float speedKmh = speed * 3.6f;
    float speedFactor = 1.0f - abs(speedKmh - 130.0f) / 50.0f;
    speedFactor = glm::clamp(speedFactor, 0.5f, 1.0f);
    
    return baseSwing * speedFactor;
}
```

#### Seam Movement

Seam movement occurs when ball bounces on the pitch.

**Factors**:
- **Grass Coverage**: More grass = more seam movement
- **Pitch Moisture**: Damp pitch = more grip
- **Ball Condition**: New ball seams more

```cpp
glm::vec3 CalculateSeamMovement(const EnvironmentParams& env) {
    float seamFactor = 0.0f;
    
    // Grass effect
    if (env.grassCoverage > 0.6f) {
        seamFactor += 0.3f * (env.grassCoverage - 0.6f) / 0.4f;
    }
    
    // Moisture effect
    seamFactor += 0.2f * env.moisture;
    
    // Random direction (left or right)
    float direction = (rand() % 2 == 0) ? 1.0f : -1.0f;
    
    return glm::vec3(direction * seamFactor, 0.0f, 0.0f);
}
```

#### Pitch Conditions

**Hard Pitch**:
- Higher bounce
- Faster ball speed after bounce
- Favors fast bowlers and batsmen

**Soft Pitch**:
- Lower bounce
- Slower ball speed
- Favors spin bowlers

**Green Pitch** (grass):
- More seam movement
- Unpredictable bounce
- Favors seam bowlers

```cpp
float CalculateBounceHeight(float pitchHardness, float incomingSpeed) {
    float baseBounce = 1.0f;
    
    // Hardness effect (15-25% increase for hard pitch)
    float hardnessFactor = 1.0f + 0.2f * pitchHardness;
    
    // Speed effect (faster balls bounce higher)
    float speedFactor = 1.0f + 0.1f * (incomingSpeed / 40.0f);
    
    return baseBounce * hardnessFactor * speedFactor;
}
```

### Collision Physics

#### Ball-Bat Collision

```cpp
struct CollisionResult {
    bool occurred;
    glm::vec3 contactPoint;
    glm::vec3 contactNormal;
    float penetrationDepth;
};

CollisionResult CheckBatBallCollision(const Ball& ball, const Bat& bat) {
    // 1. Transform ball to bat's local space
    glm::vec3 localBallPos = bat.WorldToLocal(ball.position);
    
    // 2. Check if ball intersects bat's bounding box
    if (!bat.boundingBox.Contains(localBallPos, ball.radius)) {
        return {false};
    }
    
    // 3. Find closest point on bat to ball center
    glm::vec3 closestPoint = bat.GetClosestPoint(localBallPos);
    
    // 4. Calculate collision response
    glm::vec3 normal = normalize(localBallPos - closestPoint);
    float penetration = ball.radius - length(localBallPos - closestPoint);
    
    return {true, closestPoint, normal, penetration};
}
```

**Collision Response**:
```cpp
void ResolveBatBallCollision(Ball& ball, const Bat& bat, 
                             const CollisionResult& collision) {
    // 1. Calculate relative velocity
    glm::vec3 relativeVelocity = ball.velocity - bat.velocity;
    
    // 2. Calculate impulse
    float restitution = 0.8f; // Coefficient of restitution
    float impulse = -(1.0f + restitution) * 
                    dot(relativeVelocity, collision.contactNormal);
    impulse /= (1.0f / ball.mass + 1.0f / bat.mass);
    
    // 3. Apply impulse
    ball.velocity += (impulse / ball.mass) * collision.contactNormal;
    
    // 4. Add spin from bat angle
    glm::vec3 spinTransfer = cross(bat.angularVelocity, 
                                   collision.contactNormal);
    ball.spin += spinTransfer * 0.5f;
    
    // 5. Separate objects
    ball.position += collision.contactNormal * collision.penetrationDepth;
}
```

---

## Rendering Concepts

### 3D Graphics Pipeline

```
Vertex Data
    │
    ▼
┌─────────────────┐
│ Vertex Shader   │  ← Transform vertices to clip space
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Rasterization   │  ← Convert triangles to fragments
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Fragment Shader │  ← Calculate pixel colors
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Framebuffer     │  ← Final image
└─────────────────┘
```

### Coordinate Systems

#### World Space
Global 3D coordinate system where all objects exist.

```
Y (Up)
│
│     Z (Forward)
│    ╱
│   ╱
│  ╱
│ ╱
└──────────── X (Right)
```

#### View Space (Camera Space)
Coordinates relative to camera position and orientation.

```cpp
glm::mat4 viewMatrix = glm::lookAt(
    cameraPosition,  // Camera position
    targetPosition,  // Look-at target
    upVector        // Up direction
);
```

#### Clip Space
Normalized device coordinates after projection.

```cpp
glm::mat4 projectionMatrix = glm::perspective(
    glm::radians(fov),  // Field of view
    aspectRatio,        // Width / Height
    nearPlane,          // Near clipping plane (0.1)
    farPlane           // Far clipping plane (1000.0)
);
```

#### MVP Transformation

```cpp
// Model-View-Projection matrix
glm::mat4 model = glm::translate(glm::mat4(1.0f), objectPosition);
glm::mat4 view = camera.GetViewMatrix();
glm::mat4 projection = camera.GetProjectionMatrix(aspectRatio);

glm::mat4 MVP = projection * view * model;

// In vertex shader:
gl_Position = MVP * vec4(vertexPosition, 1.0);
```

### Lighting Models

#### Phong Lighting

Combines three components:

1. **Ambient**: Base lighting (no direction)
```glsl
vec3 ambient = ambientStrength * lightColor;
```

2. **Diffuse**: Directional lighting (Lambert's cosine law)
```glsl
float diff = max(dot(normal, lightDir), 0.0);
vec3 diffuse = diff * lightColor;
```

3. **Specular**: Shiny highlights
```glsl
vec3 reflectDir = reflect(-lightDir, normal);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
vec3 specular = specularStrength * spec * lightColor;
```

**Final Color**:
```glsl
vec3 result = (ambient + diffuse + specular) * objectColor;
```

### Camera Modes

#### Batsman View
```cpp
void UpdateBatsmanCamera(Camera& camera, const Player& batsman) {
    glm::vec3 offset(0.0f, 2.0f, 5.0f); // Behind and above
    camera.position = batsman.position + offset;
    camera.target = batsman.position + glm::vec3(0.0f, 1.0f, -10.0f);
}
```

#### Follow Ball
```cpp
void UpdateFollowBallCamera(Camera& camera, const Ball& ball) {
    // Smooth interpolation
    glm::vec3 desiredPos = ball.position + glm::vec3(0.0f, 5.0f, 10.0f);
    camera.position = glm::mix(camera.position, desiredPos, 0.1f);
    camera.target = ball.position;
}
```

---

## AI Concepts

### Decision Making

#### Batting AI

**Shot Selection Algorithm**:
```cpp
ShotType DecideBattingShot(const BallTrajectory& ball, 
                          const FieldPlacement& field) {
    // 1. Analyze ball parameters
    float ballLine = ball.GetLineFromStumps();    // -1.0 to 1.0
    float ballLength = ball.GetLength();          // 0.0 to 1.0
    float ballSpeed = ball.GetSpeed();
    
    // 2. Determine shot category
    if (ballLength < 0.3f) {
        // Short ball - pull or cut
        return (ballLine < 0.0f) ? ShotType::Pull : ShotType::Cut;
    }
    else if (ballLength > 0.7f) {
        // Full ball - drive or defensive
        if (ballSpeed > 130.0f) {
            return ShotType::Defensive;
        }
        return ShotType::Drive;
    }
    else {
        // Good length - defensive or sweep
        return ShotType::Defensive;
    }
}
```

**Timing Calculation**:
```cpp
float CalculateShotTiming(const Ball& ball, Difficulty difficulty) {
    // 1. Calculate ideal timing
    float timeToReach = CalculateTimeToReachBatsman(ball);
    float idealTiming = timeToReach - 0.05f; // 50ms before contact
    
    // 2. Apply difficulty-based error
    float reactionTime = GetReactionTime(difficulty);
    float accuracy = GetAccuracy(difficulty);
    
    // 3. Add random variation
    float randomError = RandomGaussian(0.0f, 1.0f - accuracy);
    
    // 4. Calculate actual timing
    float actualTiming = idealTiming + reactionTime + randomError;
    
    return actualTiming;
}
```

#### Bowling AI

**Delivery Selection**:
```cpp
DeliveryType DecideBowlingDelivery(const BatsmanStats& batsman,
                                   int ballsInOver) {
    // 1. Analyze batsman weaknesses
    float shortBallWeakness = batsman.GetWeakness(ShotType::Pull);
    float fullBallWeakness = batsman.GetWeakness(ShotType::Drive);
    
    // 2. Ensure variation (30% minimum)
    if (ballsInOver > 0 && SameDeliveryCount() > 2) {
        return GetVariationDelivery();
    }
    
    // 3. Exploit weaknesses
    if (shortBallWeakness > 0.7f) {
        return DeliveryType::Bouncer;
    }
    if (fullBallWeakness > 0.7f) {
        return DeliveryType::Yorker;
    }
    
    // 4. Default to stock delivery
    return DeliveryType::Fast;
}
```

### Difficulty Scaling

```cpp
struct DifficultyParams {
    float reactionTime;  // Time delay before AI reacts
    float accuracy;      // Shot/delivery accuracy (0-1)
    float aggression;    // Risk-taking tendency (0-1)
};

DifficultyParams GetDifficultyParams(Difficulty level) {
    switch (level) {
        case Difficulty::Beginner:
            return {0.300f, 0.60f, 0.3f};
        case Difficulty::Medium:
            return {0.150f, 0.75f, 0.5f};
        case Difficulty::Hard:
            return {0.075f, 0.85f, 0.7f};
        case Difficulty::Expert:
            return {0.050f, 0.95f, 0.8f};
    }
}
```

---

## Audio Concepts

### Sound Design

#### Spatial Audio

```cpp
void PlaySpatialSound(const std::string& soundName, 
                     const glm::vec3& position,
                     const glm::vec3& listenerPos) {
    // 1. Calculate distance
    float distance = glm::length(position - listenerPos);
    
    // 2. Apply distance attenuation
    float maxDistance = 100.0f;
    float attenuation = 1.0f - glm::clamp(distance / maxDistance, 0.0f, 1.0f);
    
    // 3. Calculate stereo panning
    glm::vec3 toSound = normalize(position - listenerPos);
    float pan = dot(toSound, glm::vec3(1.0f, 0.0f, 0.0f)); // -1 (left) to 1 (right)
    
    // 4. Play sound with calculated parameters
    PlaySound(soundName, attenuation, pan);
}
```

#### Dynamic Music

```cpp
void UpdateMusic(const MatchState& state) {
    // Adjust music intensity based on match situation
    float intensity = CalculateMatchIntensity(state);
    
    if (intensity > 0.8f) {
        // Tense situation - play dramatic music
        TransitionToMusic("dramatic.ogg", 2.0f);
    }
    else if (intensity < 0.3f) {
        // Calm situation - play ambient music
        TransitionToMusic("ambient.ogg", 3.0f);
    }
}

float CalculateMatchIntensity(const MatchState& state) {
    float wicketsIntensity = state.wickets / 10.0f;
    float runRateIntensity = abs(state.runRate - state.requiredRunRate) / 10.0f;
    float oversIntensity = state.overs / (float)state.totalOvers;
    
    return (wicketsIntensity + runRateIntensity + oversIntensity) / 3.0f;
}
```

---

## Performance Concepts

### Frame Rate Management

#### Fixed Timestep

Ensures consistent physics simulation regardless of frame rate.

```cpp
const float FIXED_TIMESTEP = 1.0f / 120.0f;  // 120 Hz physics
float accumulator = 0.0f;

void GameLoop() {
    float deltaTime = CalculateDeltaTime();
    accumulator += deltaTime;
    
    // Run physics at fixed rate
    while (accumulator >= FIXED_TIMESTEP) {
        PhysicsUpdate(FIXED_TIMESTEP);
        accumulator -= FIXED_TIMESTEP;
    }
    
    // Render at variable rate
    Render();
}
```

**Benefits**:
- Deterministic physics
- Network synchronization friendly
- Prevents physics instability

### Memory Management

#### Object Pooling

```cpp
template<typename T>
class ObjectPool {
public:
    T* Acquire() {
        if (available.empty()) {
            return new T();
        }
        T* obj = available.back();
        available.pop_back();
        return obj;
    }
    
    void Release(T* obj) {
        obj->Reset();
        available.push_back(obj);
    }
    
private:
    std::vector<T*> available;
};

// Usage
ObjectPool<Particle> particlePool;
Particle* p = particlePool.Acquire();
// ... use particle ...
particlePool.Release(p);
```

#### Cache-Friendly Data

```cpp
// Bad: Array of structures (AoS)
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
};
std::vector<Particle> particles;

// Good: Structure of arrays (SoA)
struct ParticleSystem {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> velocities;
    std::vector<float> lifetimes;
};
```

### Rendering Optimization

#### Frustum Culling

```cpp
bool IsInFrustum(const BoundingBox& box, const Frustum& frustum) {
    // Check if bounding box intersects view frustum
    for (const Plane& plane : frustum.planes) {
        if (box.IsOutside(plane)) {
            return false;
        }
    }
    return true;
}

void RenderScene() {
    Frustum frustum = camera.GetFrustum();
    
    for (const GameObject& obj : objects) {
        if (IsInFrustum(obj.boundingBox, frustum)) {
            RenderObject(obj);
        }
    }
}
```

#### Level of Detail (LOD)

```cpp
int GetLODLevel(float distanceToCamera) {
    if (distanceToCamera < 20.0f) return 0;  // High detail
    if (distanceToCamera < 50.0f) return 1;  // Medium detail
    if (distanceToCamera < 100.0f) return 2; // Low detail
    return 3;  // Very low detail
}

void RenderPlayer(const Player& player, const Camera& camera) {
    float distance = glm::length(player.position - camera.position);
    int lodLevel = GetLODLevel(distance);
    
    const Mesh& mesh = player.GetMeshForLOD(lodLevel);
    RenderMesh(mesh);
}
```

---

## Conclusion

These concepts form the foundation of the cricket game's systems. Understanding them is crucial for:
- Implementing new features
- Debugging issues
- Optimizing performance
- Extending functionality

For practical implementation, refer to:
- [ARCHITECTURE.md](ARCHITECTURE.md) - System design
- [SETUP_GUIDE.md](SETUP_GUIDE.md) - Building and running
- Source code - Actual implementations
