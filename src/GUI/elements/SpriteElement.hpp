#ifndef SPRITEELEMENT_HPP
#define SPRITEELEMENT_HPP

#include "BaseElement.hpp"
#include <string>
#include <vector>
#include <map>
#include <GL/glew.h>
#include "../../Style.hpp"

class ImageManager;

struct SpriteFrame {
    std::string name;
    int x, y;
    int width, height;
    float u1, v1, u2, v2;
};

class SpriteElement : public BaseElement {
private:
    ImageManager* imageManager;
    std::string spriteSheetKey;
    GLuint textureId;
    
    std::vector<SpriteFrame> frames;
    std::map<std::string, std::vector<int>> animations;
    
    std::string currentAnimation;
    int currentFrame;
    float frameTime;
    float currentTime;
    bool isPlaying;
    bool loop;
    bool showBorder;

public:
    SpriteElement(RenderContext* context,
                  ImageManager* imgManager,
                  int x, int y, int width, int height,
                  const std::string& spriteSheetKey,
                  const std::string& coordsFilePath,
                  int spriteSheetWidth,                    // Add texture dimensions
                  int spriteSheetHeight,                   // Add texture dimensions
                  const std::string& defaultAnimation = "",
                  int layer = 0, Menu* parent = nullptr);

    void render() override;
    void update(float deltaTime) override;
    bool handleEvent(const SDL_Event& event) override { return false; }
    
    void play(const std::string& animationName, bool shouldLoop = true);
    void stop();
    void setFrameTime(float time) { frameTime = time; }
    void setShowBorder(bool show) { showBorder = show; }

    // Getters for current animation state
    std::string getCurrentAnimationName() const { return currentAnimation; }
    int getCurrentFrameIndex() const { return currentFrame; }
    int getAnimationFrameCount(const std::string& animationName) const;
    
    bool hasAnimation(const std::string& animationName) const {
        return animations.find(animationName) != animations.end();
    }
};

#endif