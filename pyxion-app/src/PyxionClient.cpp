#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#include "Camera.h"
#include "Collision.h"
#include "Mesh.h"
#include "Player.h"
#include "Shader.h"

struct AppState {
    ThirdPersonCamera camera;
    bool rightDragging = false;
    double lastX = 0.0;
    double lastY = 0.0;
};

static void scrollCallback(GLFWwindow* window, double, double yoffset) {
    static_cast<AppState*>(glfwGetWindowUserPointer(window))
        ->camera.zoom(static_cast<float>(yoffset));
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    auto* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        state->rightDragging = (action == GLFW_PRESS);
        if (state->rightDragging)
            glfwGetCursorPos(window, &state->lastX, &state->lastY);
    }
}

static void cursorPosCallback(GLFWwindow* window, double x, double y) {
    auto* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    float dx = static_cast<float>(x - state->lastX);
    float dy = static_cast<float>(y - state->lastY);
    state->lastX = x; state->lastY = y;
    if (!state->rightDragging) return;
    state->camera.orbit(-dx * 0.005f, dy * 0.005f);
}

static void framebufferSizeCallback(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
}

int main() {
    std::cout << "Starting Pyxion Engine Render Pipeline..." << std::endl;

    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Pyxion", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    AppState state;
    glfwSetWindowUserPointer(window, &state);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr << "GLEW init failed\n"; return -1; }

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.4f, 0.6f, 0.9f, 1.0f);

    // ---- Shaders ----
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);
    if (!checkShaderCompile(vs, "Vertex") || !checkShaderCompile(fs, "Fragment"))
        return -1;
    unsigned int program = glCreateProgram();
    glAttachShader(program, vs); glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs); glDeleteShader(fs);
    if (!checkProgramLink(program)) return -1;

    GLint modelLoc    = glGetUniformLocation(program, "model");
    GLint viewLoc     = glGetUniformLocation(program, "view");
    GLint projLoc     = glGetUniformLocation(program, "projection");
    GLint colorLoc    = glGetUniformLocation(program, "color");
    GLint lightDirLoc = glGetUniformLocation(program, "lightDir");
    GLint camPosLoc   = glGetUniformLocation(program, "camPos");

    // ---- Assets ----
    std::vector<ModelMesh> playerPieces = loadOBJ("assets/player.obj");
    if (playerPieces.empty()) return -1;

    Mesh groundMesh = createGround(25.0f);
    Mesh blockMesh  = createCube();

    // ---- World ----
    std::vector<ColBox> worldBoxes = {
        { glm::vec3( 5.0f, 1.0f,  5.0f), glm::vec3(0.5f, 1.0f, 0.5f), glm::vec3(0.6f, 0.4f, 0.3f) },
        { glm::vec3(-4.0f, 0.5f,  3.0f), glm::vec3(1.5f, 0.5f, 1.5f), glm::vec3(0.5f, 0.3f, 0.6f) },
        { glm::vec3( 2.0f, 1.5f, -5.0f), glm::vec3(1.0f, 1.5f, 1.0f), glm::vec3(0.3f, 0.6f, 0.4f) },
    };

    // ---- Player state ----
    glm::vec3 playerPos(0.0f);
    float playerYaw   = 0.0f;
    float targetYaw   = 0.0f;
    float verticalVel = 0.0f;
    bool  onGround    = true;
    bool  playerAlive = true;
    const float moveSpeed   = 5.0f;
    const float gravity     = -24.0f;
    const float jumpForce   = 10.0f;
    const float groundLevel = 0.0f;
    const float turnSpeed   = 20.0f;
    const glm::vec3 playerHalf(0.4f, 1.0f, 0.4f);
    const glm::vec3 lightDir = glm::normalize(glm::vec3(0.6f, 1.0f, 0.4f));

    const std::vector<glm::vec3> pieceColors = {
        glm::vec3(0.1f, 0.5f, 0.8f),
        glm::vec3(0.8f, 0.2f, 0.2f),
        glm::vec3(0.2f, 0.8f, 0.2f),
        glm::vec3(0.8f, 0.8f, 0.2f),
        glm::vec3(0.8f, 0.4f, 0.1f),
        glm::vec3(0.6f, 0.2f, 0.8f),
    };

    BreakEffect breakEffect;
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = static_cast<float>(now - lastTime);
        lastTime = now;

        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (playerAlive) {
            glm::vec3 forward = glm::normalize(glm::vec3(
                -std::sin(state.camera.yaw), 0.0f, -std::cos(state.camera.yaw)));
            glm::vec3 right = glm::normalize(
                glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

            glm::vec3 move(0.0f);
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += forward;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= forward;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += right;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= right;

            if (glm::length(move) > 0.0f) {
                move = glm::normalize(move);
                playerPos += move * moveSpeed * dt;
                targetYaw = std::atan2(move.x, move.z);
            }

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && onGround) {
                verticalVel = jumpForce;
                onGround    = false;
            }

            if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
                playerAlive = false;
                breakEffect.trigger(static_cast<int>(playerPieces.size()),
                                    playerPos + glm::vec3(0.0f, 3.0f, 0.0f));
            }

            verticalVel += gravity * dt;
            playerPos.y += verticalVel * dt;
            onGround = false;
            resolvePlayer(playerPos, verticalVel, onGround,
                          worldBoxes, groundLevel, playerHalf);

            float yawDiff = targetYaw - playerYaw;
            while (yawDiff >  static_cast<float>(M_PI)) yawDiff -= 2.0f * static_cast<float>(M_PI);
            while (yawDiff < -static_cast<float>(M_PI)) yawDiff += 2.0f * static_cast<float>(M_PI);
            playerYaw += yawDiff * std::min(turnSpeed * dt, 1.0f);

            state.camera.target = playerPos;
        } else {
            breakEffect.update(dt);

            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                playerAlive = true;
                breakEffect.active = false;
                playerPos   = glm::vec3(0.0f);
                verticalVel = 0.0f;
                onGround    = true;
            }
        }

        // ---- Render ----
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        float aspect = h > 0 ? static_cast<float>(w) / h : 1.0f;

        glm::mat4 view       = state.camera.view();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 200.0f);

        glUseProgram(program);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
        glUniform3fv(camPosLoc,   1, glm::value_ptr(state.camera.position()));

        // Ground
        drawMesh(groundMesh, modelLoc, colorLoc,
                 glm::mat4(1.0f), glm::vec3(0.45f, 0.6f, 0.45f));

        // World boxes
        for (auto& box : worldBoxes) {
            glm::mat4 mat = glm::translate(glm::mat4(1.0f), box.center);
            mat = glm::scale(mat, box.half * 2.0f);
            drawMesh(blockMesh, modelLoc, colorLoc, mat, box.color);
        }

        // Player
        if (playerAlive) {
            glm::mat4 playerMat = glm::translate(
                glm::mat4(1.0f), playerPos + glm::vec3(0.0f, 3.0f, 0.0f));
            playerMat = glm::rotate(playerMat, playerYaw,
                                    glm::vec3(0.0f, 1.0f, 0.0f));
            playerMat = glm::scale(playerMat, glm::vec3(1.0f));
            for (size_t i = 0; i < playerPieces.size(); i++) {
                glm::vec3 col = i < pieceColors.size() ? pieceColors[i] : glm::vec3(1.0f);
                drawModel(playerPieces[i], modelLoc, colorLoc, playerMat, col);
            }
        } else if (breakEffect.active) {
            for (size_t i = 0; i < playerPieces.size(); i++) {
                if (i >= breakEffect.pieces.size()) break;
                const PieceState& ps = breakEffect.pieces[i];
                glm::mat4 mat = glm::translate(glm::mat4(1.0f), ps.position);
                mat = glm::rotate(mat, ps.rotAngle, ps.rotAxis);
                glm::vec3 col = i < pieceColors.size() ? pieceColors[i] : glm::vec3(1.0f);
                drawModel(playerPieces[i], modelLoc, colorLoc, mat, col);
            }
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    for (auto& p : playerPieces) {
        glDeleteVertexArrays(1, &p.vao);
        glDeleteBuffers(1, &p.vbo);
    }
    glDeleteVertexArrays(1, &groundMesh.vao);
    glDeleteBuffers(1, &groundMesh.vbo);
    glDeleteBuffers(1, &groundMesh.ebo);
    glDeleteVertexArrays(1, &blockMesh.vao);
    glDeleteBuffers(1, &blockMesh.vbo);
    glDeleteBuffers(1, &blockMesh.ebo);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}