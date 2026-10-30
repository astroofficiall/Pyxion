#pragma once
#include <GL/glew.h>
#include <iostream>

inline bool checkShaderCompile(unsigned int shader, const char* name) {
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        std::cerr << name << " shader failed to compile:\n" << log << std::endl;
        return false;
    }
    return true;
}

inline bool checkProgramLink(unsigned int program) {
    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        std::cerr << "Shader program failed to link:\n" << log << std::endl;
        return false;
    }
    return true;
}

inline const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec3 FragNormal;\n"
    "out vec3 FragPos;\n"
    "void main()\n"
    "{\n"
    "    FragPos     = vec3(model * vec4(aPos, 1.0));\n"
    "    FragNormal  = mat3(transpose(inverse(model))) * aNormal;\n"
    "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}\n";

inline const char* fragmentShaderSource = "#version 330 core\n"
    "in vec3 FragNormal;\n"
    "in vec3 FragPos;\n"
    "out vec4 FragColor;\n"
    "uniform vec3 color;\n"
    "uniform vec3 lightDir;\n"
    "uniform vec3 camPos;\n"
    "void main()\n"
    "{\n"
    "    vec3 norm  = normalize(FragNormal);\n"
    "    vec3 light = normalize(lightDir);\n"
    "    vec3 view  = normalize(camPos - FragPos);\n"
    "    vec3 skyColor    = vec3(0.4, 0.6, 0.9);\n"
    "    vec3 groundColor = vec3(0.2, 0.15, 0.1);\n"
    "    float hemi   = norm.y * 0.5 + 0.5;\n"
    "    vec3 ambient = mix(groundColor, skyColor, hemi) * 0.35;\n"
    "    float diff   = max(dot(norm, light), 0.0);\n"
    "    vec3 diffuse = color * diff;\n"
    "    vec3 halfVec  = normalize(light + view);\n"
    "    float spec    = pow(max(dot(norm, halfVec), 0.0), 32.0);\n"
    "    vec3 specular = vec3(0.4) * spec;\n"
    "    float rim     = 1.0 - max(dot(norm, view), 0.0);\n"
    "    rim           = pow(rim, 3.0);\n"
    "    vec3 rimColor = vec3(0.3, 0.4, 0.6) * rim * 0.4;\n"
    "    vec3 result   = ambient * color + diffuse + specular + rimColor;\n"
    "    result = result / (result + vec3(1.0));\n"
    "    result = pow(result, vec3(1.0 / 2.2));\n"
    "    FragColor = vec4(result, 1.0);\n"
    "}\n";