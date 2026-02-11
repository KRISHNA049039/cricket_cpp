#include "Application.h"
#include "Logger.h"
#include "ConfigManager.h"
#include "../rendering/Renderer.h"
#include "../input/InputHandler.h"
#include "../physics/PhysicsEngine.h"
#include "../gameplay/MatchManager.h"
#include "../audio/AudioManager.h"
#include <glad/glad.h>
#include <SDL2/SDL.h>

Application::Application()
    : window(nullptr)
    , glContext(nullptr)
    , isRunning(false)
    , accumulator(0.0f)
{
}

Application::~Application() {
}

bool Application::Initialize() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        Logger::Log(LogLevel::Error, std::string("SDL initialization failed: ") + SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Initialization Error", 
                                 "Failed to initialize SDL", nullptr);
        return false;
    }
    
    Logger::Log(LogLevel::Info, "SDL initialized successfully");
    
    // Load configuration
    configManager = std::make_unique<ConfigManager>();
    configManager->LoadGraphicsConfig("config/graphics.json");
    configManager->LoadGameConfig("config/game.json");
    
    auto& gfxConfig = configManager->GetGraphicsConfig();
    
    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    // Create window
    Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    if (gfxConfig.fullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }
    
    window = SDL_CreateWindow(
        "Cricket Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        gfxConfig.width,
        gfxConfig.height,
        windowFlags
    );
    
    if (!window) {
        Logger::Log(LogLevel::Error, std::string("Window creation failed: ") + SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Initialization Error",
                                 "Failed to create window", nullptr);
        return false;
    }
    
    Logger::Log(LogLevel::Info, "Window created successfully");
    
    // Create OpenGL context
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        Logger::Log(LogLevel::Error, std::string("OpenGL context creation failed: ") + SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Initialization Error",
                                 "Failed to create OpenGL context", nullptr);
        return false;
    }
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        Logger::Log(LogLevel::Error, "Failed to initialize GLAD");
        return false;
    }
    
    Logger::Log(LogLevel::Info, std::string("OpenGL Version: ") + 
                (const char*)glGetString(GL_VERSION));
    
    // Enable VSync
    if (gfxConfig.vsync) {
        SDL_GL_SetSwapInterval(1);
    }
    
    // Initialize subsystems
    renderer = std::make_unique<Renderer>();
    if (!renderer->Initialize(gfxConfig.width, gfxConfig.height)) {
        Logger::Log(LogLevel::Error, "Failed to initialize renderer");
        return false;
    }
    
    inputHandler = std::make_unique<InputHandler>();
    physicsEngine = std::make_unique<PhysicsEngine>();
    matchManager = std::make_unique<MatchManager>();
    audioManager = std::make_unique<AudioManager>();
    
    if (!audioManager->Initialize()) {
        Logger::Log(LogLevel::Warning, "Failed to initialize audio system");
    }
    
    isRunning = true;
    Logger::Log(LogLevel::Info, "Application initialized successfully");
    
    return true;
}

void Application::Run() {
    Uint64 lastTime = SDL_GetPerformanceCounter();
    const Uint64 frequency = SDL_GetPerformanceFrequency();
    
    while (isRunning) {
        Uint64 currentTime = SDL_GetPerformanceCounter();
        float deltaTime = (float)(currentTime - lastTime) / frequency;
        lastTime = currentTime;
        
        // Cap delta time to prevent spiral of death
        if (deltaTime > 0.25f) {
            deltaTime = 0.25f;
        }
        
        ProcessInput();
        
        // Fixed timestep physics
        accumulator += deltaTime;
        while (accumulator >= FIXED_TIMESTEP) {
            FixedUpdate(FIXED_TIMESTEP);
            accumulator -= FIXED_TIMESTEP;
        }
        
        Update(deltaTime);
        Render();
    }
}

void Application::ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
        
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            isRunning = false;
        }
    }
    
    inputHandler->Update();
}

void Application::Update(float deltaTime) {
    matchManager->Update(deltaTime);
}

void Application::FixedUpdate(float fixedDeltaTime) {
    physicsEngine->Update(fixedDeltaTime);
}

void Application::Render() {
    renderer->BeginFrame();
    
    // Render game objects here
    
    renderer->EndFrame();
    SDL_GL_SwapWindow(window);
}

void Application::Shutdown() {
    Logger::Log(LogLevel::Info, "Shutting down application");
    
    audioManager.reset();
    matchManager.reset();
    physicsEngine.reset();
    inputHandler.reset();
    renderer.reset();
    
    if (glContext) {
        SDL_GL_DeleteContext(glContext);
    }
    
    if (window) {
        SDL_DestroyWindow(window);
    }
    
    SDL_Quit();
}
