#include "ImageElement.hpp"
#include "../../ImageManager.hpp"
#include "../../RenderContext.hpp"

ImageElement::ImageElement(RenderContext* context,
                           ImageManager* imgManager,
                           int x, int y, int width, int height,
                           const std::string& imageKey,
                           int layer, Menu* parent)
    : BaseElement(context, x, y, width, height, false, layer, parent),
      imageManager(imgManager),
      imageKey(imageKey),
      textureId(0) {
    
    if (imageManager) {
        textureId = imageManager->getTexture(imageKey);
    }
}

void ImageElement::render() {
    if (!visible || !renderContext || textureId == 0) {
        return;
    }
    
    // Queue the image draw command
    renderContext->drawImage(textureId, 
                            static_cast<float>(x), 
                            static_cast<float>(y),
                            static_cast<float>(width), 
                            static_cast<float>(height),
                            layer);
}

bool ImageElement::handleEvent(const SDL_Event& event) {
    // Images are non-interactive by default
    return false;
}

void ImageElement::setImage(const std::string& newImageKey) {
    if (imageManager) {
        imageKey = newImageKey;
        textureId = imageManager->getTexture(imageKey);
    }
}