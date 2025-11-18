#include "SelectInputMenu.hpp"
#include "../../elements/Button.hpp"
#include "../../elements/Label.hpp"
#include <iostream>
#include <algorithm>  // For std::min

SelectInputMenu::SelectInputMenu(RenderContext* context, WorkThread* workThread, DatabaseManager* dbMgr,
                                 Menu* returnMenu, BaseElement* returnFocus,
                                 std::string* outputString, std::vector<std::string> options)
    : Menu(context, workThread, dbMgr, "SelectInputMenu")  
    , returnMenu(returnMenu)
    , returnFocus(returnFocus)
    , outputString(outputString)
    , options(std::move(options))  // Move to avoid copying
{
}

void SelectInputMenu::init() {
    // Clear any existing elements
    guiElements.clear();

    // Window dimensions (assumed 800x480)
    const int windowWidth = 800;
    const int windowHeight = 480;

    // Exit button parameters (static at bottom)
    const int exitButtonHeight = 50;
    const int exitButtonY = windowHeight - exitButtonHeight - 20;  // 20px margin from bottom
    const int exitButtonWidth = 400;  // Centered, adjust as needed
    const int exitButtonX = (windowWidth - exitButtonWidth) / 2;

    // Button layout parameters for grid
    const int buttonHeight = 40;
    const int buttonSpacing = 10;
    const int marginX = 50;
    const int marginY = 50;
    const int availableHeight = windowHeight - exitButtonHeight - 2 * marginY;  // Account for exit button
    const int maxButtonsPerColumn = (availableHeight + buttonSpacing) / (buttonHeight + buttonSpacing);

    // Calculate number of columns needed
    size_t numOptions = options.size();
    size_t numColumns = (numOptions + maxButtonsPerColumn - 1) / maxButtonsPerColumn;  // Ceiling division
    numColumns = std::min(numColumns, static_cast<size_t>(4));  // Max 4 columns to fit width

    // Calculate button width based on columns
    const int availableWidth = windowWidth - 2 * marginX;
    const int buttonWidth = (availableWidth - (numColumns - 1) * buttonSpacing) / numColumns;

    // Create buttons in a grid layout
    size_t buttonIndex = 0;
    for (size_t col = 0; col < numColumns && buttonIndex < numOptions; ++col) {
        int buttonX = marginX + col * (buttonWidth + buttonSpacing);
        int buttonY = marginY;
        
        for (size_t row = 0; row < maxButtonsPerColumn && buttonIndex < numOptions; ++row) {
            const std::string& option = options[buttonIndex];
            auto optionButton = std::make_unique<Button>(
                renderContext,
                buttonX, buttonY, buttonWidth, buttonHeight,
                option,
                true,    // Show border
                24,      // Font size
                2,       // Layer
                nullptr, // parent
                CENTER   // Text alignment
            );
            optionButton->setId("button_" + std::to_string(buttonIndex));
            
            // Connect button callback - select option and close
            optionButton->setOnPress([this, option]() {
                if (outputString) {
                    *outputString = option;
                }
                closeMenu();
            });
            
            addElement(std::move(optionButton));
            buttonY += buttonHeight + buttonSpacing;
            ++buttonIndex;
        }
    }

    // Exit button (static at bottom, exits without changing output)
    auto exitButton = std::make_unique<Button>(
        renderContext,
        exitButtonX, exitButtonY, exitButtonWidth, exitButtonHeight,
        "Exit",
        true,    // Show border
        24,      // Font size
        2,       // Layer
        nullptr, // parent
        CENTER   // Text alignment
    );
    exitButton->setId("exit_button");
    exitButton->setOnPress([this]() {
        // Don't modify outputString on exit
        closeMenu();
    });
    addElement(std::move(exitButton));

    // Set focus to the first option button if available, else exit button
    if (!options.empty()) {
        BaseElement* firstButton = getElement("button_0");
        if (firstButton) {
            setFocus(firstButton);
            std::cout << "SelectInputMenu: Focus set to first option button" << std::endl;
        }
    } else {
        setFocus(exitButton.get());
        std::cout << "SelectInputMenu: Focus set to exit button" << std::endl;
    }
}

void SelectInputMenu::closeMenu() {
    // Return to the previous menu and focus
    if (renderContext && returnMenu) {
        renderContext->setCurrentMenu(returnMenu);
        
        // Restore focus if provided
        if (returnFocus && returnMenu) {
            returnMenu->setFocus(returnFocus);
        }
    }
}
