#include <iostream>
#include <algorithm>
#include "Label.hpp"

Label::Label(RenderContext* context,
             int x, int y, int width, int height,
             const std::string& text,
             bool showBorder,
             int fontSize,
             int layer,
             BaseElement* parent,
             HorizontalAlignment textAlign)
    : BaseElement(context, x, y, width, height, false, layer, parent),  // false for unselectable
      text(text), showBorder(showBorder), fontSize(fontSize),
      hovered(false), wrapText(true) {

    // Calculate text position
    int textX = x + 5;
    int textY = y + (height - fontSize) / 2;

    // Create TextField with alignment
    textField = new TextField(
        renderContext,
        static_cast<float>(textX),
        static_cast<float>(textY + fontSize),
        static_cast<float>(width - 10),
        static_cast<float>(fontSize),
        text,
        "",
        static_cast<float>(fontSize),
        colorToVec4(style.getTextColor()),
        textAlign,
        false
    );
}

Label::~Label() {
    delete textField;  // Clean up the TextField
}

void Label::render() {
    if (!visible) return;

    // Update TextField based on current state
    updateTextField();

    // Determine background color based on state (unselectable, so use not selectable or default)
    SDL_Color bgColor;
    if (hovered) {
        bgColor = style.getHighlightColor();  // Allow hover highlight for visual feedback
    } else {
        bgColor = style.getBgColor();  // Default background
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
        borderCmd.layer = layer;
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

void Label::handleEvent(const SDL_Event& event) {
    if (!visible) return;  // No check for enabled since it's unselectable, but still handle hover

    switch (event.type) {
        case SDL_MOUSEMOTION: {
            // Update hover state
            bool wasHovered = hovered;
            hovered = containsPoint(event.motion.x, event.motion.y);

            // If hover state changed, update text field wrapping
            if (hovered != wasHovered) {
                updateTextField();
            }
            break;
        }
        // No handling for SDL_MOUSEBUTTONDOWN or SDL_KEYDOWN (no press functionality)
    }
}

void Label::setText(const std::string& newText) {
    text = newText;
    updateTextField();
}

void Label::setFontSize(int size) {
    fontSize = size;
    updateTextField();
}

void Label::setWrapText(bool wrap) {
    wrapText = wrap;
    updateTextField();
}

void Label::updateTextField() {
    // Update TextField position and text based on current state
    int textX = x + 5;
    int textY = y + (height - fontSize) / 2;

    textField->setPosition(static_cast<float>(textX), static_cast<float>(textY + fontSize));
    textField->setSize(static_cast<float>(width - 10), static_cast<float>(fontSize));

    // Update text content
    textField->setText(text);

    // Determine if wrapping should be enabled based on label state
    // Disable wrapping if hovered, otherwise use wrapText setting
    bool shouldWrap = wrapText && !hovered;
    textField->setWrap(shouldWrap);
}