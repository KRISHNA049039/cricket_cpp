#pragma once
#include <string>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    bool Initialize();
    void Shutdown();
    void PlaySound(const std::string& soundName, float volume = 1.0f);
};
