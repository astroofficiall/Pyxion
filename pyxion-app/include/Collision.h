// Collision.h
#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include <algorithm>

struct AABB {
    glm::vec3 min{0.0f};
    glm::vec3 max{0.5f};
};

struct ColBox {
    glm::vec3 center{0.0f};
    glm::vec3 half{0.5f};
    glm::vec3 color{0.6f, 0.4f, 0.3f};
};

inline AABB makeAABB(glm::vec3 center, glm::vec3 halfExtents) {
    return { center - halfExtents, center + halfExtents };
}

inline bool aabbOverlap(const AABB& a, const AABB& b,
                        glm::vec3& pushOut) {
    float dx1 = b.max.x - a.min.x;
    float dx2 = a.max.x - b.min.x;
    float dy1 = b.max.y - a.min.y;
    float dy2 = a.max.y - b.min.y;
    float dz1 = b.max.z - a.min.z;
    float dz2 = a.max.z - b.min.z;

    if (dx1 < 0 || dx2 < 0 || dy1 < 0 ||
        dy2 < 0 || dz1 < 0 || dz2 < 0)
        return false;

    float px = dx1 < dx2 ? dx1 : -dx2;
    float py = dy1 < dy2 ? dy1 : -dy2;
    float pz = dz1 < dz2 ? dz1 : -dz2;

    float absPx = std::abs(px);
    float absPy = std::abs(py);
    float absPz = std::abs(pz);

    if (absPx <= absPy && absPx <= absPz)
        pushOut = glm::vec3(px, 0, 0);
    else if (absPy <= absPx && absPy <= absPz)
        pushOut = glm::vec3(0, py, 0);
    else
        pushOut = glm::vec3(0, 0, pz);

    return true;
}

// Player collision retained from the original project.
inline void resolvePlayer(glm::vec3& pos, float& verticalVel,
                          bool& onGround,
                          const std::vector<ColBox>& boxes,
                          float groundLevel,
                          glm::vec3 playerHalf) {
    if (pos.y <= groundLevel) {
        pos.y = groundLevel;
        verticalVel = 0.0f;
        onGround = true;
    }

    for (const auto& box : boxes) {
        AABB player = makeAABB(
            pos + glm::vec3(0, playerHalf.y, 0),
            playerHalf
        );
        AABB b = makeAABB(box.center, box.half);

        glm::vec3 push(0.0f);
        if (!aabbOverlap(player, b, push))
            continue;

        pos += push;

        if (push.y > 0.0f) {
            verticalVel = 0.0f;
            onGround = true;
        }
        if (push.y < 0.0f)
            verticalVel = 0.0f;
    }
}

// An oriented box. axes are the local X/Y/Z axes in world space.
struct OrientedBox {
    glm::vec3 center{0.0f};
    glm::vec3 half{0.5f};
    glm::mat3 rotation{1.0f};
};

inline OrientedBox makeOrientedBox(
    const glm::vec3& center,
    const glm::vec3& half,
    const glm::mat3& rotation
) {
    return { center, glm::max(half, glm::vec3(0.001f)), rotation };
}

inline OrientedBox makeAxisAlignedBox(const ColBox& box) {
    return makeOrientedBox(
        box.center,
        box.half,
        glm::mat3(1.0f)
    );
}

inline float projectRadius(const OrientedBox& box,
                           const glm::vec3& axis) {
    return
        box.half.x * std::abs(glm::dot(axis, box.rotation[0])) +
        box.half.y * std::abs(glm::dot(axis, box.rotation[1])) +
        box.half.z * std::abs(glm::dot(axis, box.rotation[2]));
}

// Separating Axis Test for OBB vs OBB. Returns a normal pointing
// from A toward B and the smallest overlap along the tested axes.
inline bool orientedBoxOverlap(
    const OrientedBox& a,
    const OrientedBox& b,
    glm::vec3& normal,
    float& penetration
) {
    glm::vec3 axes[15];
    int count = 0;

    for (int i = 0; i < 3; ++i)
        axes[count++] = a.rotation[i];

    for (int i = 0; i < 3; ++i)
        axes[count++] = b.rotation[i];

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j)
            axes[count++] = glm::cross(a.rotation[i], b.rotation[j]);
    }

    glm::vec3 centerDelta = b.center - a.center;
    float smallestOverlap = 1.0e30f;
    glm::vec3 smallestAxis(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < count; ++i) {
        float axisLength2 = glm::dot(axes[i], axes[i]);

        // Parallel axes have a zero cross product and can be skipped.
        if (axisLength2 < 1.0e-8f)
            continue;

        glm::vec3 axis = axes[i] / std::sqrt(axisLength2);

        float radiusA = projectRadius(a, axis);
        float radiusB = projectRadius(b, axis);
        float signedDistance = glm::dot(centerDelta, axis);
        float overlap = radiusA + radiusB - std::abs(signedDistance);

        if (overlap <= 0.0f)
            return false;

        if (overlap < smallestOverlap) {
            smallestOverlap = overlap;
            smallestAxis = signedDistance >= 0.0f ? axis : -axis;
        }
    }

    normal = smallestAxis;
    penetration = smallestOverlap;
    return true;
}

