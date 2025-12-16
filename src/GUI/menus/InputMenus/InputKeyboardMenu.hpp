#ifndef INPUTKEYBOARDMENU_HPP
#define INPUTKEYBOARDMENU_HPP

#include "../../Menu.hpp"
#include <string>
#include <functional>
#include "../../elements/Button.hpp"
#include "../../elements/Label.hpp"

class InputKeyboardMenu : public Menu {
private:
    Menu* returnMenu;
    std::string returnFocusId;
    std::string* outputString;  // External pointer - only written on Enter
    std::string internalString; // Internal accumulator
    std::function<void(const std::string&)> onEnterCallback;  // Callback for Enter press

public:
    InputKeyboardMenu(RenderContext* context, WorkThread* workThread,DatabaseManager* dbMgr, 
                     Menu* returnMenu, BaseElement* returnFocus, 
                     std::string* outputString);
    
    virtual void init() override;
    
    // Getters and setters
    const std::string& getInternalString() const { return internalString; }
    
    // Set callback for when Enter is pressed
    void setOnEnterCallback(std::function<void(const std::string&)> callback) {
        onEnterCallback = callback;
    }
    
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