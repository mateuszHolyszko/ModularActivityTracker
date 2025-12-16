#include "ProgramMenu.hpp"
#include <iostream>
#include "../../panels/ClockPanel.hpp"
#include "ProgramMenuLayout.hpp"

ProgramMenu::ProgramMenu(RenderContext* context, WorkThread* workThread, 
                         DatabaseManager* dbMgr, const std::string& userName)
    : Menu(context, workThread, dbMgr, "ProgramMenu"), userName(userName), currentProgram("") {
}

void ProgramMenu::init() {
    // Add ClockPanel (group of elements)
    auto clockPanel = ClockPanel::create(renderContext, 0, 0, this);
    for (auto& element : clockPanel) addElement(std::move(element));
    
    // Set MenuButtons styles to indicate active menu
    dynamic_cast<Button*>(getElement("menu1_button"))->setActivated(false);
    dynamic_cast<Button*>(getElement("menu2_button"))->setSelected(true);  
    dynamic_cast<Button*>(getElement("menu3_button"))->setSelected(false);
    dynamic_cast<Button*>(getElement("menu4_button"))->setSelected(false);

    // Load boxes from layout
    const Box& boxChoosenProgram = layout.at("ChoosenProgramLabel");
    const Box& boxProgramList = layout.at("ProgramList");
    const Box& boxProgramSummary = layout.at("ProgramSummaryPanel");
    const Box& boxEditProgram = layout.at("EditProgramScrollPane");

    #pragma region <Element Definitions>
    
    // Chosen Program Label // =====================================
    auto choosenProgramLabel = std::make_unique<Label>(
        renderContext,
        boxChoosenProgram.x, boxChoosenProgram.y, 
        boxChoosenProgram.width, boxChoosenProgram.height,
        "No Program Selected",
        true,   // Show border
        28,     // Font size
        1,      // Layer
        nullptr, // parent
        CENTER  // Text alignment
    );
    choosenProgramLabel->setId("choosen_program_label");
    choosenProgramLabel->setWrapText(true);
    addElement(std::move(choosenProgramLabel));

    // Program List (static buttons fitted in box) // =====================================
    loadProgramList();

    // Program Summary Panel // =====================================
    auto programSummaryLabel = std::make_unique<Label>(
        renderContext,
        boxProgramSummary.x, boxProgramSummary.y,
        boxProgramSummary.width, boxProgramSummary.height,
        "Select a program to view details",
        true,   // Show border
        20,     // Font size
        1,      // Layer
        nullptr, // parent
        LEFT    // Text alignment
    );
    programSummaryLabel->setId("program_summary_label");
    programSummaryLabel->setWrapText(true);
    addElement(std::move(programSummaryLabel));

    // EditProgramScrollPane // =====================================
    editProgramScrollPane = addScrollPane(
        boxEditProgram.x, boxEditProgram.y,
        boxEditProgram.width, boxEditProgram.height,
        boxEditProgram.width, boxEditProgram.height*3  // Virtual size for now
    );
    
    if (!editProgramScrollPane) {
        std::cerr << "Failed to create edit program scroll pane!" << std::endl;
        return;
    }

    // TODO: Implement EditProgramScrollPane content
    // Create ScrollBarWidget for the edit program scroll pane
    editProgramScrollBar = std::make_unique<ScrollBarWidget>(editProgramScrollPane, *renderContext);

    #pragma endregion <Element Definitions>

    // Set focus to the first program button if available
    BaseElement* firstProgramButton = getElement("program_button_0");
    if (firstProgramButton) {
        setFocus(firstProgramButton);
    }
}

