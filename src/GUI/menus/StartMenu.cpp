#include "StartMenu.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "../3D/Model3D.hpp"
#include "../3D/ShaderProgram.hpp"
#include "../NotificationSystem.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "../panels/ClockPanel.hpp"
#include "StartMenu_layout.hpp"

StartMenu::StartMenu(RenderContext* context, WorkThread* workThread)
    : Menu(context, workThread, "StartMenu") {   // <-- pass worker to base
}

void StartMenu::init() {
    // Load images into ImageManager
    imageManager.loadImage("src/GUI/images/test.jpg", "test");
    imageManager.applyGrayscale("test", "grayscale_test");

    // Add ClockPanel (group of elements)
    auto clockPanel = ClockPanel::create(renderContext, 0, 0, this);
    for (auto& element : clockPanel) addElement(std::move(element));

    // Load box from layout (x,y,width,height)
    const Box& boxLabel = layout.at("Label");  // get box by name
    // // Title label (layer 1 - main content, static text)
    auto titleLabel = std::make_unique<Label>(
        renderContext,  // Use renderContext instead of context
        boxLabel.x, boxLabel.y, boxLabel.width, boxLabel.height,
        "Activity Tracker",
        false,  // Show border
        42,     // Large font
        1,       // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    titleLabel->setId("title_label");
    titleLabel->setWrapText(false);
    addElement(std::move(titleLabel));

    const Box& boxButton1 = layout.at("Button1");
    // Start Game button (layer 2 - interactive elements)
    auto startButton = std::make_unique<Button>(
        renderContext,  // Use renderContext instead of context
        boxButton1.x, boxButton1.y, boxButton1.width, boxButton1.height,
        "Start New Session",
        true,  // Show border
        24,    // Medium font
        1,      // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    startButton->setId("start_button");
    // Connect button callback
    startButton->setOnPress([this]() { onStartGame(); });
    addElement(std::move(startButton));

    const Box& boxButton2 = layout.at("Button2");
    // View History button (layer 2 - interactive elements)
    auto historyButton = std::make_unique<Button>(
        renderContext,
        boxButton2.x, boxButton2.y, boxButton2.width, boxButton2.height,
        "View History testtesttesttesttesttesttesttesttest",
        true,  // Show border
        24,    // Medium font
        1,     // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    historyButton->setId("history_button");
    // Connect button callback
    historyButton->setOnPress([this]() { onViewHistory(); });
    addElement(std::move(historyButton));
    

    const Box& boxButton3 = layout.at("Button3");
    // Settings button (layer 2 - interactive elements)
    auto settingsButton = std::make_unique<Button>(
        renderContext,  // Use renderContext instead of context
        boxButton3.x, boxButton3.y, boxButton3.width, boxButton3.height,
        "Settings",
        true,  // Show border
        24,    // Medium font
        1      // Layer
    );
    settingsButton->setId("settings_button");
    // Connect button callback
    settingsButton->setOnPress([this]() { onSettings(); });
    addElement(std::move(settingsButton));

    const Box& boxButton4 = layout.at("Button4");
    // Exit button (layer 2 - interactive elements)
    auto exitButton = std::make_unique<Button>(
        renderContext,  // Use renderContext instead of context
        boxButton4.x, boxButton4.y, boxButton4.width, boxButton4.height,
        "Exit",
        true,  // Show border
        24,    // Medium font
        1      // Layer
    );
    exitButton->setId("exit_button");
    // Connect button callback
    exitButton->setOnPress([this]() { onExit(); });
    addElement(std::move(exitButton));

    const Box& boxTestSelect = layout.at("Button5");
    // New: Test Select Input button 
    auto testSelectButton = std::make_unique<Button>(
        renderContext,
        boxTestSelect.x, boxTestSelect.y, boxTestSelect.width, boxTestSelect.height,
        "Test Select Input",
        true,  // Show border
        24,    // Medium font
        1      // Layer
    );
    testSelectButton->setId("test_select_button");
    // Connect button callback
    testSelectButton->setOnPress([this]() { onTestSelectInput(); });
    addElement(std::move(testSelectButton));

    // DropDown example element
    const Box& boxDropDown = layout.at("DropDown");
    std::vector<std::string> dropdownOptions = {"TESAT", "OPTION@", "ANOTHER OPtion", "LONG OPTION TEXT EXAMPLE", "OPTION 4"};
    auto exampleDropDown = std::make_unique<DropDown>(
        renderContext,
        boxDropDown.x, boxDropDown.y, boxDropDown.width, boxDropDown.height,
        "Select Option",  // Initial text
        true,             // Show border
        24,               // Font size
        3,                // Layer
        nullptr,          // Parent
        CENTER           // Text alignment
    );
    exampleDropDown->setId("example_dropdown");
    exampleDropDown->setOptions(dropdownOptions);  // Set options after construction
    exampleDropDown->setDropDownDirection(0); // 0=down, 1=up

    // Optional: Adjust dropdown height based on number of options
    exampleDropDown->setDropDownHeight(static_cast<int>(dropdownOptions.size()) * 30); // 30px per option

    addElement(std::move(exampleDropDown));

    // NumericInput example element
    const Box& boxNumeric = layout.at("NumericInput");
    auto exampleNumeric = std::make_unique<NumericInput>(
        renderContext,
        boxNumeric.x, boxNumeric.y, boxNumeric.width, boxNumeric.height,
        "Volume",           // Prompt text
        75.0f,              // Initial value
        0.0f,               // Minimum value
        100.0f,             // Maximum value
        5.0f,               // Step value (increment/decrement by 5)
        true,               // Show border
        30,                 // Font size
        1,                  // Layer
        nullptr,            // Parent (will be set by Menu)
        LEFT,               // Prompt text alignment (LEFT, CENTER, RIGHT)
        LEFT               // Value text alignment (LEFT, CENTER, RIGHT)
    );
    exampleNumeric->setId("volume_input");

    // Add to menu
    addElement(std::move(exampleNumeric));

    const Box& imageBox = layout.at("Image");
    // Image element example
    auto img = std::make_unique<ImageElement>(
            renderContext,
            &imageManager,    // Pass pointer to the manager
            imageBox.x, imageBox.y, imageBox.width, imageBox.height,
            "grayscale_test",           // This is the key
            3                 // Layer
        );
    img->setId("img_element");
    addElement(std::move(img));
    


    // Set focus to the first interactive element (Start button)
    BaseElement* firstButton = getElement("start_button");
    if (firstButton) {
        setFocus(firstButton);
        std::cout << "StartMenu: Focus set to 'Start New Session' button" << std::endl;
    }

    const Box& boxViewPort = layout.at("ViewPort");
    // 3D TEST
    auto viewport = std::make_unique<ViewportElement>(
    renderContext,
    boxViewPort.x, boxViewPort.y, boxViewPort.width, boxViewPort.height,
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
    //std::string inputText = "";  // Your string to store input
    
    auto keyboardMenu = std::make_unique<InputKeyboardMenu>(
        renderContext, 
        worker, 
        this,                    // returnMenu
        getCurrentFocus(),       // returnFocus  
        &inputText               // pointer to your output string
    );
    keyboardMenu->init();
    
    // Set it as current menu in the context
    renderContext->setCurrentMenu(keyboardMenu.get());
    
    // Store the keyboard menu pointer so it doesn't get destroyed
    // You'll need to add this to your StartMenu class:
    // std::unique_ptr<InputKeyboardMenu> keyboardMenu;
    this->keyboardMenu = std::move(keyboardMenu);
    
    std::cout << "Opened keyboard menu" << std::endl;
}

void StartMenu::onTestSelectInput() {
    std::vector<std::string> testOptions = {"Option A", "Option B", "Option C", "Option E","Option G","Option F","Option H","Option Y"};  // Sample options
    
    auto selectMenu = std::make_unique<SelectInputMenu>(
        renderContext, 
        worker, 
        this,                    // returnMenu
        getCurrentFocus(),       // returnFocus  
        &selectText,             // pointer to output string
        testOptions              // vector of options
    );
    selectMenu->init();
    
    // Set it as current menu in the context
    renderContext->setCurrentMenu(selectMenu.get());
    
    // Store the select menu pointer so it doesn't get destroyed
    this->selectMenu = std::move(selectMenu);
    
    std::cout << "Opened select input menu" << std::endl;
}

void StartMenu::onSettings() {
    std::cout << "StartMenu: Opening settings..." << std::endl;
    renderContext->addNotification("Keyboard Input: " + inputText);
    renderContext->addNotification("Select Input: " + selectText);
}

void StartMenu::onExit() {
    std::cout << "StartMenu: Exiting application..." << std::endl;
    renderContext->addNotification("Test3Test3Test3Test3Test3Test3Test3Test3Test3Test3Test3Test3");
}