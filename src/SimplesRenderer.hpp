#pragma once

#include <SDL2/SDL.h>
#include <stdexcept>
#include <string>
#include <cmath>

// SimplesRenderer: renders basic shapes with GPU acceleration where possible
class SimplesRenderer {
public:
    explicit SimplesRenderer(SDL_Renderer* renderer)
        : m_renderer(renderer)
    {
        if (!m_renderer) {
            throw std::invalid_argument("renderer cannot be null");
        }
    }

    // Non-copyable
    SimplesRenderer(const SimplesRenderer&) = delete;
    SimplesRenderer& operator=(const SimplesRenderer&) = delete;

    ~SimplesRenderer() {
        if (m_fillTexture) {
            SDL_DestroyTexture(m_fillTexture);
        }
    }

    // Draw a line with specified thickness (GPU-accelerated using filled rect)
    void DrawThickLine(int x1, int y1, int x2, int y2, int thickness, SDL_Color color) {
        if (thickness <= 0) return;
        
        // Calculate line properties
        int dx = x2 - x1;
        int dy = y2 - y1;
        float length = std::sqrt(dx * dx + dy * dy);
        
        if (length == 0) return; // Zero-length line
        
        // Calculate angle and create a rotated filled rectangle
        float angle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
        
        SDL_Rect rect = {
            x1 - thickness/2, 
            y1 - thickness/2,
            static_cast<int>(length) + thickness,
            thickness
        };
        
        SetDrawColor(color);
        
        // Save current render target and blend mode
        SDL_Texture* oldTarget = SDL_GetRenderTarget(m_renderer);
        SDL_BlendMode oldBlendMode;
        SDL_GetRenderDrawBlendMode(m_renderer, &oldBlendMode);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        
        // Create temporary texture for the line
        SDL_Texture* lineTexture = SDL_CreateTexture(m_renderer, 
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, rect.w, rect.h);
        
        if (lineTexture) {
            // Set texture as render target
            SDL_SetRenderTarget(m_renderer, lineTexture);
            SDL_SetTextureBlendMode(lineTexture, SDL_BLENDMODE_BLEND);
            
            // Clear to transparent
            SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
            SDL_RenderClear(m_renderer);
            
            // Draw the line as a filled rectangle on the texture
            SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(m_renderer, NULL);
            
            // Restore render target
            SDL_SetRenderTarget(m_renderer, oldTarget);
            
            // Render the rotated texture
            SDL_Rect destRect = {x1, y1 - thickness/2, rect.w, rect.h};
            SDL_Point center = {0, thickness/2};
            SDL_RenderCopyEx(m_renderer, lineTexture, NULL, &destRect, angle, &center, SDL_FLIP_NONE);
            
            SDL_DestroyTexture(lineTexture);
        } else {
            // Fallback to CPU method if texture creation fails
            DrawThickLineCPU(x1, y1, x2, y2, thickness, color);
        }
        
        // Restore blend mode
        SDL_SetRenderDrawBlendMode(m_renderer, oldBlendMode);
    }

    // Draw a rectangle with specified border thickness (mixed GPU/CPU)
    void DrawThickRect(int x, int y, int width, int height, int thickness, SDL_Color color) {
        if (thickness <= 0) return;

        SetDrawColor(color);
        
        // Use GPU-accelerated filled rectangles for each side
        if (thickness > 0) {
            // Top
            SDL_Rect top = {x, y, width, thickness};
            SDL_RenderFillRect(m_renderer, &top);
            // Bottom
            SDL_Rect bottom = {x, y + height - thickness, width, thickness};
            SDL_RenderFillRect(m_renderer, &bottom);
            // Left
            SDL_Rect left = {x, y + thickness, thickness, height - 2 * thickness};
            SDL_RenderFillRect(m_renderer, &left);
            // Right
            SDL_Rect right = {x + width - thickness, y + thickness, thickness, height - 2 * thickness};
            SDL_RenderFillRect(m_renderer, &right);
        }
    }

    // Draw a filled rectangle with border (GPU-accelerated)
    void DrawFilledThickRect(int x, int y, int width, int height, int thickness, 
                            SDL_Color borderColor, SDL_Color fillColor) {
        // Draw filled interior (GPU)
        SDL_Rect fillRect = {x + thickness, y + thickness, 
                             width - 2 * thickness, height - 2 * thickness};
        SetDrawColor(fillColor);
        SDL_RenderFillRect(m_renderer, &fillRect);

        // Draw border (GPU)
        DrawThickRect(x, y, width, height, thickness, borderColor);
    }

    // Draw a simple filled rectangle (GPU-accelerated)
    void DrawFilledRect(int x, int y, int width, int height, SDL_Color color) {
        SDL_Rect rect = {x, y, width, height};
        SetDrawColor(color);
        SDL_RenderFillRect(m_renderer, &rect);
    }

    // Draw a simple rectangle outline (GPU-accelerated)
    void DrawRect(int x, int y, int width, int height, SDL_Color color) {
        DrawThickRect(x, y, width, height, 1, color);
    }

    // Pre-render common shapes to textures for maximum GPU performance
    void PrecacheCircleTexture(int radius, SDL_Color color) {
        // Implementation for cached circle textures
    }

private:
    SDL_Renderer* m_renderer;
    SDL_Texture* m_fillTexture = nullptr;

    void SetDrawColor(SDL_Color color) {
        SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    }

    // CPU fallback for thick lines
    void DrawThickLineCPU(int x1, int y1, int x2, int y2, int thickness, SDL_Color color) {
        if (thickness == 1) {
            SetDrawColor(color);
            SDL_RenderDrawLine(m_renderer, x1, y1, x2, y2);
            return;
        }

        int dx = x2 - x1;
        int dy = y2 - y1;
        float length = std::sqrt(dx * dx + dy * dy);
        if (length == 0) return;
        
        float perpX = -dy / length;
        float perpY = dx / length;
        float offset = (thickness - 1) / 2.0f;
        
        SetDrawColor(color);
        for (int i = 0; i < thickness; i++) {
            float currentOffset = i - offset;
            int offsetX1 = x1 + static_cast<int>(perpX * currentOffset);
            int offsetY1 = y1 + static_cast<int>(perpY * currentOffset);
            int offsetX2 = x2 + static_cast<int>(perpX * currentOffset);
            int offsetY2 = y2 + static_cast<int>(perpY * currentOffset);
            
            SDL_RenderDrawLine(m_renderer, offsetX1, offsetY1, offsetX2, offsetY2);
        }
    }
};