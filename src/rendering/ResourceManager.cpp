#include "ResourceManager.h"
#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"

ResourceManager& ResourceManager::Instance() {
    static ResourceManager instance;
    return instance;
}

Texture* ResourceManager::LoadTexture(const std::string& path) { return nullptr; }
Mesh* ResourceManager::LoadMesh(const std::string& path) { return nullptr; }
Shader* ResourceManager::LoadShader(const std::string& vertPath, const std::string& fragPath) { return nullptr; }
void ResourceManager::UnloadAll() { textures.clear(); meshes.clear(); shaders.clear(); }
