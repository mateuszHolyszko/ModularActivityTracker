#ifndef IMAGEELEMENT_HPP
#define IMAGEELEMENT_HPP

#include "BaseElement.hpp"
#include <string>
#include <GL/glew.h>
#include "../../Style.hpp"

class ImageManager;

class ImageElement : public BaseElement {
private:
    ImageManager* imageManager;
    std::string imageKey;
    GLuint textureId;
    bool showBorder;

public:
    ImageElement(RenderContext* context,
                 ImageManager* imgManager,
                 int x, int y, int width, int height,
                 const std::string& imageKey,
                 int layer = 0, Menu* parent = nullptr);

    void render() override;
    bool handleEvent(const SDL_Event& event) override;
    
    // Get current texture ID
    GLuint getTextureId() const { return textureId; }
    
    // Change the displayed image
    void setImage(const std::string& newImageKey);

    // Border management
    void setShowBorder(bool show) { showBorder = show; }
    bool isShowingBorder() const { return showBorder; }
};

#endif