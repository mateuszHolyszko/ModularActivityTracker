#include "StartMenu.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "../3D/Model3D.hpp"
#include "../3D/ShaderProgram.hpp"
#include <glm/gtc/matrix_transform.hpp>

StartMenu::StartMenu(RenderContext* context, WorkThread* workThread)
    : Menu(context, workThread, "StartMenu") {   // <-- pass worker to base
}

void StartMenu::init() {
    // // Title label (layer 1 - main content, static text)
    auto titleLabel = std::make_unique<Label>(
        renderContext,  // Use renderContext instead of context
        150, 70, 300, 60,
        "Activity Tracker",
        false,  // Show border
        42,     // Large font
        1,       // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    titleLabel->setId("title_label");
    addElement(std::move(titleLabel));

    // Start Game button (layer 2 - interactive elements)
    auto startButton = std::make_unique<Button>(
        renderContext,  // Use renderContext instead of context
        150, 180, 300, 50,
        "Start New Session",
        true,  // Show border
        24,    // Medium font
        2,      // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    startButton->setId("start_button");
    // Connect button callback
    startButton->setOnPress([this]() { onStartGame(); });
    addElement(std::move(startButton));

    // View History button (layer 2 - interactive elements)
    auto historyButton = std::make_unique<Button>(
        renderContext,
        150, 250, 300, 50,
        "View History testtesttesttesttesttesttesttesttest",
        true,  // Show border
        24,    // Medium font
        2,     // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
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

    // 3D TEST
    auto viewport = std::make_unique<ViewportElement>(
    renderContext,
    470, 110, 300, 300,  // x, y, width, height
    3,                   // layer
    nullptr,              // parent
    true                // show border
    );

    // Load model
    auto model = std::make_shared<Model3D>();
    if (model->loadFromOBJ("src/GUI/3D/models/Mat.obj")) {
        model->normalizeToUnit(1.6f);
    } else {
        std::cerr << "Failed to load OBJ model\n";
    }

    // Load shader
    auto shader = std::make_shared<ShaderProgram>();
    if (!shader->loadFromFiles("src/GUI/3D/shaders/model.vert", "src/GUI/3D/shaders/model.frag")) {
        std::cerr << "Failed to load shaders\n";
    }

    // Give the shader to the viewport (viewport will compute & set u_mvp)
    viewport->setShader(shader);

    // Set your 3D render callback with shader: only draw the model (viewport sets MVP)
    viewport->onRender3D = [model, shader]() {
        // Draw using the shader program (viewport already bound it and set u_mvp,
        // but drawWithShader binds again safely). Alternatively use immediate draw.
        model->drawWithShader(shader->getProgram());
    };

    viewport->setId("3d_viewport");
    addElement(std::move(viewport));

    // END 3D TEST
}

// ASYNC OPERATION EXAMPLE
void StartMenu::onStartGame() {
    std::cout << "StartMenu: Starting new session..." << std::endl;

    // Ensure no other background work is running
    if (worker && !worker->isRunning()) {
        worker->start([this]() {
            // Simulate heavy loading task
            std::cout << "[WorkThread] Loading resources..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));

            // Example: do something async (no OpenGL here!)
            std::cout << "[WorkThread] Session ready!" << std::endl;
        });
    } else {
        std::cout << "[StartMenu] Worker is busy, ignoring new task.\n";
    }
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