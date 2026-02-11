#pragma once
#include <glm/glm.hpp>

class Ball {
public:
    Ball();
    void Update(float deltaTime);
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 spin;
    float mass;
    float radius;
};
