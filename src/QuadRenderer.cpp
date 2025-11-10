#include "QuadRenderer.hpp"
#include <iostream>

static const char* quadVertSrc = R"(
attribute vec2 aPos;
attribute vec2 aTex;
varying vec2 vTex;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vTex = aTex;
}
)";

static const char* quadFragSrc = R"(
precision mediump float;
varying vec2 vTex;
uniform sampler2D uTexture;

void main() {
    gl_FragColor = texture2D(uTexture, vTex);
}
)";

QuadRenderer::QuadRenderer() {}
QuadRenderer::~QuadRenderer() { shutdown(); }

bool QuadRenderer::createShader() {
    auto compileShader = [](GLenum type, const char* src) -> GLuint {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::cerr << "[Shader Compile Error] " << log << std::endl;
        }
        return shader;
    };

    GLuint vert = compileShader(GL_VERTEX_SHADER, quadVertSrc);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, quadFragSrc);

    program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "[Shader Link Error] " << log << std::endl;
        return false;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    aPosLoc = glGetAttribLocation(program, "aPos");
    aTexLoc = glGetAttribLocation(program, "aTex");
    uTextureLoc = glGetUniformLocation(program, "uTexture");

    return true;
}

bool QuadRenderer::init() {
    if (!createShader())
        return false;

    // Fullscreen quad: NDC coordinates from -1 to 1
    float vertices[] = {
        // pos     // tex
        -1.f, -1.f, 0.f, 0.f,
         1.f, -1.f, 1.f, 0.f,
        -1.f,  1.f, 0.f, 1.f,
         1.f,  1.f, 1.f, 1.f
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}

void QuadRenderer::draw(GLuint texture) {
    glUseProgram(program);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(aPosLoc);
    glEnableVertexAttribArray(aTexLoc);

    glVertexAttribPointer(aPosLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(aTexLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(uTextureLoc, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(aPosLoc);
    glDisableVertexAttribArray(aTexLoc);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

void QuadRenderer::shutdown() {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (program) glDeleteProgram(program);
    vbo = program = 0;
}