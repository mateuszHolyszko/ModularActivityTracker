#ifndef LABEL_HPP
#define LABEL_HPP

#include "BaseElement.hpp"
#include "../Style.hpp"
#include "../RenderContext.hpp"

class Label : public BaseElement {
private:
    std::string text;
    std::string displayedText;  // The actual text to display (wrapped if needed)
    bool showBorder;
    int fontSize;
    bool hovered;
    bool wrapText;  // Whether to wrap text that's too long

public:
    Label(RenderContext* context,
          int x, int y, int width, int height, 
          const std::string& text, 
          bool showBorder = false,
          int fontSize = 16,
          int layer = 0, 
          BaseElement* parent = nullptr);
    
    void render() override;
    void handleEvent(const SDL_Event& event) override;
    
    // Text management
    void setText(const std::string& newText);
    const std::string& getText() const { return text; }
    const std::string& getDisplayedText() const { return displayedText; }
    
    // Border management
    void setShowBorder(bool show) { showBorder = show; }
    bool isShowingBorder() const { return showBorder; }
    
    // Font size management
    void setFontSize(int size);
    int getFontSize() const { return fontSize; }
    
    // Wrapping management
    void setWrapText(bool wrap);
    bool isWrapText() const { return wrapText; }
    
    // Selectable management
    void setSelectable(bool selectable);
    
    // Hover state
    bool isHovered() const { return hovered; }

private:
    void updateDisplayedText();  // Update the displayed text based on wrapping rules
    bool needsWrapping() const;  // Check if text needs to be wrapped
    std::string wrapTextToFit() const;  // Apply wrapping algorithm
    
    // Convert SDL_Color to glm::vec4
    glm::vec4 colorToVec4(const SDL_Color& color) const {
        return glm::vec4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    }
};

#endif