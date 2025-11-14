#ifndef INPUTKEYBOARDMENU_HPP
#define INPUTKEYBOARDMENU_HPP

#include "../../Menu.hpp"
#include <string>
#include "../../elements/Button.hpp"
#include "../../elements/Label.hpp"

class InputKeyboardMenu : public Menu {
private:
    Menu* returnMenu;
    BaseElement* returnFocus;
    std::string* outputString;  // External pointer - only written on Enter
    std::string internalString; // Internal accumulator

public:
    InputKeyboardMenu(RenderContext* context, WorkThread* workThread, 
                     Menu* returnMenu, BaseElement* returnFocus, 
                     std::string* outputString);
    
    virtual void init() override;
    
    // Getters and setters
    const std::string& getInternalString() const { return internalString; }
    
    // Close the keyboard and return to the previous menu
    void closeMenu();

private:
    void updateOutputLabel();
    void appendToInternal(char c);
    void appendToInternal(const std::string& str);
    void backspaceInternal();
    void clearInternal();
};

#endif