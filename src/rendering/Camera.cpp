#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
    : position(0.0f, 5.0f, 10.0f)
    , target(0.0f, 0.0f, 0.0f)
    , up(0.0f, 1.0f, 0.0f)
    , fov(75.0f)
    , currentMode(CameraMode::BatsmanView)
{
}

void Camera::SetPosition(const glm::vec3& pos) {
    position = pos;
}

void Camera::SetTarget(const glm::vec3& t) {
    target = t;
}

void Camera::SetFOV(float f) {
    fov = f;
}

void Camera::SetMode(CameraMode mode) {
    currentMode = mode;
}

void Camera::Update(float deltaTime) {
    // Camera update logic
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(position, target, up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 1000.0f);
}
