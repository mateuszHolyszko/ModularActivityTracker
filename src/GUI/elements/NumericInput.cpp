#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "NumericInput.hpp"

NumericInput::NumericInput(RenderContext* context,
                         int x, int y, int width, int height, 
                         const std::string& prompt, 
                         float initialValue,
                         float minVal,
                         float maxVal,
                         float stepValue,
                         bool showBorder,
                         int fontSize,
                         int layer, 
                         Menu* parent,
                         HorizontalAlignment promptAlign,
                         HorizontalAlignment valueAlign)
    : BaseElement(context, x, y, width, height, true, layer, parent),
      prompt(prompt), showBorder(showBorder), fontSize(fontSize), 
      hovered(false), wrapText(true),
      isActive(false), minVal(minVal), maxVal(maxVal), value(initialValue), stepValue(stepValue) {
    
    // Clamp initial value to range
    clampValue();
    updateValueString();
    
    // Calculate areas: prompt on top (1/3 height), value on bottom (2/3 height)
    int promptHeight = height / 3;
    int valueHeight = height - promptHeight;
    
    // Calculate text positions
    int promptX = x + 5;
    int valueX = x + 5;
    int promptY = y + (promptHeight - fontSize/2) / 2;
    int valueY = y + promptHeight + (valueHeight - fontSize) / 2;
    
    // Create prompt TextField
    promptTextField = new TextField(
        renderContext,
        static_cast<float>(promptX), 
        static_cast<float>(promptY),
        static_cast<float>(width - 10), 
        static_cast<float>(promptHeight),
        prompt, 
        "",
        static_cast<float>(fontSize/2),  // Half font size
        colorToVec4(style.getTextColor()),
        promptAlign,
        true
    );

    valueTextField = new TextField(
        renderContext,
        static_cast<float>(valueX), 
        static_cast<float>(valueY),
        static_cast<float>(width - 10), 
        static_cast<float>(valueHeight),
        valueString, 
        "",
        static_cast<float>(fontSize),  // Full font size
        colorToVec4(style.getTextColor()),
        valueAlign,
        false
    );
}

NumericInput::~NumericInput() {
    delete promptTextField;
    delete valueTextField;
}

void NumericInput::render() {
    if (!visible) return;

    // Update TextFields based on current state
    updateTextFields();

    // Determine background color based on state
    SDL_Color bgColor;
    if (!enabled) {
        bgColor = style.getBgColorNotSelectable();
    } else if (is_selected || isActive) {
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
        borderCmd.layer = layer;
        borderCmd.lineWidth = 1.0f;
        borderCmd.filled = false;
        
        renderContext->graphicQueue.push_back(borderCmd);
    }

    // Draw horizontal line separating prompt from value
    int valueHeight = (height * 2) / 3;
    int promptHeight = height - valueHeight;
    int lineY = y + promptHeight;
    
    GraphicCommand lineCmd;
    lineCmd.type = GraphicCommand::LINE;
    lineCmd.x1 = static_cast<float>(x);
    lineCmd.y1 = static_cast<float>(lineY);
    lineCmd.x2 = static_cast<float>(x + width);
    lineCmd.y2 = static_cast<float>(lineY);
    lineCmd.color = colorToVec4(style.getBorderColor());
    lineCmd.layer = layer + 1;
    lineCmd.lineWidth = 1.0f;
    lineCmd.filled = false;
    renderContext->graphicQueue.push_back(lineCmd);

    // Draw text using TextFields
    auto promptCommands = promptTextField->render(layer + 2);
    for (const auto& textCmd : promptCommands) {
        renderContext->textQueue.push_back(textCmd);
    }
    
    auto valueCommands = valueTextField->render(layer + 2);
    for (const auto& textCmd : valueCommands) {
        renderContext->textQueue.push_back(textCmd);
    }

    // Draw arrows on the right side when active
    if (isActive) {
        int arrowAreaWidth = width / 6; // Smaller area for arrows
        int valueAreaWidth = (width * 1) / 3; // Value area width
        int arrowX = x + width - arrowAreaWidth;
        
        // Draw up arrow (increment)
        int upArrowCenterX = arrowX + (arrowAreaWidth / 2);
        int upArrowCenterY = y + promptHeight + (valueHeight / 3);
        float arrowSize = std::min(arrowAreaWidth, height) * 0.3f;
        
        GraphicCommand upArrowCmd;
        upArrowCmd.type = GraphicCommand::ARROW;
        upArrowCmd.x1 = static_cast<float>(upArrowCenterX);
        upArrowCmd.y1 = static_cast<float>(upArrowCenterY);
        upArrowCmd.x2 = arrowSize;
        upArrowCmd.direction = 0; // Up
        upArrowCmd.color = colorToVec4(style.getTextColor());
        upArrowCmd.layer = layer + 2;
        upArrowCmd.lineWidth = 1.0f;
        upArrowCmd.filled = true;
        renderContext->graphicQueue.push_back(upArrowCmd);

        // Draw down arrow (decrement)
        int downArrowCenterX = upArrowCenterX;
        int downArrowCenterY = y + promptHeight + (2 * valueHeight / 3);
        
        GraphicCommand downArrowCmd;
        downArrowCmd.type = GraphicCommand::ARROW;
        downArrowCmd.x1 = static_cast<float>(downArrowCenterX);
        downArrowCmd.y1 = static_cast<float>(downArrowCenterY);
        downArrowCmd.x2 = arrowSize;
        downArrowCmd.direction = 2; // Down
        downArrowCmd.color = colorToVec4(style.getTextColor());
        downArrowCmd.layer = layer + 2;
        downArrowCmd.lineWidth = 1.0f;
        downArrowCmd.filled = true;
        renderContext->graphicQueue.push_back(downArrowCmd);
    }
}

