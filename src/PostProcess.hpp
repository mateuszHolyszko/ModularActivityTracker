#ifndef POSTPROCESS_HPP
#define POSTPROCESS_HPP

//#include <GLES2/gl2.h>  // mobile OpenGL ES 2.0
#include <SDL2/SDL.h>

#include<GL/glew.h> // Desktop OpenGL

#include <vector>
#include <string>
#include <unordered_map>

struct PostProcessPass {
    GLuint program = 0;
    std::unordered_map<std::string, GLint> uniforms;
    std::string name;
    GLint posLoc = -1;  // Actual attrib location for aPos
    GLint texLoc = -1;  // Actual attrib location for aTex
};

class PostProcess {
public:
    bool init(int w, int h);
    void addPass(const std::string& vertPath, const std::string& fragPath, const std::string& name);
    void setUniform(const std::string& passName, const std::string& uniformName, float value);
    GLuint process(GLuint inputTex);
    void shutdown();

private:
    std::vector<PostProcessPass> passes;
    std::vector<GLuint> tempFBOs, tempTexs;
    int width, height;
    GLuint quadVBO = 0;

    // Helpers
    std::string readFile(const std::string& path);
    GLuint compileShader(const std::string& source, GLenum type);
    bool linkProgram(GLuint vertShader, GLuint fragShader, GLuint& program);
};

#endif