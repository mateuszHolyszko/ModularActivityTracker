#include "Model3D.hpp"
#include <iostream>
#include <algorithm>
#include <cfloat>

// tinyobjloader header
#define TINYOBJLOADER_IMPLEMENTATION
#include "../../external/tinyobjloader/tiny_obj_loader.h"

Model3D::~Model3D() {
    destroyVAO();
}

bool Model3D::loadFromOBJ(const std::string& path) {
    tris.clear();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    if (!warn.empty()) std::cerr << "[Model3D] tinyobj warning: " << warn << "\n";
    if (!err.empty()) std::cerr << "[Model3D] tinyobj error: " << err << "\n";
    if (!ok) {
        std::cerr << "[Model3D] Failed to load OBJ: " << path << "\n";
        return false;
    }

    // Walk all shapes and faces, building triangles
    for (const auto& shape : shapes) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            int fv = shape.mesh.num_face_vertices[f];
            if (fv != 3) {
                index_offset += fv;
                continue;
            }
            tinyobj::index_t idx0 = shape.mesh.indices[index_offset + 0];
            tinyobj::index_t idx1 = shape.mesh.indices[index_offset + 1];
            tinyobj::index_t idx2 = shape.mesh.indices[index_offset + 2];

            glm::vec3 v0(0.0f), v1(0.0f), v2(0.0f), n(0.0f);
            if (idx0.vertex_index >= 0) {
                v0.x = attrib.vertices[3 * idx0.vertex_index + 0];
                v0.y = attrib.vertices[3 * idx0.vertex_index + 1];
                v0.z = attrib.vertices[3 * idx0.vertex_index + 2];
            }
            if (idx1.vertex_index >= 0) {
                v1.x = attrib.vertices[3 * idx1.vertex_index + 0];
                v1.y = attrib.vertices[3 * idx1.vertex_index + 1];
                v1.z = attrib.vertices[3 * idx1.vertex_index + 2];
            }
            if (idx2.vertex_index >= 0) {
                v2.x = attrib.vertices[3 * idx2.vertex_index + 0];
                v2.y = attrib.vertices[3 * idx2.vertex_index + 1];
                v2.z = attrib.vertices[3 * idx2.vertex_index + 2];
            }

            // If normals are present, take average per-vertex normal as triangle normal
            if (idx0.normal_index >= 0 && idx1.normal_index >= 0 && idx2.normal_index >= 0) {
                glm::vec3 n0(
                    attrib.normals[3 * idx0.normal_index + 0],
                    attrib.normals[3 * idx0.normal_index + 1],
                    attrib.normals[3 * idx0.normal_index + 2]);
                glm::vec3 n1(
                    attrib.normals[3 * idx1.normal_index + 0],
                    attrib.normals[3 * idx1.normal_index + 1],
                    attrib.normals[3 * idx1.normal_index + 2]);
                glm::vec3 n2(
                    attrib.normals[3 * idx2.normal_index + 0],
                    attrib.normals[3 * idx2.normal_index + 1],
                    attrib.normals[3 * idx2.normal_index + 2]);
                n = glm::normalize((n0 + n1 + n2) * (1.0f / 3.0f));
            }

            Tri t;
            t.v0 = v0; t.v1 = v1; t.v2 = v2; t.n = n;
            tris.push_back(t);

            index_offset += fv;
        }
    }

    computeNormalsIfMissing();
    createVAO();  // NEW: create VAO after loading
    std::cerr << "[Model3D] Loaded " << tris.size() << " triangles from " << path << "\n";
    return !tris.empty();
}

void Model3D::computeNormalsIfMissing() {
    for (auto& t : tris) {
        if (glm::length(t.n) < 1e-6f) {
            glm::vec3 e1 = t.v1 - t.v0;
            glm::vec3 e2 = t.v2 - t.v0;
            t.n = glm::normalize(glm::cross(e1, e2));
        } else {
            t.n = glm::normalize(t.n);
        }
    }
}

void Model3D::normalizeToUnit(float margin) {
    if (tris.empty()) return;
    glm::vec3 mn(FLT_MAX), mx(-FLT_MAX);
    for (auto& t : tris) {
        mn = glm::min(mn, t.v0); mx = glm::max(mx, t.v0);
        mn = glm::min(mn, t.v1); mx = glm::max(mx, t.v1);
        mn = glm::min(mn, t.v2); mx = glm::max(mx, t.v2);
    }
    glm::vec3 center = (mn + mx) * 0.5f;
    glm::vec3 size = mx - mn;
    float maxDim = std::max(std::max(size.x, size.y), size.z);
    if (!(maxDim > 0.0f) || maxDim < 1e-12f) {
        std::cerr << "[Model3D] normalizeToUnit: degenerate bbox\n";
        return;
    }
    float scale = (1.0f * margin) / maxDim;
    for (auto& t : tris) {
        t.v0 = (t.v0 - center) * scale;
        t.v1 = (t.v1 - center) * scale;
        t.v2 = (t.v2 - center) * scale;
    }
    
    // Recreate VAO with normalized data
    createVAO();
}

bool Model3D::createVAO() {
    destroyVAO();
    if (tris.empty()) return false;

    // Build vertex and index arrays
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    
    for (size_t i = 0; i < tris.size(); ++i) {
        const auto& tri = tris[i];
        positions.push_back(tri.v0);
        positions.push_back(tri.v1);
        positions.push_back(tri.v2);
        
        normals.push_back(tri.n);
        normals.push_back(tri.n);
        normals.push_back(tri.n);
        
        indices.push_back(i * 3 + 0);
        indices.push_back(i * 3 + 1);
        indices.push_back(i * 3 + 2);
    }

    // Create VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Position VBO
    glGenBuffers(1, &posVBO);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    // Normal VBO
    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    // Index buffer
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    return true;
}

void Model3D::destroyVAO() {
    if (indexBuffer) glDeleteBuffers(1, &indexBuffer);
    if (normalVBO) glDeleteBuffers(1, &normalVBO);
    if (posVBO) glDeleteBuffers(1, &posVBO);
    if (vao) glDeleteVertexArrays(1, &vao);
    indexBuffer = normalVBO = posVBO = vao = 0;
    indices.clear();
}

void Model3D::drawImmediate() const {
    glBegin(GL_TRIANGLES);
    for (const auto& t : tris) {
        glNormal3f(t.n.x, t.n.y, t.n.z);
        glVertex3f(t.v0.x, t.v0.y, t.v0.z);
        glVertex3f(t.v1.x, t.v1.y, t.v1.z);
        glVertex3f(t.v2.x, t.v2.y, t.v2.z);
    }
    glEnd();
}

void Model3D::drawWithShader(GLuint shaderProgram) const {
    if (!vao || indices.empty()) return;

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}