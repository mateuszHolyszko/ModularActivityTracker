#include "SimpleRenderer.hpp"
#include "Style.hpp"
#include <algorithm>
#include <iostream>

// Use Desktop OpenGL with NanoVG
#include <SDL2/SDL_opengl.h>
#include "nanovg.h"
//#define NANOVG_GLES2_IMPLEMENTATION  // for mobile
#define NANOVG_GL2_IMPLEMENTATION  // for desktop
//#define NANOVG_GLES2_NO_HIGH_PRECISION
#include "nanovg_gl.h"

SimpleRenderer::SimpleRenderer() {}
SimpleRenderer::~SimpleRenderer() { shutdown(); }

bool SimpleRenderer::init(RenderContext* ctx, int width, int height) {
    context = ctx;
    texWidth = width;
    texHeight = height;

    // Set the renderer in the context so UI elements can access it
    if (ctx) {
        ctx->renderer = this;  
    }

    if (!createFBO(width, height)) {
        std::cerr << "[SimpleRenderer] Failed to create FBO.\n";
        return false;
    }

    vg = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);  // Change from GLES2 to GL3
    if (!vg) {
        std::cerr << "[SimpleRenderer] Failed to create NanoVG context.\n";
        return false;
    }

    defaultFont = nvgCreateFont(vg, "default", "fonts/VT323-Regular.ttf");
    if (defaultFont == -1) {
        std::cerr << "[SimpleRenderer] Failed to load default font.\n";
        return false;
    }

    std::cout << "[SimpleRenderer] Initialized successfully.\n";
    return true;
}

