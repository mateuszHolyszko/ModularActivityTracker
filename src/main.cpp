#define SDL_MAIN_HANDLED
#include "SimpleRenderer.hpp"
#include "QuadRenderer.hpp"
#include "GUI/Menus/StartMenu.hpp"
#include "PostProcess.hpp"  // Add this include
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glew.h>  // Keep for desktop; 
//#include <GLES2/gl2.h>  // Use GLES 2.0 headers instead of GL/glew.h for mobile
#include <iostream>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    const int screenW = 800;
    const int screenH = 480;

    // === Request an OpenGL ES 2.0 context ===
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    

    SDL_Window* window = SDL_CreateWindow("Activity Tracker",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenW, screenH,
        SDL_WINDOW_OPENGL);

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    
    // For desktop: Keep GLEW init; for mobile: Remove and use GLES2 directly
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "GLEW Error: " << glewGetErrorString(glewError) << std::endl;
        return 1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Initialize render context and renderer
    RenderContext ctx;
    SimpleRenderer renderer;
    QuadRenderer quad;
    PostProcess postProc;  // Add PostProcess instance

    if (!renderer.init(&ctx, screenW, screenH)) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
        return 1;
    }
    
    if (!quad.init()) {
        std::cerr << "Failed to initialize quad renderer!" << std::endl;
        return 1;
    }

    // Initialize PostProcess and add passes
    if (!postProc.init(screenW, screenH)) {
        std::cerr << "Failed to initialize post processor!" << std::endl;
        return 1;
    }
    postProc.addPass("src/GUI/shaders/barrel.vert", "src/GUI/shaders/distortion.frag", "distortion");  // Add your distortion  pass
    postProc.addPass("src/GUI/shaders/barrel.vert", "src/GUI/shaders/barrel.frag", "barrel");  // Add your barrel  pass

    // Create and initialize the start menu
    StartMenu startMenu(&ctx);
    startMenu.init();

    bool running = true;
    SDL_Event e;

    // Main game loop
    while (running) {
        // Handle events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            // Pass events to the menu
            startMenu.handleEvent(e);
        }

        // Update menu (for animations, etc.)
        startMenu.update(0.016f); // ~60 FPS delta time

        // Clear OpenGL framebuffer
        glClearColor(style.bg_color.r / 255.0f, 
             style.bg_color.g / 255.0f, 
             style.bg_color.b / 255.0f, 
             1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render menu - this clears queues and collects render commands from UI elements
        startMenu.render();

        // Process all rendering commands to texture
        renderer.renderToTexture(ctx.textQueue, ctx.graphicQueue);

        // Apply post-processing
        postProc.setUniform("distortion", "time", SDL_GetTicks() / 1000.0f);
        postProc.setUniform("distortion", "intensity", 0.2f);  // Adjust 0.0-1.0 for strength
        postProc.setUniform("barrel", "time", SDL_GetTicks() / 1000.0f);  // Example: Pass time in seconds
        GLuint finalTex = postProc.process(renderer.getTexture());

        // Draw the final texture to screen
        quad.draw(finalTex);  // Use processed texture

        SDL_GL_SwapWindow(window);

        // Cap frame rate (optional)
        SDL_Delay(32); // ~30 FPS
    }

    // Cleanup
    postProc.shutdown();  // Add PostProcess shutdown
    renderer.shutdown();
    quad.shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}