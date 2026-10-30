#pragma once

// IMPORTANT: Include this implementation in only ONE .cpp translation unit.
// If multiple .cpp files include Mesh.h, move TINYOBJLOADER_IMPLEMENTATION
// into a dedicated tinyobj_loader.cpp file.
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <limits>

struct Mesh {
    unsigned int vao = 0, vbo = 0, ebo = 0;
    int indexCount = 0;
};

struct ModelMesh {
    unsigned int vao = 0, vbo = 0;
    int vertexCount = 0;
    std::string name;

    // Original position of this piece's calculated pivot.
    glm::vec3 pivot{0.0f};

    // Bounds are now relative to the individual pivot.
    glm::vec3 boundsMin{0.0f};
    glm::vec3 boundsMax{0.0f};

    bool hasBounds = false;
};

inline Mesh createMesh(const std::vector<float>& vertices,
                       const std::vector<unsigned int>& indices) {
    Mesh m;
    m.indexCount = static_cast<int>(indices.size());

    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);

    glBindVertexArray(m.vao);

    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return m;
}

inline Mesh createCube() {
    std::vector<float> verts = {
        -0.5f,-0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
         0.5f,-0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
         0.5f, 0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
        -0.5f, 0.5f,-0.5f, 0.0f, 0.0f,-1.0f,
        -0.5f,-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
         0.5f,-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f,-0.5f,-0.5f,-1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,-1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,-1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f,-0.5f,-1.0f, 0.0f, 0.0f,
         0.5f,-0.5f,-0.5f, 1.0f, 0.0f, 0.0f,
         0.5f,-0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, 0.5f,-0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f,-0.5f, 0.0f, 1.0f, 0.0f,
         0.5f, 0.5f,-0.5f, 0.0f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f,-0.5f,-0.5f, 0.0f,-1.0f, 0.0f,
         0.5f,-0.5f,-0.5f, 0.0f,-1.0f, 0.0f,
         0.5f,-0.5f, 0.5f, 0.0f,-1.0f, 0.0f,
        -0.5f,-0.5f, 0.5f, 0.0f,-1.0f, 0.0f,
    };

    std::vector<unsigned int> idx = {
         0, 1, 2,  2, 3, 0,
         4, 5, 6,  6, 7, 4,
         8, 9,10, 10,11, 8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20,
    };

    return createMesh(verts, idx);
}

inline Mesh createGround(float halfSize) {
    std::vector<float> verts = {
        -halfSize, 0.0f,-halfSize, 0.0f, 1.0f, 0.0f,
         halfSize, 0.0f,-halfSize, 0.0f, 1.0f, 0.0f,
         halfSize, 0.0f, halfSize, 0.0f, 1.0f, 0.0f,
        -halfSize, 0.0f, halfSize, 0.0f, 1.0f, 0.0f,
    };

    std::vector<unsigned int> idx = { 0, 1, 2, 2, 3, 0 };
    return createMesh(verts, idx);
}

inline void drawMesh(const Mesh& mesh, GLint modelLoc, GLint colorLoc,
                     const glm::mat4& model, const glm::vec3& color) {
    if (modelLoc >= 0)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    if (colorLoc >= 0)
        glUniform3fv(colorLoc, 1, glm::value_ptr(color));

    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
}

inline ModelMesh uploadShape(
    const std::string& name,
    const tinyobj::attrib_t& attrib,
    const tinyobj::shape_t& shape
) {
    std::vector<float> verts;

    glm::vec3 bmin(std::numeric_limits<float>::max());
    glm::vec3 bmax(std::numeric_limits<float>::lowest());

    for (const auto& idx : shape.mesh.indices) {
        glm::vec3 position(
            attrib.vertices[3 * idx.vertex_index + 0],
            attrib.vertices[3 * idx.vertex_index + 1],
            attrib.vertices[3 * idx.vertex_index + 2]
        );

        bmin = glm::min(bmin, position);
        bmax = glm::max(bmax, position);

        verts.push_back(position.x);
        verts.push_back(position.y);
        verts.push_back(position.z);

        if (idx.normal_index >= 0) {
            verts.push_back(attrib.normals[3 * idx.normal_index + 0]);
            verts.push_back(attrib.normals[3 * idx.normal_index + 1]);
            verts.push_back(attrib.normals[3 * idx.normal_index + 2]);
        }
        else {
            verts.push_back(0.0f);
            verts.push_back(1.0f);
            verts.push_back(0.0f);
        }
    }

    ModelMesh m;
    m.name = name;
    m.vertexCount = static_cast<int>(verts.size()) / 6;

    if (!verts.empty()) {
        // Calculate this individual piece's pivot.
        m.pivot = (bmin + bmax) * 0.5f;

        // Convert geometry into pivot-relative coordinates.
        for (size_t i = 0; i < verts.size(); i += 6) {
            verts[i + 0] -= m.pivot.x;
            verts[i + 1] -= m.pivot.y;
            verts[i + 2] -= m.pivot.z;
        }

        // Store bounds relative to the new pivot.
        m.boundsMin = bmin - m.pivot;
        m.boundsMax = bmax - m.pivot;
        m.hasBounds = true;
    }

    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);

    glBindVertexArray(m.vao);

    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        verts.size() * sizeof(float),
        verts.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return m;
}

inline std::vector<ModelMesh> loadOBJ(const char* path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string filePath(path);
    size_t slash = filePath.find_last_of("/\\");
    std::string baseDir = slash == std::string::npos
        ? std::string()
        : filePath.substr(0, slash + 1);

    bool ok = tinyobj::LoadObj(
        &attrib, &shapes, &materials, &warn, &err,
        path, baseDir.c_str()
    );

    if (!warn.empty())
        std::cout << "OBJ warn: " << warn << std::endl;
    if (!err.empty())
        std::cerr << "OBJ err: " << err << std::endl;

    if (!ok) {
        std::cerr << "Failed to load " << path << std::endl;
        return {};
    }

    std::cout << "Loaded " << shapes.size()
              << " piece(s) from " << path << std::endl;

    std::vector<ModelMesh> meshes;
    for (const auto& shape : shapes) {
        std::cout << "  piece: " << shape.name << std::endl;
        meshes.push_back(uploadShape(shape.name, attrib, shape));
    }

    return meshes;
}

inline void drawModel(const ModelMesh& m, GLint modelLoc, GLint colorLoc,
                      const glm::mat4& model, const glm::vec3& color) {
    if (modelLoc >= 0)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    if (colorLoc >= 0)
        glUniform3fv(colorLoc, 1, glm::value_ptr(color));

    glBindVertexArray(m.vao);
    glDrawArrays(GL_TRIANGLES, 0, m.vertexCount);
}
