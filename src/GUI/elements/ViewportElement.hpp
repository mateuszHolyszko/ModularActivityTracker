#pragma once
#include "BaseElement.hpp"
#include "../../RenderContext.hpp"
#include <GL/glew.h>
#include <functional>
#include <memory>
#include "../3D/ShaderProgram.hpp"

class ViewportElement : public BaseElement {
public:
    ViewportElement(RenderContext* ctx, int x, int y, int w, int h, int layer = 0, Menu* parent = nullptr, bool showBorder = true);
    ~ViewportElement();

    void render() override;
    bool handleEvent(const SDL_Event& event) override;
    void update(float dt) override;

    // Render 3D content into the internal texture
    void render3DToTexture();

    // Shader control: set a shader to be used for 3D draws inside this viewport
    void setShader(std::shared_ptr<ShaderProgram> s) { shader = s; }
    std::shared_ptr<ShaderProgram> getShader() const { return shader; }

    // Get the texture ID for external use if needed
    GLuint getTextureId() const { return colorTex; }

    // Border management
    void setShowBorder(bool show) { showBorder = show; }
    bool isShowingBorder() const { return showBorder; }

    // You can set a callback to supply custom 3D render code
    std::function<void()> onRender3D;

    // Resize the viewport
    void resize(int w, int h);

private:
    GLuint fbo = 0;
    GLuint colorTex = 0;
    GLuint rbo = 0;
    int texW = 0;
    int texH = 0;
    bool showBorder;  // Add border flag

    float rotationDeg = 0.0f;
    float rotationSpeedDegPerSec = 20.0f;

    bool createFBO(int w, int h);
    void destroyFBO();

    std::shared_ptr<ShaderProgram> shader; // << added: shader owned by viewport

    // Convert SDL_Color to glm::vec4
    glm::vec4 colorToVec4(const SDL_Color& color) const {
        return glm::vec4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    }
};