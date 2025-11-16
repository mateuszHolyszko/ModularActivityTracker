#ifndef DROPDOWN_HPP
#define DROPDOWN_HPP

#include "BaseElement.hpp"
#include "../../Style.hpp"
#include "../../RenderContext.hpp"
#include "../TextField.hpp"
#include "../Menu.hpp"
#include <functional>
#include <vector>
#include <memory>

class DropDown : public BaseElement {
private:
    std::string prompt;
    TextField* textField;  // Use pointer since we need dynamic initialization
    bool showBorder;
    int fontSize;
    bool hovered;
    bool wrapText;
    std::function<void()> onPressCallback;
    
    // Additional fields for DropDown
    std::vector<std::string> options;
    std::vector<std::unique_ptr<TextField>> optionTextFields; // TextFields for each option
    bool isActive;
    int dropDownDirection;
    int dropDownHeight;
    int selectedOptionIndex; // Track which option is selected

public:
    DropDown(RenderContext* context,
             int x, int y, int width, int height, 
             const std::string& prompt, 
             bool showBorder = false,
             int fontSize = 16,
             int layer = 0, 
             Menu* parent = nullptr,
             HorizontalAlignment textAlign = LEFT);
    
    ~DropDown();  // Add destructor to clean up TextField
    
    void render() override;
    bool handleEvent(const SDL_Event& event) override;
    
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
    
    // Hover state
    bool isHovered() const { return hovered; }
    
    // DropDown specific methods
    const std::vector<std::string>& getOptions() const { return options; }
    void setOptions(const std::vector<std::string>& newOptions);
    void addOption(const std::string& option);
    
    bool getIsActive() const { return isActive; }
    void setIsActive(bool active);
    
    int getDropDownDirection() const { return dropDownDirection; }
    void setDropDownDirection(int direction);
    
    int getDropDownHeight() const { return dropDownHeight; }
    void setDropDownHeight(int height);
    
    // Selection management
    int getSelectedOptionIndex() const { return selectedOptionIndex; }
    const std::string& getSelectedOption() const;
    void setSelectedOptionIndex(int index);

private:
    void onPress();
    void updateTextField();
    void updateOptionTextFields();
    void navigateOptions(int delta);
    void selectCurrentOption();
    
    // Convert SDL_Color to glm::vec4
    glm::vec4 colorToVec4(const SDL_Color& color) const {
        return glm::vec4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    }
};

#endif