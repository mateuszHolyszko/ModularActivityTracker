#include "EditUserMenu.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "../../3D/Model3D.hpp"
#include "../../3D/ShaderProgram.hpp"
#include "../../NotificationSystem.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "../../panels/ClockPanel.hpp"
#include "EditUserMenuLayout.hpp"
#include "InitMenu.hpp"

EditUserMenu::EditUserMenu(RenderContext* context, WorkThread* workThread, DatabaseManager* dbMgr, InitMenu* returnMenu,const std::string& user)
    : Menu(context, workThread, dbMgr, "EditUserMenu"), 
      returnMenu(returnMenu),  // Initialize the returnMenu member
      user(user)
{
}

void EditUserMenu::init() {
    #pragma region <LoadStuff>
    // Load model
    
    #pragma endregion <LoadStuff>

    // Add ClockPanel (group of elements)
    auto clockPanel = ClockPanel::create(renderContext, 0, 0, this);
    for (auto& element : clockPanel) addElement(std::move(element));

    // Set MenuButtons styles to indicate active menu, since its submenu the should be deactivated return to parrent menu handelded elsewhere (so that we dont init the return menu again)
    dynamic_cast<Button*>(getElement("menu1_button"))->setEnabled(false);
    dynamic_cast<Button*>(getElement("menu2_button"))->setEnabled(false);
    dynamic_cast<Button*>(getElement("menu3_button"))->setEnabled(false);
    dynamic_cast<Button*>(getElement("menu4_button"))->setEnabled(false);

    // Fetch stuff from database
    

    #pragma region <fetch boxes>
    // Load boxes from layout (x,y,width,height) 
    const Box& boxReturnButton = layout.at("returnButton"); 
    const Box& boxStats = layout.at("Stats");
    const Box& boxWeight = layout.at("weight");
    const Box& boxMenuLabel = layout.at("MenuLabel");
    const Box& boxStatsQuery = layout.at("StatsQuery");
    const Box& boxCommitButton = layout.at("CommitButton");

    const Box& boxPrevMeasurements = layout.at("prev_Mesurements");
    const Box& boxMeasurements = layout.at("Mesurements");
    #pragma endregion <fetch boxes>

    #pragma region <Element Definitions>

    auto menuLabel = std::make_unique<Label>(
        renderContext,  // Use renderContext instead of context
        boxMenuLabel.x, boxMenuLabel.y, boxMenuLabel.width, boxMenuLabel.height,
        "Edit User > "+user,
        false,  // Show border
        34,     // Large font
        2,       // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    menuLabel->setId("menu_label");
    menuLabel->setWrapText(false);
    addElement(std::move(menuLabel));



    auto returnButton = std::make_unique<Button>(
        renderContext,  // Use renderContext instead of context
        boxReturnButton.x, boxReturnButton.y, boxReturnButton.width, boxReturnButton.height,
        "Close Edit",
        true,  // Show border
        24,    // Medium font
        1,      // Layer
        nullptr, // parent
        CENTER  // Text alignment - CENTER or RIGHT or LEFT
    );
    returnButton->setId("return_button");
    // Connect button callback
    returnButton->setOnPress([this]() { onReturnButton(); });
    addElement(std::move(returnButton));

    // ========================
    // Input mesurments
    // ========================

    // Calculate layout parameters
    int numMeasurements = Constants::MEASUREMENTS.size();
    int elementHeight = 30; // Height of each numeric input
    int spacing = 10; // Spacing between elements
    int totalHeightNeeded = numMeasurements * elementHeight + (numMeasurements - 1) * spacing;

    // Calculate starting Y position to center vertically within the panel
    int startY = boxMeasurements.y + (boxMeasurements.height - totalHeightNeeded) / 2;

    // Create a numeric input for each measurement
    for (int i = 0; i < numMeasurements; i++) {
        const std::string& measurement = Constants::MEASUREMENTS[i];
        
        // Calculate Y position for this element
        int elementY = startY + i * (elementHeight + spacing);
        
        // Capitalize first letter for display
        std::string displayName = measurement;
        if (!displayName.empty()) {
            displayName[0] = std::toupper(displayName[0]);
        }
        
        auto measurementInput = std::make_unique<NumericInput>(
            renderContext,
            boxMeasurements.x, 
            elementY, 
            boxMeasurements.width, 
            elementHeight,
            displayName,
            getMeasurementValue(measurement),  // Use the helper function
            0.0f,
            200.0f,
            0.5f,
            true,
            20,
            1,
            nullptr,
            LEFT,
            RIGHT
        );
        // Set ID based on measurement name
        measurementInput->setId(measurement + "_input");
        measurementInput->setVertical(false);        
        // Add to menu
        addElement(std::move(measurementInput));

        // ========================
        // Previous Labels
        // ========================

        // And update the previous labels too:
        auto previousMesurementLabel = std::make_unique<Label>(
            renderContext,
            boxPrevMeasurements.x, 
            elementY, 
            boxPrevMeasurements.width, 
            elementHeight,
            "Prev | " + formatFloat(getMeasurementValue(measurement)), // Format to 2 decimal places
            false,
            20,
            1,
            nullptr,
            LEFT
        );
        previousMesurementLabel->setId(measurement + "_label");
        previousMesurementLabel->setWrapText(false);
        addElement(std::move(previousMesurementLabel));
    }

    // ========================
    // Input weight
    // ========================

    auto weightInput = std::make_unique<NumericInput>(
            renderContext,
            boxWeight.x, 
            boxWeight.y, 
            boxWeight.width, 
            boxWeight.height,
            "Weight >>",           // Prompt text 
            latest.weight,          // Initial value 
            30.0f,                  // Minimum value
            200.0f,                // Maximum value (adjust as needed for measurements)
            0.5f,                  // Step value (increment/decrement by 0.5)
            true,                  // Show border
            26,                    // Font size 
            1,                     // Layer
            nullptr,               // Parent
            LEFT,                  // Prompt text alignment
            RIGHT                  // Value text alignment (right-align numbers)
        );
        
        // Set ID based on measurement name
        weightInput->setId("weight_input");
        weightInput->setVertical(false);
        
        // Add to menu
        addElement(std::move(weightInput));

        // ========================
        // Commit button
        // ========================

        auto commitButton = std::make_unique<Button>(
        renderContext,  // Use renderContext instead of context
        boxCommitButton.x, boxCommitButton.y, boxCommitButton.width, boxCommitButton.height,
        "Commit",
        true,  // Show border
        22,    // Medium font
        1,      // Layer
        nullptr, // parent
        LEFT  // Text alignment - CENTER or RIGHT or LEFT
    );
    commitButton->setId("commit_button");
    // Connect button callback
    commitButton->setOnPress([this]() { onCommitButton(); });
    commitButton->setWrapText(false);
    addElement(std::move(commitButton));

    // ========================
    // Plotter
    // ========================
    std::vector<std::string> testX = {"2024-01-01", "2024-01-05", "2024-01-08","2024-02-21"};
    std::vector<float> testY = {12.4,21.2,51.1,91.7};

    auto plotter = std::make_unique<Plotter2dElement>(
        renderContext,
        boxStats.x,boxStats.y,boxStats.width,boxStats.height,
        "Test Plot",       // title
        1,                 // Layer
        nullptr            // parent
    );
    plotter->addPlotLine(testX,testY,"test",glm::vec4(style.text_color.r/255.0f, style.text_color.g/255.0f, style.text_color.b/255.0f, 1));
    plotter->setId("plotter");
    addElement(std::move(plotter));

    #pragma endregion <Element Definitions>

    // Set focus to the first interactive element (Select User dropdown)
    
    
}

#pragma region <overrides>
void EditUserMenu::onReturnButton() {
    renderContext->setCurrentMenu(returnMenu);
}

void EditUserMenu::onCommitButton() {
    try {
        // Get current date in YYYY-MM-DD format
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time_t);
        
        std::stringstream dateSS;
        dateSS << std::put_time(&tm, "%Y-%m-%d");
        std::string currentDate = dateSS.str();

        // Create UserMeasurements object
        UserMeasurements newMeasurements;
        newMeasurements.date = currentDate;

        // Get weight value
        NumericInput* weightInput = dynamic_cast<NumericInput*>(getElement("weight_input"));
        if (weightInput) {
            newMeasurements.weight = weightInput->getValue();
        }

        // Get all measurement values
        for (const std::string& measurement : Constants::MEASUREMENTS) {
            NumericInput* input = dynamic_cast<NumericInput*>(getElement(measurement + "_input"));
            if (input) {
                float value = input->getValue();
                
                // Map measurement names to struct fields
                if (measurement == "arms") newMeasurements.arms = value;
                else if (measurement == "calves") newMeasurements.calves = value;
                else if (measurement == "neck") newMeasurements.neck = value;
                else if (measurement == "thighs") newMeasurements.thighs = value;
                else if (measurement == "chest") newMeasurements.chest = value;
                else if (measurement == "waist") newMeasurements.waist = value;
                else if (measurement == "hips") newMeasurements.hips = value;
                else if (measurement == "forearms") newMeasurements.forearms = value;
            }
        }

        
        bool success = dbManager->insertUserMeasurements(this->user, newMeasurements);
        
        // Optionally update the latest measurements cache
        int userId = dbManager->getUserIdByName(this->user);
        if (userId != -1) {
            this->latest = dbManager->getLatestUserMeasurements(userId);
        }
           
        // Show immediate feedback to user
        renderContext->addNotification("Updated for > " + user);
        
        // Optional: Update the previous labels to show the newly committed values
        updatePreviousLabels();

    } catch (const std::exception& e) {
        std::cerr << "Error in onCommitButton: " << e.what() << std::endl;
        renderContext->addNotification("Error on commit");
    }
}

