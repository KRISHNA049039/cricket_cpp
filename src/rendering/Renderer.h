#pragma once

#include <memory>
#include <glm/glm.hpp>

class Shader;
class Camera;
class Mesh;

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    bool Initialize(int width, int height);
    void BeginFrame();
    void EndFrame();
    
    void RenderMesh(const Mesh& mesh, const glm::mat4& transform);
    void SetCamera(Camera* camera);
    
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    
private:
    int width;
    int height;
    Camera* activeCamera;
    
    std::unique_ptr<Shader> defaultShader;
};