bool SimpleRenderer::createFBO(int width, int height) {
    // Generate FBO
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Generate color texture only (no depth/stencil)
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

    // Check status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    bool success = (status == GL_FRAMEBUFFER_COMPLETE);
    
    if (!success) {
        glDeleteTextures(1, &colorTex);
        glDeleteFramebuffers(1, &fbo);
        colorTex = 0;
        fbo = 0;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return success;
}

void SimpleRenderer::destroyFBO() {
    if (rbo) glDeleteRenderbuffers(1, &rbo);
    if (colorTex) glDeleteTextures(1, &colorTex);
    if (fbo) glDeleteFramebuffers(1, &fbo);
    rbo = colorTex = fbo = 0;
}

void SimpleRenderer::renderToTexture(std::vector<TextCommand>& textCommands, std::vector<GraphicCommand>& graphicCommands) {
    if (!vg) return;
    if (textCommands.empty() && graphicCommands.empty()) return;

    // Combine all commands into a single sorted list by layer
    struct RenderCommand {
        enum Type { TEXT, GRAPHIC };
        Type type;
        int layer;
        union {
            TextCommand* textCmd;
            GraphicCommand* graphicCmd;
        };
    };

    std::vector<RenderCommand> allCommands;
    
    // Add text commands
    for (auto& cmd : textCommands) {
        allCommands.push_back({RenderCommand::TEXT, cmd.layer, .textCmd = &cmd});
    }
    
    // Add graphic commands
    for (auto& cmd : graphicCommands) {
        allCommands.push_back({RenderCommand::GRAPHIC, cmd.layer, .graphicCmd = &cmd});
    }
    
    // Sort by layer
    std::sort(allCommands.begin(), allCommands.end(), [](const RenderCommand& a, const RenderCommand& b) {
        return a.layer < b.layer;
    });

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, texWidth, texHeight);
    glClearColor(style.bg_color.r/255.0f, style.bg_color.g/255.0f, style.bg_color.b/255.0f, 0); // transparent background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    nvgBeginFrame(vg, texWidth, texHeight, 1.0f);

    for (const auto& cmd : allCommands) {
        if (cmd.type == RenderCommand::TEXT) {
            const auto& textCmd = *cmd.textCmd;
            nvgFontSize(vg, textCmd.fontSize);
            nvgFontFace(vg, textCmd.font.empty() ? "default" : textCmd.font.c_str());
            nvgFillColor(vg, nvgRGBA(
                (unsigned char)(textCmd.color.r * 255),
                (unsigned char)(textCmd.color.g * 255),
                (unsigned char)(textCmd.color.b * 255),
                (unsigned char)(textCmd.color.a * 255)
            ));
            nvgText(vg, textCmd.x, textCmd.y, textCmd.text.c_str(), nullptr);
        } else { // GRAPHIC
            const auto& graphicCmd = *cmd.graphicCmd;
            nvgStrokeWidth(vg, graphicCmd.lineWidth);
            nvgStrokeColor(vg, nvgRGBA(
                (unsigned char)(graphicCmd.color.r * 255),
                (unsigned char)(graphicCmd.color.g * 255),
                (unsigned char)(graphicCmd.color.b * 255),
                (unsigned char)(graphicCmd.color.a * 255)
            ));
            nvgFillColor(vg, nvgRGBA(
                (unsigned char)(graphicCmd.color.r * 255),
                (unsigned char)(graphicCmd.color.g * 255),
                (unsigned char)(graphicCmd.color.b * 255),
                (unsigned char)(graphicCmd.color.a * 255)
            ));

            if (graphicCmd.type == GraphicCommand::LINE) {
                nvgBeginPath(vg);
                nvgMoveTo(vg, graphicCmd.x1, graphicCmd.y1);
                nvgLineTo(vg, graphicCmd.x2, graphicCmd.y2);
                nvgStroke(vg);
            } else if (graphicCmd.type == GraphicCommand::BOX) {
                nvgBeginPath(vg);
                nvgRect(vg, graphicCmd.x1, graphicCmd.y1, 
                        graphicCmd.x2 - graphicCmd.x1, 
                        graphicCmd.y2 - graphicCmd.y1);
                if (graphicCmd.filled) {
                    nvgFill(vg);
                } else {
                    nvgStroke(vg);
                }
            } 
            else if (graphicCmd.type == GraphicCommand::CIRCLE) {
                nvgBeginPath(vg);
                nvgCircle(vg, graphicCmd.x1, graphicCmd.y1, graphicCmd.x2);
                if (graphicCmd.filled) {
                    nvgFill(vg);
                } else {
                    nvgStroke(vg);
                }
            }
            else if (graphicCmd.type == GraphicCommand::TEXTURE) {
                // Draw textured quad into the UI FBO
                GLuint tex = graphicCmd.textureId;
                if (tex) {
                    glMatrixMode(GL_PROJECTION);
                    glPushMatrix();
                    glLoadIdentity();
                    glOrtho(0, texWidth, texHeight, 0, -1, 1);

                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();

                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, tex);
                    glColor4f(1, 1, 1, 1);

                    float x1 = graphicCmd.x1;
                    float y1 = graphicCmd.y1;
                    float x2 = graphicCmd.x2;
                    float y2 = graphicCmd.y2;
                    float u1 = graphicCmd.u1;
                    float v1 = graphicCmd.v1;
                    float u2 = graphicCmd.u2;
                    float v2 = graphicCmd.v2;

                    glBegin(GL_TRIANGLE_STRIP);
                    glTexCoord2f(u1, v1);
                    glVertex2f(x1, y1);
                    glTexCoord2f(u2, v1);
                    glVertex2f(x2, y1);
                    glTexCoord2f(u1, v2);
                    glVertex2f(x1, y2);
                    glTexCoord2f(u2, v2);
                    glVertex2f(x2, y2);
                    glEnd();

                    glBindTexture(GL_TEXTURE_2D, 0);
                    glDisable(GL_TEXTURE_2D);

                    glPopMatrix();
                    glMatrixMode(GL_PROJECTION);
                    glPopMatrix();
                    glMatrixMode(GL_MODELVIEW);
                }
            }
        }
    }

    nvgEndFrame(vg);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float SimpleRenderer::measureTextWidth(const std::string& text, const std::string& font, float fontSize) {
    if (!vg) {
        std::cerr << "[SimpleRenderer] measureTextWidth: NanoVG context not initialized!\n";
        return 0.0f;
    }
    
    if (text.empty()) {
        std::cout << "[SimpleRenderer] measureTextWidth: Empty text, returning 0\n";
        return 0.0f;
    }
    
    // Set up the font and size
    nvgFontSize(vg, fontSize);
    nvgFontFace(vg, font.empty() ? "default" : font.c_str());
    
    // Measure the text bounds
    float bounds[4];
    float width = nvgTextBounds(vg, 0, 0, text.c_str(), nullptr, bounds);
    
    // Debug output
    // std::cout << "[SimpleRenderer] measureTextWidth: Text='" << text 
    //           << "', Font='" << (font.empty() ? "default" : font) 
    //           << "', Size=" << fontSize 
    //           << "', Width=" << width 
    //           << "', Bounds=[" << bounds[0] << ", " << bounds[1] << ", " << bounds[2] << ", " << bounds[3] << "]\n";
    
    return width;
}

void SimpleRenderer::shutdown() {
    if (vg) {
        nvgDeleteGL2(vg);  // Change from GLES2 to GL3
        vg = nullptr;
    }
    destroyFBO();
}