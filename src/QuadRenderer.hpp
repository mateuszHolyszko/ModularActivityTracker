#pragma once
#include <GL/glew.h>  // Desktop OpenGL
//#include <GLES2/gl2.h>  // Mobile OpenGL ES 2.0
#include <SDL2/SDL.h>


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