#ifndef BASEELEMENT_HPP
#define BASEELEMENT_HPP

#include <SDL2/SDL.h>
#include <functional>
#include <string>
#include <memory>

// Forward declarations
class TextRenderer;
class SimplesRenderer;
class RenderContext;
class Menu;

class BaseElement {
protected:
    RenderContext* renderContext;
    int x, y;
    int width, height;
    bool selectable;
    bool visible;
    bool enabled;
    bool is_selected;
    int layer;
    Menu* parent;
    std::string id;

public:
    BaseElement(RenderContext* context,
                int x, int y, int width, int height, 
                bool selectable = true, int layer = 0, Menu* parent = nullptr);
    virtual ~BaseElement() = default;

    // Getters and setters
    virtual int getX() const { return x; }
    virtual int getY() const { return y; }
    virtual int getWidth() const { return width; }
    virtual int getHeight() const { return height; }
    virtual bool isSelectable() const { return selectable; }
    virtual bool isVisible() const { return visible; }
    virtual bool isEnabled() const { return enabled; }
    virtual bool isSelected() const { return is_selected; }
    virtual int getLayer() const { return layer; }
    virtual Menu* getParent() const { return parent; }
    virtual const std::string& getId() const { return id; }
    
    virtual void setPosition(int newX, int newY) { x = newX; y = newY; }
    virtual void setSize(int newWidth, int newHeight) { width = newWidth; height = newHeight; }
    virtual void setVisible(bool isVisible) { visible = isVisible; }
    virtual void setEnabled(bool isEnabled) { enabled = isEnabled; }
    virtual void setSelected(bool selected) { is_selected = selected; }
    virtual void setLayer(int newLayer) { layer = newLayer; }
    virtual void setParent(Menu* newParent) { parent = newParent; }
    virtual void setId(const std::string& newId) { id = newId; }

    // Center coordinates method (relative to parent)
    virtual std::pair<int, int> getCenter() const {
        return {x + width / 2, y + height / 2};
    }
    
    virtual int getCenterX() const { return x + width / 2; }
    virtual int getCenterY() const { return y + height / 2; }

    // Absolute center coordinates (on screen)
    virtual std::pair<int, int> getAbsoluteCenter() const {
        auto absPos = getAbsolutePosition();
        return {absPos.first + width / 2, absPos.second + height / 2};
    }
    
    virtual int getAbsoluteCenterX() const {
        return getAbsolutePosition().first + width / 2;
    }
    
    virtual int getAbsoluteCenterY() const {
        return getAbsolutePosition().second + height / 2;
    }

    // Absolute position methods
    virtual std::pair<int, int> getAbsolutePosition() const {
        return {x, y}; // For now, no parent offset since im workin on absolute coords
    }
    
    virtual int getAbsoluteX() const {
        return getAbsolutePosition().first;
    }
    
    virtual int getAbsoluteY() const {
        return getAbsolutePosition().second;
    }

    // Absolute bounding box methods
    virtual int getAbsoluteMinX() const {
        return getAbsoluteX();
    }
    
    virtual int getAbsoluteMaxX() const {
        return getAbsoluteX() + width;
    }
    
    virtual int getAbsoluteMinY() const {
        return getAbsoluteY();
    }
    
    virtual int getAbsoluteMaxY() const {
        return getAbsoluteY() + height;
    }

    // Core virtual methods
    virtual void render() = 0;
    virtual bool handleEvent(const SDL_Event& event) = 0;

    // Optional virtual methods
    virtual void update(float deltaTime) {}
    virtual bool containsPoint(int pointX, int pointY) const {
        auto absPos = getAbsolutePosition();
        return pointX >= absPos.first && pointX <= absPos.first + width &&
               pointY >= absPos.second && pointY <= absPos.second + height;
    }

    // Event callbacks
    std::function<void()> onPress;
    std::function<void()> onRelease;
    std::function<void()> onHover;
    std::function<void()> onFocus;
    std::function<void()> onBlur;
    std::function<void()> onSelect;
    std::function<void()> onDeselect;
};

#endif
