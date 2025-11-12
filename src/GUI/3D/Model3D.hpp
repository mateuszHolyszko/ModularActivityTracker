#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct Tri {
    glm::vec3 n;
    glm::vec3 v0, v1, v2;
};

class Model3D {
public:
    Model3D() = default;
    ~Model3D() = default;

    // Load OBJ using tinyobjloader
    bool loadFromOBJ(const std::string& path);

    // Keep normalization and draw helpers
    void normalizeToUnit(float margin = 0.9f); // center and scale
    void drawImmediate() const; // immediate-mode draw (glBegin/glEnd)

    const std::vector<Tri>& triangles() const { return tris; }

private:
    std::vector<Tri> tris;
    void computeNormalsIfMissing();
};