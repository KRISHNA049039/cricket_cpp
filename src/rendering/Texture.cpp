#include "Texture.h"
#include <glad/glad.h>

Texture::Texture() : textureID(0), width(0), height(0), channels(0) {}
Texture::~Texture() { if (textureID) glDeleteTextures(1, &textureID); }
bool Texture::LoadFromFile(const std::string& path) { return false; }
void Texture::Bind(unsigned int slot) const { glActiveTexture(GL_TEXTURE0 + slot); glBindTexture(GL_TEXTURE_2D, textureID); }
