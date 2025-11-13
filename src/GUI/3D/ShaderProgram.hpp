#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram();

    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath);
    GLuint getProgram() const { return program; }
    void use() const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    GLuint program = 0;

    static GLuint compileShader(const std::string& source, GLenum type);
    static std::string readFile(const std::string& path);
};