#pragma once

#include <string>
#include <fstream>
#include <mutex>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class Logger {
public:
    static void Initialize(const std::string& logFilePath);
    static void Log(LogLevel level, const std::string& message);
    static void Shutdown();
    
private:
    static std::ofstream logFile;
    static std::mutex logMutex;
    static std::string GetTimestamp();
    static std::string LogLevelToString(LogLevel level);
};
