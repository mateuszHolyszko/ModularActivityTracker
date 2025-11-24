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
                         HorizontalAlignment valueAlign,
                         bool vertical)
    : BaseElement(context, x, y, width, height, true, layer, parent),
      prompt(prompt), showBorder(showBorder), fontSize(fontSize), 
      hovered(false), wrapText(true), vertical(vertical),
      isActive(false), minVal(minVal), maxVal(maxVal), value(initialValue), stepValue(stepValue) {
    
    clampValue();
    updateValueString();
    
    // Initial positions will be set by updateTextFields based on vertical mode
    promptTextField = new TextField(
        renderContext,
        0, 0, 0, 0,
        prompt, "",
        static_cast<float>(fontSize/2),
        colorToVec4(style.getTextColor()),
        promptAlign,
        true
    );

    valueTextField = new TextField(
        renderContext,
        0, 0, 0, 0,
        valueString, "",
        static_cast<float>(fontSize),
        colorToVec4(style.getTextColor()),
        valueAlign,
        false
    );
    
    updateTextFields();
}

NumericInput::~NumericInput() {
    delete promptTextField;
    delete valueTextField;
}

void NumericInput::render() {
    if (!visible) return;
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

    // Draw background
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

    if (vertical) {
        renderVertical();
    } else {
        renderHorizontal();
    }

    // Draw text using TextFields
    auto promptCommands = promptTextField->render(layer + 2);
    for (const auto& textCmd : promptCommands) {
        renderContext->textQueue.push_back(textCmd);
    }
    
    auto valueCommands = valueTextField->render(layer + 2);
    for (const auto& textCmd : valueCommands) {
        renderContext->textQueue.push_back(textCmd);
    }
}

void NumericInput::renderVertical() {
    int valueHeight = (height * 2) / 3;
    int promptHeight = height - valueHeight;
    int lineY = y + promptHeight;
    
    // Draw horizontal line separating prompt from value
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

    // Draw arrows on the right side when active
    if (isActive) {
        int arrowAreaWidth = width / 6;
        int arrowX = x + width - arrowAreaWidth;
        
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

void NumericInput::renderHorizontal() {
    // Layout: [Prompt (50%)] | [Value (35%)] | [Arrows (15%)]
    int promptWidth = width / 2;
    int arrowAreaWidth = width * 15 / 100;
    int valueWidth = width - promptWidth - arrowAreaWidth;
    int lineX = x + promptWidth;
    
    // Draw vertical line separating prompt from value
    GraphicCommand lineCmd;
    lineCmd.type = GraphicCommand::LINE;
    lineCmd.x1 = static_cast<float>(lineX);
    lineCmd.y1 = static_cast<float>(y);
    lineCmd.x2 = static_cast<float>(lineX);
    lineCmd.y2 = static_cast<float>(y + height);
    lineCmd.color = colorToVec4(style.getBorderColor());
    lineCmd.layer = layer + 1;
    lineCmd.lineWidth = 1.0f;
    lineCmd.filled = false;
    renderContext->graphicQueue.push_back(lineCmd);

    // Draw arrows on the right side when active
    if (isActive) {
        int arrowX = x + promptWidth + valueWidth;
        int arrowCenterX = arrowX + (arrowAreaWidth / 2);
        float arrowSize = std::min(arrowAreaWidth, height) * 0.3f;
        
        // Up arrow (top half)
        int upArrowCenterY = y + (height / 4);
        GraphicCommand upArrowCmd;
        upArrowCmd.type = GraphicCommand::ARROW;
        upArrowCmd.x1 = static_cast<float>(arrowCenterX);
        upArrowCmd.y1 = static_cast<float>(upArrowCenterY);
        upArrowCmd.x2 = arrowSize;
        upArrowCmd.direction = 0; // Up
        upArrowCmd.color = colorToVec4(style.getTextColor());
        upArrowCmd.layer = layer + 2;
        upArrowCmd.lineWidth = 1.0f;
        upArrowCmd.filled = true;
        renderContext->graphicQueue.push_back(upArrowCmd);

        // Down arrow (bottom half)
        int downArrowCenterY = y + (3 * height / 4);
        GraphicCommand downArrowCmd;
        downArrowCmd.type = GraphicCommand::ARROW;
        downArrowCmd.x1 = static_cast<float>(arrowCenterX);
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
            bool wasHovered = hovered;
            hovered = containsPoint(event.motion.x, event.motion.y);
            if (hovered && !wasHovered && onHover) {
                onHover();
            }
            if (hovered != wasHovered) {
                updateTextFields();
            }
            break;
        }
            
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT && containsPoint(event.button.x, event.button.y)) {
                if (parent && selectable) {
                    parent->setFocus(this);
                    eventConsumed = true;
                }
            }
            break;

        case SDL_KEYDOWN:
            if (isActive) {
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
                        eventConsumed = true;
                        break;
                }
            } else {
                if (event.key.keysym.sym == SDLK_RETURN && is_selected) {
                    onPress();
                    eventConsumed = true;
                }
            }
            break;
    }

    return eventConsumed;
}

