#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

// ==========================================
// THIRD-PERSON CAMERA
// ==========================================
struct ThirdPersonCamera {
    glm::vec3 target{0.0f};     // the point the camera follows (the player)
    float yaw = 0.0f;           // radians, rotation around the Y axis
    float pitch = 0.35f;        // radians, angle above the horizon
    float distance = 8.0f;      // distance from the target (zoom)

    float minDistance = 2.0f;
    float maxDistance = 25.0f;
    float minPitch = 0.05f;     // stays above ground
    float maxPitch = 1.40f;     // just under straight down
    float targetHeight = 1.0f;  // look at the player's upper body, not feet

    glm::vec3 position() const {
        glm::vec3 offset(
            distance * std::cos(pitch) * std::sin(yaw),
            distance * std::sin(pitch),
            distance * std::cos(pitch) * std::cos(yaw));
        return target + offset;
    }

    glm::mat4 view() const {
        glm::vec3 lookAt = target + glm::vec3(0.0f, targetHeight, 0.0f);
        return glm::lookAt(position(), lookAt, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void zoom(float amount) {
        distance = std::clamp(distance - amount, minDistance, maxDistance);
    }

    void orbit(float dYaw, float dPitch) {
        yaw += dYaw;
        pitch = std::clamp(pitch + dPitch, minPitch, maxPitch);
    }
};

// ==========================================
// MODEL LOADER (for my player)
// ==========================================

struct ModelMesh {
    unsigned int vao = 0, vbo = 0;
    int vertexCount = 0;
};

ModelMesh loadOBJ(const char* path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // second arg is the base dir for .mtl files
    std::string baseDir = std::string(path).substr(0, std::string(path).find_last_of("/\\") + 1);
    bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                               path, baseDir.c_str());

    if (!warn.empty()) std::cout << "OBJ warn: " << warn << std::endl;
    if (!err.empty())  std::cerr << "OBJ err: "  << err  << std::endl;
    if (!ok) { std::cerr << "Failed to load " << path << std::endl; return {}; }

    // Unpack into a flat array: x y z nx ny nz per vertex
    // (interleaved, even if the .obj has no normals — they'll be 0)
    std::vector<float> verts;
    for (auto& shape : shapes) {
        for (auto& idx : shape.mesh.indices) {
            // Position
            verts.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
            verts.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
            verts.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

            // Normal (if present)
            if (idx.normal_index >= 0) {
                verts.push_back(attrib.normals[3 * idx.normal_index + 0]);
                verts.push_back(attrib.normals[3 * idx.normal_index + 1]);
                verts.push_back(attrib.normals[3 * idx.normal_index + 2]);
            } else {
                verts.push_back(0.0f);
                verts.push_back(1.0f);
                verts.push_back(0.0f);
            }
        }
    }

    ModelMesh m;
    m.vertexCount = static_cast<int>(verts.size()) / 6;

    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    // location 0: position (x y z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // location 1: normal (nx ny nz)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return m;
}

void drawModel(const ModelMesh& m, GLint modelLoc, GLint colorLoc,
               const glm::mat4& model, const glm::vec3& color) {
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glBindVertexArray(m.vao);
    glDrawArrays(GL_TRIANGLES, 0, m.vertexCount);
}

// ==========================================
// INPUT STATE (passed to callbacks via window user pointer)
// ==========================================

struct AppState {
    ThirdPersonCamera camera;
    bool rightDragging = false;
    double lastX = 0.0;
    double lastY = 0.0;
};

static void scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
    auto* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    state->camera.zoom(static_cast<float>(yoffset) * 1.0f);  // scroll up = zoom in
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
    auto* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        state->rightDragging = (action == GLFW_PRESS);
        if (state->rightDragging) {
            glfwGetCursorPos(window, &state->lastX, &state->lastY);
        }
    }
}

