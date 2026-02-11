#pragma once

#include <string>

struct GraphicsConfig {
    int width = 1920;
    int height = 1080;
    bool fullscreen = false;
    bool vsync = true;
    std::string quality = "high";
    int fov = 75;
};

struct GameConfig {
    std::string difficulty = "medium";
    int oversPerInnings = 20;
    float humidity = 0.6f;
    float windSpeed = 5.0f;
    float pitchHardness = 0.7f;
    float grassCoverage = 0.4f;
};

class ConfigManager {
public:
    ConfigManager();
    
    bool LoadGraphicsConfig(const std::string& path);
    bool LoadGameConfig(const std::string& path);
    
    bool SaveGraphicsConfig(const std::string& path);
    bool SaveGameConfig(const std::string& path);
    
    const GraphicsConfig& GetGraphicsConfig() const { return graphicsConfig; }
    const GameConfig& GetGameConfig() const { return gameConfig; }
    
    void SetGraphicsConfig(const GraphicsConfig& config) { graphicsConfig = config; }
    void SetGameConfig(const GameConfig& config) { gameConfig = config; }
    
private:
    GraphicsConfig graphicsConfig;
    GameConfig gameConfig;
};