void NumericInput::onPress() {
    if (is_selected) {
        setIsActive(!isActive);
    }
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
    
    // Call the input callback if value changed
    if (onInputCallback) {
        onInputCallback(value);
    }
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
    float oldValue = value;
    value += stepValue;
    clampValue();
    updateValueString();
    updateTextFields();
    
    // Call the input callback if value actually changed
    if (oldValue != value && onInputCallback) {
        onInputCallback(value);
    }
}

void NumericInput::decrementValue() {
    float oldValue = value;
    value -= stepValue;
    clampValue();
    updateValueString();
    updateTextFields();
    
    // Call the input callback if value actually changed
    if (oldValue != value && onInputCallback) {
        onInputCallback(value);
    }
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
    if (vertical) {
        // Vertical layout: prompt on top (1/3 height), value on bottom (2/3 height)
        int promptHeight = height / 3;
        int valueHeight = height - promptHeight;
        
        int promptX = x + 5;
        int valueX = x + 5;
        int promptY = y + (promptHeight - fontSize/2) / 2;
        int valueY = y + promptHeight + (valueHeight - fontSize) / 2;
        
        promptTextField->setPosition(static_cast<float>(promptX), static_cast<float>(promptY));
        promptTextField->setSize(static_cast<float>(width - 10), static_cast<float>(promptHeight));
        promptTextField->setText(prompt);
        promptTextField->setFontSize(static_cast<float>(fontSize/2));
        
        valueTextField->setPosition(static_cast<float>(valueX), static_cast<float>(valueY));
        valueTextField->setSize(static_cast<float>(width - 15), static_cast<float>(valueHeight));
        valueTextField->setText(valueString);
        valueTextField->setFontSize(static_cast<float>(fontSize));
    } else {
        // Horizontal layout: [Prompt (50%)] | [Value (35%)] | [Arrows (15%)]
        int promptWidth = width / 2;
        int arrowAreaWidth = width * 15 / 100;
        int valueWidth = width - promptWidth - arrowAreaWidth;
        
        int promptX = x + 5;
        int valueX = x + promptWidth + 5;
        int textY = y + (height - fontSize) / 2;
        
        promptTextField->setPosition(static_cast<float>(promptX), static_cast<float>(textY));
        promptTextField->setSize(static_cast<float>(promptWidth - 10), static_cast<float>(height));
        promptTextField->setText(prompt);
        promptTextField->setFontSize(static_cast<float>(fontSize));
        
        valueTextField->setPosition(static_cast<float>(valueX), static_cast<float>(textY));
        valueTextField->setSize(static_cast<float>(valueWidth - 10), static_cast<float>(height));
        valueTextField->setText(valueString);
        valueTextField->setFontSize(static_cast<float>(fontSize));
    }
    
    bool shouldWrap = wrapText && !(is_selected || hovered || isActive);
    promptTextField->setWrap(shouldWrap);
    valueTextField->setWrap(false);
}