void ProgramMenu::loadProgramList() {
    // Clear focus explicitly before removing elements to avoid dangling pointer issues
    clearFocus();

    // Clear existing program buttons (assume max 25 buttons including the "Add" button)
    for (int i = 0; i < 25; ++i) {
        if (getElement("program_button_" + std::to_string(i))){
            removeElement("program_button_" + std::to_string(i));
        }
        if (getElement("program_delete_button_" + std::to_string(i))){
            removeElement("program_delete_button_" + std::to_string(i));
        }    
    }
    if (getElement("add_program_button")) removeElement("add_program_button");

    // Fetch programs from database for this userName
    int userId = dbManager->getUserIdByName(userName);
    std::vector<std::string> programNames = dbManager->getProgramNamesForUser(userId);

    const Box& boxProgramList = layout.at("ProgramList");
    
    // Add 1 to size for the "Add" button
    size_t totalButtons = programNames.size() + 1;
    
    // Calculate button dimensions to fit within the ProgramList box
    int buttonHeight = 50;
    int fractionSize = boxProgramList.height / totalButtons;
    
    // Create buttons for existing programs
    for (size_t i = 0; i < programNames.size(); ++i) {
        int buttonWidth = (boxProgramList.width - 20);
        int programButtonWidth = (buttonWidth * 5) / 6;
        int deleteButtonWidth = buttonWidth - programButtonWidth;
        int xOffset = boxProgramList.x + 10;
        int yPosition = (i + 1) * fractionSize - fractionSize / 2 - buttonHeight / 2 + boxProgramList.y;
        
        // Create program selection button (5/6 of width)
        auto programButton = std::make_unique<Button>(
            renderContext,
            xOffset,
            yPosition,
            programButtonWidth,
            buttonHeight,
            programNames[i],
            true,   // Show border
            20,     // Font size
            1,      // Layer
            nullptr, // parent
            CENTER    // Text alignment
        );
        
        programButton->setId("program_button_" + std::to_string(i));
        programButton->setWrapText(false);
        
        // Set callback for program selection
        std::string programName = programNames[i];
        programButton->setOnPress([this, programName]() {
            onProgramSelected(programName);
        });
        
        addElement(std::move(programButton));
        
        // Create delete button (1/6 of width)
        auto deleteButton = std::make_unique<Button>(
            renderContext,
            xOffset + programButtonWidth,
            yPosition,
            deleteButtonWidth,
            buttonHeight,
            "X",
            true,   // Show border
            20,     // Font size
            1,      // Layer
            nullptr, // parent
            CENTER    // Text alignment
        );
        
        deleteButton->setId("program_delete_button_" + std::to_string(i));
        deleteButton->setWrapText(false);
        
        // Set callback for delete button
        std::string programNameForDelete = programNames[i];
        deleteButton->setOnPress([this, programNameForDelete]() {
            onDeleteProgram(programNameForDelete);
        });
        
        addElement(std::move(deleteButton));
    }
    
    // Create "Add" button at the end
    auto addButton = std::make_unique<Button>(
        renderContext,
        boxProgramList.x + 10 + ((boxProgramList.width - 20)/4), 
        programNames.size() * fractionSize + fractionSize / 2 - buttonHeight / 2 + boxProgramList.y,
        (boxProgramList.width - 20)/2, 
        buttonHeight,
        "Add",
        true,   // Show border
        20,     // Font size
        1,      // Layer
        nullptr, // parent
        CENTER  // Text alignment
    );
    
    addButton->setId("add_program_button");
    addButton->setWrapText(false);
    
    // Set callback for add button
    addButton->setOnPress([this]() {
        onAddNewProgram();
    });
    
    addElement(std::move(addButton));

    // Restore focus only if it previously belonged to the program-list area
    if (programNames.size() > 0) {
        BaseElement* firstButton = getElement("program_button_0");
        if (firstButton && firstButton->isSelectable() && firstButton->isEnabled()) {
            setFocus(firstButton);
        }
    }else {
        BaseElement* addButtonElement = getElement("add_program_button");
        if (addButtonElement && addButtonElement->isSelectable() && addButtonElement->isEnabled()) {
            setFocus(addButtonElement);
        }
    }
}

void ProgramMenu::onProgramSelected(const std::string& programName) {
    std::cout << "ProgramMenu: Selected program: " << programName << std::endl;
    
    // Update current program
    currentProgram = programName;
    
    // Update the chosen program label
    auto* label = dynamic_cast<Label*>(getElement("choosen_program_label"));
    if (label) {
        label->setText(programName);
    }
    
    // Update program summary
    updateProgramSummary();
    
    // Force update to reflect changes
    update(0.0f);
}

void ProgramMenu::onAddNewProgram() {
    std::cout << "ProgramMenu: Opening keyboard input for new program..." << std::endl;
    
    auto keyboardMenu = std::make_unique<InputKeyboardMenu>(
        renderContext, 
        worker, 
        dbManager,
        this,                    // returnMenu
        getCurrentFocus(),       // returnFocus  
        &newProgramName          // pointer to output string
    );
    
    // Set callback for when Enter is pressed
    keyboardMenu->setOnEnterCallback([this](const std::string& programName) {
        std::cerr << "New program name entered: " << programName << std::endl;

        // Add the new program to the database
        dbManager->addProgramForUser(
            dbManager->getUserIdByName(userName), 
            programName
        );
        // Force update program list
        loadProgramList();
        update(0.0f); // Force update to reflect changes
    });
    
    keyboardMenu->init();
    
    // Set it as current menu in the context
    renderContext->setCurrentMenu(keyboardMenu.get());
    
    // Store the keyboard menu pointer so it doesn't get destroyed
    this->keyboardMenu = std::move(keyboardMenu);
    
    std::cout << "Opened keyboard menu for new program" << std::endl;
}

void ProgramMenu::updateProgramSummary() {
    auto* summaryLabel = dynamic_cast<Label*>(getElement("program_summary_label"));
    if (!summaryLabel) return;

    if (currentProgram.empty()) {
        summaryLabel->setText("Select a program to view details");
        return;
    }

    // TODO: Fetch actual program details from database
    // For now, display placeholder information
    std::string summaryText = "Program: " + currentProgram + "\n";
    summaryText += "Duration: 8 weeks\n";
    summaryText += "Sessions per week: 3\n";
    summaryText += "Difficulty: Intermediate";
    
    summaryLabel->setText(summaryText);
}

void ProgramMenu::onDeleteProgram(const std::string& programName) {
    std::cout << "ProgramMenu: Deleting program: " << programName << std::endl;
    
    // Get the program ID from the program name
    int userId = dbManager->getUserIdByName(userName);
    int programId = dbManager->getProgramIdByName(userId,programName);
    
    // Delete the program from the database
    dbManager->removeProgramById(programId);
    
    // If currentProgram was deleted, clear it
    if (currentProgram == programName) {
        currentProgram = "";
        auto* label = dynamic_cast<Label*>(getElement("choosen_program_label"));
        if (label) {
            label->setText("No Program Selected");
        }
    }
    
    // Refresh the program list
    loadProgramList();
    update(0.0f);
}

void ProgramMenu::update(float deltaTime) {
    // Call base Menu update to handle element updates
    Menu::update(deltaTime);

    // Update scrollbar for edit program area
    if (editProgramScrollBar) {
        editProgramScrollBar->update(deltaTime);
    }

    // Additional ProgramMenu-specific updates can go here
}