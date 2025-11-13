#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

struct Tri {
    glm::vec3 n;
    glm::vec3 v0, v1, v2;
};

class Model3D {
public:
    Model3D() = default;
    ~Model3D();

    // Load OBJ using tinyobjloader
    bool loadFromOBJ(const std::string& path);

    // Normalization
    void normalizeToUnit(float margin = 0.9f);

    // Drawing methods
    void drawImmediate() const;  // Keep for compatibility
    void drawWithShader(GLuint shaderProgram) const;  // NEW: shader-based drawing

    const std::vector<Tri>& triangles() const { return tris; }

private:
    std::vector<Tri> tris;
    GLuint vao = 0;
    GLuint posVBO = 0;
    GLuint normalVBO = 0;
    GLuint indexBuffer = 0;
    std::vector<GLuint> indices;

    void computeNormalsIfMissing();
    bool createVAO();  // NEW: create VAO for shader rendering
    void destroyVAO();  // NEW: clean up VAO
};