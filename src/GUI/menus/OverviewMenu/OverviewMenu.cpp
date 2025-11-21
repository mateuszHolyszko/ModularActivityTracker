#include "OverviewMenu.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "../../3D/Model3D.hpp"
#include "../../3D/ShaderProgram.hpp"
#include "../../NotificationSystem.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "../../panels/ClockPanel.hpp"
#include "OverviewMenuLayout.hpp"

OverviewMenu::OverviewMenu(RenderContext* context, WorkThread* workThread, DatabaseManager* dbMgr)
    : Menu(context, workThread, dbMgr,"OverviewMenu") {   // <-- pass worker to base
}

void OverviewMenu::init() {
    #pragma region <LoadStuff>
    // Load model
    auto model = std::make_shared<Model3D>();
    if (model->loadFromOBJ("src/GUI/3D/models/BodyBuilderTest.obj")) {
        model->normalizeToUnit(1.6f);
    } else {
        std::cerr << "Failed to load OBJ model\n";
    }

    // Load shader
    auto shader = std::make_shared<ShaderProgram>();
    if (!shader->loadFromFiles("src/GUI/3D/shaders/model.vert", "src/GUI/3D/shaders/model.frag")) {
        std::cerr << "Failed to load shaders\n";
    }
    #pragma endregion <LoadStuff>

    // Add ClockPanel (group of elements)
    auto clockPanel = ClockPanel::create(renderContext, 0, 0, this);
    for (auto& element : clockPanel) addElement(std::move(element));

    // Set MenuButtons styles to indicate active menu, since its init menu none should be active
    dynamic_cast<Button*>(getElement("menu1_button"))->setActivated(true);
    dynamic_cast<Button*>(getElement("menu2_button"))->setSelected(false);
    dynamic_cast<Button*>(getElement("menu3_button"))->setSelected(false);
    dynamic_cast<Button*>(getElement("menu4_button"))->setSelected(false);

    // Fetch stuff from database
    std::vector<std::string> users = dbManager->getAllUserNames();

    // Load boxes from layout (x,y,width,height)  
    const Box& boxViewport = layout.at("3D_viewport");
    const Box& boxVolumeInfo = layout.at("VolumeInfo");
    const Box& boxMuscleGroupsVol = layout.at("MuscleGroupsVol");
    const Box& boxPlotter = layout.at("Plotter");
    const Box& boxPloterInput = layout.at("PloterInput");
    const Box& boxLogOutUser = layout.at("LogOutUser");

    #pragma region <Element Definitions>

    // 3D TEST
    auto viewport = std::make_unique<ViewportElement>(
    renderContext,
    boxViewport.x, boxViewport.y, boxViewport.width, boxViewport.height,
    3,                   // layer
    nullptr,              // parent
    true                // show border
    );
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


    
    #pragma endregion <Element Definitions>

    // Set focus to the first interactive element (Select User dropdown)
    
    
}

// Menu update override
void OverviewMenu::update(float deltaTime) {
    // Call base Menu update to handle element updates
    Menu::update(deltaTime);

    // Additional OverviewMenu-specific updates 
    
}
