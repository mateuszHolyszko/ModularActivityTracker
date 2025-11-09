#ifndef RENDERCONTEXT_HPP
#define RENDERCONTEXT_HPP

#include <SDL2/SDL.h>

// Forward declarations
class TextRenderer;
class SimplesRenderer;

struct RenderContext {
    SDL_Renderer* sdlRenderer;
    TextRenderer* textRenderer;
    SimplesRenderer* simpleRenderer;
    
    RenderContext(SDL_Renderer* sdlRender, TextRenderer* textRender, SimplesRenderer* simpleRender)
        : sdlRenderer(sdlRender), textRenderer(textRender), simpleRenderer(simpleRender) {}
};

#endif