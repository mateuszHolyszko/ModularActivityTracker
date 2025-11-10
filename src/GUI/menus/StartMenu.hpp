#ifndef STARTMENU_HPP
#define STARTMENU_HPP

#include "../Menu.hpp"
#include "../Label.hpp"
#include "../Button.hpp"

class StartMenu : public Menu {
public:
    StartMenu(RenderContext* context);
    void init() override;

private:
    void onStartGame();
    void onViewHistory();
    void onSettings();
    void onExit();
};

#endif