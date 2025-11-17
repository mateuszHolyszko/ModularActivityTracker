#ifndef LABEL_HPP
#define LABEL_HPP

#include <string>
#include "BaseElement.hpp"
#include "../../Style.hpp"
#include "../../RenderContext.hpp"
#include "../TextField.hpp"
#include <functional>

class Label : public BaseElement {
public:
    Label(RenderContext* context,
          int x, int y, int width, int height,
          const std::string& text,
          bool showBorder = false,
          int fontSize = 12,
          int layer = 0,
          Menu* parent = nullptr,
          HorizontalAlignment textAlign = HorizontalAlignment::LEFT);

    ~Label();

    void render() override;
    bool handleEvent(const SDL_Event& event) override;

    void setText(const std::string& newText);
    void setFontSize(int size);
    void setWrapText(bool wrap);
    void setBackgroundColor(const SDL_Color& color);

private:
    std::string text;
    bool showBorder;
    int fontSize;
    bool hovered;
    bool wrapText;
    TextField* textField;
    SDL_Color backgroundColor;

    void updateTextField();

    // Convert SDL_Color to glm::vec4
    glm::vec4 colorToVec4(const SDL_Color& color) const {
        return glm::vec4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    }
};

#endif // LABEL_HPP