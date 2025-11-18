#ifndef SELECTINPUTMENU_HPP
#define SELECTINPUTMENU_HPP

#include "../../Menu.hpp"
#include <string>
#include <vector>

class SelectInputMenu : public Menu {
private:
    Menu* returnMenu;
    BaseElement* returnFocus;
    std::string* outputString;
    std::vector<std::string> options;

public:
    SelectInputMenu(RenderContext* context, WorkThread* workThread, DatabaseManager* dbMgr,
                    Menu* returnMenu, BaseElement* returnFocus,
                    std::string* outputString, std::vector<std::string> options);


    void init() override;
    void closeMenu();
};

#endif