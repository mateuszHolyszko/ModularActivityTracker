#include "Menu.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

Menu::Menu(RenderContext* context, const std::string& menuName)
    : renderContext(context), name(menuName), current_focus(nullptr) {
}

void Menu::render() {
    // Create a temporary vector of visible elements
    std::vector<BaseElement*> visibleElements;
    
    // Collect all visible elements
    for (auto& element : guiElements) {
        if (element->isVisible()) {
            visibleElements.push_back(element.get());
        }
    }
    
    // Sort visible elements by layer (lower layers first, higher layers on top)
    std::sort(visibleElements.begin(), visibleElements.end(),
        [](BaseElement* a, BaseElement* b) {
            return a->getLayer() < b->getLayer();
        });
    
    // Render elements in sorted order
    for (auto* element : visibleElements) {
        element->render();
    }
}

void Menu::handleEvent(const SDL_Event& event) {
    // Handle arrow key navigation
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                navigate_arrows(0, -1);  // Up
                break;
            case SDLK_DOWN:
                navigate_arrows(0, 1);   // Down
                break;
            case SDLK_LEFT:
                navigate_arrows(-1, 0);  // Left
                break;
            case SDLK_RIGHT:
                navigate_arrows(1, 0);   // Right
                break;
        }
    }
    
    // Let the focused element handle the event first
    if (current_focus && current_focus->isEnabled() && current_focus->isVisible()) {
        current_focus->handleEvent(event);
    }
    
    // Also pass events to all elements (for hover effects, etc.)
    for (auto& element : guiElements) {
        if (element->isEnabled() && element->isVisible() && element.get() != current_focus) {
            element->handleEvent(event);
        }
    }
}

void Menu::navigate_arrows(int dirX, int dirY) {
    if (!current_focus) return;
    
    // Get current focus absolute center
    auto [currentX, currentY] = current_focus->getAbsoluteCenter();
    
    BaseElement* bestCandidate = nullptr;
    double bestDistance = std::numeric_limits<double>::max();
    
    // Check all selectable elements
    for (auto& element : guiElements) {
        if (!element->isSelectable() || !element->isEnabled() || !element->isVisible()) continue;
        if (element.get() == current_focus) continue; // Skip current focus
        
        auto [elementX, elementY] = element->getAbsoluteCenter();
        
        // Calculate direction vector from current focus to candidate
        int deltaX = elementX - currentX;
        int deltaY = elementY - currentY;
        
        // Skip if element is in the wrong direction (90-degree cone check)
        if (dirX != 0) {
            // Horizontal navigation - check if element is in the horizontal cone
            if (std::abs(deltaY) > std::abs(deltaX)) continue; // Too vertical
            if (dirX > 0 && deltaX <= 0) continue; // Right arrow but element is left
            if (dirX < 0 && deltaX >= 0) continue; // Left arrow but element is right
        }
        
        if (dirY != 0) {
            // Vertical navigation - check if element is in the vertical cone
            if (std::abs(deltaX) > std::abs(deltaY)) continue; // Too horizontal
            if (dirY > 0 && deltaY <= 0) continue; // Down arrow but element is up
            if (dirY < 0 && deltaY >= 0) continue; // Up arrow but element is down
        }
        
        // Calculate distance (squared for performance, no need for sqrt)
        double distance = deltaX * deltaX + deltaY * deltaY;
        
        // Find the closest element in the cone
        if (distance < bestDistance) {
            bestDistance = distance;
            bestCandidate = element.get();
        }
    }
    
    // Set focus to the best candidate if found
    if (bestCandidate) {
        setFocus(bestCandidate);
    }
}

void Menu::update(float deltaTime) {
    // Update all elements
    for (auto& element : guiElements) {
        if (element->isEnabled() && element->isVisible()) {
            element->update(deltaTime);
        }
    }
}

void Menu::setFocus(BaseElement* element) {
    if (current_focus == element) return;
    
    // Deselect current focus if it exists
    if (current_focus) {
        current_focus->setSelected(false);
    }
    
    // Set new focus and select it
    current_focus = element;
    if (current_focus) {
        current_focus->setSelected(true);
    }
}

void Menu::clearFocus() {
    if (current_focus) {
        current_focus->setSelected(false);
    }
    current_focus = nullptr;
}

void Menu::addElement(std::unique_ptr<BaseElement> element) {
    guiElements.push_back(std::move(element));
    
    // Auto-focus first selectable element if no focus is set
    if (!current_focus && guiElements.back()->isSelectable()) {
        setFocus(guiElements.back().get());
    }
}

void Menu::removeElement(const std::string& elementId) {
    auto it = std::find_if(guiElements.begin(), guiElements.end(),
        [&elementId](const std::unique_ptr<BaseElement>& element) {
            return element->getId() == elementId;
        });
    
    if (it != guiElements.end()) {
        // If we're removing the focused element, clear focus
        if (current_focus == it->get()) {
            clearFocus();  // This now properly deselects before clearing
        }
        guiElements.erase(it);
    }
}

BaseElement* Menu::getElement(const std::string& elementId) {
    auto it = std::find_if(guiElements.begin(), guiElements.end(),
        [&elementId](const std::unique_ptr<BaseElement>& element) {
            return element->getId() == elementId;
        });
    
    return (it != guiElements.end()) ? it->get() : nullptr;
}