static void cursorPosCallback(GLFWwindow* window, double x, double y) {
    auto* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    double dx = x - state->lastX;
    double dy = y - state->lastY;
    state->lastX = x;
    state->lastY = y;

    if (!state->rightDragging) return;

    const float sensitivity = 0.005f;
    state->camera.orbit(static_cast<float>(-dx) * sensitivity,
                        static_cast<float>(dy) * sensitivity);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

// ==========================================
// SHADERS
// ==========================================

const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec3 FragNormal;\n"
    "out vec3 FragPos;\n"
    "void main()\n"
    "{\n"
    "    FragPos    = vec3(model * vec4(aPos, 1.0));\n"
    "    FragNormal = mat3(transpose(inverse(model))) * aNormal;\n"
    "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}\n";

const char* fragmentShaderSource = "#version 330 core\n"
    "in vec3 FragNormal;\n"
    "in vec3 FragPos;\n"
    "out vec4 FragColor;\n"
    "uniform vec3 color;\n"
    "uniform vec3 lightDir;\n"   // direction the light is coming FROM
    "void main()\n"
    "{\n"
    "    vec3  norm    = normalize(FragNormal);\n"
    "    vec3  light   = normalize(lightDir);\n"
    "    float diff    = max(dot(norm, light), 0.0);\n"
    "    float ambient = 0.2;\n"
    "    float bright  = ambient + (1.0 - ambient) * diff;\n"
    "    FragColor = vec4(color * bright, 1.0);\n"
    "}\n";

bool checkShaderCompile(unsigned int shader, const char* name) {
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
        std::cerr << name << " shader failed to compile:\n" << infoLog << std::endl;
        return false;
    }
    return true;
}

bool checkProgramLink(unsigned int program) {
    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
        std::cerr << "Shader program failed to link:\n" << infoLog << std::endl;
        return false;
    }
    return true;
}

// ==========================================
// MESHES
// ==========================================

struct Mesh {
    unsigned int vao = 0, vbo = 0, ebo = 0;
    int indexCount = 0;
};

Mesh createMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    Mesh m;
    m.indexCount = static_cast<int>(indices.size());

    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);

    glBindVertexArray(m.vao);

    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return m;
}

// A unit cube centered at the origin (edges from -0.5 to 0.5)
Mesh createCube() {
    std::vector<float> verts = {
        -0.5f, -0.5f, -0.5f,  // 0
         0.5f, -0.5f, -0.5f,  // 1
         0.5f,  0.5f, -0.5f,  // 2
        -0.5f,  0.5f, -0.5f,  // 3
        -0.5f, -0.5f,  0.5f,  // 4
         0.5f, -0.5f,  0.5f,  // 5
         0.5f,  0.5f,  0.5f,  // 6
        -0.5f,  0.5f,  0.5f   // 7
    };
    std::vector<unsigned int> idx = {
        0, 1, 2,  2, 3, 0,   // back
        4, 5, 6,  6, 7, 4,   // front
        0, 4, 7,  7, 3, 0,   // left
        1, 5, 6,  6, 2, 1,   // right
        3, 2, 6,  6, 7, 3,   // top
        0, 1, 5,  5, 4, 0    // bottom
    };
    return createMesh(verts, idx);
}

// A flat square on the XZ plane at y = 0
Mesh createGround(float halfSize) {
    std::vector<float> verts = {
        -halfSize, 0.0f, -halfSize,
         halfSize, 0.0f, -halfSize,
         halfSize, 0.0f,  halfSize,
        -halfSize, 0.0f,  halfSize
    };
    std::vector<unsigned int> idx = { 0, 1, 2,  2, 3, 0 };
    return createMesh(verts, idx);
}

void drawMesh(const Mesh& mesh, GLint modelLoc, GLint colorLoc,
              const glm::mat4& model, const glm::vec3& color) {
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
}

// ==========================================
// MAIN
// ==========================================

