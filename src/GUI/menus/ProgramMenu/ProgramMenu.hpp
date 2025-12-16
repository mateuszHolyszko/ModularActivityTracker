#ifndef PROGRAMMENU_HPP
#define PROGRAMMENU_HPP

#include "../../Menu.hpp"
#include "../../elements/Button.hpp"
#include "../../elements/Label.hpp"
#include "../../elements/DropDown.hpp"
#include "../../WorkThread.hpp"
#include "../../ScrollPane.hpp"
#include "../../widgets/ScrollBarWidget.hpp"
#include "../InputMenus/InputKeyboardMenu.hpp"
#include <string>

class ProgramMenu : public Menu {
private:
    std::string userName;
    std::string currentProgram;  // Holds name/id of current program
    
    ScrollPane* editProgramScrollPane = nullptr;
    std::unique_ptr<ScrollBarWidget> editProgramScrollBar;
    
    std::unique_ptr<InputKeyboardMenu> keyboardMenu;
    std::string newProgramName = "";  // Text that will hold keyboard input for new program
    
public:
    ProgramMenu(RenderContext* context, WorkThread* workThread, 
                DatabaseManager* dbMgr, const std::string& userName);
    
    void init() override;
    void update(float deltaTime) override;
    
    // Getters/Setters for currentProgram
    const std::string& getCurrentProgram() const { return currentProgram; }
    void setCurrentProgram(const std::string& program) { currentProgram = program; }

private:
    void onProgramSelected(const std::string& programName);
    void onAddNewProgram();
    void onDeleteProgram(const std::string& programName);
    void loadProgramList();
    void updateProgramSummary();
};

#endif