#pragma once
#include "../../RenderContext.hpp"
#include <string>

class LoadingWidget {
public:
    enum class Style {
        DOTS_PULSE,      // Pulsing dots (default)
        DOTS_ROTATING,   // Rotating dot circle
        BAR,            // Progress bar
        SPINNER         // Circular spinner
    };

    LoadingWidget();
    
    void update(float dt);
    void render(RenderContext& ctx);
    
    void setPosition(float x, float y) { posX = x; posY = y; }
    void setSize(float size) { this->size = size; }
    void setColor(const glm::vec4& color) { this->color = color; }
    void setVisible(bool visible) { this->visible = visible; }
    void setStyle(Style style) { this->style = style; }
    void setProgress(float progress) { this->progress = progress; } // For progress bar
    
private:
    float posX = 0.0f, posY = 0.0f;
    float size = 40.0f; // Base size for the widget
    glm::vec4 color = glm::vec4(0.2f, 0.4f, 0.8f, 1.0f);
    
    float animationTime = 0.0f;
    bool visible = true;
    Style style = Style::DOTS_PULSE;
    float progress = 0.0f; // For progress bar style
    
    void renderDotsPulse(RenderContext& ctx);
    void renderDotsRotating(RenderContext& ctx);
    void renderProgressBar(RenderContext& ctx);
    void renderSpinner(RenderContext& ctx);
};