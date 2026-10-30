#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

struct ThirdPersonCamera {
    glm::vec3 target{0.0f};
    float yaw       = 0.0f;
    float pitch     = 0.35f;
    float distance  = 8.0f;
    float minDistance  = 0.0f;
    float maxDistance  = 50.0f;
    float minPitch     = 0.05f;
    float maxPitch     = 1.40f;
    float targetHeight = 3.5f;

    glm::vec3 position() const {
        return target + glm::vec3(
            distance * std::cos(pitch) * std::sin(yaw),
            distance * std::sin(pitch),
            distance * std::cos(pitch) * std::cos(yaw));
    }

    glm::mat4 view() const {
        return glm::lookAt(position(),
                           target + glm::vec3(0.0f, targetHeight, 0.0f),
                           glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void zoom(float amount) {
        distance = std::clamp(distance - amount, minDistance, maxDistance);
    }

    void orbit(float dYaw, float dPitch) {
        yaw += dYaw;
        pitch = std::clamp(pitch + dPitch, minPitch, maxPitch);
    }
};