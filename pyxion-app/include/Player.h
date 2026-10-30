#pragma once

#include "Collision.h"
#include "Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

struct PieceState {
    // This is the model transform origin, not the hitbox center.
    // Rotation is applied around this origin, matching the render transform.
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};

    glm::vec3 rotAxis{0.0f, 1.0f, 0.0f};
    float rotAngle = 0.0f;
    float rotSpeed = 0.0f;
    glm::mat3 baseRotation{1.0f};

    // Automatically copied from ModelMesh's OBJ vertex bounds.
    glm::vec3 localCenter{0.0f};
    glm::vec3 halfExtents{0.25f};

    float drag = 0.94f;
    float restitution = 0.30f;

    bool grounded = false;
    bool sleeping = false;
};

struct BreakEffect {
    std::vector<PieceState> pieces;
    bool active = false;

    std::mt19937 rng{std::random_device{}()};

    float randomFloat(float minimum, float maximum) {
        return std::uniform_real_distribution<float>(
            minimum, maximum
        )(rng);
    }

    glm::vec3 randomUnitVector() {
        glm::vec3 v(
            randomFloat(-1.0f, 1.0f),
            randomFloat(-1.0f, 1.0f),
            randomFloat(-1.0f, 1.0f)
        );

        float length2 = glm::dot(v, v);
        if (length2 < 0.0001f)
            return glm::vec3(0.0f, 1.0f, 0.0f);

        return v / std::sqrt(length2);
    }

    // Preferred overload: hitboxes are calculated from each OBJ mesh.
    // origin is the shared transform origin used by the rendered player.
    void trigger(
        const std::vector<ModelMesh>& models,
        const glm::vec3& origin,
        float initialYaw = 0.0f,
        const glm::vec3& inheritedVelocity = glm::vec3(0.0f)
    ) {
        pieces.clear();
        pieces.reserve(models.size());
        active = !models.empty();

        // Preserve the player's orientation at the moment of breaking.
        glm::mat3 playerRotation(
            glm::rotate(
                glm::mat4(1.0f),
                initialYaw,
                glm::vec3(0.0f, 1.0f, 0.0f)
            )
        );

        for (const ModelMesh& model : models) {
            PieceState p;

            p.baseRotation = playerRotation;

            // Position the fragment's individual pivot in world space.
            p.position = origin + playerRotation * model.pivot;
            p.velocity = inheritedVelocity;

            // Geometry and bounds are now centered around the pivot.
            p.localCenter = glm::vec3(0.0f);

            if (model.hasBounds) {
                p.halfExtents = glm::max(
                    (model.boundsMax - model.boundsMin) * 0.5f,
                    glm::vec3(0.025f)
                );
            }
            else {
                p.halfExtents = glm::vec3(0.25f);
            }

            glm::vec3 outward = randomUnitVector();

            outward.y = std::abs(outward.y) * 0.75f + 0.20f;
            outward = glm::normalize(outward);

            float blastSpeed = randomFloat(2.2f, 6.0f);
            p.velocity += outward * blastSpeed;

            p.rotAxis = randomUnitVector();

            // Start at the original orientation.
            // Rotation builds naturally as the fragment spins.
            p.rotAngle = 0.0f;
            p.rotSpeed = randomFloat(-10.0f, 10.0f);

            p.drag = randomFloat(0.90f, 0.975f);
            p.restitution = randomFloat(0.18f, 0.42f);

            p.grounded = false;
            p.sleeping = false;

            pieces.push_back(p);
        }
    }

    // Compatibility overload for older calls. This uses fallback hitboxes,
    // so use trigger(playerPieces, origin, velocity) for automatic bounds.
    void trigger(int pieceCount, glm::vec3 origin) {
        pieces.clear();
        pieces.resize(std::max(0, pieceCount));
        active = !pieces.empty();

        for (auto& p : pieces) {
            p.position = origin;
            p.localCenter = glm::vec3(0.0f);
            p.halfExtents = glm::vec3(0.25f);

            glm::vec3 outward = randomUnitVector();
            outward.y = std::abs(outward.y) * 0.75f + 0.20f;
            outward = glm::normalize(outward);

            p.velocity = outward * randomFloat(2.2f, 6.0f);
            p.rotAxis = randomUnitVector();
            p.rotAngle = randomFloat(-3.14159265f, 3.14159265f);
            p.rotSpeed = randomFloat(-10.0f, 10.0f);
            p.drag = randomFloat(0.90f, 0.975f);
            p.restitution = randomFloat(0.18f, 0.42f);
            p.grounded = false;
            p.sleeping = false;
        }
    }

