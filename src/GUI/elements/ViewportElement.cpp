#include "ViewportElement.hpp"
#include "../../Style.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
 
ViewportElement::ViewportElement(RenderContext* ctx, int x_, int y_, int w_, int h_, int layer, BaseElement* parent)
    : BaseElement(ctx, x_, y_, w_, h_, false, layer, parent), texW(w_), texH(h_) {
    renderContext = ctx;
    if (!createFBO(texW, texH)) {
        std::cerr << "[ViewportElement] Failed to create FBO.\n";
    }
}

ViewportElement::~ViewportElement() {
    destroyFBO();
}

bool ViewportElement::createFBO(int w, int h) {
    texW = w;
    texH = h;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

    // NEW: create a depth renderbuffer so 3D depth testing works
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, texW, texH);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    bool ok = (status == GL_FRAMEBUFFER_COMPLETE);

    if (!ok) {
        std::cerr << "[ViewportElement] FBO status: " << status << "\n";
        glDeleteTextures(1, &colorTex);
        glDeleteFramebuffers(1, &fbo);
        if (rbo) { glDeleteRenderbuffers(1, &rbo); rbo = 0; }
        colorTex = 0;
        fbo = 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return ok;
}

void ViewportElement::destroyFBO() {
    if (colorTex) glDeleteTextures(1, &colorTex);
    if (fbo) glDeleteFramebuffers(1, &fbo);
    if (rbo) glDeleteRenderbuffers(1, &rbo);
    colorTex = 0;
    fbo = 0;
    rbo = 0;
}

void ViewportElement::update(float dt) {
    // advance rotation (degrees)
    rotationDeg += rotationSpeedDegPerSec * dt;
    if (rotationDeg >= 360.0f) rotationDeg -= 360.0f;
}

void ViewportElement::render3DToTexture() {
    if (!fbo) return;

    // Bind viewport FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, texW, texH);

    // Clear with a default color and clear depth
    glClearColor(style.bg_color.r / 255.0f,
                 style.bg_color.g / 255.0f,
                 style.bg_color.b / 255.0f,
                 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- robust GL state for 3D rendering ---
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    //glDisable(GL_CULL_FACE);                 // keep disabled while debugging/wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
    glEnable(GL_NORMALIZE);

    // Setup projection using glm (avoids glu dependency)
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)texW / (float)texH, 0.1f, 10.0f);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(proj));

    // Setup a simple view (camera back a bit + tilt) and apply spin
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
    // apply element spin around Y
    view = glm::rotate(view, glm::radians(rotationDeg), glm::vec3(0.0f, 1.0f, 0.0f));
    // small tilt
    //view = glm::rotate(view, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(view));

    // Call user-provided render callback (should only emit 3D draw calls)
    if (onRender3D) {
        onRender3D();
    } else {
        static bool warned = false;
        if (!warned) {
            std::cerr << "[ViewportElement] onRender3D callback is null\n";
            warned = true;
        }
    }

    // Restore matrices and GL state
    glPopMatrix(); // MODELVIEW
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_NORMALIZE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ViewportElement::render() {
    if (!visible) return;

    // Render 3D content to texture first
    render3DToTexture();

    // Push a TEXTURE command to render the viewport into the UI
    GraphicCommand texCmd;
    texCmd.type = GraphicCommand::TEXTURE;
    texCmd.x1 = static_cast<float>(getAbsoluteX());
    texCmd.y1 = static_cast<float>(getAbsoluteY());
    texCmd.x2 = static_cast<float>(getAbsoluteX() + width);
    texCmd.y2 = static_cast<float>(getAbsoluteY() + height);
    texCmd.layer = layer;
    texCmd.textureId = colorTex;
    texCmd.u1 = 0.0f;
    texCmd.v1 = 0.0f;
    texCmd.u2 = 1.0f;
    texCmd.v2 = 1.0f;

    renderContext->graphicQueue.push_back(texCmd);
}

void ViewportElement::handleEvent(const SDL_Event& event) {
    (void)event; // Suppress unused parameter warning
}

void ViewportElement::resize(int w, int h) {
    if (w == texW && h == texH) return;

    destroyFBO();
    width = w;
    height = h;
    createFBO(w, h);
}