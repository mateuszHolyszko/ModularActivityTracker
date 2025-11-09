#include "BaseElement.hpp"
#include "../RenderContext.hpp"

BaseElement::BaseElement(RenderContext* context,
                       int x, int y, int width, int height, 
                       bool selectable, int layer, BaseElement* parent)
    : renderContext(context),
      x(x), y(y), width(width), height(height), 
      selectable(selectable), visible(true), enabled(true),
      is_selected(false), layer(layer), parent(parent) {
}