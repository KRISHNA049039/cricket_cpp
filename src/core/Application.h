#pragma once

#include <SDL2/SDL.h>
#include <memory>

class Renderer;
class InputHandler;
class PhysicsEngine;
class MatchManager;
class AudioManager;
class ConfigManager;

class Application {
public:
    Application();
    ~Application();
    
    bool Initialize();
    void Run();
    void Shutdown();
    
private:
    void ProcessInput();
    void Update(float deltaTime);
    void FixedUpdate(float fixedDeltaTime);
    void Render();
    
    SDL_Window* window;
    SDL_GLContext glContext;
    bool isRunning;
    
    // Core systems
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<InputHandler> inputHandler;
    std::unique_ptr<PhysicsEngine> physicsEngine;
    std::unique_ptr<MatchManager> matchManager;
    std::unique_ptr<AudioManager> audioManager;
    std::unique_ptr<ConfigManager> configManager;
    
    // Timing
    float accumulator;
    const float FIXED_TIMESTEP = 1.0f / 120.0f;  // 120Hz physics
};
