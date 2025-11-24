#ifndef EditUserMenu_HPP
#define EditUserMenu_HPP

#include "../../Menu.hpp"
#include "../../elements/Button.hpp"
#include "../../elements/Label.hpp"
#include "../../elements/DropDown.hpp"
#include "../../elements/NumericInput.hpp"
#include "../../elements/ViewportElement.hpp"
#include "../../elements/Plotter2dElement.hpp"
#include "../../WorkThread.hpp"  
#include "../InputMenus/InputKeyboardMenu.hpp"
#include "../InputMenus/SelectInputMenu.hpp"
#include "../../../ImageManager.hpp"
#include "../../elements/ImageElement.hpp"
#include "../../../Constants.hpp"

// Forward declaration instead of including InitMenu.hpp
class InitMenu;

class EditUserMenu : public Menu {
private:    
    std::unique_ptr<SelectInputMenu> selectMenu; 
    std::string selectText; // New for select input
    ImageManager imageManager;  // Create instance as member variable
    InitMenu* returnMenu;
    std::string user;
    UserMeasurements latest;
    
    // Plotter query fields
    std::string plotterQueryMeasurement;
    int plotterQueryWeeks;
    
public:
    EditUserMenu(RenderContext* context, WorkThread* workThread, DatabaseManager* dbMgr, InitMenu* returnMenu, const std::string& user);
    void init() override;
    void update(float deltaTime) override;
    void loadAsyncData();
private:
    // Helper function to get measurement value by name
    float getMeasurementValue(const std::string& measurementName) const {
        if (measurementName == "arms") return latest.arms;
        if (measurementName == "calves") return latest.calves;
        if (measurementName == "neck") return latest.neck;
        if (measurementName == "thighs") return latest.thighs;
        if (measurementName == "chest") return latest.chest;
        if (measurementName == "waist") return latest.waist;
        if (measurementName == "hips") return latest.hips;
        if (measurementName == "forearms") return latest.forearms;
        return 0.0f; // default if not found
    }
    // Helper function to format float to 2 decimal places
    std::string formatFloat(float value) const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << value;
        return ss.str();
    }
    void updatePreviousLabels();
    void updatePlotter();
    
    void onReturnButton();
    void onCommitButton();
};

#endif