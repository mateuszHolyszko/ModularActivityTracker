#include "ClockPanel.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "ClockPanelLayout.hpp"

std::vector<std::unique_ptr<BaseElement>> ClockPanel::create(
    RenderContext* context, int x, int y, Menu* parent) {
    
    std::vector<std::unique_ptr<BaseElement>> elements;
    
    const Box& boxClock = layout.at("Clock");
    const Box& boxDate = layout.at("Date");
    const Box& boxUserPrompt = layout.at("UserPrompt");
    const Box& boxUserPromptLabel = layout.at("UserPromptLabel");
    const Box& boxUserSprite = layout.at("ClockWidget");
    
    // Helper function to get current time/date
    auto getCurrentTime = []() -> std::string {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        std::stringstream ss;
        ss << std::put_time(&tm, "%H:%M:%S");
        return ss.str();
    };
    
    auto getCurrentDate = []() -> std::string {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        std::stringstream ss;
        ss << std::put_time(&tm, "%d-%m-%Y");
        return ss.str();
    };
    
    #pragma region <Clock>

    // Time display label with custom update
    auto timeLabel = std::make_unique<Label>(
        context, x + boxClock.x, y + boxClock.y, boxClock.width, boxClock.height, 
        getCurrentTime(), true, 20, 2, parent, CENTER
    );
    timeLabel->setId("clock_time");
    timeLabel->setWrapText(false);
    
    // Override the update method for this specific instance
    timeLabel->onUpdate = [timeLabelPtr = timeLabel.get(), getCurrentTime](float dt) {
        static float updateTimer = 0.0f;
        updateTimer += dt;
        
        if (updateTimer >= 1.0f) {
            timeLabelPtr->setText(getCurrentTime());
            updateTimer = 0.0f;
        }
    };
    
    elements.push_back(std::move(timeLabel));
    #pragma endregion <Clock>
    
    #pragma region <Date>
    
    // Date label with custom update
    auto dateLabel = std::make_unique<Label>(
        context, x + boxDate.x, y + boxDate.y, boxDate.width, boxDate.height, 
        getCurrentDate(), true, 18, 2, parent, CENTER
    );
    dateLabel->setId("clock_date");
    dateLabel->setWrapText(false);
    
    // Override the update method for this specific instance
    dateLabel->onUpdate = [dateLabelPtr = dateLabel.get(), getCurrentDate](float dt) {
        static float updateTimer = 0.0f;
        updateTimer += dt;
        
        if (updateTimer >= 1.0f) {
            dateLabelPtr->setText(getCurrentDate());
            updateTimer = 0.0f;
        }
    };
    
    elements.push_back(std::move(dateLabel));
    #pragma endregion <Date>

    #pragma region <UserPrompt>
    // Top Lable
    auto userLabelPrompt = std::make_unique<Label>(
        context,  // Use renderContext instead of context
        boxUserPromptLabel.x, boxUserPromptLabel.y, boxUserPromptLabel.width, boxUserPromptLabel.height,
        "User",
        true,  // Show border
        20,     // Large font
        2,       // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    userLabelPrompt->setId("user_label");
    userLabelPrompt->setBackgroundColor(style.getLgBgColor());
    userLabelPrompt->setWrapText(false);
    elements.push_back(std::move(userLabelPrompt));

    // Helper function to get current user name from AppGlobals
    auto getCurrentUser = []() -> std::string {
        if (AppGlobals::has("CurrentUser")) {
            return AppGlobals::get<std::string>("CurrentUser");
        } else {
            return "None";
        }
    };

    std::string currentUser = getCurrentUser();
    // User display label with custom update
    auto userLabel = std::make_unique<Label>(
        context, x + boxUserPrompt.x, y + boxUserPrompt.y, boxUserPrompt.width, boxUserPrompt.height, 
        currentUser, true, 20, 2, parent, CENTER
    );
    userLabel->setId("user_prompt");
    userLabel->setWrapText(false);
    
    elements.push_back(std::move(userLabel));
    #pragma endregion <UserPrompt>

    #pragma region <MenuSelect>
    // Get menu select boxes
    const Box& boxMenu1 = layout.at("Menu1");
    const Box& boxMenu2 = layout.at("Menu2");
    const Box& boxMenu3 = layout.at("Menu3");
    const Box& boxMenu4 = layout.at("Menu4");

    // Define the common onUpdate function once
    auto menuButtonUpdate = [](Button* buttonPtr, float dt) {
        bool loggedIn = AppGlobals::get<bool>("IsLoggedIn");
        //std::cerr << "' update: loggedIn=" << loggedIn << std::endl;
        buttonPtr->setSelectable(loggedIn);
        //buttonPtr->setEnabled(loggedIn); // disabeld elements dont get onUpdate called, retard :3
    };

    // Menu buttons
    auto Menu1Button = std::make_unique<Button>(
        context,  // Use renderContext instead of context
        boxMenu1.x, boxMenu1.y, boxMenu1.width, boxMenu1.height,
        "Overview",
        true,  // Show border
        24,    // Medium font
        2,      // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    Menu1Button->setId("menu1_button");
    Menu1Button->onUpdate = [menuButtonUpdate, buttonPtr = Menu1Button.get()](float dt) {
        menuButtonUpdate(buttonPtr, dt);
    };
    
    // Set onPress callback - no parameters needed
    Menu1Button->setOnPress([]() {
        //std::cout << "Overview button pressed!" << std::endl;
        RenderContext* context = AppGlobals::get<RenderContext*>("RenderContext");
        WorkThread* worker = AppGlobals::get<WorkThread*>("WorkerThread");
        DatabaseManager* dbManager = AppGlobals::get<DatabaseManager*>("DatabaseManager");
       // Create the menu dynamically
        auto overViewMenu = std::make_unique<OverviewMenu>(context, worker, dbManager);
        overViewMenu->init(); 

        context->setCurrentMenu(overViewMenu.release()); // Release ownership to context
    });

    elements.push_back(std::move(Menu1Button));

    auto Menu2Button = std::make_unique<Button>(
        context,  // Use renderContext instead of context
        boxMenu2.x, boxMenu2.y, boxMenu2.width, boxMenu2.height,
        "Menu 2",
        true,  // Show border
        24,    // Medium font
        2,      // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    Menu2Button->setId("menu2_button");
    Menu2Button->onUpdate = [menuButtonUpdate, buttonPtr = Menu2Button.get()](float dt) {
        menuButtonUpdate(buttonPtr, dt);
    };
    // Connect button callback
    // TO DO: implement callbacks
    elements.push_back(std::move(Menu2Button));

    auto Menu3Button = std::make_unique<Button>(
        context,  // Use renderContext instead of context
        boxMenu3.x, boxMenu3.y, boxMenu3.width, boxMenu3.height,
        "Menu 3",
        true,  // Show border
        24,    // Medium font
        2,      // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    Menu3Button->setId("menu3_button");
    Menu3Button->onUpdate = [menuButtonUpdate, buttonPtr = Menu3Button.get()](float dt) {
        menuButtonUpdate(buttonPtr, dt);
    };
    // Connect button callback
    // TO DO: implement callbacks
    elements.push_back(std::move(Menu3Button));

    auto Menu4Button = std::make_unique<Button>(
        context,  // Use renderContext instead of context
        boxMenu4.x, boxMenu4.y, boxMenu4.width, boxMenu4.height,
        "Menu 4",
        true,  // Show border
        24,    // Medium font
        2,      // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    Menu4Button->setId("menu4_button");
    Menu4Button->onUpdate = [menuButtonUpdate, buttonPtr = Menu4Button.get()](float dt) {
        menuButtonUpdate(buttonPtr, dt);
    };
    // Connect button callback
    // TO DO: implement callbacks
    elements.push_back(std::move(Menu4Button));

    #pragma endregion <MenuSelect>

    #pragma region <SpriteElement>
    // Get ImageManager from AppGlobals
    ImageManager* imageManager = AppGlobals::get<ImageManager*>("ImageManager");
    // Create sprite element
    auto catSprite = std::make_unique<CatWidget>(
        context,
        imageManager,
        boxUserSprite.x, boxUserSprite.y, boxUserSprite.width, boxUserSprite.height,  // position and display size
        160,840,
        1  // layer
    );

    catSprite->setId("clock_sprite");
    catSprite->setShowBorder(true);
    catSprite->setFrameTime(0.5f);
    // Add to menu
    elements.push_back(std::move(catSprite));

    #pragma endregion <SpriteElement>

    return elements;
}