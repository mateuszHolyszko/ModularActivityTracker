#include "InitMenu.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "../../3D/Model3D.hpp"
#include "../../3D/ShaderProgram.hpp"
#include "../../NotificationSystem.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "../../panels/ClockPanel.hpp"
#include "InitMenuLayout.hpp"

InitMenu::InitMenu(RenderContext* context, WorkThread* workThread, DatabaseManager* dbMgr)
    : Menu(context, workThread, dbMgr,"InitMenu") {   // <-- pass worker to base
}

void InitMenu::init() {
    // Load images into ImageManager
    imageManager.loadImage("src/GUI/images/logo.png", "logo");
    imageManager.replaceNonTransparentWithColor("logo",style.text_color.r/255.f,style.text_color.g/255.f,style.text_color.b/255.f ,"processed_logo");

    // Add ClockPanel (group of elements)
    auto clockPanel = ClockPanel::create(renderContext, 0, 0, this);
    for (auto& element : clockPanel) addElement(std::move(element));

    // Set MenuButtons styles to indicate active menu, since its init menu none should be active
    dynamic_cast<Button*>(getElement("menu1_button"))->setActivated(false);
    dynamic_cast<Button*>(getElement("menu2_button"))->setSelected(false);
    dynamic_cast<Button*>(getElement("menu3_button"))->setSelected(false);
    dynamic_cast<Button*>(getElement("menu4_button"))->setSelected(false);

    // Fetch stuff from database
    std::vector<std::string> users = dbManager->getAllUserNames();

    // Load boxes from layout (x,y,width,height)
    const Box& boxLogo = layout.at("MAT_Logo");  
    const Box& boxMeta = layout.at("Meta_Info");
    const Box& boxSelectUsr = layout.at("SelectUser");
    const Box& boxEditButton = layout.at("EditButton");

    #pragma region <Element Definitions>
    // Logo Image // =====================================
    auto img = std::make_unique<ImageElement>(
            renderContext,
            &imageManager,    // Pass pointer to the manager
            boxLogo.x, boxLogo.y, boxLogo.width, boxLogo.height,
            "processed_logo",           // This is the key
            1                           // Layer
        );
    img->setId("img_element");
    img->setShowBorder(false);
    addElement(std::move(img));

    // Meta Info Label // =====================================
    auto metaLabel = std::make_unique<Label>(
        renderContext,  // Use renderContext instead of context
        boxMeta.x, boxMeta.y, boxMeta.width, boxMeta.height,
        "Meta Information\nVersion 0.1\nBuild Date: 17/11/2025\nAuthor: Mat",
        false,  // Show border
        24,     // Large font
        1,       // Layer
        nullptr, // parent
        LEFT  // Text alignment - CENTER or RIGHT or LEFT
    );
    metaLabel->setId("meta_label");
    metaLabel->setWrapText(false);
    addElement(std::move(metaLabel));

    // Select user dropdown // =================================
    auto usersDropDown = std::make_unique<DropDown>(
        renderContext,
        boxSelectUsr.x, boxSelectUsr.y, boxSelectUsr.width, boxSelectUsr.height,
        "Select User",  // Initial text
        true,             // Show border
        24,               // Font size
        2,                // Layer
        nullptr,          // Parent
        CENTER           // Text alignment
    );
    usersDropDown->setId("users_dropdown");
    usersDropDown->setOptions(users);  // Set options after construction
    usersDropDown->setDropDownDirection(1); // 0=down, 1=up

    // Optional: Adjust dropdown height based on number of options
    usersDropDown->setDropDownHeight(static_cast<int>(users.size()) * 30); // 30px per option

    // Set the callback to update CurrentUser when an option is selected
    usersDropDown->setOnOptionSelected([this](int index, const std::string& selectedUser) {
        AppGlobals::set<std::string>("CurrentUser", selectedUser);
        AppGlobals::set<bool>("IsLoggedIn", true);
        // Update User prompt in ClockPanel - cast to Label to use setText()
        auto* label = dynamic_cast<Label*>(getElement("user_prompt"));
        if (label) {
            label->setText(selectedUser);
        }
        update(0.0f); // Force update to reflect changes
    });

    addElement(std::move(usersDropDown));

    // Edit Users Button // =================================
    auto editUserButton = std::make_unique<Button>(
        renderContext,  // Use renderContext instead of context
        boxEditButton.x, boxEditButton.y, boxEditButton.width, boxEditButton.height,
        "Edit",
        true,  // Show border
        24,    // Medium font
        1,      // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    editUserButton->setId("edit_user_button");
    // Connect button callback
    editUserButton->setOnPress([this]() { onEditUserButton(); });
    addElement(std::move(editUserButton));

    #pragma endregion <Element Definitions>

    // Set focus to the first interactive element (Select User dropdown)
    setFocus(getElement("users_dropdown"));
    
}

void InitMenu::onEditUserButton() {
    std::cerr << "Test Edit button" << std::endl;
    renderContext->addNotification("Test Edit button");

}

// Menu update override
void InitMenu::update(float deltaTime) {
    // Call base Menu update to handle element updates
    Menu::update(deltaTime);

    // Additional InitMenu-specific updates 
    bool loggedIn = AppGlobals::get<bool>("IsLoggedIn");
    getElement("edit_user_button")->setSelectable(loggedIn);
}
