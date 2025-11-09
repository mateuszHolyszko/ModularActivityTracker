#ifndef STYLE_H
#define STYLE_H

#include <SDL2/SDL.h>  // Include SDL for SDL_Color

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
    
    Color bg_color;
    Color bg_color_notSelectable;
    Color lg_bg_color;
    Color text_color;
    Color border_color;
    Color highlight_color;
    Color active_bg_color;
    void* font;

    Style() :
        bg_color(30, 30, 30),
        bg_color_notSelectable(50, 50, 50),
        lg_bg_color(55, 55, 55),
        text_color(255, 255, 255),
        border_color(100, 100, 100),
        highlight_color(80, 80, 80),
        active_bg_color(30, 80, 180),
        font(nullptr)
    {}

    // Helper methods to get SDL_Colors directly
    SDL_Color getBgColor(Uint8 alpha = 255) const { return bg_color.toSDLColor(alpha); }
    SDL_Color getBgColorNotSelectable(Uint8 alpha = 255) const { return bg_color_notSelectable.toSDLColor(alpha); }
    SDL_Color getLgBgColor(Uint8 alpha = 255) const { return lg_bg_color.toSDLColor(alpha); }
    SDL_Color getTextColor(Uint8 alpha = 255) const { return text_color.toSDLColor(alpha); }
    SDL_Color getBorderColor(Uint8 alpha = 255) const { return border_color.toSDLColor(alpha); }
    SDL_Color getHighlightColor(Uint8 alpha = 255) const { return highlight_color.toSDLColor(alpha); }
    SDL_Color getActiveBgColor(Uint8 alpha = 255) const { return active_bg_color.toSDLColor(alpha); }
};

// Define the style object right in the header
static Style style;

#endif