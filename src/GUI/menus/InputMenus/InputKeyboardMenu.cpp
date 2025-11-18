#include "InputKeyboardMenu.hpp"
#include "InputKeyboardMenuLayout.hpp"
#include <iostream>

InputKeyboardMenu::InputKeyboardMenu(RenderContext* context, WorkThread* workThread, DatabaseManager* dbMgr,
                                   Menu* returnMenu, BaseElement* returnFocus,
                                   std::string* outputString)
    : Menu(context, workThread, dbMgr, "InputKeyboardMenu")  
    , returnMenu(returnMenu)
    , returnFocus(returnFocus)
    , outputString(outputString)  // Store the pointer (don't modify until Enter)
    , internalString(outputString ? *outputString : "")  // Initialize with current value
{
}

void InputKeyboardMenu::updateOutputLabel() {
    // Update the output label with internal string
    auto* label = dynamic_cast<Label*>(getElement("output_label"));
    if (label) {
        label->setText(internalString);
    }
}

void InputKeyboardMenu::appendToInternal(char c) {
    internalString += c;
    updateOutputLabel();
}

void InputKeyboardMenu::appendToInternal(const std::string& str) {
    internalString += str;
    updateOutputLabel();
}

void InputKeyboardMenu::backspaceInternal() {
    if (!internalString.empty()) {
        internalString.pop_back();
        updateOutputLabel();
    }
}

void InputKeyboardMenu::clearInternal() {
    internalString.clear();
    updateOutputLabel();
}

void InputKeyboardMenu::init() {
    // Clear any existing elements
    guiElements.clear();

    // Output display label
    const Box& boxLabel = layout.at("Label");
    auto outputLabel = std::make_unique<Label>(
        renderContext,
        boxLabel.x, boxLabel.y, boxLabel.width, boxLabel.height,
        internalString,  // Show internal string, not the pointer
        true,          // Show border
        34,            // Font size
        1,             // Layer
        nullptr,       // parent
        CENTER         // Text alignment
    );
    outputLabel->setId("output_label");
    addElement(std::move(outputLabel));

    const Box& outputLabelMarkBox = layout.at("Output");  
    auto outputLabelMark = std::make_unique<Label>(
        renderContext,
        outputLabelMarkBox.x, outputLabelMarkBox.y, outputLabelMarkBox.width, outputLabelMarkBox.height,
        "Output:",
        false,  // Show border
        34,     // Large font
        1,      // Layer
        nullptr, // parent
        CENTER  // Text alignment
    );
    outputLabelMark->setId("output_label_mark");  // Fixed ID conflict
    outputLabelMark->setWrapText(false);
    addElement(std::move(outputLabelMark));

    // Create A-Z buttons
    for (char c = 'A'; c <= 'Z'; c++) {
        std::string buttonName = std::string(1, c);
        const Box& boxButton = layout.at(buttonName);
        
        auto letterButton = std::make_unique<Button>(
            renderContext,
            boxButton.x, boxButton.y, boxButton.width, boxButton.height,
            buttonName,
            true,    // Show border
            34,      // Font size
            2,       // Layer
            nullptr, // parent
            CENTER   // Text alignment
        );
        letterButton->setId("button_" + buttonName);
        
        // Connect button callback - append to internal string
        letterButton->setOnPress([this, c]() {
            appendToInternal(c);
        });
        
        addElement(std::move(letterButton));
    }

    // Space button
    const Box& boxSpace = layout.at("ButtonSpace");
    auto spaceButton = std::make_unique<Button>(
        renderContext,
        boxSpace.x, boxSpace.y, boxSpace.width, boxSpace.height,
        "Space",
        true,    // Show border
        28,      // Font size
        2,       // Layer
        nullptr, // parent
        CENTER   // Text alignment
    );
    spaceButton->setId("space_button");
    spaceButton->setOnPress([this]() {
        appendToInternal(" ");
    });
    addElement(std::move(spaceButton));

    // Backspace button
    const Box& boxBackspace = layout.at("ButtonBackspace");
    auto backspaceButton = std::make_unique<Button>(
        renderContext,
        boxBackspace.x, boxBackspace.y, boxBackspace.width, boxBackspace.height,
        "Backspace",
        true,    // Show border
        28,      // Font size
        2,       // Layer
        nullptr, // parent
        CENTER   // Text alignment
    );
    backspaceButton->setId("backspace_button");
    backspaceButton->setOnPress([this]() {
        backspaceInternal();
    });
    addElement(std::move(backspaceButton));

    // Clear button
    const Box& boxClear = layout.at("ButtonClear");
    auto clearButton = std::make_unique<Button>(
        renderContext,
        boxClear.x, boxClear.y, boxClear.width, boxClear.height,
        "Clear",
        true,    // Show border
        28,      // Font size
        2,       // Layer
        nullptr, // parent
        CENTER   // Text alignment
    );
    clearButton->setId("clear_button");
    clearButton->setOnPress([this]() {
        clearInternal();
    });
    addElement(std::move(clearButton));

    // Cancel button
    const Box& boxCancel = layout.at("ButtonCancel");
    auto cancelButton = std::make_unique<Button>(
        renderContext,
        boxCancel.x, boxCancel.y, boxCancel.width, boxCancel.height,
        "Cancel",
        true,    // Show border
        28,      // Font size
        2,       // Layer
        nullptr, // parent
        CENTER   // Text alignment
    );
    cancelButton->setId("cancel_button");
    cancelButton->setOnPress([this]() {
        // Don't write to outputString on cancel
        closeMenu();
    });
    addElement(std::move(cancelButton));

    // Enter button
    const Box& boxEnter = layout.at("ButtonEnter");
    auto enterButton = std::make_unique<Button>(
        renderContext,
        boxEnter.x, boxEnter.y, boxEnter.width, boxEnter.height,
        "Enter",
        true,    // Show border
        28,      // Font size
        2,       // Layer
        nullptr, // parent
        CENTER   // Text alignment
    );
    enterButton->setId("enter_button");
    enterButton->setOnPress([this]() {
        //std::cout << "DEBUG: HERE" << std::endl;
        // Write internal string to output pointer only when Enter is pressed
        if (outputString) {
            *outputString = internalString;
        }
        //std::cout << "DEBUG: HERE after Write" << std::endl;
        closeMenu();
    });
    addElement(std::move(enterButton));

    // Set focus to the first button (ButtonA)
    BaseElement* firstButton = getElement("button_A");
    if (firstButton) {
        setFocus(firstButton);
        std::cout << "InputKeyboardMenu: Focus set to 'A' button" << std::endl;
    }
}

void InputKeyboardMenu::closeMenu() {
    // Return to the previous menu and focus
    if (renderContext && returnMenu) {
        renderContext->setCurrentMenu(returnMenu);
        
        // Restore focus if provided
        if (returnFocus && returnMenu) {
            returnMenu->setFocus(returnFocus);
        }
    }
}