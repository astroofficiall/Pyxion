#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <cmath>

struct AABB {
    glm::vec3 min{0.0f};
    glm::vec3 max{0.0f};
};

struct ColBox {
    glm::vec3 center{0.0f};
    glm::vec3 half{0.5f};
    glm::vec3 color{0.6f, 0.4f, 0.3f};
};

inline AABB makeAABB(glm::vec3 center, glm::vec3 halfExtents) {
    return { center - halfExtents, center + halfExtents };
}

inline bool aabbOverlap(const AABB& a, const AABB& b, glm::vec3& pushOut) {
    float dx1 = b.max.x - a.min.x, dx2 = a.max.x - b.min.x;
    float dy1 = b.max.y - a.min.y, dy2 = a.max.y - b.min.y;
    float dz1 = b.max.z - a.min.z, dz2 = a.max.z - b.min.z;

    if (dx1 < 0 || dx2 < 0 || dy1 < 0 || dy2 < 0 || dz1 < 0 || dz2 < 0)
        return false;

    float px = dx1 < dx2 ? dx1 : -dx2;
    float py = dy1 < dy2 ? dy1 : -dy2;
    float pz = dz1 < dz2 ? dz1 : -dz2;

    float absPx = std::abs(px), absPy = std::abs(py), absPz = std::abs(pz);
    if      (absPx <= absPy && absPx <= absPz) pushOut = glm::vec3(px, 0, 0);
    else if (absPy <= absPx && absPy <= absPz) pushOut = glm::vec3(0, py, 0);
    else                                        pushOut = glm::vec3(0, 0, pz);

    return true;
}

inline void resolvePlayer(glm::vec3& pos, float& verticalVel, bool& onGround,
                           const std::vector<ColBox>& boxes,
                           float groundLevel, glm::vec3 playerHalf) {
    if (pos.y <= groundLevel) {
        pos.y = groundLevel; verticalVel = 0.0f; onGround = true;
    }
    for (auto& box : boxes) {
        AABB player = makeAABB(pos + glm::vec3(0, playerHalf.y, 0), playerHalf);
        AABB b      = makeAABB(box.center, box.half);
        glm::vec3 push{0.0f};
        if (!aabbOverlap(player, b, push)) continue;
        pos += push;
        if (push.y > 0.0f) { verticalVel = 0.0f; onGround = true; }
        if (push.y < 0.0f) { verticalVel = 0.0f; }
    }
}