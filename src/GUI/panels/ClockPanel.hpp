#ifndef CLOCKPANEL_HPP
#define CLOCKPANEL_HPP

#include "../Menu.hpp"
#include "../menus/StartMenu.hpp"
#include "../menus/OverviewMenu/OverviewMenu.hpp"
#include "../elements/Label.hpp"
#include "../elements/Button.hpp"
#include "../elements/SpriteElement.hpp"
#include "../AppGlobals.h"
#include "../widgets/CatWidget.hpp"
#include <memory>
#include <vector>
#include <functional>

class ClockPanel {
public:
    static std::vector<std::unique_ptr<BaseElement>> create(
        RenderContext* context, 
        int x, int y, 
        Menu* parent = nullptr
    );
    
    // Static method to update all clock elements in a menu
    static void updateClocks(Menu* menu, float dt);
    
private:
    static void updateClockDisplay(Label* timeLabel, Label* dateLabel);
    static std::string getCurrentTime();
    static std::string getCurrentDate();

    void onMenu1Button();
};

#endif