#define _USE_MATH_DEFINES

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "Camera.h"
#include "Collision.h"
#include "Mesh.h"
#include "Player.h"
#include "Shader.h"

// ============================================================
// WINDOW / INPUT STATE
// ============================================================

struct AppState
{
    ThirdPersonCamera camera;

    bool rightDragging = false;
    double lastX = 0.0;
    double lastY = 0.0;
};

static void scrollCallback(GLFWwindow* window, double, double yoffset)
{
    auto* state = static_cast<AppState*>(
        glfwGetWindowUserPointer(window)
    );

    if (state)
        state->camera.zoom(static_cast<float>(yoffset));
}

static void mouseButtonCallback(
    GLFWwindow* window,
    int button,
    int action,
    int)
{
    auto* state = static_cast<AppState*>(
        glfwGetWindowUserPointer(window)
    );

    if (!state)
        return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        state->rightDragging = (action == GLFW_PRESS);

        if (state->rightDragging)
            glfwGetCursorPos(window, &state->lastX, &state->lastY);
    }
}

static void cursorPosCallback(GLFWwindow* window, double x, double y)
{
    auto* state = static_cast<AppState*>(
        glfwGetWindowUserPointer(window)
    );

    if (!state)
        return;

    float dx = static_cast<float>(x - state->lastX);
    float dy = static_cast<float>(y - state->lastY);

    state->lastX = x;
    state->lastY = y;

    if (!state->rightDragging)
        return;

    state->camera.orbit(-dx * 0.005f, dy * 0.005f);
}

static void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

// ============================================================
// SHADER PROGRAM CREATION
// ============================================================

