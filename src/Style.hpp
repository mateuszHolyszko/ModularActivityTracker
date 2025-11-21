#ifndef STYLE_H
#define STYLE_H

#include <SDL2/SDL.h>  // Include SDL for SDL_Color
#include <string>
#include <unordered_map>
#include <vector>

struct Style {
    struct Color { 
        int r, g, b; 
        Color(int red, int green, int blue) : r(red), g(green), b(blue) {}
        
        // Convert to SDL_Color
        SDL_Color toSDLColor(Uint8 alpha = 255) const {
            return {
                static_cast<Uint8>(r),
                static_cast<Uint8>(g), 
                static_cast<Uint8>(b),
                alpha
            };
        }
    };

    #pragma region <Muscle colour>
    // Muscle color structure
    struct MuscleColors {
        std::unordered_map<std::string, Color> colorMap;
        std::vector<std::string> muscleList;
        
        MuscleColors() {
            // Initialize the color mapping
            colorMap = {
                {"Chest", Color(255, 102, 102)},
                {"Back", Color(255, 178, 102)},
                {"Quads", Color(153, 255, 51)},
                {"Hamstrings", Color(51, 255, 51)},
                {"Glutes", Color(51, 255, 153)},
                {"Shoulders", Color(102, 102, 255)},
                {"Biceps", Color(102, 255, 255)},
                {"Triceps", Color(102, 178, 255)},
                {"Abs", Color(178, 102, 255)},
                {"Calves", Color(255, 102, 255)},
                {"Forearms", Color(255, 102, 178)}
            };
            
        }
        
        // Get color by muscle name
        Color getColor(const std::string& muscle) const {
            auto it = colorMap.find(muscle);
            if (it != colorMap.end()) {
                return it->second;
            }
            return Color(255, 255, 255); // Default white if not found
        }
        
        // Get SDL color by muscle name
        SDL_Color getSDLColor(const std::string& muscle, Uint8 alpha = 255) const {
            return getColor(muscle).toSDLColor(alpha);
        }
        
        // Check if muscle exists
        bool hasMuscle(const std::string& muscle) const {
            return colorMap.find(muscle) != colorMap.end();
        }
        
    };
    #pragma endregion <Muscle colour>
    
    Color bg_color;
    Color bg_color_notSelectable;
    Color lg_bg_color;
    Color text_color;
    Color border_color;
    Color highlight_color;
    Color active_bg_color;
    void* font;
    MuscleColors muscleColors;  // Add muscle colors

    Style() :
        bg_color(20, 20, 20),
        bg_color_notSelectable(50, 50, 50),
        lg_bg_color(35, 35, 35),
        text_color(255, 255, 255),
        border_color(255, 255, 255),
        highlight_color(80, 80, 80),
        active_bg_color(30, 80, 180),
        font(nullptr),
        muscleColors()  // Initialize muscle colors
    {}

    // Helper methods to get SDL_Colors directly
    SDL_Color getBgColor(Uint8 alpha = 255) const { return bg_color.toSDLColor(alpha); }
    SDL_Color getBgColorNotSelectable(Uint8 alpha = 255) const { return bg_color_notSelectable.toSDLColor(alpha); }
    SDL_Color getLgBgColor(Uint8 alpha = 255) const { return lg_bg_color.toSDLColor(alpha); }
    SDL_Color getTextColor(Uint8 alpha = 255) const { return text_color.toSDLColor(alpha); }
    SDL_Color getBorderColor(Uint8 alpha = 255) const { return border_color.toSDLColor(alpha); }
    SDL_Color getHighlightColor(Uint8 alpha = 255) const { return highlight_color.toSDLColor(alpha); }
    SDL_Color getActiveBgColor(Uint8 alpha = 255) const { return active_bg_color.toSDLColor(alpha); }
    
    // Muscle color helper methods
    Color getMuscleColor(const std::string& muscle) const { return muscleColors.getColor(muscle); }
    SDL_Color getMuscleSDLColor(const std::string& muscle, Uint8 alpha = 255) const { return muscleColors.getSDLColor(muscle, alpha); }
    bool hasMuscle(const std::string& muscle) const { return muscleColors.hasMuscle(muscle); }
};

// Define the style object right in the header
static Style style;

#endif