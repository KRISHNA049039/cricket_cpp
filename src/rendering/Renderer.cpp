#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "../core/Logger.h"
#include <glad/glad.h>

Renderer::Renderer()
    : width(0)
    , height(0)
    , activeCamera(nullptr)
{
}

Renderer::~Renderer() {
}

bool Renderer::Initialize(int w, int h) {
    width = w;
    height = h;
    
    // Set viewport
    glViewport(0, 0, width, height);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Enable face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Set clear color
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);  // Sky blue
    
    Logger::Log(LogLevel::Info, "Renderer initialized successfully");
    return true;
}

void Renderer::BeginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame() {
    // Frame complete
}

void Renderer::RenderMesh(const Mesh& mesh, const glm::mat4& transform) {
    // MVP rendering implementation
    // For MVP, basic rendering stub
}

void Renderer::SetCamera(Camera* camera) {
    activeCamera = camera;
}
