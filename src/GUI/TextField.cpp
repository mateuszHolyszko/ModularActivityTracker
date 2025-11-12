#include "TextField.hpp"
#include <algorithm>  // For std::min
#include <iostream>

TextField::TextField(RenderContext* context, float x, float y, float width, float height, const std::string& text, const std::string& font, float font_size, const glm::vec4& color, HorizontalAlignment hAlign, bool wrap)
    : context_(context), x_(x), y_(y), width_(width), height_(height), text_(text), font_(font), font_size_(font_size), color_(color), hAlign_(hAlign), wrap_(wrap) {}

std::vector<TextCommand> TextField::render(int layer) {
    std::vector<TextCommand> commands;

    if (text_.empty()) return commands;

    std::string displayText = text_;
    if (wrap_) {
        displayText = truncateText(text_, width_);
    }

    // If no context, return empty or show error text
    if (!context_ || !context_->renderer) {
        // Option 1: Return empty (no rendering)
        // return commands;
        
        // Option 2: Show error text
        TextCommand cmd;
        cmd.layer = layer;
        cmd.fontSize = font_size_;
        cmd.font = font_;
        cmd.color = color_;
        cmd.x = x_;
        cmd.y = y_;
        cmd.width = width_;
        cmd.height = font_size_;
        cmd.text = "null ctx";
        commands.push_back(cmd);
        return commands;
    }

    // Calculate horizontal position based on alignment
    float textX = x_;
    float textWidth = context_->measureTextWidth(displayText, font_, font_size_);
    
    // Apply horizontal alignment
    switch (hAlign_) {
        case CENTER:
            textX = x_ + (width_ - textWidth) / 2.0f;
            break;
        case RIGHT:
            textX = x_ + width_ - textWidth;
            break;
        case LEFT:
        default:
            // textX remains as x_ (left-aligned)
            break;
    }

    TextCommand cmd;
    cmd.layer = layer;
    cmd.fontSize = font_size_;
    cmd.font = font_;
    cmd.color = color_;
    cmd.x = textX;  // Use calculated X position
    cmd.y = y_;
    cmd.width = width_;
    cmd.height = font_size_;
    cmd.text = displayText;
    commands.push_back(cmd);

    return commands;
}

// Truncate text to fit within maxWidth, appending "..." if needed
std::string TextField::truncateText(const std::string& text, float maxWidth) {
    if (text.empty()) return text;

    // If no context, return error text
    if (!context_ || !context_->renderer) {
        return "null ctx";
    }

    float textWidth = context_->measureTextWidth(text, font_, font_size_);
    
    // If text fits within the width, return it as is
    if (textWidth <= maxWidth) {
        return text;
    }

    // Text is too long, need to truncate with binary search
    int low = 0;
    int high = text.length();
    int bestFit = 0;
    
    while (low <= high) {
        int mid = (low + high) / 2;
        std::string testText = text.substr(0, mid) + "...";
        float testWidth = context_->measureTextWidth(testText, font_, font_size_);
        
        if (testWidth <= maxWidth) {
            bestFit = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    
    // Ensure we have at least 1 character before "..."
    if (bestFit <= 0) {
        return "...";
    }
    
    return text.substr(0, bestFit) + "...";
}