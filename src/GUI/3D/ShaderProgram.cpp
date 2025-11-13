#include "ShaderProgram.hpp"
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

ShaderProgram::~ShaderProgram() {
    if (program) glDeleteProgram(program);
}

std::string ShaderProgram::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[ShaderProgram] Cannot open: " << path << "\n";
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

GLuint ShaderProgram::compileShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "[ShaderProgram] Compilation failed: " << log << "\n";
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool ShaderProgram::loadFromFiles(const std::string& vertPath, const std::string& fragPath) {
    std::string vertSrc = readFile(vertPath);
    std::string fragSrc = readFile(fragPath);
    if (vertSrc.empty() || fragSrc.empty()) return false;

    GLuint vert = compileShader(vertSrc, GL_VERTEX_SHADER);
    GLuint frag = compileShader(fragSrc, GL_FRAGMENT_SHADER);
    if (!vert || !frag) return false;

    program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glBindAttribLocation(program, 0, "in_pos");
    glBindAttribLocation(program, 1, "in_normal");
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "[ShaderProgram] Link failed: " << log << "\n";
        glDeleteProgram(program);
        glDeleteShader(vert);
        glDeleteShader(frag);
        return false;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return true;
}

void ShaderProgram::use() const {
    glUseProgram(program);
}

void ShaderProgram::setMat4(const std::string& name, const glm::mat4& mat) const {
    GLint loc = glGetUniformLocation(program, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}