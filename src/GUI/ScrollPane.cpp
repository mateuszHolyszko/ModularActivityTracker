#include "ScrollPane.hpp"
#include <algorithm>
//#include <iostream>

ScrollPane::ScrollPane(float displayX, float displayY, float displayWidth, float displayHeight,
                       float virtualWidth, float virtualHeight)
    : displayX(displayX), displayY(displayY), displayWidth(displayWidth), displayHeight(displayHeight),
      virtualWidth(virtualWidth), virtualHeight(virtualHeight) {}

ScrollPane::~ScrollPane() { shutdown(); }

bool ScrollPane::init() {
    paneRenderer = std::make_unique<SimpleRenderer>();
    if (!paneRenderer->init(&paneContext, (int)virtualWidth, (int)virtualHeight)) {
        return false;
    }
    return true;
}

void ScrollPane::setScrollOffset(float offsetX, float offsetY) {
    // Clamp to valid range
    scrollX = std::max(0.0f, std::min(offsetX, virtualWidth - displayWidth));
    scrollY = std::max(0.0f, std::min(offsetY, virtualHeight - displayHeight));
    
    // Update element visibility when scroll offset changes
    updateElementVisibility();
}

void ScrollPane::getScrollOffset(float& offsetX, float& offsetY) const {
    offsetX = scrollX;
    offsetY = scrollY;
}

void ScrollPane::scroll(float deltaX, float deltaY) {
    setScrollOffset(scrollX + deltaX, scrollY + deltaY);
}

void ScrollPane::registerElement(BaseElement* element) {
    registeredElements.push_back(element);
    // Set the scroll pane reference in the element
    element->setScrollPane(this);
    // Initial visibility check
    element->setEnabled(isElementVisible(element));
}

bool ScrollPane::isElementVisible(BaseElement* element) const {
    // Get element bounds in virtual space
    float elemX = element->getX();
    float elemY = element->getY();
    float elemWidth = element->getWidth();
    float elemHeight = element->getHeight();
    
    // Calculate visible viewport bounds in virtual space
    float viewportLeft = scrollX;
    float viewportRight = scrollX + displayWidth;
    float viewportTop = virtualHeight - (scrollY + displayHeight);
    float viewportBottom = virtualHeight - scrollY;
    
    // Calculate element bounds
    float elemLeft = elemX;
    float elemRight = elemX + elemWidth;
    float elemTop = elemY;
    float elemBottom = elemY + elemHeight;
    
    // Check if element intersects with viewport
    bool horizontalOverlap = (elemRight > viewportLeft) && (elemLeft < viewportRight);
    bool verticalOverlap = (elemBottom > viewportTop) && (elemTop < viewportBottom);
    
    return horizontalOverlap && verticalOverlap;
}

void ScrollPane::updateElementVisibility() {
    //std::cerr << "im here 1\n";
    for (auto* element : registeredElements) {
        //std::cerr << "im here 2\n";
        bool isVisible = isElementVisible(element);
        element->setSelectable(isVisible);
        element->setEnabled(isVisible);
        //std::cerr << "Element " << element->getId() << " visibility set to " << isVisible << "\n";
    }
}

void ScrollPane::update() {
    // Update element visibility before rendering
    updateElementVisibility();
    
    // Render pane's command queues to its own texture
    if (!paneContext.textQueue.empty() || !paneContext.graphicQueue.empty()) {
        paneRenderer->renderToTexture(paneContext.textQueue, paneContext.graphicQueue);
        paneContext.clearQueues();
    }
}

GLuint ScrollPane::getTexture() const {
    return paneRenderer->getTexture();
}

void ScrollPane::compositeToMain(RenderContext& mainCtx, int screenHeight) {
    // Draw this pane's texture into the main render context
    // Only the visible portion of the virtual texture is shown
    mainCtx.drawImageUV(
        getTexture(),
        displayX,
        displayY,
        displayWidth,
        displayHeight,
        scrollX / virtualWidth,           // u1
        scrollY / virtualHeight,          // v1
        (scrollX + displayWidth) / virtualWidth,   // u2
        (scrollY + displayHeight) / virtualHeight, // v2
        2  // layer
    );
}

void ScrollPane::shutdown() {
    if (paneRenderer) {
        paneRenderer->shutdown();
    }
}