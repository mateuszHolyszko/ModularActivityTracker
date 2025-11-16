#include <iostream>
#include <algorithm>
#include "DropDown.hpp"

DropDown::DropDown(RenderContext* context,
                 int x, int y, int width, int height, 
                 const std::string& prompt, 
                 bool showBorder,
                 int fontSize,
                 int layer, 
                 Menu* parent,
                 HorizontalAlignment textAlign)
    : BaseElement(context, x, y, width, height, true, layer, parent),
      prompt(prompt), showBorder(showBorder), fontSize(fontSize), 
      hovered(false), wrapText(true),
      isActive(false), dropDownDirection(0), dropDownHeight(100),
      selectedOptionIndex(-1) { // Initialize with no selection
    
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
        prompt, 
        "",
        static_cast<float>(fontSize),
        colorToVec4(style.getTextColor()),
        textAlign,
        false
    );
}

DropDown::~DropDown() {
    delete textField;  // Clean up the TextField
    // optionTextFields will be automatically cleaned up by unique_ptr
}

void DropDown::render() {
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
        borderCmd.layer = layer;
        borderCmd.lineWidth = 1.0f;
        borderCmd.filled = false;
        
        renderContext->graphicQueue.push_back(borderCmd);
    }

    // Calculate areas: 3/4 for prompt, 1/4 for arrow
    int promptWidth = (width * 3) / 4;
    int arrowAreaWidth = width - promptWidth;
    
    // Update TextField to use only the prompt area
    int textX = x + 5;
    int textY = y + (height - fontSize) / 2;
    
    textField->setPosition(static_cast<float>(textX), static_cast<float>(textY + fontSize));
    textField->setSize(static_cast<float>(promptWidth - 10), static_cast<float>(fontSize));
    
    // Draw text using TextField
    auto textCommands = textField->render(layer + 1);
    for (const auto& textCmd : textCommands) {
        renderContext->textQueue.push_back(textCmd);
    }

    // Draw vertical line separating prompt from arrow area
    int lineX = x + promptWidth;
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

    // Draw arrow based on dropdown direction and active state
    int arrowCenterX = x + promptWidth + (arrowAreaWidth / 2);
    int arrowCenterY = y + (height / 2);
    float arrowSize = std::min(arrowAreaWidth, height) * 0.4f; // 40% of available space
    
    GraphicCommand arrowCmd;
    arrowCmd.type = GraphicCommand::ARROW;
    arrowCmd.x1 = static_cast<float>(arrowCenterX);
    arrowCmd.y1 = static_cast<float>(arrowCenterY);
    arrowCmd.x2 = arrowSize;
    
    // Set arrow direction: point up when closed, down when open (or vice versa based on your preference)
    if (isActive) {
        // When dropdown is open, arrow points up (to indicate it can be closed)
        arrowCmd.direction = 0; // Up
    } else {
        // When dropdown is closed, arrow points down (to indicate it can be opened)
        arrowCmd.direction = 2; // Down
    }
    
    arrowCmd.color = colorToVec4(style.getTextColor());
    arrowCmd.layer = layer + 2;
    arrowCmd.lineWidth = 1.0f;
    arrowCmd.filled = true;
    
    renderContext->graphicQueue.push_back(arrowCmd);

    // Render dropdown options if active
    if (isActive && !options.empty()) {
        // Draw dropdown background
        GraphicCommand dropdownBgCmd;
        dropdownBgCmd.type = GraphicCommand::BOX;
        dropdownBgCmd.x1 = static_cast<float>(getAbsoluteX());
        
        // Calculate dropdown position based on direction
        if (dropDownDirection == 0) { // Down
            dropdownBgCmd.y1 = static_cast<float>(getAbsoluteY() + height);
        } else { // Up
            dropdownBgCmd.y1 = static_cast<float>(getAbsoluteY() - dropDownHeight);
        }
        
        dropdownBgCmd.x2 = static_cast<float>(getAbsoluteX() + width);
        dropdownBgCmd.y2 = dropdownBgCmd.y1 + static_cast<float>(dropDownHeight);
        dropdownBgCmd.color = colorToVec4(style.getLgBgColor());
        dropdownBgCmd.layer = layer + 1; // Render above main dropdown
        dropdownBgCmd.lineWidth = 0.0f;
        dropdownBgCmd.filled = true;
        
        renderContext->graphicQueue.push_back(dropdownBgCmd);

        // Draw dropdown border
        GraphicCommand dropdownBorderCmd;
        dropdownBorderCmd.type = GraphicCommand::BOX;
        dropdownBorderCmd.x1 = dropdownBgCmd.x1;
        dropdownBorderCmd.y1 = dropdownBgCmd.y1;
        dropdownBorderCmd.x2 = dropdownBgCmd.x2;
        dropdownBorderCmd.y2 = dropdownBgCmd.y2;
        dropdownBorderCmd.color = colorToVec4(style.getBorderColor());
        dropdownBorderCmd.layer = layer + 1;
        dropdownBorderCmd.lineWidth = 1.0f;
        dropdownBorderCmd.filled = false;
        
        renderContext->graphicQueue.push_back(dropdownBorderCmd);

        // Render option text fields with highlighting for selected option
        for (size_t i = 0; i < optionTextFields.size(); ++i) {
            // Highlight the selected option
            if (static_cast<int>(i) == selectedOptionIndex) {
                int optionHeight = dropDownHeight / static_cast<int>(options.size());
                optionHeight = std::max(optionHeight, fontSize + 4);
                
                int optionY;
                if (dropDownDirection == 0) { // Down
                    optionY = getAbsoluteY() + height + static_cast<int>(i) * optionHeight;
                } else { // Up
                    optionY = getAbsoluteY() - dropDownHeight + static_cast<int>(i) * optionHeight;
                }
                
                // Draw highlight background for selected option
                GraphicCommand highlightCmd;
                highlightCmd.type = GraphicCommand::BOX;
                highlightCmd.x1 = static_cast<float>(getAbsoluteX());
                highlightCmd.y1 = static_cast<float>(optionY);
                highlightCmd.x2 = static_cast<float>(getAbsoluteX() + width);
                highlightCmd.y2 = static_cast<float>(optionY + optionHeight);
                highlightCmd.color = colorToVec4(style.getHighlightColor());
                highlightCmd.layer = layer + 2;
                highlightCmd.lineWidth = 0.0f;
                highlightCmd.filled = true;
                
                renderContext->graphicQueue.push_back(highlightCmd);
            }
            
            auto optionCommands = optionTextFields[i]->render(layer + 3);
            for (const auto& optionCmd : optionCommands) {
                renderContext->textQueue.push_back(optionCmd);
            }
        }
    }
}

