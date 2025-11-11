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
    : BaseElement(context, x, y, width, height, true, layer, parent),  // selectable = true by default
      text(text), showBorder(showBorder), fontSize(fontSize), 
      hovered(false), wrapText(true) {
    updateDisplayedText();
}

void Button::render() {
    if (!visible) return;

    // Update displayed text based on current state (including selection)
    updateDisplayedText();

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
        borderCmd.layer = layer + 1;  // Draw border on top of background
        borderCmd.lineWidth = 1.0f;
        borderCmd.filled = false;
        
        renderContext->graphicQueue.push_back(borderCmd);
    }

    // Draw text
    SDL_Color textColor = style.getTextColor();
    
    // Calculate text position (centered within the button)
    int textX = x + 5;  // Small padding from left edge
    int textY = y + (height - fontSize) / 2;  // Vertically centered
    
    TextCommand textCmd;
    textCmd.text = displayedText;
    textCmd.font = "";  // Use default font
    textCmd.x = static_cast<float>(textX);
    textCmd.y = static_cast<float>(textY + fontSize);  // NanoVG y is baseline, so adjust
    textCmd.width = 0;
    textCmd.height = 0;
    textCmd.color = colorToVec4(textColor);
    textCmd.layer = layer + 2;  // Draw text on top of everything
    textCmd.fontSize = static_cast<float>(fontSize);
    
    renderContext->textQueue.push_back(textCmd);
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
    updateDisplayedText();
}

void Button::setFontSize(int size) {
    fontSize = size;
    updateDisplayedText();
}

void Button::setWrapText(bool wrap) {
    wrapText = wrap;
    updateDisplayedText();
}

void Button::updateDisplayedText() {
    // If element is focused or wrapping is disabled, show full text
    if (is_selected || !wrapText) {
        displayedText = text;
        return;
    }
    
    // Check if text needs wrapping
    if (needsWrapping()) {
        displayedText = wrapTextToFit();
    } else {
        displayedText = text;
    }
}

bool Button::needsWrapping() const {
    if (text.empty()) return false;
    
    // Rough estimation: average character width is about 0.65 * font size
    // This is a simple approximation - for exact measurement you'd need text rendering metrics
    double estimatedTextWidth = text.length() * fontSize * 0.4;
    double availableWidth = width - 10;  // Account for padding
    
    return estimatedTextWidth > availableWidth;
}

std::string Button::wrapTextToFit() const {
    if (text.empty()) return text;
    
    // Calculate how many characters can fit
    double availableWidth = width - 10;  // Account for padding
    int maxChars = static_cast<int>(availableWidth / (fontSize * 0.4));
    
    // Ensure we have at least 3 characters for "..." + some text
    if (maxChars <= 3) {
        return "...";
    }
    
    // Calculate how many characters to show before adding "..."
    int charsToShow = maxChars - 3;  // Reserve 3 characters for "..."
    
    if (charsToShow >= static_cast<int>(text.length())) {
        return text;  // Text actually fits, no need to wrap
    }
    
    // Return the beginning of the text with "..."
    return text.substr(0, charsToShow) + "...";
}