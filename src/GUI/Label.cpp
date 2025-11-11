#include <iostream>
#include <algorithm>
#include "Label.hpp"

Label::Label(RenderContext* context,
             int x, int y, int width, int height, 
             const std::string& text, 
             bool showBorder,
             int fontSize,
             int layer, 
             BaseElement* parent)
    : BaseElement(context, x, y, width, height, false, layer, parent),
      text(text), showBorder(showBorder), fontSize(fontSize), 
      hovered(false), wrapText(true) {
    updateDisplayedText();
}

void Label::render() {
    if (!visible) return;

    // Update displayed text based on current state (including selection)
    updateDisplayedText();

    // Only draw background if the label is selectable, selected, or hovered
    // Otherwise, it will be transparent (same as parent background)
    if (selectable || is_selected || hovered) {
        SDL_Color bgColor;
        if (!enabled) {
            bgColor = style.getBgColorNotSelectable();
        } else if (is_selected) {
            bgColor = style.getActiveBgColor();
        } else if (hovered && selectable) {
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
        
        renderContext->graphicQueue.push_back(bgCmd);  // Fixed: use renderContext
    }

    // Draw border only if explicitly enabled
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
        
        renderContext->graphicQueue.push_back(borderCmd);  // Fixed: use renderContext
    }

    // Draw text
    SDL_Color textColor = style.getTextColor();
    
    // Calculate text position (centered within the label)
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
    
    renderContext->textQueue.push_back(textCmd);  // Fixed: use renderContext
}

void Label::handleEvent(const SDL_Event& event) {
    if (!enabled || !visible) return;

    switch (event.type) {
        case SDL_MOUSEMOTION: {
            // Update hover state
            bool wasHovered = hovered;
            hovered = containsPoint(event.motion.x, event.motion.y);
            
            // Trigger hover callback if state changed
            if (hovered && !wasHovered && onHover && selectable) {
                onHover();
            }
            break;
        }
            
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT && containsPoint(event.button.x, event.button.y)) {
                if (onPress) onPress();
            }
            break;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_RETURN && is_selected) {
                if (onPress) onPress();
            }
            break;
    }
}

void Label::setText(const std::string& newText) {
    text = newText;
    updateDisplayedText();
}

void Label::setFontSize(int size) {
    fontSize = size;
    updateDisplayedText();
}

void Label::setWrapText(bool wrap) {
    wrapText = wrap;
    updateDisplayedText();
}

void Label::setSelectable(bool selectable) {
    this->selectable = selectable;
}

void Label::updateDisplayedText() {
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

bool Label::needsWrapping() const {
    if (text.empty()) return false;
    
    // Rough estimation: average character width is about 0.65 * font size
    // This is a simple approximation - for exact measurement you'd need text rendering metrics
    double estimatedTextWidth = text.length() * fontSize * 0.4;
    double availableWidth = width - 10;  // Account for padding
    
    return estimatedTextWidth > availableWidth;
}

std::string Label::wrapTextToFit() const {
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