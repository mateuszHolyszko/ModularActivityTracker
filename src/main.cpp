#define SDL_MAIN_HANDLED
#include "SimpleRenderer.hpp"
#include "QuadRenderer.hpp"
#include "GUI/Menus/StartMenu.hpp"
#include "GUI/Menus/InitMenu/InitMenu.hpp"
#include "PostProcess.hpp"  
#include "GUI/widgets/LoadingWidget.hpp"  
#include "GUI/WorkThread.hpp"
#include "GUI/NotificationSystem.hpp"
#include "database/DatabaseManager.hpp"
#include "GUI/AppGlobals.h"
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

    // Initialize database
    DatabaseManager dbManager;
    if (!dbManager.initialize()) {
        std::cerr << "Failed to initialize database!" << std::endl;
        return 1; // Uncomment to exit if database is critical
    } else {
        std::cerr << "Database initialized successfully!" << std::endl;
    }
    AppGlobals::set<DatabaseManager*>("DatabaseManager", &dbManager);

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
    AppGlobals::set<RenderContext*>("RenderContext", &ctx);

    WorkThread worker;
    AppGlobals::set<WorkThread*>("WorkerThread", &worker);
    LoadingWidget defultLoading;
    defultLoading.setStyle(LoadingWidget::Style::SPINNER);
    defultLoading.setPosition(screenW/2, screenH/2);
    defultLoading.setColor(glm::vec4(style.text_color.r/255, style.text_color.g/255, style.text_color.b/255, 1.0f));

    SimpleRenderer renderer;
    QuadRenderer quad;
    PostProcess postProc;  // Add PostProcess instance

    // Initialize notification system
    auto notificationSystem = std::make_unique<NotificationSystem>(
        &ctx, 
        screenW/2-150.0f,    // x position (left side)
        50,    // y position (from top)
        300.0f,   // width
        200.0f,   // height
        5.0f,     // default delay in seconds
        "default", // font
        32.0f,    // font size
        0.5f      // fade duration in seconds (new parameter)
    );
    // Connect it to the render context
    ctx.notificationSystem = notificationSystem.get();
    AppGlobals::set<NotificationSystem*>("NotificationSystem", notificationSystem.get());

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

    ImageManager imageManager; // Create ImageManager instance for Cat Sprite since it will be used in all menus (navbar)
    // otherwise imageManager should be member of each menu that uses it so that images persist while menu is active and are freed when menu is destroyed
    imageManager.loadImage("src/GUI/images/cat/spritesheet.png", "cat_sprite_sheet");
    imageManager.applyGrayscale("cat_sprite_sheet","cat_sprite_sheet");
    AppGlobals::set<ImageManager*>("ImageManager", &imageManager);

    // Create and initialize the start menu
    //StartMenu startMenu(&ctx, &worker, &dbManager);
    //startMenu.init();
    AppGlobals::set<bool>("IsLoggedIn", false);  // Set global state log in status to false
    InitMenu initMenu(&ctx, &worker, &dbManager);
    initMenu.init();


    // Set the start menu as current menu in the context
    //ctx.setCurrentMenu(&startMenu);
    ctx.setCurrentMenu(&initMenu);

    bool running = true;
    SDL_Event e;
    
    // Delta time variables
    Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime;
    float dt;

    // Main game loop
    while (running) {
        // Calculate delta time
        currentTime = SDL_GetTicks();
        dt = (currentTime - lastTime) / 1000.0f; // Convert to seconds
        lastTime = currentTime;
        
        // Clamp delta time to avoid spiral of death
        if (dt > 0.1f) dt = 0.1f;

        // Handle events only if no background task is running
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (!worker.isRunning()) {
                ctx.currentMenu->handleEvent(e);
            }
        }

        // Update notification system (remove expired notifications)
        if (ctx.notificationSystem) {
            ctx.notificationSystem->update();
        }

        // Check background worker
        if (worker.isRunning()) {
            // Run loading animation with proper delta time
            defultLoading.update(dt);
            defultLoading.render(ctx);
        } else if (worker.isFinished()) {
            // Background task finished — process results or reset
            try {
                worker.checkError(); // rethrow if any exception
            } catch (std::exception& ex) {
                std::cerr << "Background error: " << ex.what() << std::endl;
            }
            worker.reset();
        }

        // Update current menu (for animations, etc.) with proper delta time
        if (ctx.currentMenu) {
            ctx.currentMenu->update(dt);
        }

        // Clear OpenGL framebuffer
        glClearColor(style.bg_color.r / 255.0f,
                    style.bg_color.g / 255.0f,
                    style.bg_color.b / 255.0f,
                    1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render current menu from context - this collects render commands from UI elements
        if (ctx.currentMenu) {
            ctx.currentMenu->render();
        }

        // Render notifications (this adds notification commands to the queues)
        if (ctx.notificationSystem) {
            ctx.notificationSystem->render(100); // Use high layer for notifications
        }

        // Process all rendering commands to texture
        renderer.renderToTexture(ctx.textQueue, ctx.graphicQueue);

        // Clear render context queues for next frame
        ctx.clearQueues();

        // Apply post-processing 
        postProc.setUniform("distortion", "time", SDL_GetTicks() / 1000.0f); 
        postProc.setUniform("distortion", "intensity", 0.2f); // Adjust 0.0-1.0 for strength 
        postProc.setUniform("barrel", "time", SDL_GetTicks() / 1000.0f); // Example: Pass time in seconds 
        GLuint finalTex = postProc.process(renderer.getTexture()); 
        
        // Draw the final texture to screen 
        quad.draw(finalTex); // Use processed texture 

        SDL_GL_SwapWindow(window); 
        
        // Frame rate cap with adaptive delay
        Uint32 frameTime = SDL_GetTicks() - currentTime;
        const Uint32 targetFrameTime = 16; // ~60 FPS
        if (frameTime < targetFrameTime) {
            SDL_Delay(targetFrameTime - frameTime);
        }
    }

    postProc.shutdown();
    renderer.shutdown();
    quad.shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}