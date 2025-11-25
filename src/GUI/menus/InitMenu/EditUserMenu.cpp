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
      returnMenu(returnMenu),
      user(user),
      plotterQueryMeasurement("weight"),  // Initialize with "weight"
      plotterQueryWeeks(3)                // Initialize with 3 weeks
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
    const Box& boxQueryWeeks = layout.at("QueryWeeks");
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
    // QuerySelect - DropDown for measurement selection
    // ========================

    auto measurementDropDown = std::make_unique<DropDown>(
        renderContext,
        boxStatsQuery.x,
        boxStatsQuery.y,
        boxStatsQuery.width,
        boxStatsQuery.height,
        "Weight",  // Initial prompt showing "Weight"
        true,      // Show border
        20,        // Font size
        5,         // Layer
        nullptr,   // Parent
        LEFT       // Text alignment
    );
    
    measurementDropDown->setId("measurement_dropdown");
    
    // Build options list with weight first, then all measurements (capitalized)
    std::vector<std::string> measurementOptions;
    measurementOptions.push_back("Weight");  // Add weight first
    for (const std::string& measurement : Constants::MEASUREMENTS) {
        std::string displayName = measurement;
        if (!displayName.empty()) {
            displayName[0] = std::toupper(displayName[0]);
        }
        measurementOptions.push_back(displayName);
    }
    measurementDropDown->setOptions(measurementOptions);
    measurementDropDown->setSelectedOptionIndex(0); // Select "Weight" by default
    
    // Set callback to update plotterQueryMeasurement and refresh plot
    measurementDropDown->setOnOptionSelected([this](int index, const std::string& selected) {
        // Convert back to lowercase for database query (except Weight)
        if (selected == "Weight") {
            this->plotterQueryMeasurement = "weight";
        } else {
            std::string lowercase = selected;
            lowercase[0] = std::tolower(lowercase[0]);
            this->plotterQueryMeasurement = lowercase;
        }
        this->updatePlotter();
        std::cout << "Selected measurement: " << this->plotterQueryMeasurement << std::endl;
    });
    
    measurementDropDown->setDropDownDirection(1); // UP
    measurementDropDown->setDropDownHeight(250);  // Adjust height as needed
    
    addElement(std::move(measurementDropDown));

    // ========================
    // QueryWeeksInputNumeric
    // ========================

    auto weeksInput = std::make_unique<NumericInput>(
        renderContext,
        boxQueryWeeks.x,
        boxQueryWeeks.y,
        boxQueryWeeks.width,
        boxQueryWeeks.height,
        "Weeks",               // Prompt text
        static_cast<float>(plotterQueryWeeks), // Initial value (3)
        1.0f,                  // Minimum value (at least 1 week)
        52.0f,                 // Maximum value (1 year)
        1.0f,                  // Step value (whole weeks)
        true,                  // Show border
        26,                    // Font size
        1,                     // Layer
        nullptr,               // Parent
        LEFT,                  // Prompt text alignment
        RIGHT                  // Value text alignment
    );
    
    weeksInput->setId("weeks_input");
    weeksInput->setVertical(true);
    
    // Set callback to update plotterQueryWeeks and refresh plot when value changes
    weeksInput->setOnInput([this](float newValue) {
        this->plotterQueryWeeks = static_cast<int>(newValue);
        this->updatePlotter();
        std::cout << "Query weeks changed to: " << this->plotterQueryWeeks << std::endl;
    });
    
    addElement(std::move(weeksInput));

    // ========================
    // Plotter
    // ========================

    auto plotter = std::make_unique<Plotter2dElement>(
        renderContext,
        boxStats.x,boxStats.y,boxStats.width,boxStats.height,
        plotterQueryMeasurement + " - Last " + std::to_string(plotterQueryWeeks) + " weeks",       // title
        1,                 // Layer
        nullptr            // parent
    );

    std::vector<std::string> testX_1;
    std::vector<float> testY_1;
    dbManager->getUserHistory(dbManager->getUserIdByName(user), plotterQueryMeasurement, plotterQueryWeeks, testX_1, testY_1);

    glm::vec4 colour_1 = glm::vec4(style.getTextColor().r/255.0f,style.getTextColor().g/255.0f,style.getTextColor().b/255.0f,1.0f);
    plotter->addPlotLine(testX_1,testY_1,"test_1",colour_1);

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

    // Update Plot with new data
    updatePlotter();
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

void EditUserMenu::updatePlotter() {
    Plotter2dElement* plotter = dynamic_cast<Plotter2dElement*>(getElement("plotter"));
    if (!plotter) return;
    
    // Clear existing plot lines
    plotter->clearPlotLines();
    
    // Fetch new data based on current query parameters
    std::vector<std::string> dates;
    std::vector<float> values;
    int userId = dbManager->getUserIdByName(user);
    
    if (userId != -1) {
        dbManager->getUserHistory(userId, plotterQueryMeasurement, plotterQueryWeeks, dates, values);
        
        // Add new plot line
        glm::vec4 colour = glm::vec4(
            style.getMuscleColor("Chest").r/255.0f,
            style.getMuscleColor("Chest").g/255.0f,
            style.getMuscleColor("Chest").b/255.0f,
            1.0f
        );
        
        // Capitalize measurement name for display
        std::string displayName = plotterQueryMeasurement;
        if (!displayName.empty()) {
            displayName[0] = std::toupper(displayName[0]);
        }
        // If its weight querry change colour to textColor
        if (plotterQueryMeasurement == "weight") {
            colour = glm::vec4(
                style.getTextColor().r/255.0f,
                style.getTextColor().g/255.0f,
                style.getTextColor().b/255.0f,
                1.0f
            );
        }
        
        plotter->addPlotLine(dates, values, displayName, colour);

        // If query is not weight, add weight alongside for reference
        if (plotterQueryMeasurement != "weight") {
            std::vector<std::string> weightDates;
            std::vector<float> weightValues;
            dbManager->getUserHistory(userId, "weight", plotterQueryWeeks, weightDates, weightValues);
            plotter->addPlotLine(weightDates, weightValues, "Weight", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Light grey for weight
        } 
        
        // Update plotter title
        plotter->setTitle(displayName + " - Last " + std::to_string(plotterQueryWeeks) + " weeks");
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