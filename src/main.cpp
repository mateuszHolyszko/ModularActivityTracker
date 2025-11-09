#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <memory>
#include <vector>
#include "TextRenderer.hpp"
#include "SimplesRenderer.hpp"
#include "Style.hpp"
#include "RenderContext.hpp"
#include "GUI/Menus/StartMenu.hpp"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Activity Tracker - Start Menu",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 480,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Create renderers
    TextRenderer textRenderer("C:/dev/ModularActivityTracker/fonts/RasterForgeRegular-JpBgm.ttf");
    SimplesRenderer simpleRenderer(renderer);

    // Create render context
    RenderContext renderContext(renderer, &textRenderer, &simpleRenderer);

    // Create and initialize StartMenu
    auto startMenu = std::make_unique<StartMenu>(&renderContext);
    startMenu->init();

    bool running = true;
    SDL_Event event;

    // Calculate frame time for 48fps
    const Uint32 frameTime48fps = 1000 / 48;
    Uint32 lastFrameTime = SDL_GetTicks();

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsedTime = currentTime - lastFrameTime;

        // Handle events CONTINUALLY - no frame rate limit
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    }
                    std::cerr << "Key pressed: " << SDL_GetKeyName(event.key.keysym.sym) << std::endl;
                    break;
            }

            // Pass events to the menu
            startMenu->handleEvent(event);
        }

        // Only render at 48fps
        if (elapsedTime >= frameTime48fps) {
            // Clear screen
            SDL_Color bgColor = style.getBgColor();
            SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
            SDL_RenderClear(renderer);

            // Render the menu (elements will be sorted by layer)
            startMenu->render();

            // Present renderer
            SDL_RenderPresent(renderer);

            lastFrameTime = currentTime;
        } else {
            // Sleep if we have time left
            Uint32 sleepTime = frameTime48fps - elapsedTime;
            if (sleepTime > 0) {
                SDL_Delay(sleepTime);
            }
        }
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}