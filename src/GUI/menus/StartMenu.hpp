#ifndef STARTMENU_HPP
#define STARTMENU_HPP

#include "../Menu.hpp"
#include "../elements/Button.hpp"
#include "../elements/Label.hpp"
#include "../elements/DropDown.hpp"
#include "../elements/NumericInput.hpp"
#include "../elements/ViewportElement.hpp"
#include "../WorkThread.hpp"  
#include "InputMenus/InputKeyboardMenu.hpp"
#include "InputMenus/SelectInputMenu.hpp"
#include "../../ImageManager.hpp"
#include "../elements/ImageElement.hpp"

class StartMenu : public Menu {
private:
    std::unique_ptr<InputKeyboardMenu> keyboardMenu; 
    std::unique_ptr<SelectInputMenu> selectMenu; 
    std::string inputText = ""; // Text that will hold keyboard input
    std::string selectText; // New for select input
    ImageManager imageManager;  // Create instance as member variable
public:
    StartMenu(RenderContext* context, WorkThread* workThread, DatabaseManager* dbMgr);
    void init() override;

private:
    void onStartGame();
    void onViewHistory();
    void onSettings();
    void onExit();
    void onTestSelectInput();
};

#endif
