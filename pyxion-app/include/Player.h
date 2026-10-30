#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>

struct PieceState {
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    glm::vec3 rotAxis{0.0f, 1.0f, 0.0f};
    float rotAngle = 0.0f;
    float rotSpeed = 0.0f;
    float drag     = 0.85f;
};

struct BreakEffect {
    std::vector<PieceState> pieces;
    bool active = false;

    void trigger(int pieceCount, glm::vec3 origin) {
        active = true;
        pieces.clear();
        pieces.resize(pieceCount);
        srand(static_cast<unsigned>(time(0)));

        for (auto& p : pieces) {
            p.position = origin;
            float angle  = (rand() % 360) * (3.14159265f / 180.0f);
            float hSpeed = 3.0f + (rand() % 400) / 100.0f;
            float vSpeed = 4.0f + (rand() % 600) / 100.0f;
            p.velocity   = glm::vec3(std::cos(angle) * hSpeed, vSpeed,
                                     std::sin(angle) * hSpeed);
            p.rotAxis    = glm::normalize(glm::vec3(
                (rand() % 200 - 100) / 100.0f,
                (rand() % 200 - 100) / 100.0f,
                (rand() % 200 - 100) / 100.0f));
            p.rotSpeed   = 3.0f + (rand() % 500) / 100.0f;
            p.drag       = 0.6f + (rand() % 30)  / 100.0f;
        }
    }

    void update(float dt) {
        if (!active) return;
        const float gravity = -18.0f;
        for (auto& p : pieces) {
            p.velocity.y += gravity * dt;
            p.velocity   *= std::pow(p.drag, dt);
            p.position   += p.velocity * dt;
            p.rotAngle   += p.rotSpeed * dt;
            if (p.position.y < 0.0f) {
                p.position.y = 0.0f;
                p.velocity.y = 0.0f;
                p.velocity  *= 0.4f;
                p.rotSpeed  *= 0.5f;
            }
        }
    }
};