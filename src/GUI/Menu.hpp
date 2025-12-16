#ifndef MENU_HPP
#define MENU_HPP

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "elements/BaseElement.hpp"
#include "../RenderContext.hpp"
#include "WorkThread.hpp"
#include "../database/DatabaseManager.hpp"
#include "ScrollPane.hpp"

class Menu {
protected:
    std::string name;
    BaseElement* current_focus;
    std::vector<std::unique_ptr<BaseElement>> guiElements;
    std::vector<std::unique_ptr<ScrollPane>> scrollPanes;  // Add scroll panes
    RenderContext* renderContext;
    WorkThread* worker;
    DatabaseManager* dbManager;

public:
    Menu(RenderContext* context, WorkThread* workThread, DatabaseManager* dbMgr, const std::string& menuName);
    virtual ~Menu() = default;

    // Pure virtual method that derived menus must implement
    virtual void init() = 0;

    // Common menu methods
    virtual void render();
    virtual void handleEvent(const SDL_Event& event);
    virtual void update(float deltaTime);
    
    // Focus management
    virtual void setFocus(BaseElement* element);
    virtual void clearFocus();
    virtual BaseElement* getCurrentFocus() const { return current_focus; }
    
    // Element management
    virtual void addElement(std::unique_ptr<BaseElement> element);
    virtual void removeElement(const std::string& elementId);
    virtual BaseElement* getElement(const std::string& elementId);
    
    // ScrollPane management
    virtual ScrollPane* addScrollPane(float displayX, float displayY, float displayWidth, 
                                      float displayHeight, float virtualWidth, float virtualHeight);
    virtual ScrollPane* getScrollPane(size_t index);
    virtual size_t getScrollPaneCount() const { return scrollPanes.size(); }
    
    // Getters
    virtual const std::string& getName() const { return name; }
    virtual size_t getElementCount() const { return guiElements.size(); }
    virtual bool isEmpty() const { return guiElements.empty(); }
    RenderContext* getRenderContext() const { return renderContext; }
    DatabaseManager* getDatabaseManager() const { return dbManager; }

private:
    // Arrow navigation helper method
    void navigate_arrows(int dirX, int dirY);
    // Helper to check if element is mostly clipped by scroll pane
    double navigationConeAngleDegrees = 60.0;
};

#endif