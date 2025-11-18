#include "Menu.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

Menu::Menu(RenderContext* context, WorkThread* workThread, DatabaseManager* dbMgr, const std::string& menuName)
    : renderContext(context), worker(workThread), dbManager(dbMgr), name(menuName), current_focus(nullptr) {}


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
    
    // Render elements in sorted order - they will push commands to renderContext queues
    for (auto* element : visibleElements) {
        element->render();
    }
    
    // Note: The actual rendering to texture is done outside this class
    // by calling renderer.renderToTexture(renderContext->textQueue, renderContext->graphicQueue)
}

void Menu::handleEvent(const SDL_Event& event) {
    bool eventConsumed = false;
    
    // Let the focused element handle the event first
    if (current_focus && current_focus->isEnabled() && current_focus->isVisible()) {
        eventConsumed = current_focus->handleEvent(event);
    }
    
    // If the focused element didn't consume the event, also pass to other elements
    // (for hover effects, etc.) but don't let them consume navigation keys
    if (!eventConsumed) {
        for (auto& element : guiElements) {
            if (element->isEnabled() && element->isVisible() && element.get() != current_focus) {
                // For non-focused elements, only let them handle non-navigation events
                if (event.type != SDL_KEYDOWN || 
                    (event.key.keysym.sym != SDLK_UP && 
                     event.key.keysym.sym != SDLK_DOWN &&
                     event.key.keysym.sym != SDLK_LEFT &&
                     event.key.keysym.sym != SDLK_RIGHT)) {
                    element->handleEvent(event);
                }
            }
        }
        
        // Handle arrow key navigation only if event wasn't consumed
        if (event.type == SDL_KEYDOWN && !eventConsumed) {
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
    }
}

#pragma region <Navigation>
// Helper function for ray-AABB intersection
// Returns the distance along the ray to the intersection, or -1 if no intersection
double rayIntersectsBox(double rayOriginX, double rayOriginY, double rayDirX, double rayDirY,
                        double boxMinX, double boxMaxX, double boxMinY, double boxMaxY) {
    double tMin = -std::numeric_limits<double>::infinity();
    double tMax = std::numeric_limits<double>::infinity();
    
    // Check X-axis
    if (rayDirX != 0.0) {
        double t1 = (boxMinX - rayOriginX) / rayDirX;
        double t2 = (boxMaxX - rayOriginX) / rayDirX;
        tMin = std::max(tMin, std::min(t1, t2));
        tMax = std::min(tMax, std::max(t1, t2));
    } else if (rayOriginX < boxMinX || rayOriginX > boxMaxX) {
        return -1.0;  // Ray parallel to X and outside box
    }
    
    // Check Y-axis
    if (rayDirY != 0.0) {
        double t1 = (boxMinY - rayOriginY) / rayDirY;
        double t2 = (boxMaxY - rayOriginY) / rayDirY;
        tMin = std::max(tMin, std::min(t1, t2));
        tMax = std::min(tMax, std::max(t1, t2));
    } else if (rayOriginY < boxMinY || rayOriginY > boxMaxY) {
        return -1.0;  // Ray parallel to Y and outside box
    }
    
    if (tMax >= tMin && tMax >= 0.0) {
        return tMin >= 0.0 ? tMin : tMax;  // Return the entry distance
    }
    return -1.0;  // No intersection
}

void Menu::navigate_arrows(int dirX, int dirY) {
    if (!current_focus) return;
    
    // Define the navigation direction as a unit vector
    double navDirX = dirX;  // 1 for right, -1 for left, 0 otherwise
    double navDirY = dirY;  // 1 for down, -1 for up, 0 otherwise
    
    // Get current focus center and dimensions
    auto [currentX, currentY] = current_focus->getAbsoluteCenter();
    double width = current_focus->getWidth();
    double height = current_focus->getHeight();
    
    BaseElement* bestCandidate = nullptr;
    
    // Primary: Perform 3 ray casts
    double bestRayDistance = std::numeric_limits<double>::max();
    BaseElement* rayCandidate = nullptr;
    
    // Define the 3 ray origins based on direction
    std::vector<std::pair<double, double>> rayOrigins;
    if (dirX != 0) {  // Horizontal direction: rays from center, top, and bottom of the edge
        double edgeX = (dirX > 0) ? (currentX + width / 2.0) : (currentX - width / 2.0);
        rayOrigins = {
            {currentX, currentY},  // Center
            {edgeX, currentY - height / 2.0},  // Top edge
            {edgeX, currentY + height / 2.0}   // Bottom edge
        };
    } else if (dirY != 0) {  // Vertical direction: rays from center, left, and right of the edge
        double edgeY = (dirY > 0) ? (currentY + height / 2.0) : (currentY - height / 2.0);
        rayOrigins = {
            {currentX, currentY},  // Center
            {currentX - width / 2.0, edgeY},  // Left edge
            {currentX + width / 2.0, edgeY}   // Right edge
        };
    }
    
    // Perform ray casts for each origin
    for (auto& [rayOriginX, rayOriginY] : rayOrigins) {
        for (auto& element : guiElements) {
            if (!element->isSelectable() || !element->isEnabled() || !element->isVisible()) continue;
            if (element.get() == current_focus) continue;
            
            // Get bounding box
            double minX = element->getAbsoluteMinX();
            double maxX = element->getAbsoluteMaxX();
            double minY = element->getAbsoluteMinY();
            double maxY = element->getAbsoluteMaxY();
            
            // Check ray intersection
            double distance = rayIntersectsBox(rayOriginX, rayOriginY, navDirX, navDirY, minX, maxX, minY, maxY);
            if (distance >= 0.0 && distance < bestRayDistance) {
                bestRayDistance = distance;
                rayCandidate = element.get();
            }
        }
        // If we found a candidate with this ray, no need to check other rays
        if (rayCandidate){
            setFocus(rayCandidate);
            return;
        }
    }
    
    bestCandidate = rayCandidate;
    
    // Fallback: If no candidate found via rays, use angle-constrained nearest neighbor
    if (!bestCandidate) {
        // Compute starting point based on direction (from the relevant edge)
        double startX = currentX;
        double startY = currentY;
        if (dirX > 0) {  // RIGHT: start from right edge
            startX = currentX + width / 2.0;
        } else if (dirX < 0) {  // LEFT: start from left edge
            startX = currentX - width / 2.0;
        }
        if (dirY > 0) {  // DOWN: start from bottom edge
            startY = currentY + height / 2.0;
        } else if (dirY < 0) {  // UP: start from top edge
            startY = currentY - height / 2.0;
        }
        
        double bestDistance = std::numeric_limits<double>::max();
        double bestAngleDeviation = std::numeric_limits<double>::max();  // For tie-breaking
        
        // Iterate through all elements to find candidates (angle-constrained nearest neighbor)
        for (auto& element : guiElements) {
            if (!element->isSelectable() || !element->isEnabled() || !element->isVisible()) continue;
            if (element.get() == current_focus) continue;  // Skip current focus
            
            // Get candidate center
            auto [candX, candY] = element->getAbsoluteCenter();
            
            // Vector from starting point to candidate center
            double deltaX = candX - startX;
            double deltaY = candY - startY;
            
            // Compute squared distance (for performance, avoid sqrt)
            double distanceSq = deltaX * deltaX + deltaY * deltaY;
            if (distanceSq == 0.0) continue;  // Skip if exactly at the same position
            
            // Compute angle between navigation direction and candidate vector
            double dotProduct = navDirX * deltaX + navDirY * deltaY;
            double magCand = std::sqrt(distanceSq);  // Magnitude of candidate vector
            double cosAngle = dotProduct / magCand;  // Since navDir is unit-length
            
            // Convert to degrees for threshold check
            double angleDegrees = std::acos(std::clamp(cosAngle, -1.0, 1.0)) * 180.0 / 3.1415;
            
            // Check if within the cone (angle <= threshold)
            if (angleDegrees <= navigationConeAngleDegrees) {
                // This candidate is in the acceptable direction
                // Prefer the closest one; if tied, prefer the one with smallest angle deviation
                if (distanceSq < bestDistance || 
                    (distanceSq == bestDistance && angleDegrees < bestAngleDeviation)) {
                    bestDistance = distanceSq;
                    bestAngleDeviation = angleDegrees;
                    bestCandidate = element.get();
                }
            }
        }
    }
    
    // Set focus to the best candidate if found
    if (bestCandidate) {
        setFocus(bestCandidate);
    }
}
#pragma endregion <Navigation>

void Menu::update(float deltaTime) {
    // Update all elements
    for (auto& element : guiElements) {
        if (element->isEnabled() && element->isVisible()) {
            element->update(deltaTime);   // virtual update method
            element->onUpdate(deltaTime); // lambda assigned to specific instance
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
    // Set this menu as the element's parent before adding it
    element->setParent(this);

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