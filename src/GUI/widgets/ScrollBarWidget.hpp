#pragma once
#include "../../RenderContext.hpp"
#include "../ScrollPane.hpp"
#include "../../Style.hpp"

class ScrollBarWidget {
public:
    // enum class Orientation {  // for now just vertical
    //     HORIZONTAL,
    //     VERTICAL
    // }; 

    ScrollBarWidget(ScrollPane* parent, RenderContext& ctx);

    void update(float dt);
    void render();

    void setParentScrollPanel(ScrollPane* pane) { parentPane = pane; }
    //void setOrientation(Orientation orient) { orientation = orient; }
    void setOffset(int x, int y) { paddingX = x; paddingY = y; }
    void setThumbPosition(int position) { thumbPosition = position; }
    void setThumbHeight(int height) { thumbHeight = height; }
private:
    ScrollPane* parentPane = nullptr;
    RenderContext* renderContext = nullptr;
    //Orientation orientation = Orientation::VERTICAL;
    int paddingX = 5;
    int paddingY = 5;
    int height = 0;
    int width = 5;
    int x = 0;
    int y = 0;

    int thumbPosition = 0;
    int thumbHeight = 0;  // Based on the size of the displayed area vs virtual size
};