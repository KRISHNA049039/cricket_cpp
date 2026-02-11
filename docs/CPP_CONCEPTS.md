# C++ Concepts and Best Practices

## Table of Contents
1. [C++17 Features Used](#c17-features-used)
2. [Memory Management](#memory-management)
3. [Object-Oriented Programming](#object-oriented-programming)
4. [Templates and Generic Programming](#templates-and-generic-programming)
5. [STL Containers and Algorithms](#stl-containers-and-algorithms)
6. [Modern C++ Idioms](#modern-c-idioms)
7. [Error Handling](#error-handling)
8. [Concurrency and Threading](#concurrency-and-threading)
9. [Performance Optimization](#performance-optimization)
10. [Best Practices](#best-practices)

---

## C++17 Features Used

### Structured Bindings

Decompose objects into individual variables.

```cpp
// Example: Returning multiple values
std::pair<bool, std::string> LoadConfig() {
    return {true, "config.json"};
}

// Usage with structured bindings
auto [success, filename] = LoadConfig();
if (success) {
    std::cout << "Loaded: " << filename << std::endl;
}
```

### std::optional

Represent optional values without using pointers.

```cpp
#include <optional>

std::optional<Player*> FindPlayerByName(const std::string& name) {
    for (auto& player : players) {
        if (player.name == name) {
            return &player;
        }
    }
    return std::nullopt;  // No player found
}

// Usage
if (auto player = FindPlayerByName("Sachin")) {
    std::cout << "Found: " << player.value()->name << std::endl;
}
```

### std::variant

Type-safe union for holding one of several types.

```cpp
#include <variant>

using GameEvent = std::variant<BallDelivered, RunsScored, WicketFallen>;

void HandleEvent(const GameEvent& event) {
    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, BallDelivered>) {
            // Handle ball delivered
        } else if constexpr (std::is_same_v<T, RunsScored>) {
            // Handle runs scored
        }
    }, event);
}
```

### std::filesystem

Modern file system operations.

```cpp
#include <filesystem>
namespace fs = std::filesystem;

void CreateLogDirectory() {
    fs::path logPath = "logs";
    if (!fs::exists(logPath)) {
        fs::create_directories(logPath);
    }
}

void ListAssets() {
    for (const auto& entry : fs::directory_iterator("assets")) {
        std::cout << entry.path() << std::endl;
    }
}
```

### if/switch with Initializers

```cpp
// if with initializer
if (auto result = LoadTexture("ball.png"); result.success) {
    UseTexture(result.texture);
}

// switch with initializer
switch (auto state = GetGameState(); state) {
    case GameState::Playing:
        UpdateGame();
        break;
    case GameState::Paused:
        ShowPauseMenu();
        break;
}
```

---

## Memory Management

### Smart Pointers

#### std::unique_ptr

Exclusive ownership of dynamically allocated objects.

```cpp
#include <memory>

class Renderer {
private:
    std::unique_ptr<Shader> shader;  // Renderer owns the shader
    
public:
    Renderer() {
        shader = std::make_unique<Shader>();
    }
    
    // Automatically deleted when Renderer is destroyed
    // No manual delete needed
};

// Creating unique_ptr
auto renderer = std::make_unique<Renderer>();

// Transferring ownership
std::unique_ptr<Shader> shader1 = std::make_unique<Shader>();
std::unique_ptr<Shader> shader2 = std::move(shader1);  // shader1 is now nullptr
```

**When to use**:
- Single owner of resource
- Clear ownership semantics
- Automatic cleanup
- Zero overhead compared to raw pointers

#### std::shared_ptr

Shared ownership with reference counting.

```cpp
class Texture {
public:
    Texture(const std::string& path) { /* Load texture */ }
    ~Texture() { /* Cleanup */ }
};

// Multiple owners
std::shared_ptr<Texture> texture1 = std::make_shared<Texture>("ball.png");
std::shared_ptr<Texture> texture2 = texture1;  // Reference count = 2

// Check reference count
std::cout << "Ref count: " << texture1.use_count() << std::endl;

// Texture deleted when last shared_ptr goes out of scope
```

**When to use**:
- Multiple owners needed
- Shared resources (textures, meshes)
- Circular references (use std::weak_ptr to break cycles)

#### std::weak_ptr

Non-owning reference to shared_ptr.

```cpp
class Player {
    std::shared_ptr<Team> team;  // Player owns team reference
};

class Team {
    std::vector<std::weak_ptr<Player>> players;  // Team doesn't own players
};

// Usage
auto player = std::make_shared<Player>();
team->AddPlayer(player);  // Stores weak_ptr

// Check if still valid
if (auto p = weakPlayer.lock()) {
    // Use p (shared_ptr)
}
```

### RAII (Resource Acquisition Is Initialization)

Resources are acquired in constructor and released in destructor.

```cpp
class FileHandle {
private:
    FILE* file;
    
public:
    FileHandle(const char* filename) {
        file = fopen(filename, "r");
        if (!file) {
            throw std::runtime_error("Failed to open file");
        }
    }
    
    ~FileHandle() {
        if (file) {
            fclose(file);  // Automatic cleanup
        }
    }
    
    // Delete copy operations
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    
    // Allow move operations
    FileHandle(FileHandle&& other) noexcept : file(other.file) {
        other.file = nullptr;
    }
};

// Usage - no manual cleanup needed
void ProcessFile() {
    FileHandle handle("data.txt");
    // Use file
    // Automatically closed when handle goes out of scope
}
```

### Move Semantics

Efficiently transfer resources instead of copying.

```cpp
class Mesh {
private:
    unsigned int VAO, VBO, EBO;
    std::vector<Vertex> vertices;
    
public:
    // Move constructor
    Mesh(Mesh&& other) noexcept 
        : VAO(other.VAO), VBO(other.VBO), EBO(other.EBO),
          vertices(std::move(other.vertices)) {
        // Invalidate source
        other.VAO = 0;
        other.VBO = 0;
        other.EBO = 0;
    }
    
    // Move assignment
    Mesh& operator=(Mesh&& other) noexcept {
        if (this != &other) {
            // Clean up existing resources
            Cleanup();
            
            // Transfer ownership
            VAO = other.VAO;
            VBO = other.VBO;
            EBO = other.EBO;
            vertices = std::move(other.vertices);
            
            // Invalidate source
            other.VAO = 0;
            other.VBO = 0;
            other.EBO = 0;
        }
        return *this;
    }
};

// Usage
Mesh CreateMesh() {
    Mesh mesh;
    // Setup mesh
    return mesh;  // Move, not copy
}

Mesh mesh = CreateMesh();  // Efficient move
```

---

## Object-Oriented Programming

### Classes and Encapsulation

```cpp
class Ball {
private:
    // Private data members
    glm::vec3 position;
    glm::vec3 velocity;
    float mass;
    
public:
    // Constructor
    Ball(const glm::vec3& pos, float m) 
        : position(pos), mass(m), velocity(0.0f) {}
    
    // Getters (const methods don't modify object)
    glm::vec3 GetPosition() const { return position; }
    float GetMass() const { return mass; }
    
    // Setters
    void SetVelocity(const glm::vec3& vel) { velocity = vel; }
    
    // Public interface
    void Update(float deltaTime);
    void ApplyForce(const glm::vec3& force);
};
```

### Inheritance

```cpp
// Base class
class GameObject {
protected:
    glm::vec3 position;
    glm::vec3 rotation;
    bool active;
    
public:
    virtual ~GameObject() = default;  // Virtual destructor
    
    virtual void Update(float deltaTime) = 0;  // Pure virtual
    virtual void Render() const = 0;
    
    glm::vec3 GetPosition() const { return position; }
};

// Derived class
class Player : public GameObject {
private:
    PlayerStats stats;
    AnimationController animator;
    
public:
    void Update(float deltaTime) override {
        animator.Update(deltaTime);
        // Player-specific update logic
    }
    
    void Render() const override {
        // Player-specific rendering
    }
};
```

### Polymorphism

```cpp
// Interface (abstract base class)
class IGameState {
public:
    virtual ~IGameState() = default;
    
    virtual void Enter() = 0;
    virtual void Exit() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
};

// Concrete implementations
class MenuState : public IGameState {
public:
    void Enter() override { /* Setup menu */ }
    void Exit() override { /* Cleanup menu */ }
    void Update(float deltaTime) override { /* Update menu */ }
    void Render() override { /* Render menu */ }
};

class PlayingState : public IGameState {
public:
    void Enter() override { /* Start match */ }
    void Exit() override { /* Pause match */ }
    void Update(float deltaTime) override { /* Update game */ }
    void Render() override { /* Render game */ }
};

// Usage with polymorphism
std::unique_ptr<IGameState> currentState;
currentState = std::make_unique<MenuState>();
currentState->Update(deltaTime);  // Calls MenuState::Update

currentState = std::make_unique<PlayingState>();
currentState->Update(deltaTime);  // Calls PlayingState::Update
```

### Composition over Inheritance

Prefer composition for flexibility.

```cpp
// Components
class Transform {
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

class PhysicsBody {
public:
    glm::vec3 velocity;
    float mass;
    void ApplyForce(const glm::vec3& force);
};

class Renderer {
public:
    Mesh* mesh;
    Material* material;
    void Draw();
};

// Entity composed of components
class Entity {
private:
    Transform transform;
    PhysicsBody physics;
    Renderer renderer;
    
public:
    void Update(float deltaTime) {
        physics.Update(deltaTime);
        transform.position += physics.velocity * deltaTime;
    }
    
    void Render() {
        renderer.Draw();
    }
};
```

---

## Templates and Generic Programming

### Function Templates

```cpp
// Generic function
template<typename T>
T Max(T a, T b) {
    return (a > b) ? a : b;
}

// Usage
int maxInt = Max(5, 10);
float maxFloat = Max(3.14f, 2.71f);
```

### Class Templates

```cpp
template<typename T>
class ObjectPool {
private:
    std::vector<T*> available;
    std::vector<std::unique_ptr<T>> owned;
    
public:
    T* Acquire() {
        if (available.empty()) {
            owned.push_back(std::make_unique<T>());
            return owned.back().get();
        }
        T* obj = available.back();
        available.pop_back();
        return obj;
    }
    
    void Release(T* obj) {
        obj->Reset();
        available.push_back(obj);
    }
};

// Usage
ObjectPool<Particle> particlePool;
Particle* p = particlePool.Acquire();
particlePool.Release(p);
```

### Template Specialization

```cpp
// Generic template
template<typename T>
class Serializer {
public:
    static void Serialize(const T& obj, std::ostream& out) {
        out.write(reinterpret_cast<const char*>(&obj), sizeof(T));
    }
};

// Specialization for std::string
template<>
class Serializer<std::string> {
public:
    static void Serialize(const std::string& str, std::ostream& out) {
        size_t length = str.length();
        out.write(reinterpret_cast<const char*>(&length), sizeof(length));
        out.write(str.data(), length);
    }
};
```

### Variadic Templates

```cpp
// Base case
void Log() {
    std::cout << std::endl;
}

// Recursive case
template<typename T, typename... Args>
void Log(T first, Args... args) {
    std::cout << first << " ";
    Log(args...);  // Recursive call
}

// Usage
Log("Score:", 150, "Wickets:", 3);  // Score: 150 Wickets: 3
```

---

## STL Containers and Algorithms

### Containers

#### std::vector

Dynamic array with contiguous memory.

```cpp
#include <vector>

std::vector<Player> players;

// Add elements
players.push_back(Player("Sachin"));
players.emplace_back("Dhoni");  // Construct in-place

// Access
Player& first = players[0];
Player& last = players.back();

// Iterate
for (const auto& player : players) {
    std::cout << player.name << std::endl;
}

// Remove
players.erase(players.begin() + 2);  // Remove 3rd element
players.clear();  // Remove all

// Reserve capacity
players.reserve(100);  // Avoid reallocations
```

**When to use**: Default choice, fast random access, cache-friendly

#### std::unordered_map

Hash table for key-value pairs.

```cpp
#include <unordered_map>

std::unordered_map<std::string, Texture*> textures;

// Insert
textures["ball"] = new Texture("ball.png");
textures.emplace("bat", new Texture("bat.png"));

// Access
if (textures.count("ball") > 0) {
    Texture* tex = textures["ball"];
}

// Safe access with find
auto it = textures.find("ball");
if (it != textures.end()) {
    Texture* tex = it->second;
}

// Iterate
for (const auto& [name, texture] : textures) {
    std::cout << name << std::endl;
}
```

**When to use**: Fast lookup by key, O(1) average case

#### std::array

Fixed-size array with STL interface.

```cpp
#include <array>

std::array<float, 3> position = {0.0f, 1.0f, 2.0f};

// Access
float x = position[0];
float y = position.at(1);  // Bounds checking

// Size known at compile time
constexpr size_t size = position.size();
```

### Algorithms

```cpp
#include <algorithm>

std::vector<int> scores = {45, 78, 23, 90, 12};

// Sort
std::sort(scores.begin(), scores.end());

// Find
auto it = std::find(scores.begin(), scores.end(), 78);
if (it != scores.end()) {
    std::cout << "Found at index: " << std::distance(scores.begin(), it);
}

// Count
int count = std::count_if(scores.begin(), scores.end(), 
    [](int score) { return score > 50; });

// Transform
std::vector<int> doubled(scores.size());
std::transform(scores.begin(), scores.end(), doubled.begin(),
    [](int score) { return score * 2; });

// Remove
scores.erase(std::remove_if(scores.begin(), scores.end(),
    [](int score) { return score < 30; }), scores.end());

// Accumulate
int total = std::accumulate(scores.begin(), scores.end(), 0);

// Min/Max
auto [min, max] = std::minmax_element(scores.begin(), scores.end());
```

---

## Modern C++ Idioms

### Lambda Expressions

```cpp
// Basic lambda
auto add = [](int a, int b) { return a + b; };
int result = add(5, 3);

// Capture by value
int multiplier = 10;
auto multiply = [multiplier](int x) { return x * multiplier; };

// Capture by reference
int counter = 0;
auto increment = [&counter]() { counter++; };

// Capture all by value
auto lambda1 = [=]() { /* Use all local variables by value */ };

// Capture all by reference
auto lambda2 = [&]() { /* Use all local variables by reference */ };

// Mutable lambda
auto mutableLambda = [counter]() mutable { counter++; };

// Generic lambda (C++14)
auto genericAdd = [](auto a, auto b) { return a + b; };
```

### Range-based For Loops

```cpp
std::vector<Player> players;

// Iterate by value (copy)
for (Player player : players) {
    // player is a copy
}

// Iterate by reference
for (Player& player : players) {
    player.Update();  // Modify original
}

// Iterate by const reference (efficient, read-only)
for (const Player& player : players) {
    std::cout << player.name << std::endl;
}

// Auto type deduction
for (const auto& player : players) {
    // Compiler deduces type
}
```

### Auto Type Deduction

```cpp
// Instead of:
std::vector<std::string>::iterator it = names.begin();

// Use auto:
auto it = names.begin();

// Complex types
auto texture = std::make_unique<Texture>("ball.png");
auto [success, message] = LoadConfig();

// Function return type deduction
auto GetPlayer() -> Player {
    return Player("Virat");
}

// Trailing return type
auto Add(int a, int b) -> decltype(a + b) {
    return a + b;
}
```

### constexpr

Compile-time constants and functions.

```cpp
// Compile-time constant
constexpr float PI = 3.14159f;
constexpr int MAX_PLAYERS = 11;

// Compile-time function
constexpr int Square(int x) {
    return x * x;
}

// Used in array size
std::array<int, Square(5)> data;  // Size = 25

// Compile-time if (C++17)
template<typename T>
void Process(T value) {
    if constexpr (std::is_integral_v<T>) {
        // Integer-specific code
    } else if constexpr (std::is_floating_point_v<T>) {
        // Float-specific code
    }
}
```

### Enum Class

Type-safe enumerations.

```cpp
// Old-style enum (not type-safe)
enum Color { Red, Green, Blue };

// Modern enum class
enum class GameState {
    Menu,
    Playing,
    Paused,
    GameOver
};

// Usage
GameState state = GameState::Playing;

// Cannot implicitly convert to int
// int x = GameState::Playing;  // Error!

// Explicit conversion
int stateValue = static_cast<int>(GameState::Playing);

// Specify underlying type
enum class LogLevel : uint8_t {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3
};
```

### nullptr

Type-safe null pointer.

```cpp
// Old way
Player* player = NULL;  // or 0

// Modern way
Player* player = nullptr;

// Function overloading works correctly
void Process(int x) { }
void Process(Player* p) { }

Process(nullptr);  // Calls Process(Player*)
// Process(NULL);  // Ambiguous!
```

---

## Error Handling

### Exceptions

```cpp
class FileNotFoundException : public std::runtime_error {
public:
    FileNotFoundException(const std::string& filename)
        : std::runtime_error("File not found: " + filename) {}
};

// Throwing exceptions
Texture* LoadTexture(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw FileNotFoundException(path);
    }
    // Load texture
    return new Texture();
}

// Catching exceptions
try {
    Texture* tex = LoadTexture("ball.png");
    UseTexture(tex);
}
catch (const FileNotFoundException& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    // Use default texture
}
catch (const std::exception& e) {
    std::cerr << "Unexpected error: " << e.what() << std::endl;
}
catch (...) {
    std::cerr << "Unknown error" << std::endl;
}
```

### noexcept

Specify functions that don't throw exceptions.

```cpp
class Vector3 {
public:
    // Move operations should be noexcept
    Vector3(Vector3&& other) noexcept 
        : x(other.x), y(other.y), z(other.z) {}
    
    Vector3& operator=(Vector3&& other) noexcept {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }
    
    // Simple operations
    float Length() const noexcept {
        return std::sqrt(x*x + y*y + z*z);
    }
    
private:
    float x, y, z;
};

// Conditional noexcept
template<typename T>
void Swap(T& a, T& b) noexcept(std::is_nothrow_move_constructible_v<T>) {
    T temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}
```

### Error Codes vs Exceptions

```cpp
// Error codes (C-style)
enum class ErrorCode {
    Success,
    FileNotFound,
    InvalidFormat,
    OutOfMemory
};

ErrorCode LoadTexture(const std::string& path, Texture** out) {
    if (!FileExists(path)) {
        return ErrorCode::FileNotFound;
    }
    *out = new Texture();
    return ErrorCode::Success;
}

// Modern alternative: std::expected (C++23) or std::optional
std::optional<Texture> LoadTexture(const std::string& path) {
    if (!FileExists(path)) {
        return std::nullopt;
    }
    return Texture(path);
}
```

---

## Concurrency and Threading

### std::thread

```cpp
#include <thread>

void LoadAssets() {
    // Time-consuming task
}

// Create and start thread
std::thread loadThread(LoadAssets);

// Wait for completion
loadThread.join();

// Detach thread (runs independently)
std::thread backgroundThread([]() {
    // Background task
});
backgroundThread.detach();
```

### std::mutex

Thread-safe access to shared data.

```cpp
#include <mutex>

class Logger {
private:
    static std::mutex logMutex;
    static std::ofstream logFile;
    
public:
    static void Log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        logFile << message << std::endl;
        // Mutex automatically released when lock goes out of scope
    }
};

// Scoped locking
void UpdateScore(int& score) {
    std::lock_guard<std::mutex> lock(scoreMutex);
    score += 10;
}  // Mutex released here
```

### std::atomic

Lock-free atomic operations.

```cpp
#include <atomic>

class GameStats {
private:
    std::atomic<int> frameCount{0};
    std::atomic<bool> isRunning{true};
    
public:
    void IncrementFrames() {
        frameCount++;  // Atomic increment
    }
    
    int GetFrameCount() const {
        return frameCount.load();
    }
    
    void Stop() {
        isRunning.store(false);
    }
    
    bool IsRunning() const {
        return isRunning.load();
    }
};
```

### std::async and std::future

Asynchronous task execution.

```cpp
#include <future>

// Launch async task
std::future<Texture> futureTexture = std::async(std::launch::async, []() {
    return LoadTextureFromDisk("ball.png");
});

// Do other work...

// Get result (blocks if not ready)
Texture texture = futureTexture.get();

// Check if ready without blocking
if (futureTexture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
    Texture tex = futureTexture.get();
}
```

---

## Performance Optimization

### Inline Functions

```cpp
// Inline suggestion to compiler
inline float Square(float x) {
    return x * x;
}

// Class methods defined in header are implicitly inline
class Vector3 {
public:
    float Dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    
private:
    float x, y, z;
};
```

### Move Semantics for Performance

```cpp
// Return by value (efficient with move)
std::vector<int> CreateLargeVector() {
    std::vector<int> data(1000000);
    // Fill data
    return data;  // Move, not copy
}

// Perfect forwarding
template<typename T>
void Wrapper(T&& arg) {
    Process(std::forward<T>(arg));  // Preserve value category
}
```

### Reserve and Emplace

```cpp
std::vector<Player> players;

// Reserve capacity to avoid reallocations
players.reserve(100);

// Emplace constructs in-place (no copy/move)
players.emplace_back("Sachin", 100, 50);  // Construct Player directly

// vs push_back (creates temporary then moves)
players.push_back(Player("Dhoni", 90, 80));
```

### Cache-Friendly Code

```cpp
// Bad: Array of Structures (AoS)
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
    glm::vec4 color;
};
std::vector<Particle> particles;

// Update positions (poor cache locality)
for (auto& p : particles) {
    p.position += p.velocity * dt;  // Jumps around memory
}

// Good: Structure of Arrays (SoA)
struct ParticleSystem {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> velocities;
    std::vector<float> lifetimes;
    std::vector<glm::vec4> colors;
};

// Update positions (excellent cache locality)
for (size_t i = 0; i < positions.size(); ++i) {
    positions[i] += velocities[i] * dt;  // Sequential memory access
}
```

### Const Correctness

```cpp
class Player {
private:
    std::string name;
    int score;
    
public:
    // Const methods don't modify object
    std::string GetName() const { return name; }
    int GetScore() const { return score; }
    
    // Non-const methods can modify
    void SetScore(int s) { score = s; }
    
    // Const reference parameters (avoid copies)
    void SetName(const std::string& n) { name = n; }
};

// Const correctness enables optimizations
void DisplayPlayer(const Player& player) {
    std::cout << player.GetName();  // OK: const method
    // player.SetScore(100);  // Error: non-const method
}
```

---

## Best Practices

### Rule of Zero/Three/Five

**Rule of Zero**: If you don't need custom destructor, copy, or move operations, don't define them.

```cpp
// Good: Rule of Zero
class Player {
private:
    std::string name;
    std::vector<int> scores;
    std::unique_ptr<Stats> stats;
    
public:
    // No custom destructor, copy, or move needed
    // Compiler-generated versions work correctly
};
```

**Rule of Five**: If you define one of destructor, copy constructor, copy assignment, move constructor, or move assignment, define all five.

```cpp
class Texture {
private:
    unsigned int textureID;
    
public:
    // Destructor
    ~Texture() {
        glDeleteTextures(1, &textureID);
    }
    
    // Copy constructor
    Texture(const Texture& other) {
        // Deep copy
    }
    
    // Copy assignment
    Texture& operator=(const Texture& other) {
        if (this != &other) {
            // Clean up, then copy
        }
        return *this;
    }
    
    // Move constructor
    Texture(Texture&& other) noexcept {
        textureID = other.textureID;
        other.textureID = 0;
    }
    
    // Move assignment
    Texture& operator=(Texture&& other) noexcept {
        if (this != &other) {
            glDeleteTextures(1, &textureID);
            textureID = other.textureID;
            other.textureID = 0;
        }
        return *this;
    }
};
```

### Prefer Stack to Heap

```cpp
// Bad: Unnecessary heap allocation
void ProcessData() {
    Player* player = new Player("Virat");
    player->Update();
    delete player;  // Easy to forget!
}

// Good: Stack allocation
void ProcessData() {
    Player player("Virat");
    player.Update();
}  // Automatically destroyed

// Use heap only when necessary
std::unique_ptr<LargeObject> obj = std::make_unique<LargeObject>();
```

### Use const References for Parameters

```cpp
// Bad: Pass by value (copies)
void ProcessPlayer(Player player) {
    // Works with copy
}

// Good: Pass by const reference (no copy)
void ProcessPlayer(const Player& player) {
    // Read-only access, no copy
}

// If modification needed, pass by reference
void UpdatePlayer(Player& player) {
    player.SetScore(100);
}

// Small types can be passed by value
void SetPosition(float x, float y, float z) {
    // Floats are cheap to copy
}
```

### Initialize Members in Constructor

```cpp
// Bad: Assignment in constructor body
class Player {
    std::string name;
    int score;
    
public:
    Player(const std::string& n, int s) {
        name = n;      // Default constructed, then assigned
        score = s;
    }
};

// Good: Member initializer list
class Player {
    std::string name;
    int score;
    
public:
    Player(const std::string& n, int s) 
        : name(n), score(s)  // Direct initialization
    {
    }
};

// Even better: Default member initialization
class Player {
    std::string name;
    int score = 0;  // Default value
    bool active = true;
    
public:
    Player(const std::string& n) : name(n) {}
};
```

### Use override and final

```cpp
class GameObject {
public:
    virtual void Update(float dt) = 0;
    virtual void Render() const = 0;
};

class Player : public GameObject {
public:
    // override: Compiler checks this overrides base method
    void Update(float dt) override {
        // Implementation
    }
    
    // final: Prevents further overriding
    void Render() const override final {
        // Implementation
    }
};

class SpecialPlayer : public Player {
    // void Render() const override;  // Error: Render is final
};
```

### Avoid Raw Pointers for Ownership

```cpp
// Bad: Raw pointer ownership unclear
class Game {
    Renderer* renderer;  // Who owns this?
    
public:
    Game() {
        renderer = new Renderer();
    }
    
    ~Game() {
        delete renderer;  // Easy to forget
    }
};

// Good: Clear ownership with smart pointers
class Game {
    std::unique_ptr<Renderer> renderer;  // Game owns renderer
    
public:
    Game() : renderer(std::make_unique<Renderer>()) {}
    // Destructor automatically cleans up
};

// Raw pointers OK for non-owning references
class Player {
    Team* team;  // Player doesn't own team
    
public:
    void SetTeam(Team* t) { team = t; }
};
```

### Use Namespaces

```cpp
// Organize code into namespaces
namespace cricket {
    namespace physics {
        class Ball { };
        class CollisionDetector { };
    }
    
    namespace rendering {
        class Renderer { };
        class Camera { };
    }
    
    namespace audio {
        class AudioManager { };
    }
}

// Usage
cricket::physics::Ball ball;
cricket::rendering::Renderer renderer;

// Namespace alias
namespace cr = cricket::rendering;
cr::Camera camera;

// Using declaration (use sparingly)
using cricket::physics::Ball;
Ball ball;
```

### Header Guards vs #pragma once

```cpp
// Traditional header guards
#ifndef PLAYER_H
#define PLAYER_H

class Player {
    // ...
};

#endif // PLAYER_H

// Modern alternative: #pragma once
#pragma once

class Player {
    // ...
};
```

**Recommendation**: Use `#pragma once` for simplicity (supported by all modern compilers).

### Forward Declarations

Reduce compilation dependencies.

```cpp
// Player.h
#pragma once

// Forward declarations instead of #include
class Team;
class Stats;

class Player {
private:
    Team* team;              // Pointer: forward declaration OK
    std::unique_ptr<Stats> stats;  // Pointer: forward declaration OK
    
public:
    void SetTeam(Team* t);
    Stats* GetStats() const;
};

// Player.cpp
#include "Player.h"
#include "Team.h"    // Include in .cpp file
#include "Stats.h"

void Player::SetTeam(Team* t) {
    team = t;
}
```

---

## Common Patterns in Cricket Game

### Singleton Pattern

```cpp
class ResourceManager {
public:
    static ResourceManager& Instance() {
        static ResourceManager instance;
        return instance;
    }
    
    // Delete copy and move
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    Texture* LoadTexture(const std::string& path);
    
private:
    ResourceManager() = default;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
};

// Usage
auto& rm = ResourceManager::Instance();
Texture* tex = rm.LoadTexture("ball.png");
```

### Factory Pattern

```cpp
class Player {
public:
    virtual ~Player() = default;
    virtual void Update(float dt) = 0;
};

class HumanPlayer : public Player {
    void Update(float dt) override { /* Human input */ }
};

class AIPlayer : public Player {
    void Update(float dt) override { /* AI logic */ }
};

class PlayerFactory {
public:
    static std::unique_ptr<Player> Create(PlayerType type) {
        switch (type) {
            case PlayerType::Human:
                return std::make_unique<HumanPlayer>();
            case PlayerType::AI:
                return std::make_unique<AIPlayer>();
        }
    }
};
```

### Observer Pattern

```cpp
class IEventListener {
public:
    virtual ~IEventListener() = default;
    virtual void OnEvent(const Event& event) = 0;
};

class EventManager {
private:
    std::unordered_map<EventType, std::vector<IEventListener*>> listeners;
    
public:
    void Subscribe(EventType type, IEventListener* listener) {
        listeners[type].push_back(listener);
    }
    
    void Publish(const Event& event) {
        for (auto* listener : listeners[event.type]) {
            listener->OnEvent(event);
        }
    }
};

// Usage
class ScoreDisplay : public IEventListener {
    void OnEvent(const Event& event) override {
        if (event.type == EventType::RunsScored) {
            UpdateDisplay(event.data);
        }
    }
};
```

### State Pattern

```cpp
class IGameState {
public:
    virtual ~IGameState() = default;
    virtual void Enter() = 0;
    virtual void Exit() = 0;
    virtual void Update(float dt) = 0;
};

class GameStateManager {
private:
    std::unique_ptr<IGameState> currentState;
    
public:
    void ChangeState(std::unique_ptr<IGameState> newState) {
        if (currentState) {
            currentState->Exit();
        }
        currentState = std::move(newState);
        currentState->Enter();
    }
    
    void Update(float dt) {
        if (currentState) {
            currentState->Update(dt);
        }
    }
};
```

---

## Debugging and Profiling

### Assertions

```cpp
#include <cassert>

void SetScore(int score) {
    assert(score >= 0 && "Score cannot be negative");
    this->score = score;
}

// Custom assert with message
#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "Assertion failed: " << message << std::endl; \
            std::abort(); \
        } \
    } while(false)

ASSERT(players.size() <= 11, "Too many players in team");
```

### Static Analysis

```cpp
// Use compiler warnings
// g++ -Wall -Wextra -Wpedantic

// Suppress specific warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void Function(int unused) {
    // Code
}
#pragma GCC diagnostic pop
```

### Profiling

```cpp
#include <chrono>

class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::string name;
    
public:
    Timer(const std::string& n) : name(n) {
        start = std::chrono::high_resolution_clock::now();
    }
    
    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << name << ": " << duration.count() << " μs" << std::endl;
    }
};

// Usage
void ExpensiveFunction() {
    Timer timer("ExpensiveFunction");
    // Code to profile
}
```

---

## Summary

### Key Takeaways

1. **Use Modern C++ Features**
   - Smart pointers for memory management
   - Auto for type deduction
   - Range-based for loops
   - Lambda expressions

2. **Follow Best Practices**
   - RAII for resource management
   - Const correctness
   - Rule of Zero/Five
   - Prefer stack to heap

3. **Write Efficient Code**
   - Move semantics
   - Reserve container capacity
   - Cache-friendly data structures
   - Inline small functions

4. **Design Patterns**
   - Singleton for global managers
   - Factory for object creation
   - Observer for event systems
   - State for game states

5. **Error Handling**
   - Use exceptions for exceptional cases
   - noexcept for performance-critical code
   - RAII for cleanup

6. **Concurrency**
   - std::thread for parallelism
   - std::mutex for synchronization
   - std::atomic for lock-free operations

### Further Reading

- **Books**:
  - "Effective Modern C++" by Scott Meyers
  - "C++ Concurrency in Action" by Anthony Williams
  - "Game Programming Patterns" by Robert Nystrom

- **Online Resources**:
  - cppreference.com - C++ reference
  - isocpp.org - C++ standards
  - learncpp.com - C++ tutorials

- **Tools**:
  - Compiler Explorer (godbolt.org)
  - Valgrind (memory debugging)
  - gprof/perf (profiling)
  - clang-tidy (static analysis)

---

## Conclusion

This document covers the essential C++ concepts used in the Cricket Game project. Understanding these concepts is crucial for:
- Writing maintainable code
- Avoiding common pitfalls
- Optimizing performance
- Following modern C++ best practices

For project-specific implementation details, refer to:
- [ARCHITECTURE.md](ARCHITECTURE.md) - System design
- [CONCEPTS.md](CONCEPTS.md) - Game concepts
- Source code - Practical examples
