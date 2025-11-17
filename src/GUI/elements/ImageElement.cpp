#include <iostream>
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

    // Draw border if enabled
    if (showBorder) {
        SDL_Color borderColor = style.getBorderColor();
        
        GraphicCommand borderCmd;
        borderCmd.type = GraphicCommand::BOX;
        borderCmd.x1 = static_cast<float>(x);
        borderCmd.y1 = static_cast<float>(y);
        borderCmd.x2 = static_cast<float>(x + width);
        borderCmd.y2 = static_cast<float>(y + height);
        borderCmd.color = glm::vec4(borderColor.r / 255.0f, borderColor.g / 255.0f, borderColor.b / 255.0f, borderColor.a / 255.0f);
        borderCmd.layer = layer;
        borderCmd.lineWidth = 1.0f;
        borderCmd.filled = false;
        
        renderContext->graphicQueue.push_back(borderCmd);
    }
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