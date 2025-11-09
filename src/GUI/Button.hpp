#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "BaseElement.hpp"
#include "../Style.hpp"

class Button : public BaseElement {
private:
    std::string text;
    std::string displayedText;  // The actual text to display (wrapped if needed)
    bool showBorder;
    int fontSize;
    bool hovered;
    bool wrapText;  // Whether to wrap text that's too long

public:
    Button(RenderContext* context,
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
    
    // Hover state
    bool isHovered() const { return hovered; }

protected:
    // Default onPress implementation that prints text field and center position
    virtual void onPress();

private:
    void updateDisplayedText();  // Update the displayed text based on wrapping rules
    bool needsWrapping() const;  // Check if text needs to be wrapped
    std::string wrapTextToFit() const;  // Apply wrapping algorithm
};

#endif