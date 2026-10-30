
#pragma once

#include <GL/glew.h>
#include <iostream>

// ============================================================
// SHADER ERROR CHECKING
// ============================================================

inline bool checkShaderCompile(unsigned int shader, const char* name)
{
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        std::cerr << name << " shader failed to compile:\n"
                  << log << std::endl;
        return false;
    }

    return true;
}

inline bool checkProgramLink(unsigned int program)
{
    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        std::cerr << "Shader program failed to link:\n"
                  << log << std::endl;
        return false;
    }

    return true;
}

// ============================================================
// MAIN VERTEX SHADER
// ============================================================

inline const char* vertexShaderSource = "#version 330 core\n"

    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"

    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 lightSpaceMatrix;\n"

    "out vec3 FragNormal;\n"
    "out vec3 FragPos;\n"
    "out vec4 FragPosLightSpace;\n"

    "void main()\n"
    "{\n"
    "    vec4 worldPosition = model * vec4(aPos, 1.0);\n"

    "    FragPos = worldPosition.xyz;\n"

    "    FragNormal = normalize(mat3(transpose(inverse(model))) * aNormal);\n"

    "    FragPosLightSpace = lightSpaceMatrix * worldPosition;\n"

    "    gl_Position = projection * view * worldPosition;\n"
    "}\n";

// ============================================================
// MAIN FRAGMENT SHADER
// ============================================================

inline const char* fragmentShaderSource = "#version 330 core\n"

    "in vec3 FragNormal;\n"
    "in vec3 FragPos;\n"
    "in vec4 FragPosLightSpace;\n"

    "out vec4 FragColor;\n"

    "uniform vec3 color;\n"
    "uniform vec3 lightDir;\n"
    "uniform vec3 camPos;\n"
    "uniform sampler2D shadowMap;\n"
    "uniform float shadowStrength;\n"

    // --------------------------------------------------------
    // SHADOW CALCULATION
    // --------------------------------------------------------

    "float CalculateShadow(vec3 normal, vec3 light)\n"
    "{\n"
    "    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;\n"
    "    projCoords = projCoords * 0.5 + 0.5;\n"

    "    if (projCoords.z > 1.0 ||\n"
    "        projCoords.x < 0.0 || projCoords.x > 1.0 ||\n"
    "        projCoords.y < 0.0 || projCoords.y > 1.0)\n"
    "        return 0.0;\n"

    "    float bias = max(0.002 * (1.0 - dot(normal, light)), 0.0004);\n"

    "    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));\n"

    "    float shadow = 0.0;\n"

    "    for (int x = -1; x <= 1; x++)\n"
    "    {\n"
    "        for (int y = -1; y <= 1; y++)\n"
    "        {\n"
    "            float closestDepth = texture(\n"
    "                shadowMap,\n"
    "                projCoords.xy + vec2(x, y) * texelSize\n"
    "            ).r;\n"

    "            shadow += (projCoords.z - bias > closestDepth) ? 1.0 : 0.0;\n"
    "        }\n"
    "    }\n"

    "    return (shadow / 9.0) * shadowStrength;\n"
    "}\n"

    // --------------------------------------------------------
    // MAIN
    // --------------------------------------------------------

    "void main()\n"
    "{\n"
    "    vec3 norm = normalize(FragNormal);\n"
    "    vec3 light = normalize(lightDir);\n"
    "    vec3 viewDir = normalize(camPos - FragPos);\n"

    // Ambient

    "    vec3 skyColor = vec3(0.48, 0.68, 0.95);\n"
    "    vec3 groundColor = vec3(0.18, 0.20, 0.25);\n"

    "    float hemi = norm.y * 0.5 + 0.5;\n"
    "    vec3 ambient = mix(groundColor, skyColor, hemi) * 0.42;\n"

    // Diffuse

    "    float NdotL = max(dot(norm, light), 0.0);\n"
    "    float diffuseLight = smoothstep(0.05, 0.95, NdotL);\n"
    "    diffuseLight = mix(0.45, 1.0, diffuseLight);\n"

    "    vec3 diffuse = color * diffuseLight;\n"

    // Specular

    "    vec3 halfVector = normalize(light + viewDir);\n"
    "    float spec = pow(max(dot(norm, halfVector), 0.0), 48.0);\n"
    "    vec3 specular = vec3(0.75, 0.82, 1.0) * spec * 0.18;\n"

    // Rim lighting

    "    float fresnel = 1.0 - max(dot(norm, viewDir), 0.0);\n"
    "    fresnel = pow(fresnel, 3.0);\n"

    "    vec3 rim = vec3(0.35, 0.55, 0.95) * fresnel * 0.22;\n"

    // Shadows

    "    float shadow = CalculateShadow(norm, light);\n"

    // Final lighting

    "    vec3 result = color * ambient;\n"
    "    result += diffuse * 0.72 * (1.0 - shadow);\n"
    "    result += specular * (1.0 - shadow);\n"
    "    result += rim;\n"

    // Color grading

    "    result = result / (result + vec3(0.85));\n"
    "    result = pow(result, vec3(1.0 / 2.2));\n"

    "    FragColor = vec4(result, 1.0);\n"
    "}\n";

// ============================================================
// SHADOW DEPTH SHADERS
// ============================================================

inline const char* shadowVertexShaderSource = "#version 330 core\n"

    "layout (location = 0) in vec3 aPos;\n"

    "uniform mat4 model;\n"
    "uniform mat4 lightSpaceMatrix;\n"

    "void main()\n"
    "{\n"
    "    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);\n"
    "}\n";

inline const char* shadowFragmentShaderSource = "#version 330 core\n"

    "void main()\n"
    "{\n"
    "}\n";

// ============================================================
// SHADOW MAP MANAGER
// ============================================================

class ShadowMap
{
public:

    static constexpr unsigned int WIDTH = 1024;
    static constexpr unsigned int HEIGHT = 1024;

    unsigned int FBO = 0;
    unsigned int depthMap = 0;

    bool init()
    {
        glGenFramebuffers(1, &FBO);
        glGenTextures(1, &depthMap);

        glBindTexture(GL_TEXTURE_2D, depthMap);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_DEPTH_COMPONENT24,
            WIDTH,
            HEIGHT,
            0,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            nullptr
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        glTexParameterfv(
            GL_TEXTURE_2D,
            GL_TEXTURE_BORDER_COLOR,
            borderColor
        );

        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D,
            depthMap,
            0
        );

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        bool complete =
            glCheckFramebufferStatus(GL_FRAMEBUFFER)
            == GL_FRAMEBUFFER_COMPLETE;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (!complete)
        {
            std::cerr << "Shadow framebuffer incomplete!\n";
            return false;
        }

        return true;
    }

    void begin()
    {
        glViewport(0, 0, WIDTH, HEIGHT);

        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void end(int screenWidth, int screenHeight)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, screenWidth, screenHeight);
    }

    void bind(unsigned int textureUnit = 1)
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, depthMap);
    }

    void cleanup()
    {
        if (depthMap)
            glDeleteTextures(1, &depthMap);

        if (FBO)
            glDeleteFramebuffers(1, &FBO);

        depthMap = 0;
        FBO = 0;
    }
};
