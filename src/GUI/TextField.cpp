#include "TextField.hpp"
#include <algorithm>  // For std::min
#include <iostream>
#include <sstream>    // For splitting strings

TextField::TextField(RenderContext* context, float x, float y, float width, float height, const std::string& text, const std::string& font, float font_size, const glm::vec4& color, HorizontalAlignment hAlign, bool wrap)
    : context_(context), x_(x), y_(y), width_(width), height_(height), text_(text), font_(font), font_size_(font_size), color_(color), hAlign_(hAlign), wrap_(wrap) {}

std::vector<TextCommand> TextField::render(int layer) {
    std::vector<TextCommand> commands;

    if (text_.empty()) return commands;

    // If no context, return error text
    if (!context_ || !context_->renderer) {
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

    // Split text by newlines
    std::vector<std::string> lines = splitByNewlines(text_);
    
    // Calculate line height using actual measured text height
    float measuredHeight = context_->measureTextHeight("Ag", font_, font_size_);
    float lineSpacing = measuredHeight * 0.2f;  // 20% spacing
    float lineHeight = measuredHeight + lineSpacing;
    
    // Count how many lines will fit
    int linesToRender = 0;
    float accumulatedHeight = 0.0f;
    for (const auto& line : lines) {
        if (accumulatedHeight + measuredHeight > height_) {
            break;
        }
        linesToRender++;
        accumulatedHeight += (linesToRender == 1) ? measuredHeight : lineHeight;
    }
    
    // Calculate total height of the text block: first line + (n-1) lines with spacing
    float totalTextHeight = measuredHeight;
    if (linesToRender > 1) {
        totalTextHeight += (linesToRender - 1) * lineHeight;
    }
    
    // Calculate centers
    float textFieldCenterY = y_ + height_ / 2.0f;
    float textBlockCenterY = totalTextHeight / 2.0f;
    
    // Start Y is where the first line's baseline should be
    // = textFieldCenter - textBlockCenter + half of first line height (to get to baseline)
    float startY = textFieldCenterY - textBlockCenterY + measuredHeight / 2.0f;
    float currentY = startY;

    // Render each line
    for (int i = 0; i < linesToRender; i++) {
        const auto& line = lines[i];
        
        std::string displayText = line;
        if (wrap_) {
            displayText = truncateText(line, width_);
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
        cmd.x = textX;
        cmd.y = currentY;
        cmd.width = width_;
        cmd.height = measuredHeight;
        cmd.text = displayText;
        commands.push_back(cmd);

        // Move to next line
        currentY += lineHeight;
    }

    return commands;
}

// Split text by newline characters
std::vector<std::string> TextField::splitByNewlines(const std::string& text) {
    std::vector<std::string> lines;
    std::stringstream ss(text);
    std::string line;
    
    while (std::getline(ss, line, '\n')) {
        lines.push_back(line);
    }
    
    return lines;
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