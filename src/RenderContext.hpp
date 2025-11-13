#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <GL/glew.h>

// Forward declarations
class SimpleRenderer;
class NotificationSystem;

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
    enum Type { LINE, BOX, CIRCLE, TEXTURE };
    Type type;
    float x1, y1, x2, y2;
    glm::vec4 color;
    int layer;
    float lineWidth;
    bool filled;
    
    GLuint textureId = 0;
    float u1 = 0.0f, v1 = 0.0f, u2 = 1.0f, v2 = 1.0f;
};

// ==============================
// RenderContext
// ==============================
struct RenderContext {
    std::vector<TextCommand> textQueue;
    std::vector<GraphicCommand> graphicQueue;

    SimpleRenderer* renderer = nullptr;
    NotificationSystem* notificationSystem = nullptr;  // Notification system reference

    void clearQueues() {
        textQueue.clear();
        graphicQueue.clear();
    }
    
    float measureTextWidth(const std::string& text, const std::string& font, float fontSize);
    
    // Forward declaration only - implementation in .cpp
    void addNotification(const std::string& text);
};