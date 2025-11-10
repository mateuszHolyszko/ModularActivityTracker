// Prevent SDL from redefining main — we provide the program entry point.
#define SDL_MAIN_HANDLED
#include "TextRenderer.hpp"
#include "QuadRenderer.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    const int screenW = 600;
    const int screenH = 480;

    SDL_Window* window = SDL_CreateWindow("Text Texture Test",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenW, screenH,
        SDL_WINDOW_OPENGL);

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    glewInit();

    // Initialize render context and text renderer
    RenderContext ctx;
    TextRenderer textRenderer;
    QuadRenderer quad;

    textRenderer.init(&ctx, screenW, screenH); // texture matches window size
    quad.init();

    // Add some text commands (layers optional)
    ctx.textQueue.push_back({"Hello GPU!", "", 30, 60, 0, 0, {1, 1, 1, 1}, 0, 24});
    ctx.textQueue.push_back({"Layer 1", "", 40, 90, 0, 0, {1, 0, 0, 1}, 1, 12});

    // Render text to the fullscreen texture
    textRenderer.renderToTexture(ctx.textQueue);

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the text texture as a fullscreen quad
        quad.draw(textRenderer.getTexture());

        SDL_GL_SwapWindow(window);
    }

    textRenderer.shutdown();
    quad.shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}