#ifndef STARTMENU_HPP
#define STARTMENU_HPP

#include "../Menu.hpp"
#include "../Label.hpp"

class StartMenu : public Menu {
public:
    StartMenu(RenderContext* context);
    void init() override;
    
private:
    void onStartGame();
    void onOptions();
    void onExit();
};

#endif