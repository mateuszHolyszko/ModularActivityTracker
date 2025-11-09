#include <iostream>
#include <algorithm>
#include "Button.hpp"
#include "../TextRenderer.hpp"
#include "../SimplesRenderer.hpp"
#include "../RenderContext.hpp"

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

    // Draw background
    SDL_SetRenderDrawColor(renderContext->sdlRenderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_Rect bgRect = {x, y, width, height};
    SDL_RenderFillRect(renderContext->sdlRenderer, &bgRect);

    // Draw border if enabled
    if (showBorder) {
        SDL_Color borderColor = style.getBorderColor();
        renderContext->simpleRenderer->DrawThickRect(x, y, width, height, 1, borderColor);
    }

    // Draw text
    SDL_Color textColor = style.getTextColor();
    
    // Calculate text position (centered within the button)
    int textX = x + 5;  // Small padding from left edge
    int textY = y + (height - fontSize) / 2;  // Vertically centered
    
    try {
        renderContext->textRenderer->RenderTextToRenderer(
            renderContext->sdlRenderer, 
            displayedText,  // Use the wrapped text
            textX, 
            textY, 
            fontSize, 
            textColor
        );
    } catch (const std::exception& e) {
        std::cerr << "Button text render error: " << e.what() << std::endl;
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
    // Default implementation: print text field and center position
    int centerX = x + width / 2;
    int centerY = y + height / 2;
    std::cerr << "Button pressed - Text: '" << text 
              << "', Center Position: (" << centerX << ", " << centerY << ")" << std::endl;
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
    
    // Rough estimation: average character width is about 0.7 * font size
    // This is a simple approximation - for exact measurement you'd need text rendering metrics
    double estimatedTextWidth = text.length() * fontSize * 0.7;
    double availableWidth = width - 10;  // Account for padding
    
    return estimatedTextWidth > availableWidth;
}

std::string Button::wrapTextToFit() const {
    if (text.empty()) return text;
    
    // Calculate how many characters can fit
    double availableWidth = width - 10;  // Account for padding
    int maxChars = static_cast<int>(availableWidth / (fontSize * 0.6));
    
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