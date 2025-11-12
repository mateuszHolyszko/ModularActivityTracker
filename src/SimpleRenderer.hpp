#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>  // Desktop OpenGL
//#include <GLES2/gl2.h>  // Mobile OpenGL ES 2.0
#include <SDL2/SDL.h>

#include "RenderContext.hpp"  // Include the single source of truth

// ==============================
// SimpleRenderer (formerly TextRenderer)
// ==============================
class SimpleRenderer {
public:
    SimpleRenderer();
    ~SimpleRenderer();

    bool init(RenderContext* ctx, int texWidth, int texHeight);
    void renderToTexture(std::vector<TextCommand>& textCommands, std::vector<GraphicCommand>& graphicCommands);
    GLuint getTexture() const { return colorTex; }
    void shutdown();
    
    // Text measurement method - now private, only called by RenderContext
    float measureTextWidth(const std::string& text, const std::string& font, float fontSize);

private:
    RenderContext* context = nullptr;
    struct NVGcontext* vg = nullptr;

    GLuint fbo = 0;
    GLuint colorTex = 0;
    GLuint rbo = 0;
    int texWidth = 0;
    int texHeight = 0;

    int defaultFont = -1;

    bool createFBO(int width, int height);
    void destroyFBO();
};