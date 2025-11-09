#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <stdexcept>
#include <map>

// TextRenderer: renders text onto an SDL_Surface. You call RenderTextToSurface()
// with the target surface, x/y, optional size (default 24) and color (default white).
// The renderer does not own or create an SDL_Renderer; it only creates TTF_Font
// instances (cached per size) and uses SDL_Surface operations so you can convert
// the result into a texture yourself (or blit to a window surface).

class TextRenderer {
public:
    // fontPath should point to the TTF file to use (eg. "fonts/RasterForgeRegular-JpBgm.ttf").
    explicit TextRenderer(const char* fontPath = "C:/dev/ModularActivityTracker/fonts/RasterForgeRegular-JpBgm.ttf")
        : m_fontPath(fontPath)
    {
        if (TTF_WasInit() == 0) {
            if (TTF_Init() < 0) {
                throw std::runtime_error(std::string("TTF_Init failed: ") + TTF_GetError());
            }
            m_initializedTTF = true;
        }
    }

    // Non-copyable
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    ~TextRenderer() {
        for (auto &p : m_fontCache) {
            if (p.second) TTF_CloseFont(p.second);
        }
        m_fontCache.clear();

        if (m_initializedTTF) {
            TTF_Quit();
        }
    }

    // Render `text` onto the provided `target` surface at (x,y). The text will
    // be rendered using `size` (points). Color default is white. Throws on error.
    void RenderTextToSurface(SDL_Surface* target,
                             const std::string& text,
                             int x,
                             int y,
                             int size = 24,
                             SDL_Color color = {255, 255, 255, 255})
    {
        if (!target) {
            throw std::invalid_argument("target surface is null");
        }

        TTF_Font* font = getFont(size);
        if (!font) {
            throw std::runtime_error("Failed to obtain font for size " + std::to_string(size));
        }

        SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
        if (!textSurface) {
            throw std::runtime_error(std::string("TTF_RenderText_Blended failed: ") + TTF_GetError());
        }

        // Ensure the text surface uses a compatible format for blitting to target.
        SDL_Surface* converted = textSurface;
        if (textSurface->format->format != target->format->format) {
            converted = SDL_ConvertSurfaceFormat(textSurface, target->format->format, 0);
            SDL_FreeSurface(textSurface);
            if (!converted) {
                throw std::runtime_error(std::string("SDL_ConvertSurfaceFormat failed: ") + SDL_GetError());
            }
        }

        SDL_Rect dst;
        dst.x = x;
        dst.y = y;
        dst.w = converted->w;
        dst.h = converted->h;

        // Use alpha blending if available on target
        if (SDL_BlitSurface(converted, NULL, target, &dst) != 0) {
            if (converted != textSurface) SDL_FreeSurface(converted);
            throw std::runtime_error(std::string("SDL_BlitSurface failed: ") + SDL_GetError());
        }

        if (converted != textSurface) SDL_FreeSurface(converted);
    }

    // Render directly using an existing SDL_Renderer. This creates a temporary
    // surface for the glyphs, converts it to a texture, renders the texture and
    // then cleans up. This avoids creating large intermediate surfaces for the
    // whole screen and is the recommended path when you already have an
    // `SDL_Renderer`.
    void RenderTextToRenderer(SDL_Renderer* renderer,
                              const std::string& text,
                              int x,
                              int y,
                              int size = 24,
                              SDL_Color color = {255, 255, 255, 255})
    {
        if (!renderer) {
            throw std::invalid_argument("renderer is null");
        }

        TTF_Font* font = getFont(size);
        if (!font) {
            throw std::runtime_error("Failed to obtain font for size " + std::to_string(size));
        }

        SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
        if (!textSurface) {
            throw std::runtime_error(std::string("TTF_RenderText_Blended failed: ") + TTF_GetError());
        }

        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, textSurface);
        // free the surface immediately; texture now owns the pixel data
        SDL_FreeSurface(textSurface);

        if (!tex) {
            throw std::runtime_error(std::string("SDL_CreateTextureFromSurface failed: ") + SDL_GetError());
        }

        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

        SDL_Rect dst = { x, y, 0, 0 };
        if (SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h) != 0) {
            SDL_DestroyTexture(tex);
            throw std::runtime_error(std::string("SDL_QueryTexture failed: ") + SDL_GetError());
        }

        if (SDL_RenderCopy(renderer, tex, NULL, &dst) != 0) {
            SDL_DestroyTexture(tex);
            throw std::runtime_error(std::string("SDL_RenderCopy failed: ") + SDL_GetError());
        }

        SDL_DestroyTexture(tex);
    }

private:
    const char* m_fontPath;
    bool m_initializedTTF = false;
    std::map<int, TTF_Font*> m_fontCache; // key = font size

    TTF_Font* getFont(int size) {
        auto it = m_fontCache.find(size);
        if (it != m_fontCache.end()) return it->second;

        TTF_Font* f = TTF_OpenFont(m_fontPath, size);
        if (!f) return nullptr;
        m_fontCache[size] = f;
        return f;
    }
};