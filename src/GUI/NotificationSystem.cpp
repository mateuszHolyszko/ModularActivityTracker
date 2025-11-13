#include "NotificationSystem.hpp"
#include "../RenderContext.hpp"  
#include "TextField.hpp"         
#include <algorithm>
#include <iostream>

NotificationSystem::NotificationSystem(RenderContext* context, float x, float y, float width, float height, 
                                     float delay, const std::string& font, float fontSize, float fadeDuration)
    : context_(context), x_(x), y_(y), width_(width), height_(height), 
      defaultDelay_(delay), defaultFadeDuration_(fadeDuration), font_(font), fontSize_(fontSize), maxNotifications_(5) {
}

void NotificationSystem::addNotification(const std::string& text) {
    // Remove oldest notification if we've reached the maximum
    if (notifications_.size() >= maxNotifications_) {
        notifications_.erase(notifications_.begin());
    }
    
    notifications_.emplace_back(text, defaultDelay_, defaultFadeDuration_);
    
    std::cout << "[NotificationSystem] Added notification: " << text << std::endl;
}

void NotificationSystem::update() {
    // Remove expired notifications
    notifications_.erase(
        std::remove_if(notifications_.begin(), notifications_.end(),
            [](const Notification& notif) {
                return notif.isExpired();
            }),
        notifications_.end()
    );
}

void NotificationSystem::render(int layer) {
    if (!context_) return;
    
    update(); // Remove expired notifications first
    
    if (notifications_.empty()) return;
    
    // Render notifications from bottom to top (newest at bottom)
    for (int i = 0; i < notifications_.size(); ++i) {
        int reverseIndex = notifications_.size() - 1 - i; // Reverse order for rendering
        const auto& notif = notifications_[reverseIndex];
        
        if (!notif.active) continue;
        
        float yPos = calculateNotificationY(i);
        float alpha = notif.getAlpha();
        
        // Skip rendering if completely transparent
        if (alpha <= 0.0f) continue;
        
        // Add background box with fade effect
        GraphicCommand bgCmd;
        bgCmd.type = GraphicCommand::BOX;
        bgCmd.x1 = x_;
        bgCmd.y1 = yPos - fontSize_; // Some padding above text
        bgCmd.x2 = x_ + width_;
        bgCmd.y2 = yPos + fontSize_ * 1.5f; // Some padding below text
        bgCmd.color = applyAlpha(bgBaseColor_, alpha); // Apply alpha to background
        bgCmd.layer = layer;
        bgCmd.filled = true;
        context_->graphicQueue.push_back(bgCmd);
        
        // Create and render text field with fade effect
        auto textField = createTextField(notif.text, yPos, alpha);
        auto textCommands = textField->render(layer + 1); // Render text above background
        
        // Add text commands to context
        context_->textQueue.insert(context_->textQueue.end(), textCommands.begin(), textCommands.end());
    }
}

float NotificationSystem::calculateNotificationY(int index) const {
    // Calculate Y position based on index, starting from bottom
    float spacing = fontSize_ * 1.8f; // Space between notifications
    return y_ + height_ - (index * spacing) - fontSize_;
}

std::unique_ptr<TextField> NotificationSystem::createTextField(const std::string& text, float yPos, float alpha) const {
    glm::vec4 textColorWithAlpha = applyAlpha(textBaseColor_, alpha);
    
    return std::make_unique<TextField>(
        context_,
        x_,                    // x position
        yPos,                  // y position
        width_,               // width
        fontSize_,            // height
        text,                 // text content
        font_,                // font
        fontSize_,            // font size
        textColorWithAlpha,   // text color with alpha applied
        CENTER,               // alignment
        true                  // wrap text
    );
}

glm::vec4 NotificationSystem::applyAlpha(const glm::vec4& baseColor, float alpha) const {
    return glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a * alpha);
}