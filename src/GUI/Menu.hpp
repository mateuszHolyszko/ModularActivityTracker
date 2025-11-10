#ifndef MENU_HPP
#define MENU_HPP

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "BaseElement.hpp"
#include "../RenderContext.hpp"

class Menu {
protected:
    std::string name;
    BaseElement* current_focus;
    std::vector<std::unique_ptr<BaseElement>> guiElements;
    RenderContext* renderContext;

public:
    Menu(RenderContext* context, const std::string& menuName);
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
    
    // Getters
    virtual const std::string& getName() const { return name; }
    virtual size_t getElementCount() const { return guiElements.size(); }
    virtual bool isEmpty() const { return guiElements.empty(); }

private:
    // Arrow navigation helper method
    void navigate_arrows(int dirX, int dirY);
};

#endif