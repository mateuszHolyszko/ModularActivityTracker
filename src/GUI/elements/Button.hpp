#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "BaseElement.hpp"
#include "../../Style.hpp"
#include "../../RenderContext.hpp"
#include "../TextField.hpp"
#include "../Menu.hpp"
#include <functional>

class Button : public BaseElement {
private:
    std::string text;
    TextField* textField;  // Use pointer since we need dynamic initialization
    bool showBorder;
    int fontSize;
    bool hovered;
    bool wrapText;
    bool is_activated;
    std::function<void()> onPressCallback;

public:
    Button(RenderContext* context,
           int x, int y, int width, int height, 
           const std::string& text, 
           bool showBorder = false,
           int fontSize = 16,
           int layer = 0, 
           Menu* parent = nullptr,
           HorizontalAlignment textAlign = LEFT);
    
    ~Button();  // Add destructor to clean up TextField
    
    void render() override;
    bool handleEvent(const SDL_Event& event) override;
    
    // Text management
    void setText(const std::string& newText);
    const std::string& getText() const { return text; }
    
    // Border management
    void setShowBorder(bool show) { showBorder = show; }
    bool isShowingBorder() const { return showBorder; }
    
    // Font size management
    void setFontSize(int size);
    int getFontSize() const { return fontSize; }
    
    // Wrapping management
    void setWrapText(bool wrap);
    bool isWrapText() const { return wrapText; }
    
    // Callback management
    void setOnPress(std::function<void()> callback) { onPressCallback = callback; }
    
    // Hover state
    bool isHovered() const { return hovered; }

    bool isActivated() const { return is_activated; }
    void setActivated(bool activated) { is_activated = activated; }

private:
    void onPress();
    void updateTextField();
    
    // Convert SDL_Color to glm::vec4
    glm::vec4 colorToVec4(const SDL_Color& color) const {
        return glm::vec4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    }
};

#endif