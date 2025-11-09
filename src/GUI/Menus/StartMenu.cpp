#include "StartMenu.hpp"
#include <iostream>

StartMenu::StartMenu(RenderContext* context)
    : Menu(context, "StartMenu") {
}

void StartMenu::init() {
    // Title label (layer 1 - main content, static text)
    auto titleLabel = std::make_unique<Label>(
        renderContext,
        150, 70, 300, 60,
        "Activity Tracker",
        false,  // Show border
        28     // Large font
    );
    titleLabel->setLayer(1);
    titleLabel->setId("title_label");
    titleLabel->setSelectable(false);  // Static text, not selectable
    addElement(std::move(titleLabel));

    // Subtitle label (layer 1 - main content, static text)
    auto subtitleLabel = std::make_unique<Label>(
        renderContext,
        200, 130, 200, 30,
        "Track Your Progress",
        false,  // No border
        16      // Normal font
    );
    subtitleLabel->setLayer(1);
    subtitleLabel->setId("subtitle_label");
    subtitleLabel->setSelectable(false);  // Static text, not selectable
    addElement(std::move(subtitleLabel));

    // Start Game button (layer 2 - interactive elements)
    auto startButton = std::make_unique<Button>(
        renderContext,
        150, 180, 300, 50,
        "Start New Session",
        true,  // Show border
        18     // Medium font
    );
    startButton->setLayer(2);
    startButton->setId("start_button");
    // Button is selectable by default, no need to set it
    addElement(std::move(startButton));

    // View History button (layer 2 - interactive elements)
    auto historyButton = std::make_unique<Button>(
        renderContext,
        150, 250, 300, 50,
        "View History",
        true,  // Show border
        18     // Medium font
    );
    historyButton->setLayer(2);
    historyButton->setId("history_button");
    addElement(std::move(historyButton));

    // Settings button (layer 2 - interactive elements)
    auto settingsButton = std::make_unique<Button>(
        renderContext,
        150, 320, 300, 50,
        "Settings",
        true,  // Show border
        18     // Medium font
    );
    settingsButton->setLayer(2);
    settingsButton->setId("settings_button");
    addElement(std::move(settingsButton));

    // Exit button (layer 2 - interactive elements)
    auto exitButton = std::make_unique<Button>(
        renderContext,
        150, 390, 300, 50,
        "Exit",
        true,  // Show border
        18     // Medium font
    );
    exitButton->setLayer(2);
    exitButton->setId("exit_button");
    addElement(std::move(exitButton));

    // Set focus to the first interactive element (Start button)
    BaseElement* firstButton = getElement("start_button");
    if (firstButton) {
        setFocus(firstButton);
        std::cerr << "StartMenu: Focus set to 'Start New Session' button" << std::endl;
    }
}

void StartMenu::onStartGame() {
    std::cerr << "StartMenu: Starting new session..." << std::endl;
    // TODO: Transition to session menu
}

void StartMenu::onViewHistory() {
    std::cerr << "StartMenu: Opening history..." << std::endl;
    // TODO: Transition to history menu
}

void StartMenu::onSettings() {
    std::cerr << "StartMenu: Opening settings..." << std::endl;
    // TODO: Transition to settings menu
}

void StartMenu::onExit() {
    std::cerr << "StartMenu: Exiting application..." << std::endl;
    // TODO: Handle application exit
}