#ifndef STARTMENU_HPP
#define STARTMENU_HPP

#include "../Menu.hpp"
#include "../elements/Button.hpp"
#include "../elements/Label.hpp"
#include "../elements/ViewportElement.hpp"
#include "../WorkThread.hpp"  
#include "InputMenus/InputKeyboardMenu.hpp"

class StartMenu : public Menu {
private:
    std::unique_ptr<InputKeyboardMenu> keyboardMenu; 
    std::string inputText = ""; // Text that will hold keyboard input
public:
    StartMenu(RenderContext* context, WorkThread* workThread);
    void init() override;

private:
    void onStartGame();
    void onViewHistory();
    void onSettings();
    void onExit();
};

#endif
