#include "ScrollBarWidget.hpp"
#include <algorithm>

ScrollBarWidget::ScrollBarWidget(ScrollPane* parent, RenderContext& ctx) 
    : parentPane(parent), renderContext(&ctx) {
    // Calculate scrollbar dimensions
    height = static_cast<int>(parent->getDisplayHeight()) - 2 * paddingY;
    x = static_cast<int>(parent->getDisplayX()) + static_cast<int>(parent->getDisplayWidth()) - width - paddingX;
    y = static_cast<int>(parent->getDisplayY()) + paddingY;
    
    // Calculate thumb height based on display:virtual ratio
    // This is correct: thumb represents the visible portion relative to total content
    float ratio = parent->getDisplayHeight() / parent->getVirtualHeight();
    thumbHeight = std::max(20, static_cast<int>(height * ratio)); // Minimum 20px thumb
}

void ScrollBarWidget::update(float dt) {
    // Update thumb position based on parent scroll pane's offset
    if (parentPane) {
        float scrollX, scrollY;
        parentPane->getScrollOffset(scrollX, scrollY);
        float virtualHeight = parentPane->getVirtualHeight();
        float displayHeight = parentPane->getDisplayHeight();

        // Calculate thumb position
        if (virtualHeight > displayHeight) {
            float scrollableHeight = virtualHeight - displayHeight;
            float trackHeight = height - thumbHeight;
            
            // Note about coordinate system:
            // - scrollY = 0 means showing bottom of content (thumb at bottom)
            // - scrollY = scrollableHeight means showing top of content (thumb at top)
            // The current calculation inverts this correctly
            
            // This formula maps scroll position to thumb position
            // When scrollY = scrollableHeight (showing top), thumbPosition = 0 (top)
            // When scrollY = 0 (showing bottom), thumbPosition = trackHeight (bottom)
            thumbPosition = static_cast<int>((scrollY / scrollableHeight) * trackHeight);
            
            // IMPORTANT: If the coordinate system is inverted (y=0 at top, increasing downward),
            // we need to invert the thumb position so it moves in the opposite direction
            // of the content scrolling
            thumbPosition = static_cast<int>(trackHeight - thumbPosition);
            
            // Clamp thumb position
            thumbPosition = std::max(0, std::min(thumbPosition, static_cast<int>(trackHeight)));
        } else {
            thumbPosition = 0; // No scrolling needed
        }
        render();
    }
}

void ScrollBarWidget::render() {
    if (!renderContext) return;

    // Use Style colors instead of hardcoded values
    // Render scrollbar background (track)
    GraphicCommand bgCmd;
    bgCmd.type = GraphicCommand::BOX;
    bgCmd.x1 = static_cast<float>(x);
    bgCmd.y1 = static_cast<float>(y);
    bgCmd.x2 = static_cast<float>(x + width);
    bgCmd.y2 = static_cast<float>(y + height);
    SDL_Color trackColor = style.getLgBgColor();
    bgCmd.color = glm::vec4(trackColor.r, trackColor.g, trackColor.b, 0.6f); // Darker gray for background/track
    bgCmd.filled = true;
    bgCmd.layer = 3; // UI layer
    renderContext->graphicQueue.push_back(bgCmd);

    // Render scrollbar thumb
    GraphicCommand thumbCmd;
    thumbCmd.type = GraphicCommand::BOX;
    thumbCmd.x1 = static_cast<float>(x);
    thumbCmd.y1 = static_cast<float>(y + thumbPosition);
    thumbCmd.x2 = static_cast<float>(x + width);
    thumbCmd.y2 = static_cast<float>(y + thumbPosition + thumbHeight);
    
    SDL_Color thumbColor = style.getTextColor();
    thumbCmd.color = glm::vec4(thumbColor.r, thumbColor.g, thumbColor.b, 0.6f); // Lighter gray for thumb
    thumbCmd.filled = true;
    thumbCmd.layer = 4; // Above background
    renderContext->graphicQueue.push_back(thumbCmd);
}