bool DropDown::handleEvent(const SDL_Event& event) {
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
                updateTextField();
            }
            break;
        }
            
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (containsPoint(event.button.x, event.button.y)) {
                    // Instead of calling onPress, request focus from parent menu
                    if (parent && selectable) {
                        parent->setFocus(this);
                        eventConsumed = true; // Mouse click is consumed
                    }
                } else if (isActive) {
                    // Check if click is in dropdown area
                    int dropdownX = getAbsoluteX();
                    int dropdownY;
                    if (dropDownDirection == 0) { // Down
                        dropdownY = getAbsoluteY() + height;
                    } else { // Up
                        dropdownY = getAbsoluteY() - dropDownHeight;
                    }
                    
                    if (event.button.x >= dropdownX && 
                        event.button.x <= dropdownX + width &&
                        event.button.y >= dropdownY && 
                        event.button.y <= dropdownY + dropDownHeight) {
                        // Calculate which option was clicked
                        int optionHeight = dropDownHeight / static_cast<int>(options.size());
                        optionHeight = std::max(optionHeight, fontSize + 4);
                        int clickedIndex = (event.button.y - dropdownY) / optionHeight;
                        
                        if (clickedIndex >= 0 && clickedIndex < static_cast<int>(options.size())) {
                            setSelectedOptionIndex(clickedIndex);
                            selectCurrentOption();
                        }
                        
                        eventConsumed = true; // Click in dropdown area is consumed
                    } else {
                        // Click outside dropdown, close it
                        setIsActive(false);
                    }
                }
            }
            break;

        case SDL_KEYDOWN:
            if (isActive) {
                // When dropdown is active, consume all navigation events
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        navigateOptions(-1);
                        eventConsumed = true;
                        break;
                    case SDLK_DOWN:
                        navigateOptions(1);
                        eventConsumed = true;
                        break;
                    case SDLK_RETURN:
                        selectCurrentOption();
                        eventConsumed = true;
                        break;
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

void DropDown::onPress() {
    // Toggle dropdown active state
    setIsActive(!isActive);
    
    // If opening dropdown and no option is selected, select first one
    if (isActive && selectedOptionIndex == -1 && !options.empty()) {
        selectedOptionIndex = 0;
    }
    
    // Call the callback if set
    if (onPressCallback) {
        onPressCallback();
    } else {
        // Default implementation: print prompt field and center position
        int centerX = x + width / 2;
        int centerY = y + height / 2;
        std::cout << "DropDown pressed - Prompt: '" << prompt 
                  << "', Active: " << (isActive ? "true" : "false")
                  << ", Center Position: (" << centerX << ", " << centerY << ")" << std::endl;
    }
}

void DropDown::navigateOptions(int delta) {
    if (options.empty()) return;
    
    int newIndex = selectedOptionIndex + delta;
    
    // Wrap around if needed
    if (newIndex < 0) {
        newIndex = static_cast<int>(options.size()) - 1;
    } else if (newIndex >= static_cast<int>(options.size())) {
        newIndex = 0;
    }
    
    setSelectedOptionIndex(newIndex);
}

void DropDown::selectCurrentOption() {
    if (selectedOptionIndex >= 0 && selectedOptionIndex < static_cast<int>(options.size())) {
        // Update prompt to show selected option
        setPrompt(options[selectedOptionIndex]);
        setIsActive(false);
        
        std::cerr << "Option selected: '" << options[selectedOptionIndex] 
                  << "' (index: " << selectedOptionIndex << ")" << std::endl;
    }
}

const std::string& DropDown::getSelectedOption() const {
    static const std::string emptyString = "";
    if (selectedOptionIndex >= 0 && selectedOptionIndex < static_cast<int>(options.size())) {
        return options[selectedOptionIndex];
    }
    return emptyString;
}

void DropDown::setSelectedOptionIndex(int index) {
    if (index >= -1 && index < static_cast<int>(options.size())) {
        selectedOptionIndex = index;
        updateOptionTextFields(); // Update to show new selection highlight and wrapping
    }
}

void DropDown::setPrompt(const std::string& newPrompt) {
    prompt = newPrompt;
    updateTextField();
}

void DropDown::setFontSize(int size) {
    fontSize = size;
    updateTextField();
    updateOptionTextFields();
}

void DropDown::setWrapText(bool wrap) {
    wrapText = wrap;
    updateTextField();
}

void DropDown::setOptions(const std::vector<std::string>& newOptions) {
    options = newOptions;
    selectedOptionIndex = -1; // Reset selection when options change
    updateOptionTextFields();
}

void DropDown::addOption(const std::string& option) {
    options.push_back(option);
    updateOptionTextFields();
}

void DropDown::setIsActive(bool active) {
    isActive = active;
    updateOptionTextFields(); // Update positions when active state changes
}

void DropDown::setDropDownDirection(int direction) {
    dropDownDirection = direction;
    updateOptionTextFields();
}

void DropDown::setDropDownHeight(int height) {
    dropDownHeight = height;
    updateOptionTextFields();
}

void DropDown::updateTextField() {
    // Update TextField position and text based on current state
    int textX = x + 5;
    int textY = y + (height - fontSize) / 2;
    
    textField->setPosition(static_cast<float>(textX), static_cast<float>(textY + fontSize));
    textField->setSize(static_cast<float>(width - 10), static_cast<float>(fontSize));
    
    // Update text content
    textField->setText(prompt);
    
    // Determine if wrapping should be enabled based on button state
    // Disable wrapping if button is selected or hovered, otherwise use wrapText setting
    bool shouldWrap = wrapText && !(is_selected || hovered);
    textField->setWrap(shouldWrap);
}

void DropDown::updateOptionTextFields() {
    optionTextFields.clear();
    
    if (options.empty()) return;
    
    // Calculate option height based on dropdown height and number of options
    int optionHeight = dropDownHeight / static_cast<int>(options.size());
    optionHeight = std::max(optionHeight, fontSize + 4); // Minimum height
    
    // Calculate starting position based on dropdown direction
    int startX = getAbsoluteX() + 5;
    int startY;
    
    if (dropDownDirection == 0) { // Down
        startY = getAbsoluteY() + height;
    } else { // Up
        startY = getAbsoluteY() - dropDownHeight;
    }
    
    // Create TextFields for each option
    for (size_t i = 0; i < options.size(); ++i) {
        int optionY = startY + static_cast<int>(i) * optionHeight;
        
        auto optionTextField = std::make_unique<TextField>(
            renderContext,
            static_cast<float>(startX),
            static_cast<float>(optionY + fontSize),
            static_cast<float>(width - 10),
            static_cast<float>(fontSize),
            options[i],
            "",
            static_cast<float>(fontSize),
            colorToVec4(style.getTextColor()),
            HorizontalAlignment::LEFT,
            true  // Enable wrapping by default for all options
        );
        
        // Disable wrapping for the currently selected option
        if (static_cast<int>(i) == selectedOptionIndex) {
            optionTextField->setWrap(false);
        }
        
        optionTextFields.push_back(std::move(optionTextField));
    }
}