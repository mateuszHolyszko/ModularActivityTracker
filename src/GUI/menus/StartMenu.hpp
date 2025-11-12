#ifndef STARTMENU_HPP
#define STARTMENU_HPP

#include "../Menu.hpp"
#include "../elements/Button.hpp"
#include "../elements/Label.hpp"
#include "../WorkThread.hpp"  

class StartMenu : public Menu {
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