void EditUserMenu::updatePreviousLabels() {
    // Update all previous measurement labels with current input values
    for (const std::string& measurement : Constants::MEASUREMENTS) {
        Label* label = dynamic_cast<Label*>(getElement(measurement + "_label"));
        NumericInput* input = dynamic_cast<NumericInput*>(getElement(measurement + "_input"));
        
        if (label && input) {
            float value = input->getValue();
            label->setText("Prev | " + formatFloat(value));
        }
    }
    
    // Also update weight in the latest cache for consistency
    NumericInput* weightInput = dynamic_cast<NumericInput*>(getElement("weight_input"));
    if (weightInput) {
        latest.weight = weightInput->getValue();
    }
}

#pragma endregion <overrides>

// Menu update override
void EditUserMenu::update(float deltaTime) {
    // Call base Menu update to handle element updates
    Menu::update(deltaTime);

    // Additional EditUserMenu-specific updates 
    
}

// Async data load, propably call before init(), wrap in worker thread
void EditUserMenu::loadAsyncData(){
    int userId = dbManager->getUserIdByName(user);
    this->latest =  dbManager->getLatestUserMeasurements(userId);  
    // std::cout << "Latest measurements for user 'test':" << std::endl;
    // std::cout << "Date: " << latest.date << std::endl;
    // std::cout << "Weight: " << latest.weight << " kg" << std::endl;
    // std::cout << "Chest: " << latest.chest << " cm" << std::endl;
    // std::cout << "Waist: " << latest.waist << " cm" << std::endl;     
}