// Build an OBB from the model's transform origin, its local-space
// bounds center/half extents, and its current rotation matrix.
inline OrientedBox makeModelHitbox(
    const glm::vec3& transformOrigin,
    const glm::vec3& localCenter,
    const glm::vec3& localHalf,
    const glm::mat3& rotation
) {
    glm::vec3 worldCenter = transformOrigin + rotation * localCenter;
    return makeOrientedBox(worldCenter, localHalf, rotation);
}

// Resolve a moving oriented hitbox against a fixed world box.
// transformOrigin is adjusted, preserving rotation around the model origin.
inline bool resolveOrientedBoxStatic(
    glm::vec3& transformOrigin,
    const glm::vec3& localCenter,
    const glm::vec3& localHalf,
    const glm::mat3& rotation,
    glm::vec3& velocity,
    const OrientedBox& fixedBox,
    float restitution = 0.28f
) {
    OrientedBox moving = makeModelHitbox(
        transformOrigin, localCenter, localHalf, rotation
    );

    glm::vec3 normal;
    float penetration = 0.0f;

    if (!orientedBoxOverlap(moving, fixedBox, normal, penetration))
        return false;

    transformOrigin -= normal * (penetration + 0.001f);

    float intoSurface = glm::dot(velocity, normal);
    if (intoSurface > 0.0f) {
        velocity -= normal * ((1.0f + restitution) * intoSurface);
    }

    return true;
}

// Resolve two moving oriented hitboxes. Equal-mass impulse response.
inline bool resolveOrientedBoxPair(
    glm::vec3& originA,
    const glm::vec3& localCenterA,
    const glm::vec3& halfA,
    const glm::mat3& rotationA,
    glm::vec3& velocityA,
    glm::vec3& originB,
    const glm::vec3& localCenterB,
    const glm::vec3& halfB,
    const glm::mat3& rotationB,
    glm::vec3& velocityB,
    float restitution = 0.22f
) {
    OrientedBox a = makeModelHitbox(
        originA, localCenterA, halfA, rotationA
    );
    OrientedBox b = makeModelHitbox(
        originB, localCenterB, halfB, rotationB
    );

    glm::vec3 normal;
    float penetration = 0.0f;

    if (!orientedBoxOverlap(a, b, normal, penetration))
        return false;

    const float correction = (penetration + 0.001f) * 0.5f;
    originA -= normal * correction;
    originB += normal * correction;

    glm::vec3 relativeVelocity = velocityB - velocityA;
    float closingSpeed = glm::dot(relativeVelocity, normal);

    if (closingSpeed < 0.0f) {
        float impulse = -(1.0f + restitution) * closingSpeed * 0.5f;
        velocityA -= normal * impulse;
        velocityB += normal * impulse;
    }

    return true;
}

// Ground collision for an oriented box. Returns true when touching ground.
inline bool resolveOrientedBoxGround(
    glm::vec3& transformOrigin,
    const glm::vec3& localCenter,
    const glm::vec3& localHalf,
    const glm::mat3& rotation,
    glm::vec3& velocity,
    float groundLevel,
    float restitution
) {
    OrientedBox box = makeModelHitbox(
        transformOrigin, localCenter, localHalf, rotation
    );

    // Project the oriented box onto world Y.
    float verticalRadius =
        box.half.x * std::abs(box.rotation[0].y) +
        box.half.y * std::abs(box.rotation[1].y) +
        box.half.z * std::abs(box.rotation[2].y);

    float bottom = box.center.y - verticalRadius;

    if (bottom >= groundLevel)
        return false;

    transformOrigin.y += groundLevel - bottom;

    if (velocity.y < 0.0f)
        velocity.y = -velocity.y * restitution;

    return true;
}
