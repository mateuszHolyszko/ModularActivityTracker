#include "ViewportElement.hpp"
#include "../../Style.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../3D/ShaderProgram.hpp"
 
ViewportElement::ViewportElement(RenderContext* ctx, int x_, int y_, int w_, int h_, int layer, Menu* parent, bool showBorder_)
    : BaseElement(ctx, x_, y_, w_, h_, false, layer, parent), texW(w_), texH(h_), showBorder(showBorder_) {
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
    // Convert rotation to 0-360 range
    float normalizedRotation = fmod(rotationDeg, 360.0f);
    if (normalizedRotation < 0) normalizedRotation += 360.0f;
    
    // Calculate how close we are to 180 degrees (backward facing)
    float distanceFromBack = fabs(normalizedRotation - 180.0f);
    
    // Speed multiplier: much faster when facing backward
    float speedMultiplier;
    if (distanceFromBack < 45.0f) {
        // Very fast when within ±45 degrees of 180
        speedMultiplier = 5.0f; // 5x speed
    } else if (distanceFromBack < 90.0f) {
        // Fast when within ±90 degrees of 180
        speedMultiplier = 2.5f; // 2.5x speed
    } else {
        // Normal speed otherwise
        speedMultiplier = 1.0f;
    }
    
    // Apply rotation with speed multiplier
    rotationDeg += rotationSpeedDegPerSec * speedMultiplier * dt;
    
    // Keep rotation in 0-360 range
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
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
    glEnable(GL_NORMALIZE);

    // Setup ORTHOGRAPHIC projection instead of perspective
    float aspect = (float)texW / (float)texH;
    float orthoSize = 1.0f; // Adjust this to control zoom level
    glm::mat4 proj = glm::ortho(-orthoSize * aspect, orthoSize * aspect, 
                                -orthoSize, orthoSize, 
                                -10.0f, 10.0f);

    // Simple view - just move back a bit to see the object
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
    
    // model matrix: rotate model around Y using viewport's rotationDeg
    glm::mat4 model_mat = glm::rotate(glm::mat4(1.0f), glm::radians(rotationDeg), glm::vec3(0.0f, 1.0f, 0.0f));

    // Compose MVP for shader
    glm::mat4 mvp = proj * view * model_mat;

    // Push matrices for any fixed-pipeline draws (keep compatibility)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(proj));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(view * model_mat));

    // Bind shader and set uniform if provided
    if (shader) {
        shader->use();
        shader->setMat4("u_mvp", mvp);
    }

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

    // Unbind shader to avoid leaking state
    if (shader) {
        glUseProgram(0);
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

    // Draw border if enabled (similar to Button class)
    if (showBorder) {
        SDL_Color borderColor = style.getBorderColor();
        
        GraphicCommand borderCmd;
        borderCmd.type = GraphicCommand::BOX;
        borderCmd.x1 = static_cast<float>(getAbsoluteX());
        borderCmd.y1 = static_cast<float>(getAbsoluteY());
        borderCmd.x2 = static_cast<float>(getAbsoluteX() + width);
        borderCmd.y2 = static_cast<float>(getAbsoluteY() + height);
        borderCmd.color = colorToVec4(borderColor);
        borderCmd.layer = layer + 1;  // Render border on top of the texture
        borderCmd.lineWidth = 1.0f;
        borderCmd.filled = false;
        
        renderContext->graphicQueue.push_back(borderCmd);
    }
}

bool ViewportElement::handleEvent(const SDL_Event& event) {
    (void)event; // Suppress unused parameter warning
    return false; // Viewport doesn't consume navigation events
}

void ViewportElement::resize(int w, int h) {
    if (w == texW && h == texH) return;

    destroyFBO();
    width = w;
    height = h;
    createFBO(w, h);
}