#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>

// Forward declaration
class TextRenderer;

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

// ==============================
// RenderContext
// ==============================
struct RenderContext {
    std::vector<TextCommand> textQueue;  // commands accumulated by GUI elements

    TextRenderer* textRenderer = nullptr;  // pointer, not owned

    void clearQueues() {
        textQueue.clear();
    }
};