#include "LoadingWidget.hpp"
#include <cmath>

LoadingWidget::LoadingWidget() {
    posX = 400.0f;
    posY = 300.0f;
}

void LoadingWidget::update(float dt) {
    if (!visible) return;
    
    animationTime += dt;
    
    // Wrap animation time to prevent overflow
    if (animationTime > 100.0f) {
        animationTime = fmod(animationTime, 100.0f);
    }
}

void LoadingWidget::render(RenderContext& ctx) {
    if (!visible) return;

    switch (style) {
        case Style::DOTS_PULSE:
            renderDotsPulse(ctx);
            break;
        case Style::DOTS_ROTATING:
            renderDotsRotating(ctx);
            break;
        case Style::BAR:
            renderProgressBar(ctx);
            break;
        case Style::SPINNER:
            renderSpinner(ctx);
            break;
    }
}

void LoadingWidget::renderDotsPulse(RenderContext& ctx) {
    const int DOT_COUNT = 3;
    const float DOT_SPACING = size * 0.4f;
    const float BASE_DOT_SIZE = size * 0.1f;
    const float PULSE_AMPLITUDE = size * 0.08f;

    // Subtle background circle
    GraphicCommand bg;
    bg.type = GraphicCommand::CIRCLE;
    bg.x1 = posX;
    bg.y1 = posY;
    bg.x2 = size * 0.6f;
    bg.color = glm::vec4(0.1f, 0.1f, 0.1f, 0.4f);
    bg.layer = 5;
    bg.filled = true;
    bg.lineWidth = 1.0f;
    ctx.graphicQueue.push_back(bg);

    // Create three pulsing dots
    for (int i = 0; i < DOT_COUNT; ++i) {
        float timeOffset = i * 0.3f; // Stagger the animation
        float pulse = (sin(animationTime * 3.0f + timeOffset) + 1.0f) * 0.5f;
        
        float dotX = posX - DOT_SPACING + i * DOT_SPACING;
        float dotY = posY;
        float dotSize = BASE_DOT_SIZE + pulse * PULSE_AMPLITUDE;
        
        // Color intensity also pulses
        glm::vec4 dotColor = color;
        dotColor.a = 0.6f + pulse * 0.4f;

        GraphicCommand dot;
        dot.type = GraphicCommand::CIRCLE;
        dot.x1 = dotX;
        dot.y1 = dotY;
        dot.x2 = dotSize;
        dot.color = dotColor;
        dot.layer = 5;
        dot.filled = true;
        dot.lineWidth = 1.0f;
        
        ctx.graphicQueue.push_back(dot);
    }
}

void LoadingWidget::renderDotsRotating(RenderContext& ctx) {
    const int DOT_COUNT = 8;
    const float CIRCLE_RADIUS = size * 0.4f;
    const float DOT_SIZE = size * 0.08f;

    // Rotating dots in a circle
    float angle = animationTime * 3.0f; // Rotation speed
    
    for (int i = 0; i < DOT_COUNT; ++i) {
        float dotAngle = angle + (i * 2.0f * 3.14f / DOT_COUNT);
        float dotX = posX + cos(dotAngle) * CIRCLE_RADIUS;
        float dotY = posY + sin(dotAngle) * CIRCLE_RADIUS;
        
        // Alpha decreases for dots further in the rotation
        float alpha = 0.3f + (cos(dotAngle - angle) + 1.0f) * 0.35f;

        GraphicCommand dot;
        dot.type = GraphicCommand::CIRCLE;
        dot.x1 = dotX;
        dot.y1 = dotY;
        dot.x2 = DOT_SIZE;
        dot.color = glm::vec4(color.r, color.g, color.b, alpha);
        dot.layer = 5;
        dot.filled = true;
        dot.lineWidth = 1.0f;
        
        ctx.graphicQueue.push_back(dot);
    }
}

void LoadingWidget::renderProgressBar(RenderContext& ctx) {
    const float BAR_WIDTH = size * 1.5f;
    const float BAR_HEIGHT = size * 0.15f;
    const float PADDING = size * 0.1f;

    // Background bar
    GraphicCommand bgBar;
    bgBar.type = GraphicCommand::BOX;
    bgBar.x1 = posX - BAR_WIDTH/2;
    bgBar.y1 = posY - BAR_HEIGHT/2;
    bgBar.x2 = posX + BAR_WIDTH/2;
    bgBar.y2 = posY + BAR_HEIGHT/2;
    bgBar.color = glm::vec4(0.2f, 0.2f, 0.2f, 0.8f);
    bgBar.layer = 5;
    bgBar.filled = true;
    bgBar.lineWidth = 1.0f;
    ctx.graphicQueue.push_back(bgBar);

    // Animated progress fill (pulsing width)
    float animatedProgress = progress;
    if (progress <= 0.0f) {
        // If no progress set, create bouncing animation
        float bounce = (sin(animationTime * 4.0f) + 1.0f) * 0.5f;
        animatedProgress = 0.1f + bounce * 0.4f;
    }
    
    float fillWidth = BAR_WIDTH * animatedProgress;
    
    GraphicCommand fillBar;
    fillBar.type = GraphicCommand::BOX;
    fillBar.x1 = posX - BAR_WIDTH/2;
    fillBar.y1 = posY - BAR_HEIGHT/2;
    fillBar.x2 = posX - BAR_WIDTH/2 + fillWidth;
    fillBar.y2 = posY + BAR_HEIGHT/2;
    fillBar.color = color;
    fillBar.layer = 5;
    fillBar.filled = true;
    fillBar.lineWidth = 1.0f;
    ctx.graphicQueue.push_back(fillBar);

    // Progress text
    TextCommand text;
    text.text = progress > 0.0f ? 
                std::to_string(int(progress * 100)) + "%" : "Loading...";
    text.font = "default";
    text.fontSize = size * 0.2f;
    text.x = posX;
    text.y = posY + BAR_HEIGHT + PADDING;
    text.color = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
    text.layer = 6;
    
    float textWidth = ctx.measureTextWidth(text.text, text.font, text.fontSize);
    text.x -= textWidth / 2.0f;
    
    ctx.textQueue.push_back(text);
}

void LoadingWidget::renderSpinner(RenderContext& ctx) {
    const float SPINNER_RADIUS = size * 0.4f;
    const float SPINNER_WIDTH = size * 0.1f;

    // Draw spinner arc
    float startAngle = animationTime * 3.0f;
    float endAngle = startAngle + 3.14f * 1.5f; // 270 degree arc
    
    // For simplicity, we'll draw multiple line segments to approximate an arc
    const int SEGMENTS = 16;
    for (int i = 0; i < SEGMENTS; ++i) {
        float segmentStart = startAngle + (endAngle - startAngle) * (i / float(SEGMENTS));
        float segmentEnd = startAngle + (endAngle - startAngle) * ((i + 1) / float(SEGMENTS));
        
        float x1 = posX + cos(segmentStart) * SPINNER_RADIUS;
        float y1 = posY + sin(segmentStart) * SPINNER_RADIUS;
        float x2 = posX + cos(segmentEnd) * SPINNER_RADIUS;
        float y2 = posY + sin(segmentEnd) * SPINNER_RADIUS;
        
        GraphicCommand segment;
        segment.type = GraphicCommand::LINE;
        segment.x1 = x1;
        segment.y1 = y1;
        segment.x2 = x2;
        segment.y2 = y2;
        segment.color = color;
        segment.layer = i;
        segment.lineWidth = SPINNER_WIDTH;
        segment.filled = false;
        
        ctx.graphicQueue.push_back(segment);
    }
}