static GLuint createShaderProgram(
    const char* vertexSource,
    const char* fragmentSource,
    const char* programName)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    if (!checkShaderCompile(vertexShader, "Vertex"))
    {
        glDeleteShader(vertexShader);
        return 0;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    if (!checkShaderCompile(fragmentShader, "Fragment"))
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if (!checkProgramLink(program))
    {
        std::cerr << programName << " program failed to link.\n";
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

// ============================================================
// MAIN
// ============================================================

int main()
{
    std::cout << "Starting Pyxion Engine Render Pipeline...\n";

    // --------------------------------------------------------
    // GLFW
    // --------------------------------------------------------

    if (!glfwInit())
    {
        std::cerr << "GLFW initialization failed.\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 720;

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "Pyxion",
        nullptr,
        nullptr
    );

    if (!window)
    {
        std::cerr << "Window creation failed.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // --------------------------------------------------------
    // GLEW MUST BE INITIALIZED BEFORE ANY OTHER GL OBJECTS
    // --------------------------------------------------------

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW initialization failed.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // GLEW can leave an OpenGL error behind on some drivers.
    glGetError();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearColor(0.40f, 0.60f, 0.90f, 1.0f);

    // --------------------------------------------------------
    // INPUT CALLBACKS
    // --------------------------------------------------------

    AppState state;

    glfwSetWindowUserPointer(window, &state);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    // --------------------------------------------------------
    // SHADOW MAP
    // Shader.h must contain ShadowMap and both shadow shaders.
    // --------------------------------------------------------

    ShadowMap shadows;

    if (!shadows.init())
    {
        std::cerr << "Shadow map initialization failed.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // --------------------------------------------------------
    // MAIN AND SHADOW SHADER PROGRAMS
    // --------------------------------------------------------

    GLuint program = createShaderProgram(
        vertexShaderSource,
        fragmentShaderSource,
        "Main"
    );

    if (!program)
    {
        shadows.cleanup();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    GLuint shadowProgram = createShaderProgram(
        shadowVertexShaderSource,
        shadowFragmentShaderSource,
        "Shadow"
    );

    if (!shadowProgram)
    {
        glDeleteProgram(program);
        shadows.cleanup();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Main shader uniform locations.
    const GLint modelLoc =
        glGetUniformLocation(program, "model");

    const GLint viewLoc =
        glGetUniformLocation(program, "view");

    const GLint projLoc =
        glGetUniformLocation(program, "projection");

    const GLint colorLoc =
        glGetUniformLocation(program, "color");

    const GLint lightDirLoc =
        glGetUniformLocation(program, "lightDir");

    const GLint camPosLoc =
        glGetUniformLocation(program, "camPos");

    const GLint lightSpaceLoc =
        glGetUniformLocation(program, "lightSpaceMatrix");

    const GLint shadowMapLoc =
        glGetUniformLocation(program, "shadowMap");

    const GLint shadowStrengthLoc =
        glGetUniformLocation(program, "shadowStrength");

    // Shadow shader uniform locations.
    const GLint shadowModelLoc =
        glGetUniformLocation(shadowProgram, "model");

    const GLint shadowLightSpaceLoc =
        glGetUniformLocation(shadowProgram, "lightSpaceMatrix");

    // --------------------------------------------------------
    // ASSETS
    // --------------------------------------------------------

    std::vector<ModelMesh> playerPieces = loadOBJ("assets/player.obj");

    if (playerPieces.empty())
    {
        std::cerr << "Could not load assets/player.obj\n";

        glDeleteProgram(shadowProgram);
        glDeleteProgram(program);
        shadows.cleanup();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    Mesh groundMesh = createGround(25.0f);
    Mesh blockMesh = createCube();

    // --------------------------------------------------------
    // WORLD
    // --------------------------------------------------------

    std::vector<ColBox> worldBoxes =
    {
        {
            glm::vec3( 5.0f, 1.0f,  5.0f),
            glm::vec3(0.5f, 1.0f, 0.5f),
            glm::vec3(0.6f, 0.4f, 0.3f)
        },
        {
            glm::vec3(-4.0f, 0.5f,  3.0f),
            glm::vec3(1.5f, 0.5f, 1.5f),
            glm::vec3(0.5f, 0.3f, 0.6f)
        },
        {
            glm::vec3( 2.0f, 1.5f, -5.0f),
            glm::vec3(1.0f, 1.5f, 1.0f),
            glm::vec3(0.3f, 0.6f, 0.4f)
        }
    };

    // --------------------------------------------------------
    // PLAYER STATE
    // --------------------------------------------------------

    glm::vec3 playerPos(0.0f);

    float playerYaw = 0.0f;
    float targetYaw = 0.0f;
    float verticalVel = 0.0f;

    bool onGround = true;
    bool playerAlive = true;

    const float moveSpeed = 5.0f;
    const float gravity = -24.0f;
    const float jumpForce = 10.0f;
    const float groundLevel = 0.0f;
    const float turnSpeed = 20.0f;

    const glm::vec3 playerHalf(0.4f, 1.0f, 0.4f);

    // Direction from the surface toward the sun.
    const glm::vec3 lightDir =
        glm::normalize(glm::vec3(0.6f, 1.0f, 0.4f));

    const std::vector<glm::vec3> pieceColors =
    {
        glm::vec3(0.1f, 0.5f, 0.8f),
        glm::vec3(0.8f, 0.2f, 0.2f),
        glm::vec3(0.2f, 0.8f, 0.2f),
        glm::vec3(0.8f, 0.8f, 0.2f),
        glm::vec3(0.8f, 0.4f, 0.1f),
        glm::vec3(0.6f, 0.2f, 0.8f)
    };

    BreakEffect breakEffect;

    // --------------------------------------------------------
    // DIRECTIONAL LIGHT CAMERA
    // --------------------------------------------------------

    const glm::mat4 lightProjection = glm::ortho(
        -30.0f, 30.0f,
        -30.0f, 30.0f,
        1.0f, 80.0f
    );

    // The light camera looks from the sun toward the world origin.
    const glm::mat4 lightView = glm::lookAt(
        lightDir * 30.0f,
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    const glm::mat4 lightSpaceMatrix =
        lightProjection * lightView;

    // --------------------------------------------------------
    // SCENE DRAW FUNCTION
    //
    // Pass the uniform locations belonging to the active program.
    // For the depth-only shader, colorLoc is -1. OpenGL ignores
    // uniform updates to location -1.
    // --------------------------------------------------------

    auto renderScene = [&](GLint activeModelLoc, GLint activeColorLoc)
    {
        // Ground.
        drawMesh(
            groundMesh,
            activeModelLoc,
            activeColorLoc,
            glm::mat4(1.0f),
            glm::vec3(0.45f, 0.60f, 0.45f)
        );

        // Static world objects.
        for (const auto& box : worldBoxes)
        {
            glm::mat4 model = glm::translate(
                glm::mat4(1.0f),
                box.center
            );

            model = glm::scale(model, box.half * 2.0f);

            drawMesh(
                blockMesh,
                activeModelLoc,
                activeColorLoc,
                model,
                box.color
            );
        }

        // Player or broken player pieces.
        if (playerAlive)
        {
            glm::mat4 playerModel = glm::translate(
                glm::mat4(1.0f),
                playerPos + glm::vec3(0.0f, 3.0f, 0.0f)
            );

            playerModel = glm::rotate(
                playerModel,
                playerYaw,
                glm::vec3(0.0f, 1.0f, 0.0f)
            );

            for (size_t i = 0; i < playerPieces.size(); ++i)
            {
                const glm::vec3 pieceColor =
                    i < pieceColors.size()
                    ? pieceColors[i]
                    : glm::vec3(1.0f);

                    glm::mat4 pieceModel = glm::translate(
                        playerModel,
                        playerPieces[i].pivot
                    );

                    drawModel(
                        playerPieces[i],
                        activeModelLoc,
                        activeColorLoc,
                        pieceModel,
                        pieceColor
                    );
            }
        }
        else if (breakEffect.active)
        {
            for (size_t i = 0; i < playerPieces.size(); ++i)
            {
                if (i >= breakEffect.pieces.size())
                    break;

                const PieceState& piece = breakEffect.pieces[i];

                glm::mat4 model = glm::translate(
                    glm::mat4(1.0f),
                    piece.position
                );

                model *= glm::mat4(
                    breakEffect.rotationMatrix(piece)
                );

                const glm::vec3 pieceColor =
                    i < pieceColors.size()
                    ? pieceColors[i]
                    : glm::vec3(1.0f);

                drawModel(
                    playerPieces[i],
                    activeModelLoc,
                    activeColorLoc,
                    model,
                    pieceColor
                );
            }
        }
    };

    // --------------------------------------------------------
    // MAIN LOOP
    // --------------------------------------------------------

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        const double now = glfwGetTime();

        float dt = static_cast<float>(now - lastTime);
        lastTime = now;

        // Avoid huge physics steps after window dragging or pauses.
        dt = std::min(dt, 0.05f);

        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // ----------------------------------------------------
        // PLAYER MOVEMENT / PHYSICS
        // ----------------------------------------------------

        if (playerAlive)
        {
            glm::vec3 forward = glm::normalize(glm::vec3(
                -std::sin(state.camera.yaw),
                0.0f,
                -std::cos(state.camera.yaw)
            ));

            glm::vec3 right = glm::normalize(
                glm::cross(
                    forward,
                    glm::vec3(0.0f, 1.0f, 0.0f)
                )
            );

            glm::vec3 move(0.0f);

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                move += forward;

            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                move -= forward;

            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                move += right;

            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                move -= right;

            if (glm::length(move) > 0.0f)
            {
                move = glm::normalize(move);
                playerPos += move * moveSpeed * dt;
                targetYaw = std::atan2(move.x, move.z);
            }

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && onGround)
            {
                verticalVel = jumpForce;
                onGround = false;
            }

            if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
            {
                playerAlive = false;

                breakEffect.trigger(
                    playerPieces,
                    playerPos + glm::vec3(0.0f, 3.0f, 0.0f),
                    playerYaw
                );
            }

            verticalVel += gravity * dt;
            playerPos.y += verticalVel * dt;

            onGround = false;

            resolvePlayer(
                playerPos,
                verticalVel,
                onGround,
                worldBoxes,
                groundLevel,
                playerHalf
            );

            // Smooth player turning.
            float yawDiff = targetYaw - playerYaw;

            constexpr float PI = 3.14159265358979323846f;

            while (yawDiff > PI)
                yawDiff -= 2.0f * PI;

            while (yawDiff < -PI)
                yawDiff += 2.0f * PI;

            playerYaw += yawDiff * std::min(turnSpeed * dt, 1.0f);

            state.camera.target = playerPos;
        }
        else
        {
            breakEffect.update(dt, worldBoxes, groundLevel);

            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            {
                playerAlive = true;
                breakEffect.active = false;
                playerPos = glm::vec3(0.0f);
                verticalVel = 0.0f;
                onGround = true;
            }
        }

        // ----------------------------------------------------
        // FRAME SIZE / CAMERA MATRICES
        // ----------------------------------------------------

        int screenWidth = 0;
        int screenHeight = 0;

        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

        if (screenWidth <= 0 || screenHeight <= 0)
        {
            glfwSwapBuffers(window);
            continue;
        }

        const float aspect =
            static_cast<float>(screenWidth) /
            static_cast<float>(screenHeight);

        const glm::mat4 view = state.camera.view();

        const glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            aspect,
            0.1f,
            200.0f
        );

        // ====================================================
        // PASS 1: RENDER DEPTH FROM THE LIGHT'S PERSPECTIVE
        // ====================================================

        shadows.begin();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 4.0f);

        glUseProgram(shadowProgram);

        glUniformMatrix4fv(
            shadowLightSpaceLoc,
            1,
            GL_FALSE,
            glm::value_ptr(lightSpaceMatrix)
        );

        renderScene(shadowModelLoc, -1);

        glDisable(GL_POLYGON_OFFSET_FILL);

        shadows.end(screenWidth, screenHeight);

        // ====================================================
        // PASS 2: NORMAL CAMERA RENDER WITH SHADOW SAMPLING
        // ====================================================

        glClearColor(0.40f, 0.60f, 0.90f, 1.0f);

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        glUseProgram(program);

        glUniformMatrix4fv(
            viewLoc,
            1,
            GL_FALSE,
            glm::value_ptr(view)
        );

        glUniformMatrix4fv(
            projLoc,
            1,
            GL_FALSE,
            glm::value_ptr(projection)
        );

        glUniformMatrix4fv(
            lightSpaceLoc,
            1,
            GL_FALSE,
            glm::value_ptr(lightSpaceMatrix)
        );

        glUniform3fv(
            lightDirLoc,
            1,
            glm::value_ptr(lightDir)
        );

        const glm::vec3 cameraPosition = state.camera.position();

        glUniform3fv(
            camPosLoc,
            1,
            glm::value_ptr(cameraPosition)
        );

        glUniform1i(shadowMapLoc, 1);
        glUniform1f(shadowStrengthLoc, 0.75f);

        shadows.bind(1);

        renderScene(modelLoc, colorLoc);

        // ----------------------------------------------------
        // PRESENT FRAME
        // ----------------------------------------------------

        glfwSwapBuffers(window);
    }

    // ========================================================
    // CLEANUP
    // ========================================================

    shadows.cleanup();

    for (const auto& piece : playerPieces)
    {
        glDeleteVertexArrays(1, &piece.vao);
        glDeleteBuffers(1, &piece.vbo);
    }

    glDeleteVertexArrays(1, &groundMesh.vao);
    glDeleteBuffers(1, &groundMesh.vbo);
    glDeleteBuffers(1, &groundMesh.ebo);

    glDeleteVertexArrays(1, &blockMesh.vao);
    glDeleteBuffers(1, &blockMesh.vbo);
    glDeleteBuffers(1, &blockMesh.ebo);

    glDeleteProgram(shadowProgram);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Pyxion shut down cleanly.\n";

    return 0;
}
