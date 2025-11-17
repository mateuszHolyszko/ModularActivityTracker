#include "RenderContext.hpp"
#include "SimpleRenderer.hpp"
#include "GUI/NotificationSystem.hpp" 
#include <iostream>

float RenderContext::measureTextWidth(const std::string& text, const std::string& font, float fontSize) {
    if (!renderer) {
        std::cerr << "[RenderContext] measureTextWidth: Renderer is null!\n";
        return 0.0f;
    }
    return renderer->measureTextWidth(text, font, fontSize);
}
float RenderContext::measureTextHeight(const std::string& text, const std::string& font, float fontSize) {
    if (!renderer) {
        std::cerr << "[RenderContext] measureTextHeight: Renderer is null!\n";
        return 0.0f;
    }
    return renderer->measureTextHeight(text, font, fontSize);
}

void RenderContext::addNotification(const std::string& text) {
    if (notificationSystem) {
        notificationSystem->addNotification(text);
    }
}