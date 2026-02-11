#pragma once
#include <string>

class Texture {
public:
    Texture();
    ~Texture();
    bool LoadFromFile(const std::string& path);
    void Bind(unsigned int slot = 0) const;
    unsigned int GetID() const { return textureID; }
private:
    unsigned int textureID;
    int width, height, channels;
};
