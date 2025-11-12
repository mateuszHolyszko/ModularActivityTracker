#ifndef TEXTFIELD_HPP
#define TEXTFIELD_HPP

#include "../RenderContext.hpp"  // Now use RenderContext instead of SimpleRenderer
#include <glm/glm.hpp>         // For glm::vec4
#include <string>
#include <vector>

enum HorizontalAlignment {
    LEFT,
    CENTER,
    RIGHT
};

class TextField {
public:
    // Constructor: Now takes RenderContext instead of SimpleRenderer
    TextField(RenderContext* context, float x, float y, float width, float height, const std::string& text, const std::string& font, float font_size, const glm::vec4& color, HorizontalAlignment hAlign, bool wrap);

    // Render method: Returns vector of TextCommand(s) (single command, possibly truncated)
    std::vector<TextCommand> render(int layer = 0);

    // Setters for dynamic updates
    void setText(const std::string& text) { text_ = text; }
    void setPosition(float x, float y) { x_ = x; y_ = y; }
    void setSize(float width, float height) { width_ = width; height_ = height; }
    void setWrap(bool wrap) { wrap_ = wrap; }

private:
    RenderContext* context_;  // Reference to RenderContext for text measurement
    float x_, y_, width_, height_;
    std::string text_;
    std::string font_;
    float font_size_;
    glm::vec4 color_;
    HorizontalAlignment hAlign_;
    bool wrap_;

    // Helper to truncate text with "..." if it exceeds width
    std::string truncateText(const std::string& text, float maxWidth);
};

#endif // TEXTFIELD_HPP