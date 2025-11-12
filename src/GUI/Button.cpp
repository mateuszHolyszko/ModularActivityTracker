#include <iostream>
#include <algorithm>
#include "Button.hpp"

Button::Button(RenderContext* context,
             int x, int y, int width, int height, 
             const std::string& text, 
             bool showBorder,
             int fontSize,
             int layer, 
             BaseElement* parent)
    : BaseElement(context, x, y, width, height, true, layer, parent),
      text(text), showBorder(showBorder), fontSize(fontSize), 
      hovered(false), wrapText(true) {
    
    // Calculate text position
    int textX = x + 5;
    int textY = y + (height - fontSize) / 2;
    
    // Create TextField - now just pass the renderContext
    textField = new TextField(
        renderContext,  // Simplified - just pass the context
        static_cast<float>(textX), 
        static_cast<float>(textY + fontSize),
        static_cast<float>(width - 10), 
        static_cast<float>(fontSize),
        text, 
        "",
        static_cast<float>(fontSize),
        colorToVec4(style.getTextColor()),
        LEFT,
        false
    );
}

Button::~Button() {
    delete textField;  // Clean up the TextField
}

void Button::render() {
    if (!visible) return;

    // Update TextField based on current state
    updateTextField();

    // Determine background color based on state
    SDL_Color bgColor;
    if (!enabled) {
        bgColor = style.getBgColorNotSelectable();
    } else if (is_selected) {
        bgColor = style.getActiveBgColor();
    } else if (hovered) {
        bgColor = style.getHighlightColor();
    } else {
        bgColor = style.getLgBgColor();
    }

    // Draw background using graphic command
    GraphicCommand bgCmd;
    bgCmd.type = GraphicCommand::BOX;
    bgCmd.x1 = static_cast<float>(x);
    bgCmd.y1 = static_cast<float>(y);
    bgCmd.x2 = static_cast<float>(x + width);
    bgCmd.y2 = static_cast<float>(y + height);
    bgCmd.color = colorToVec4(bgColor);
    bgCmd.layer = layer;
    bgCmd.lineWidth = 0.0f;
    bgCmd.filled = true;
    
    renderContext->graphicQueue.push_back(bgCmd);

    // Draw border if enabled
    if (showBorder) {
        SDL_Color borderColor = style.getBorderColor();
        
        GraphicCommand borderCmd;
        borderCmd.type = GraphicCommand::BOX;
        borderCmd.x1 = static_cast<float>(x);
        borderCmd.y1 = static_cast<float>(y);
        borderCmd.x2 = static_cast<float>(x + width);
        borderCmd.y2 = static_cast<float>(y + height);
        borderCmd.color = colorToVec4(borderColor);
        borderCmd.layer = layer + 1;
        borderCmd.lineWidth = 1.0f;
        borderCmd.filled = false;
        
        renderContext->graphicQueue.push_back(borderCmd);
    }

    // Draw text using TextField
    auto textCommands = textField->render(layer + 2);
    for (const auto& textCmd : textCommands) {
        renderContext->textQueue.push_back(textCmd);
    }
}

void Button::handleEvent(const SDL_Event& event) {
    if (!enabled || !visible) return;

    switch (event.type) {
        case SDL_MOUSEMOTION: {
            // Update hover state
            bool wasHovered = hovered;
            hovered = containsPoint(event.motion.x, event.motion.y);
            
            // Trigger hover callback if state changed
            if (hovered && !wasHovered && onHover) {
                onHover();
            }
            
            // If hover state changed, we need to update text field wrapping
            if (hovered != wasHovered) {
                updateTextField();
            }
            break;
        }
            
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT && containsPoint(event.button.x, event.button.y)) {
                onPress();
            }
            break;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_RETURN && is_selected) {
                onPress();
            }
            break;
    }
}

void Button::onPress() {
    // Call the callback if set
    if (onPressCallback) {
        onPressCallback();
    } else {
        // Default implementation: print text field and center position
        int centerX = x + width / 2;
        int centerY = y + height / 2;
        std::cout << "Button pressed - Text: '" << text 
                  << "', Center Position: (" << centerX << ", " << centerY << ")" << std::endl;
    }
}

void Button::setText(const std::string& newText) {
    text = newText;
    updateTextField();  // Use updateTextField to ensure proper wrapping state
}

void Button::setFontSize(int size) {
    fontSize = size;
    updateTextField();  // Use updateTextField to ensure proper wrapping state
}

void Button::setWrapText(bool wrap) {
    wrapText = wrap;
    updateTextField();  // Use updateTextField to ensure proper wrapping state
}

void Button::updateTextField() {
    // Update TextField position and text based on current state
    int textX = x + 5;
    int textY = y + (height - fontSize) / 2;
    
    textField->setPosition(static_cast<float>(textX), static_cast<float>(textY + fontSize));
    textField->setSize(static_cast<float>(width - 10), static_cast<float>(fontSize));
    
    // Update text content
    textField->setText(text);
    
    // Determine if wrapping should be enabled based on button state
    // Disable wrapping if button is selected or hovered, otherwise use wrapText setting
    bool shouldWrap = wrapText && !(is_selected || hovered);
    textField->setWrap(shouldWrap);
}