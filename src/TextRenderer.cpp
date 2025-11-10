#include "TextRenderer.hpp"

#include <algorithm>
#include <iostream>

// Include GL + NanoVG
#include <SDL2/SDL_opengl.h>
#include "nanovg.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg_gl.h"

TextRenderer::TextRenderer() {}
TextRenderer::~TextRenderer() { shutdown(); }

bool TextRenderer::init(RenderContext* ctx, int width, int height) {
    context = ctx;
    texWidth = width;
    texHeight = height;

    if (!createFBO(width, height)) {
        std::cerr << "[TextRenderer] Failed to create FBO.\n";
        return false;
    }

    vg = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    if (!vg) {
        std::cerr << "[TextRenderer] Failed to create NanoVG context.\n";
        return false;
    }

    defaultFont = nvgCreateFont(vg, "default", "fonts/RasterForgeRegular-JpBgm.ttf");
    if (defaultFont == -1) {
        std::cerr << "[TextRenderer] Failed to load default font.\n";
        return false;
    }

    std::cout << "[TextRenderer] Initialized successfully.\n";
    return true;
}

bool TextRenderer::createFBO(int width, int height) {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create texture
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

    // Create renderbuffer (for depth/stencil)
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    bool ok = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return ok;
}

void TextRenderer::destroyFBO() {
    if (rbo) glDeleteRenderbuffers(1, &rbo);
    if (colorTex) glDeleteTextures(1, &colorTex);
    if (fbo) glDeleteFramebuffers(1, &fbo);
    rbo = colorTex = fbo = 0;
}

void TextRenderer::renderToTexture(std::vector<TextCommand>& commands) {
    if (!vg) return;
    if (commands.empty()) return;

    // Sort by layer
    std::sort(commands.begin(), commands.end(), [](const TextCommand& a, const TextCommand& b) {
        return a.layer < b.layer;
    });

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, texWidth, texHeight);
    glClearColor(0, 0, 0, 0); // transparent background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    nvgBeginFrame(vg, texWidth, texHeight, 1.0f);

    for (const auto& cmd : commands) {
        nvgFontSize(vg, cmd.fontSize);
        nvgFontFace(vg, cmd.font.empty() ? "default" : cmd.font.c_str());
        nvgFillColor(vg, nvgRGBA(
            (unsigned char)(cmd.color.r * 255),
            (unsigned char)(cmd.color.g * 255),
            (unsigned char)(cmd.color.b * 255),
            (unsigned char)(cmd.color.a * 255)
        ));
        nvgText(vg, cmd.x, cmd.y, cmd.text.c_str(), nullptr);
    }

    nvgEndFrame(vg);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TextRenderer::shutdown() {
    if (vg) {
        nvgDeleteGLES2(vg);
        vg = nullptr;
    }
    destroyFBO();
}