#pragma once
#include <memory>
#include <unordered_map>
#include <string>

class Texture;
class Mesh;
class Shader;

class ResourceManager {
public:
    static ResourceManager& Instance();
    Texture* LoadTexture(const std::string& path);
    Mesh* LoadMesh(const std::string& path);
    Shader* LoadShader(const std::string& vertPath, const std::string& fragPath);
    void UnloadAll();
private:
    ResourceManager() = default;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
};