    glm::mat3 rotationMatrix(const PieceState& p) const {
        glm::vec3 axis = p.rotAxis;
        float axisLength2 = glm::dot(axis, axis);

        if (axisLength2 < 0.0001f)
            axis = glm::vec3(0.0f, 1.0f, 0.0f);
        else
            axis *= 1.0f / std::sqrt(axisLength2);

        glm::mat3 spinRotation(
            glm::rotate(
                glm::mat4(1.0f),
                p.rotAngle,
                axis
            )
        );

        return spinRotation * p.baseRotation;
    }

    void update(
        float dt,
        const std::vector<ColBox>& worldBoxes,
        float groundLevel = 0.0f
    ) {
        if (!active)
            return;

        // Clamp large frame times to keep the simulation stable.
        dt = std::min(dt, 0.033f);

        constexpr float gravity = -18.0f;

        // Integrate each piece and resolve static geometry.
        for (PieceState& p : pieces) {
            if (p.sleeping)
                continue;

            p.velocity.y += gravity * dt;
            p.velocity *= std::pow(p.drag, dt);
            p.position += p.velocity * dt;
            p.rotAngle += p.rotSpeed * dt;

            glm::mat3 rotation = rotationMatrix(p);

            bool touchedGround = resolveOrientedBoxGround(
                p.position,
                p.localCenter,
                p.halfExtents,
                rotation,
                p.velocity,
                groundLevel,
                p.restitution
            );

            p.grounded = touchedGround;

            // Ground friction and angular damping.
            if (touchedGround) {
                float friction = std::pow(0.68f, dt);
                p.velocity.x *= friction;
                p.velocity.z *= friction;
                p.rotSpeed *= std::pow(0.20f, dt);

                if (std::abs(p.velocity.y) < 0.45f)
                    p.velocity.y = 0.0f;
            }

            // Collide with static world boxes.
            for (const ColBox& worldBox : worldBoxes) {
                OrientedBox fixed = makeAxisAlignedBox(worldBox);

                bool hit = resolveOrientedBoxStatic(
                    p.position,
                    p.localCenter,
                    p.halfExtents,
                    rotation,
                    p.velocity,
                    fixed,
                    0.26f
                );

                if (hit) {
                    p.grounded = p.grounded || false;
                    p.velocity *= 0.88f;
                    p.rotSpeed *= 0.92f;
                }
            }
        }

        // Fragment-to-fragment collisions.
        // This is O(n^2), suitable for modest player-piece counts.
        for (size_t i = 0; i < pieces.size(); ++i) {
            for (size_t j = i + 1; j < pieces.size(); ++j) {
                PieceState& a = pieces[i];
                PieceState& b = pieces[j];

                if (a.sleeping && b.sleeping)
                    continue;

                glm::mat3 rotationA = rotationMatrix(a);
                glm::mat3 rotationB = rotationMatrix(b);

                bool hit = resolveOrientedBoxPair(
                    a.position, a.localCenter, a.halfExtents,
                    rotationA, a.velocity,
                    b.position, b.localCenter, b.halfExtents,
                    rotationB, b.velocity,
                    0.20f
                );

                if (hit) {
                    if (a.sleeping) {
                        a.sleeping = false;
                        a.velocity.y += 0.35f;
                    }
                    if (b.sleeping) {
                        b.sleeping = false;
                        b.velocity.y += 0.35f;
                    }

                    a.rotSpeed *= 0.96f;
                    b.rotSpeed *= 0.96f;
                }
            }
        }

        // Put settled pieces to sleep to save CPU time.
        for (PieceState& p : pieces) {
            if (p.sleeping)
                continue;

            if (p.grounded &&
                glm::length(p.velocity) < 0.28f &&
                std::abs(p.rotSpeed) < 0.28f) {

                p.velocity = glm::vec3(0.0f);
                p.rotSpeed = 0.0f;
                p.sleeping = true;
            }
        }

        // Deactivate once every piece has settled.
        bool anyAwake = false;
        for (const PieceState& p : pieces) {
            if (!p.sleeping) {
                anyAwake = true;
                break;
            }
        }

        if (!anyAwake)
            active = false;
    }
};
