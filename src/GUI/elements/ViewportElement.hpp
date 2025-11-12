#pragma once
#include "BaseElement.hpp"
#include "../../RenderContext.hpp"
#include <GL/glew.h>
#include <functional>

class ViewportElement : public BaseElement {
public:
    ViewportElement(RenderContext* ctx, int x, int y, int w, int h, int layer = 0, BaseElement* parent = nullptr);
    ~ViewportElement();

    void render() override;
    void handleEvent(const SDL_Event& event) override;

    // Per-frame update (delta time in seconds)
    void update(float dt);

    // Render 3D content into the internal texture
    void render3DToTexture();

    // Get the texture ID for external use if needed
    GLuint getTextureId() const { return colorTex; }

    // Set a callback to supply custom 3D render code
    std::function<void()> onRender3D;

    // Resize the viewport
    void resize(int w, int h);

private:
    GLuint fbo = 0;
    GLuint colorTex = 0;
    GLuint rbo = 0;
    int texW = 0;
    int texH = 0;

    float rotationDeg = 0.0f;        // degrees, updated via update(dt)
    float rotationSpeedDegPerSec = 30.0f; // default spin speed

    bool createFBO(int w, int h);
    void destroyFBO();
};