#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Mesh {
public:
    Mesh();
    ~Mesh();
    
    void SetupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    void Draw() const;
    
private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;
};
