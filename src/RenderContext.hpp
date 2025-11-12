#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>

// Forward declaration
class SimpleRenderer;

// ==============================
// Command structures
// ==============================
struct TextCommand {
    std::string text;
    std::string font;
    float x, y, width, height;
    glm::vec4 color;
    int layer;
    float fontSize;
};

struct GraphicCommand {
    enum Type { LINE, BOX, CIRCLE };  
    Type type;
    float x1, y1, x2, y2; 
    // For circle: x1,y1 = center, x2 = radius (y2 unused)
    glm::vec4 color;
    int layer;
    float lineWidth;
    bool filled;
};

// ==============================
// RenderContext
// ==============================
struct RenderContext {
    std::vector<TextCommand> textQueue;  // commands accumulated by GUI elements
    std::vector<GraphicCommand> graphicQueue;  // graphic commands

    SimpleRenderer* renderer = nullptr;  // pointer, not owned

    void clearQueues() {
        textQueue.clear();
        graphicQueue.clear();
    }
    
    // Text measurement method - delegates to SimpleRenderer
    float measureTextWidth(const std::string& text, const std::string& font, float fontSize);
};