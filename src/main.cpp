#include "core/Application.h"
#include "core/Logger.h"
#include <exception>

int main(int argc, char* argv[]) {
    try {
        Logger::Initialize("logs/game.log");
        Logger::Log(LogLevel::Info, "Cricket Game Starting...");
        
        Application app;
        
        if (!app.Initialize()) {
            Logger::Log(LogLevel::Fatal, "Failed to initialize application");
            return 1;
        }
        
        app.Run();
        app.Shutdown();
        
        Logger::Log(LogLevel::Info, "Cricket Game Shutdown Successfully");
        return 0;
        
    } catch (const std::exception& e) {
        Logger::Log(LogLevel::Fatal, std::string("Unhandled exception: ") + e.what());
        return 1;
    }
}