bool NumericInput::handleEvent(const SDL_Event& event) {
    if (!enabled || !visible) return false;

    bool eventConsumed = false;

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
                updateTextFields();
            }
            break;
        }
            
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT && containsPoint(event.button.x, event.button.y)) {
                // Set this element as focus in the parent menu instead of calling onPress
                if (parent && selectable) {
                    parent->setFocus(this);
                    eventConsumed = true; // Mouse click is consumed
                }
            }
            break;

        case SDL_KEYDOWN:
            if (isActive) {
                // When numeric input is active, consume all navigation events
                switch (event.key.keysym.sym) {
                    case SDLK_RIGHT:
                    case SDLK_PERIOD:
                        incrementValue();
                        eventConsumed = true;
                        break;
                    case SDLK_LEFT:
                    case SDLK_COMMA:
                        decrementValue();
                        eventConsumed = true;
                        break;
                    case SDLK_RETURN:
                    case SDLK_ESCAPE:
                        setIsActive(false);
                        eventConsumed = true;
                        break;
                    default:
                        // Consume all other keys when active to prevent external navigation
                        eventConsumed = true;
                        break;
                }
            } else {
                if (event.key.keysym.sym == SDLK_RETURN && is_selected) {
                    onPress();
                    eventConsumed = true; // Enter key is consumed
                }
            }
            break;
    }

    return eventConsumed;
}

void NumericInput::onPress() {
    // Toggle active state when focused
    if (is_selected) {
        setIsActive(!isActive);
    }
    
    // Call the callback if set
    if (onPressCallback) {
        onPressCallback();
    } else {
        std::cout << "NumericInput pressed - Value: " << value 
                  << ", Active: " << (isActive ? "true" : "false") << std::endl;
    }
}

void NumericInput::setValue(float newValue) {
    value = newValue;
    clampValue();
    updateValueString();
    updateTextFields();
}

void NumericInput::setRange(float newMin, float newMax) {
    minVal = newMin;
    maxVal = newMax;
    clampValue();
    updateValueString();
    updateTextFields();
}

void NumericInput::setPrompt(const std::string& newPrompt) {
    prompt = newPrompt;
    updateTextFields();
}

void NumericInput::setFontSize(int size) {
    fontSize = size;
    updateTextFields();
}

void NumericInput::setWrapText(bool wrap) {
    wrapText = wrap;
    updateTextFields();
}

void NumericInput::setIsActive(bool active) {
    isActive = active;
    updateTextFields();
}

void NumericInput::incrementValue() {
    value += stepValue;
    clampValue();
    updateValueString();
    updateTextFields();
}

void NumericInput::decrementValue() {
    value -= stepValue;
    clampValue();
    updateValueString();
    updateTextFields();
}

void NumericInput::clampValue() {
    if (value < minVal) value = minVal;
    if (value > maxVal) value = maxVal;
}

void NumericInput::updateValueString() {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << value;
    valueString = ss.str();
}

void NumericInput::updateTextFields() {
    // Calculate areas: prompt on top (1/3 height), value on bottom (2/3 height)
    int promptHeight = height / 3;
    int valueHeight = height - promptHeight;
    
    // Calculate text positions
    int promptX = x + 5;
    int valueX = x + 5;
    int promptY = y + (promptHeight - fontSize/2) / 2;
    int valueY = y + promptHeight + (valueHeight - fontSize) / 2;
    
    // Update prompt TextField with half font size
    promptTextField->setPosition(static_cast<float>(promptX), static_cast<float>(promptY));
    promptTextField->setSize(static_cast<float>(width - 10), static_cast<float>(promptHeight));
    promptTextField->setText(prompt);
    promptTextField->setFontSize(static_cast<float>(fontSize/2));  
    
    // Update value TextField
    valueTextField->setPosition(static_cast<float>(valueX), static_cast<float>(valueY));
    valueTextField->setSize(static_cast<float>(width - 15), static_cast<float>(valueHeight)); // Smaller to account for arrows
    valueTextField->setText(valueString);
    valueTextField->setFontSize(static_cast<float>(fontSize)); 
    
    // Determine if wrapping should be enabled
    bool shouldWrap = wrapText && !(is_selected || hovered || isActive);
    promptTextField->setWrap(shouldWrap);
    valueTextField->setWrap(false); // Never wrap the value
}