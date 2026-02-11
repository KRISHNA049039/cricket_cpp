#include "AudioManager.h"
#include "../core/Logger.h"
#include <SDL2/SDL_mixer.h>

AudioManager::AudioManager() {}

AudioManager::~AudioManager() {
    Shutdown();
}

bool AudioManager::Initialize() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        Logger::Log(LogLevel::Error, std::string("SDL_mixer init failed: ") + Mix_GetError());
        return false;
    }
    Logger::Log(LogLevel::Info, "Audio system initialized");
    return true;
}

void AudioManager::Shutdown() {
    Mix_CloseAudio();
}

void AudioManager::PlaySound(const std::string& soundName, float volume) {
    // Stub for MVP
}
