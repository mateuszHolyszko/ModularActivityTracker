#include "BaseElement.hpp"
#include "../../RenderContext.hpp"
#include "../ScrollPane.hpp"
#include <cmath>
//#include <iostream>

BaseElement::BaseElement(RenderContext* context,
                       int x, int y, int width, int height, 
                       bool selectable, int layer, Menu* parent)
    : renderContext(context),
      x(x), y(y), width(width), height(height), 
      selectable(selectable), visible(true), enabled(true),
      is_selected(false), layer(layer), parent(parent),
      scrollPane(nullptr)
{
}

// Implementations that require full ScrollPane
std::pair<int,int> BaseElement::getCenter() const {
    if (scrollPane) {
        int ax = getAbsoluteX();
        int ay = getAbsoluteY();
        return { ax + width / 2, ay + height / 2 };
    }
    return { x + width / 2, y + height / 2 };
}

int BaseElement::getCenterX() const {
    if (scrollPane) return getAbsoluteX() + width / 2;
    return x + width / 2;
}

int BaseElement::getCenterY() const {
    if (scrollPane) return getAbsoluteY() + height / 2;
    return y + height / 2;
}

std::pair<int,int> BaseElement::getAbsoluteCenter() const {
    auto abs = getAbsolutePosition();
    return { abs.first + width / 2, abs.second + height / 2 };
}

int BaseElement::getAbsoluteCenterX() const {
    return getAbsolutePosition().first + width / 2;
}

int BaseElement::getAbsoluteCenterY() const {
    return getAbsolutePosition().second + height / 2;
}

std::pair<int,int> BaseElement::getAbsolutePosition() const {
    if (scrollPane) {
        // Get current scroll offset and display (viewport) position of the pane
        float scrollX = 0.0f, scrollY = 0.0f;
        scrollPane->getScrollOffset(scrollX, scrollY);

        float dispX = scrollPane->getDisplayX();
        float dispY = scrollPane->getDisplayY();

        // Element's stored x/y are in virtual-content coordinates.
        // Map virtual -> screen: display + (virtualPos - scrollOffset)
        float absXf = dispX - (static_cast<float>(x) + scrollX);
        float absYf = (static_cast<float>(y) + scrollY) - dispY; // Y axis inverted for some reason?

        // Round to nearest pixel for consistent hit tests / rendering
        int absX = static_cast<int>(std::lround(absXf));
        int absY = static_cast<int>(std::lround(absYf));
        //std::cerr << "OffsetX: " << scrollX << " OffsetY: " << scrollY << "\n";
        return { absX, absY };
    }
    return { x, y };
}

int BaseElement::getAbsoluteX() const {
    return getAbsolutePosition().first;
}

int BaseElement::getAbsoluteY() const {
    return getAbsolutePosition().second;
}

int BaseElement::getAbsoluteMinX() const { return getAbsoluteX(); }
int BaseElement::getAbsoluteMaxX() const { return getAbsoluteX() + width; }
int BaseElement::getAbsoluteMinY() const { return getAbsoluteY(); }
int BaseElement::getAbsoluteMaxY() const { return getAbsoluteY() + height; }

SDL_FRect BaseElement::getAbsoluteRect() const {
    SDL_FRect r;
    r.x = static_cast<float>(getAbsoluteX());
    r.y = static_cast<float>(getAbsoluteY());
    r.w = static_cast<float>(getWidth());
    r.h = static_cast<float>(getHeight());
    return r;
}

bool BaseElement::containsPoint(int pointX, int pointY) const {
    auto absPos = getAbsolutePosition();
    return pointX >= absPos.first && pointX < absPos.first + width &&
           pointY >= absPos.second && pointY < absPos.second + height;
}