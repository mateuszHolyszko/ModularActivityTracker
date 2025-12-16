#pragma once
#include "../SimpleRenderer.hpp"
#include "../RenderContext.hpp"
#include "elements/BaseElement.hpp"
#include <memory>
#include <vector>

class ScrollPane {
public:
    // virtualWidth/Height: size of scrollable content
    // displayX/Y/Width/Height: where it appears on screen
    ScrollPane(float displayX, float displayY, float displayWidth, float displayHeight,
               float virtualWidth, float virtualHeight);
    ~ScrollPane();
    
    bool init();
    
    // Get the pane's own render context for UI elements to draw to
    RenderContext* getRenderContext() { return &paneContext; }
    
    // Set scroll offset (can go negative or beyond content size)
    void setScrollOffset(float offsetX, float offsetY);
    void getScrollOffset(float& offsetX, float& offsetY) const;
    
    // Modify scroll (for mouse wheel, etc.)
    void scroll(float deltaX, float deltaY);
    
    // Register an element that belongs to this scroll pane
    void registerElement(BaseElement* element);
    
    // Update all registered elements' visibility
    void updateElementVisibility();
    
    // Set the virtual size of the scrollable content
    void setVirtualSize(float newVirtualWidth, float newVirtualHeight);
    
    // Render pane content and get its texture
    void update();  // Calls renderer->renderToTexture()
    GLuint getTexture() const;
    
    // Composite this pane into main render context with clipping
    void compositeToMain(RenderContext& mainCtx, int screenHeight);
    
    // Getters for bounds
    float getDisplayX() const { return displayX; }
    float getDisplayY() const { return displayY; }
    float getDisplayWidth() const { return displayWidth; }
    float getDisplayHeight() const { return displayHeight; }
    float getVirtualWidth() const { return virtualWidth; }
    float getVirtualHeight() const { return virtualHeight; }
    
    void shutdown();
    
private:
    // Check if an element is visible in the current viewport
    bool isElementVisible(BaseElement* element) const;
    
    std::unique_ptr<SimpleRenderer> paneRenderer;
    RenderContext paneContext;  // Pane's own command queues
    
    std::vector<BaseElement*> registeredElements;  // Elements belonging to this pane
    
    float displayX, displayY, displayWidth, displayHeight;  // Screen-space bounds
    float virtualWidth, virtualHeight;  // Content size
    float scrollX = 0.0f, scrollY = 0.0f;  // Content offset
};