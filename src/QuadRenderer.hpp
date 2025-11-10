#pragma once
#include <GL/glew.h>  // Desktop OpenGL

class QuadRenderer {
public:
    QuadRenderer();
    ~QuadRenderer();

    bool init();
    void draw(GLuint texture); // fullscreen only
    void shutdown();

private:
    GLuint vbo = 0;
    GLuint program = 0;
    GLint uTextureLoc = -1;
    GLint aPosLoc = -1;
    GLint aTexLoc = -1;

    bool createShader();
};