#ifndef NUMERICINPUT_HPP
#define NUMERICINPUT_HPP

#include "BaseElement.hpp"
#include "../../Style.hpp"
#include "../../RenderContext.hpp"
#include "../TextField.hpp"
#include "../Menu.hpp"
#include <functional>
#include <string>
#include <sstream>
#include <iomanip>

class NumericInput : public BaseElement {
private:
    std::string prompt;
    TextField* promptTextField;
    TextField* valueTextField;
    bool showBorder;
    int fontSize;
    bool hovered;
    bool wrapText;
    std::function<void()> onPressCallback;
    
    // Numeric input specific fields
    bool isActive;
    float minVal;
    float maxVal;
    float value;
    float stepValue;
    std::string valueString;

public:
    NumericInput(RenderContext* context,
                 int x, int y, int width, int height, 
                 const std::string& prompt, 
                 float initialValue = 0.0f,
                 float minVal = 0.0f,
                 float maxVal = 100.0f,
                 float stepValue = 1.0f,
                 bool showBorder = false,
                 int fontSize = 16,
                 int layer = 0, 
                 Menu* parent = nullptr,
                 HorizontalAlignment promptAlign = LEFT,
                 HorizontalAlignment valueAlign = RIGHT);
    
    ~NumericInput();
    
    void render() override;
    bool handleEvent(const SDL_Event& event) override;
    
    // Value management
    void setValue(float newValue);
    float getValue() const { return value; }
    void setRange(float newMin, float newMax);
    void setStepValue(float step) { stepValue = step; }
    
    // Prompt management
    void setPrompt(const std::string& newPrompt);
    const std::string& getPrompt() const { return prompt; }
    
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
    
    // State management
    bool isHovered() const { return hovered; }
    bool getIsActive() const { return isActive; }
    void setIsActive(bool active);

private:
    void onPress();
    void updateTextFields();
    void updateValueString();
    void incrementValue();
    void decrementValue();
    void clampValue();
    
    // Convert SDL_Color to glm::vec4
    glm::vec4 colorToVec4(const SDL_Color& color) const {
        return glm::vec4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    }
};

#endif