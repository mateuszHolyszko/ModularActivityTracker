#include "ClockPanel.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "ClockPanelLayout.hpp"

std::vector<std::unique_ptr<BaseElement>> ClockPanel::create(
    RenderContext* context, int x, int y, Menu* parent) {
    
    std::vector<std::unique_ptr<BaseElement>> elements;
    
    const Box& boxClock = layout.at("Clock");
    const Box& boxDate = layout.at("Date");
    const Box& boxUserPrompt = layout.at("UserPrompt");
    
    // Helper function to get current time/date
    auto getCurrentTime = []() -> std::string {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        std::stringstream ss;
        ss << std::put_time(&tm, "%H:%M:%S");
        return ss.str();
    };
    
    auto getCurrentDate = []() -> std::string {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        std::stringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d");
        return ss.str();
    };
    
    // Time display label with custom update
    auto timeLabel = std::make_unique<Label>(
        context, x + boxClock.x, y + boxClock.y, boxClock.width, boxClock.height, 
        getCurrentTime(), true, 24, 2, parent, CENTER
    );
    timeLabel->setId("clock_time");
    timeLabel->setWrapText(false);
    
    // Override the update method for this specific instance
    timeLabel->onUpdate = [timeLabelPtr = timeLabel.get(), getCurrentTime](float dt) {
        static float updateTimer = 0.0f;
        updateTimer += dt;
        
        if (updateTimer >= 1.0f) {
            timeLabelPtr->setText(getCurrentTime());
            updateTimer = 0.0f;
        }
    };
    
    elements.push_back(std::move(timeLabel));
    
    // Date label with custom update
    auto dateLabel = std::make_unique<Label>(
        context, x + boxDate.x, y + boxDate.y, boxDate.width, boxDate.height, 
        getCurrentDate(), true, 24, 2, parent, CENTER
    );
    dateLabel->setId("clock_date");
    dateLabel->setWrapText(false);
    
    // Override the update method for this specific instance
    dateLabel->onUpdate = [dateLabelPtr = dateLabel.get(), getCurrentDate](float dt) {
        static float updateTimer = 0.0f;
        updateTimer += dt;
        
        if (updateTimer >= 1.0f) {
            dateLabelPtr->setText(getCurrentDate());
            updateTimer = 0.0f;
        }
    };
    
    elements.push_back(std::move(dateLabel));

    // User display label with custom update
    auto userLabel = std::make_unique<Label>(
        context, x + boxUserPrompt.x, y + boxUserPrompt.y, boxUserPrompt.width, boxUserPrompt.height, 
        "Mateuszek", true, 24, 2, parent, CENTER
    );
    userLabel->setId("user_prompt");
    userLabel->setWrapText(false);
    
    // Override the update method for this specific instance
    // TO DO: implement user name fetch
    
    elements.push_back(std::move(userLabel));


    return elements;
}