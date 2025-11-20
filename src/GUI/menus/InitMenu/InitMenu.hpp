#ifndef InitMenu_HPP
#define InitMenu_HPP

#include "../../Menu.hpp"
#include "../../elements/Button.hpp"
#include "../../elements/Label.hpp"
#include "../../elements/DropDown.hpp"
#include "../../elements/NumericInput.hpp"
#include "../../elements/ViewportElement.hpp"
#include "../../WorkThread.hpp"  
#include "../InputMenus/InputKeyboardMenu.hpp"
#include "../InputMenus/SelectInputMenu.hpp"
#include "../../../ImageManager.hpp"
#include "../../elements/ImageElement.hpp"

class InitMenu : public Menu {
private:    
    std::unique_ptr<SelectInputMenu> selectMenu; 
    std::string selectText; // New for select input
    ImageManager imageManager;  // Create instance as member variable
public:
    InitMenu(RenderContext* context, WorkThread* workThread, DatabaseManager* dbMgr);
    void init() override;
    void onEditUserButton();
    void update(float deltaTime) override;
private:
    
};

#endif
