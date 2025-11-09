#include "StartMenu.hpp"
#include <iostream>

StartMenu::StartMenu(RenderContext* context)
    : Menu(context, "StartMenu") {
}

void StartMenu::init() {

    // Title label (layer 1 - main content)
    auto titleLabel = std::make_unique<Label>(
        renderContext,
        150, 70, 300, 60,
        "Activity Tracker",
        true,  // Show border
        28     // Large font
    );
    titleLabel->setLayer(1);
    titleLabel->setId("title_label");
    titleLabel->setSelectable(true);
    addElement(std::move(titleLabel));

    // Subtitle label (layer 1 - main content)
    auto subtitleLabel = std::make_unique<Label>(
        renderContext,
        200, 30, 200, 30,
        "Track Your Progress",
        false,  // No border
        16      // Normal font
    );
    subtitleLabel->setLayer(1);
    subtitleLabel->setId("subtitle_label");
    subtitleLabel->setSelectable(true);
    addElement(std::move(subtitleLabel));

    // Start Game label (layer 2 - interactive elements)
    auto startLabel = std::make_unique<Label>(
        renderContext,
        150, 180, 300, 50,
        "Start New Session",
        true,  // Show border
        18     // Medium font
    );
    startLabel->setLayer(2);
    startLabel->setId("start_label");
    startLabel->setSelectable(true);  // Make it selectable
    startLabel->onPress = [this]() { this->onStartGame(); };
    addElement(std::move(startLabel));

    // View History label (layer 2 - interactive elements)
    auto historyLabel = std::make_unique<Label>(
        renderContext,
        150, 250, 300, 50,
        "View History",
        true,  // Show border
        18     // Medium font
    );
    historyLabel->setLayer(2);
    historyLabel->setId("history_label");
    historyLabel->setSelectable(true);  // Make it selectable
    historyLabel->onPress = [this]() { this->onOptions(); };
    addElement(std::move(historyLabel));

    // Settings label (layer 2 - interactive elements)
    auto settingsLabel = std::make_unique<Label>(
        renderContext,
        150, 320, 300, 50,
        "Settings",
        true,  // Show border
        18     // Medium font
    );
    settingsLabel->setLayer(2);
    settingsLabel->setId("settings_label");
    settingsLabel->setSelectable(true);  // Make it selectable
    settingsLabel->onPress = [this]() { this->onOptions(); };
    addElement(std::move(settingsLabel));

    // Exit label (layer 2 - interactive elements)
    auto exitLabel = std::make_unique<Label>(
        renderContext,
        150, 390, 300, 50,
        "Exit",
        true,  // Show border
        18     // Medium font
    );
    exitLabel->setLayer(2);
    exitLabel->setId("exit_label");
    exitLabel->setSelectable(true);  // Make it selectable
    exitLabel->onPress = [this]() { this->onExit(); };
    addElement(std::move(exitLabel));

    // Set focus to the first interactive element (Start label)
    BaseElement* firstLabel = getElement("start_label");
    if (firstLabel) {
        setFocus(firstLabel);
        std::cout << "StartMenu: Focus set to 'Start New Session' label" << std::endl;
    }
}

void StartMenu::onStartGame() {
    std::cout << "StartMenu: Starting new session..." << std::endl;
    // TODO: Transition to session menu
}

void StartMenu::onOptions() {
    std::cout << "StartMenu: Opening options..." << std::endl;
    // TODO: Transition to options menu
}

void StartMenu::onExit() {
    std::cout << "StartMenu: Exiting application..." << std::endl;
    // TODO: Handle application exit
}