#include "StartMenu.hpp"
#include <iostream>

StartMenu::StartMenu(RenderContext* context)
    : Menu(context, "StartMenu") {
}

void StartMenu::init() {
    // // Title label (layer 1 - main content, static text)
    // auto titleLabel = std::make_unique<Label>(
    //     renderContext,  // Use renderContext instead of context
    //     150, 70, 300, 60,
    //     "Activity Tracker",
    //     false,  // Show border
    //     32,     // Large font
    //     1       // Layer
    // );
    // titleLabel->setId("title_label");
    // titleLabel->setSelectable(false);  // Static text, not selectable
    // addElement(std::move(titleLabel));

    // // Subtitle label (layer 1 - main content, static text)
    // auto subtitleLabel = std::make_unique<Label>(
    //     renderContext,  // Use renderContext instead of context
    //     200, 130, 200, 30,
    //     "Track Your Progress",
    //     false,  // No border
    //     24,     // Normal font
    //     1       // Layer
    // );
    // subtitleLabel->setId("subtitle_label");
    // subtitleLabel->setSelectable(false);  // Static text, not selectable
    // addElement(std::move(subtitleLabel));

    // Start Game button (layer 2 - interactive elements)
    auto startButton = std::make_unique<Button>(
        renderContext,  // Use renderContext instead of context
        150, 180, 300, 50,
        "Start New Session",
        true,  // Show border
        24,    // Medium font
        2      // Layer
    );
    startButton->setId("start_button");
    // Connect button callback
    startButton->setOnPress([this]() { onStartGame(); });
    addElement(std::move(startButton));

    // View History button (layer 2 - interactive elements)
    auto historyButton = std::make_unique<Button>(
        renderContext,  // Use renderContext instead of context
        150, 250, 300, 50,
        "View History testtesttesttesttesttesttesttesttest",
        true,  // Show border
        24,    // Medium font
        2      // Layer
    );
    historyButton->setId("history_button");
    // Connect button callback
    historyButton->setOnPress([this]() { onViewHistory(); });
    addElement(std::move(historyButton));
    

    // Settings button (layer 2 - interactive elements)
    auto settingsButton = std::make_unique<Button>(
        renderContext,  // Use renderContext instead of context
        150, 320, 300, 50,
        "Settings",
        true,  // Show border
        24,    // Medium font
        2      // Layer
    );
    settingsButton->setId("settings_button");
    // Connect button callback
    settingsButton->setOnPress([this]() { onSettings(); });
    addElement(std::move(settingsButton));

    // Exit button (layer 2 - interactive elements)
    auto exitButton = std::make_unique<Button>(
        renderContext,  // Use renderContext instead of context
        150, 390, 300, 50,
        "Exit",
        true,  // Show border
        24,    // Medium font
        2      // Layer
    );
    exitButton->setId("exit_button");
    // Connect button callback
    exitButton->setOnPress([this]() { onExit(); });
    addElement(std::move(exitButton));

    // Set focus to the first interactive element (Start button)
    BaseElement* firstButton = getElement("start_button");
    if (firstButton) {
        setFocus(firstButton);
        std::cout << "StartMenu: Focus set to 'Start New Session' button" << std::endl;
    }
}

void StartMenu::onStartGame() {
    std::cout << "StartMenu: Starting new session..." << std::endl;
    // TODO: Transition to session menu
}

void StartMenu::onViewHistory() {
    std::cout << "StartMenu: Opening history..." << std::endl;
    // TODO: Transition to history menu
}

void StartMenu::onSettings() {
    std::cout << "StartMenu: Opening settings..." << std::endl;
    // TODO: Transition to settings menu
}

void StartMenu::onExit() {
    std::cout << "StartMenu: Exiting application..." << std::endl;
    // TODO: Handle application exit
}