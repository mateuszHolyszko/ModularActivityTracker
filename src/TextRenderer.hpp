#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>

// Forward declaration
struct RenderContext;

// ==============================
// Command structure
// ==============================
struct TextCommand {
    std::string text;
    std::string font;
    float x, y, width, height;
    glm::vec4 color;
    int layer;
    float fontSize;
};

// ==============================
// RenderContext
// ==============================
struct RenderContext {
    std::vector<TextCommand> textQueue;
    class TextRenderer* textRenderer = nullptr;

    void clearQueues() {
        textQueue.clear();
    }
};

// ==============================
// TextRenderer
// ==============================
class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();

    bool init(RenderContext* ctx, int texWidth, int texHeight);
    void renderToTexture(std::vector<TextCommand>& commands);
    GLuint getTexture() const { return colorTex; }
    void shutdown();

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