#pragma once

#include "../Style.hpp"
#include <glm/glm.hpp> 
#include <vector>
#include <string>
#include <chrono>
#include <memory>


// Forward declarations only - REMOVE the includes
struct RenderContext;
struct TextCommand;
class TextField;

struct Notification {
    std::string text;
    std::chrono::steady_clock::time_point startTime;
    float duration; // in seconds
    bool active;
    float fadeDuration; // fade out duration in seconds
    
    Notification(const std::string& msg, float delay, float fadeTime = 0.5f) 
        : text(msg), duration(delay), active(true), fadeDuration(fadeTime) {
        startTime = std::chrono::steady_clock::now();
    }
    
    float getElapsedTime() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
        return elapsed.count() / 1000.0f;
    }
    
    bool isExpired() const {
        return getElapsedTime() >= duration + fadeDuration;
    }
    
    // Returns alpha value for fade effect (1.0 = fully visible, 0.0 = fully transparent)
    float getAlpha() const {
        float elapsed = getElapsedTime();
        if (elapsed <= duration) {
            return 1.0f; // Fully visible during main duration
        } else {
            // Fade out during fade duration
            float fadeProgress = (elapsed - duration) / fadeDuration;
            return glm::clamp(1.0f - fadeProgress, 0.0f, 1.0f);
        }
    }
    
    // Check if notification is in fade-out phase
    bool isFading() const {
        float elapsed = getElapsedTime();
        return elapsed > duration && elapsed <= duration + fadeDuration;
    }
};

class NotificationSystem {
public:
    NotificationSystem(RenderContext* context, float x, float y, float width, float height, 
                      float delay = 5.0f, const std::string& font = "", float fontSize = 16.0f,
                      float fadeDuration = 0.5f); // Added fade duration parameter
    
    // Add a notification to the queue
    void addNotification(const std::string& text);
    
    // Update the notification system (remove expired notifications)
    void update();
    
    // Render all active notifications
    void render(int layer = 0);
    
    // Configuration setters
    void setPosition(float x, float y) { x_ = x; y_ = y; }
    void setSize(float width, float height) { width_ = width; height_ = height; }
    void setDelay(float delay) { defaultDelay_ = delay; }
    void setFont(const std::string& font) { font_ = font; }
    void setFontSize(float fontSize) { fontSize_ = fontSize; }
    void setMaxNotifications(int max) { maxNotifications_ = max; }
    void setFadeDuration(float fadeDuration) { defaultFadeDuration_ = fadeDuration; }
    
    // Get current active notification count
    size_t getActiveCount() const { return notifications_.size(); }
    
    // Clear all notifications
    void clear() { notifications_.clear(); }

private:
    RenderContext* context_;
    float x_, y_, width_, height_;
    float defaultDelay_;
    float defaultFadeDuration_; // Added fade duration
    std::string font_;
    float fontSize_;
    int maxNotifications_;
    
    std::vector<Notification> notifications_;
    
    // Background color for notifications (base colors without alpha)
    glm::vec4 bgBaseColor_ = glm::vec4(style.lg_bg_color.r/255.0f, style.lg_bg_color.g/255.0f, style.lg_bg_color.b/255.0f, 0.8f);
    glm::vec4 textBaseColor_ = glm::vec4(style.text_color.r/255.0f, style.text_color.g/255.0f, style.text_color.b/255.0f, 1.0f);
    
    // Calculate position for a notification at given index
    float calculateNotificationY(int index) const;
    
    // Create text field for a notification (with alpha applied)
    std::unique_ptr<TextField> createTextField(const std::string& text, float yPos, float alpha) const;
    
    // Apply alpha to color
    glm::vec4 applyAlpha(const glm::vec4& baseColor, float alpha) const;
};