int main() {
    std::cout << "Starting Pyxion Engine Render Pipeline..." << std::endl;

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Pyxion", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    AppState state;
    glfwSetWindowUserPointer(window, &state);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    ModelMesh playerModel = loadOBJ("assets/player.obj");
    if (playerModel.vao == 0) {
        std::cerr << "Model failed to load, check the path." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Set the initial viewport using the framebuffer size (handles HiDPI)
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    // 3D needs depth testing so nearer faces hide farther ones
    glEnable(GL_DEPTH_TEST);

    // ---- Shaders ----
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);

    bool shadersOk = checkShaderCompile(vs, "Vertex") && checkShaderCompile(fs, "Fragment");

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    bool linkOk = checkProgramLink(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!shadersOk || !linkOk) {
        glDeleteProgram(program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    GLint modelLoc      = glGetUniformLocation(program, "model");
    GLint viewLoc       = glGetUniformLocation(program, "view");
    GLint projLoc       = glGetUniformLocation(program, "projection");
    GLint colorLoc      = glGetUniformLocation(program, "color");
    GLint lightDirLoc = glGetUniformLocation(program, "lightDir");

    // ---- Geometry ----
    Mesh playerMesh = createCube();
    Mesh groundMesh = createGround(25.0f);
    Mesh blockMesh  = createCube();

    // ---- Player state ----
    glm::vec3 playerPos(3.0f);
    float playerYaw = 0.0f;
    const float moveSpeed = 5.0f;

    glClearColor(0.85f, 0.85f, 0.85f, 1.0f);

    double lastTime = glfwGetTime();
    // A light coming from above-right-front — tweak these to taste
    glm::vec3 lightDir = glm::normalize(glm::vec3(0.6f, 1.0f, 0.4f));

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = static_cast<float>(now - lastTime);
        lastTime = now;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // ---- Movement relative to the camera ----
        // Camera looks toward -offset, so "forward" is the horizontal direction
        // from the camera toward the player.
        glm::vec3 forward = glm::normalize(glm::vec3(-std::sin(state.camera.yaw), 0.0f,
                                                     -std::cos(state.camera.yaw)));
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

        glm::vec3 move(0.0f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += forward;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= forward;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += right;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= right;

        if (glm::length(move) > 0.0f) {
            move = glm::normalize(move);
            playerPos += move * moveSpeed * dt;
            playerYaw = std::atan2(move.x, move.z);  // face the direction of travel
        }

        // Camera follows the player
        state.camera.target = playerPos;

        // ---- Render ----
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = height > 0 ? static_cast<float>(width) / height : 1.0f;

        glm::mat4 view = state.camera.view();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 200.0f);

        glUseProgram(program);
        glUniformMatrix4fv(viewLoc,  1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc,  1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

        // Ground
        drawMesh(groundMesh, modelLoc, colorLoc,
                 glm::mat4(1.0f), glm::vec3(0.45f, 0.6f, 0.45f));

        // Static block (a reference object so you can see the camera move)
        glm::mat4 blockModel = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 1.0f, 5.0f));
        blockModel = glm::scale(blockModel, glm::vec3(1.0f, 2.0f, 1.0f));
        drawMesh(blockMesh, modelLoc, colorLoc, blockModel, glm::vec3(0.6f, 0.4f, 0.3f));

        // Player
        glm::mat4 playerMat = glm::translate(glm::mat4(1.0f), playerPos);
        playerMat = glm::rotate(playerMat, playerYaw, glm::vec3(0.0f, 1.0f, 0.0f));
        // scale if the model comes in at the wrong size
        playerMat = glm::scale(playerMat, glm::vec3(1.0f));
        drawModel(playerModel, modelLoc, colorLoc, playerMat, glm::vec3(0.7f, 0.7f, 0.7f));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &playerMesh.vao);
    glDeleteBuffers(1, &playerMesh.vbo);
    glDeleteBuffers(1, &playerMesh.ebo);
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