#pragma once

#include <glm/glm.hpp>

enum class CameraMode {
    BowlerView,
    BatsmanView,
    FollowBall,
    Cinematic
};

class Camera {
public:
    Camera();
    
    void SetPosition(const glm::vec3& pos);
    void SetTarget(const glm::vec3& target);
    void SetFOV(float fov);
    void SetMode(CameraMode mode);
    
    void Update(float deltaTime);
    
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspectRatio) const;
    
    glm::vec3 GetPosition() const { return position; }
    
private:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    float fov;
    CameraMode currentMode;
};
