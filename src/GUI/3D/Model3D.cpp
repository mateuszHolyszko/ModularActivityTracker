#include "Model3D.hpp"
#include <iostream>
#include <algorithm>
#include <cfloat>
#include <GL/glew.h>

// tinyobjloader header (must be added to external/tinyobjloader)
#define TINYOBJLOADER_IMPLEMENTATION
#include "../../external/tinyobjloader/tiny_obj_loader.h"

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
                // skip non-tri faces (shouldn't happen for triangulated OBJs)
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
}

void Model3D::drawImmediate() const {
    // Flip model on Y axis here to correct upside-down import.
    // Also flip normal Y to keep lighting sensible.
    glBegin(GL_TRIANGLES);
    for (const auto& t : tris) {
        glNormal3f(t.n.x, -t.n.y, t.n.z);                // invert Y in normals
        glVertex3f(t.v0.x, -t.v0.y, t.v0.z);             // invert Y on vertices
        glVertex3f(t.v1.x, -t.v1.y, t.v1.z);
        glVertex3f(t.v2.x, -t.v2.y, t.v2.z);
    }
    glEnd();
}