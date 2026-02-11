#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

std::ofstream Logger::logFile;
std::mutex Logger::logMutex;

void Logger::Initialize(const std::string& logFilePath) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    // Create logs directory if it doesn't exist
    std::filesystem::path path(logFilePath);
    std::filesystem::create_directories(path.parent_path());
    
    logFile.open(logFilePath, std::ios::out | std::ios::app);
    if (logFile.is_open()) {
        logFile << "\n=== Cricket Game Log Started at " << GetTimestamp() << " ===\n";
    }
}

void Logger::Log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    std::string timestamp = GetTimestamp();
    std::string levelStr = LogLevelToString(level);
    std::string logMessage = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    // Console output with color
    std::cout << logMessage << std::endl;
    
    // File output
    if (logFile.is_open()) {
        logFile << logMessage << std::endl;
        logFile.flush();
    }
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile << "=== Cricket Game Log Ended at " << GetTimestamp() << " ===\n";
        logFile.close();
    }
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
        default:                return "UNKNOWN";
    }
}
