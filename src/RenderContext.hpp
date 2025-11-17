#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <GL/glew.h>

// Forward declarations
class SimpleRenderer;
class NotificationSystem;
class Menu;  // Add forward declaration for Menu

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
    enum Type { LINE, BOX, CIRCLE, TEXTURE, ARROW };
    Type type;
    float x1, y1, x2, y2;
    glm::vec4 color;
    int layer;
    float lineWidth;
    bool filled;
    
    GLuint textureId = 0;
    float u1 = 0.0f, v1 = 0.0f, u2 = 1.0f, v2 = 1.0f;

    // Add direction field for ARROW type (0=up, 1=right, 2=down, 3=left)
    int direction = 0;
};

// ==============================
// RenderContext
// ==============================
struct RenderContext {
    std::vector<TextCommand> textQueue;
    std::vector<GraphicCommand> graphicQueue;

    SimpleRenderer* renderer = nullptr;
    NotificationSystem* notificationSystem = nullptr;  // Notification system reference
    Menu* currentMenu = nullptr;  // Add current menu pointer

    void clearQueues() {
        textQueue.clear();
        graphicQueue.clear();
    }
    
    // Images
    void drawImage(GLuint textureId, float x, float y, float width, float height, int layer = 0) {
        GraphicCommand cmd;
        cmd.type = GraphicCommand::TEXTURE;
        cmd.textureId = textureId;
        cmd.x1 = x;
        cmd.y1 = y;
        cmd.x2 = x + width;
        cmd.y2 = y + height;
        cmd.u1 = 0.0f;
        cmd.v1 = 0.0f;
        cmd.u2 = 1.0f;
        cmd.v2 = 1.0f;
        cmd.layer = layer;
        graphicQueue.push_back(cmd);
    }

    // Draw with custom UV coordinates (for sprite sheets, partial images, etc.)
    void drawImageUV(GLuint textureId, float x, float y, float width, float height, 
                    float u1, float v1, float u2, float v2, int layer = 0) {
        GraphicCommand cmd;
        cmd.type = GraphicCommand::TEXTURE;
        cmd.textureId = textureId;
        cmd.x1 = x;
        cmd.y1 = y;
        cmd.x2 = x + width;
        cmd.y2 = y + height;
        cmd.u1 = u1;
        cmd.v1 = v1;
        cmd.u2 = u2;
        cmd.v2 = v2;
        cmd.layer = layer;
        graphicQueue.push_back(cmd);
    }

    // Forward declaration only - implementation in .cpp
    float measureTextWidth(const std::string& text, const std::string& font, float fontSize);
    void addNotification(const std::string& text);
    
    // Menu management
    void setCurrentMenu(Menu* menu) { currentMenu = menu; }
    Menu* getCurrentMenu() const { return currentMenu; }
};