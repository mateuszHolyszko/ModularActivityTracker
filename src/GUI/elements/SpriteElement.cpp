#include <iostream>
#include <fstream>
#include <sstream>
#include "SpriteElement.hpp"
#include "../../ImageManager.hpp"
#include "../../RenderContext.hpp"

SpriteElement::SpriteElement(RenderContext* context,
                             ImageManager* imgManager,
                             int x, int y, int width, int height,
                             const std::string& spriteSheetKey,
                             const std::string& coordsFilePath,
                             int spriteSheetWidth,           // Add texture dimensions
                             int spriteSheetHeight,          // Add texture dimensions
                             const std::string& defaultAnimation,
                             int layer, Menu* parent)
    : BaseElement(context, x, y, width, height, false, layer, parent),
      imageManager(imgManager),
      spriteSheetKey(spriteSheetKey),
      textureId(0),
      currentFrame(0),
      frameTime(0.5f),
      currentTime(0.0f),
      isPlaying(false),
      loop(true) {
    
    if (imageManager) {
        textureId = imageManager->getTexture(spriteSheetKey);
    }
    
    // Load frame data from file
    std::ifstream file(coordsFilePath);
    if (!file.is_open()) {
        std::cerr << "[SpriteElement] Could not open: " << coordsFilePath << std::endl;
        return;
    }
    
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string name, xStr, yStr, wStr, hStr;
        
        if (std::getline(ss, name, ',') &&
            std::getline(ss, xStr, ',') &&
            std::getline(ss, yStr, ',') &&
            std::getline(ss, wStr, ',') &&
            std::getline(ss, hStr, ',')) {
            
            try {
                SpriteFrame frame;
                frame.name = name;
                frame.x = std::stoi(xStr);
                frame.y = std::stoi(yStr);
                frame.width = std::stoi(wStr);
                frame.height = std::stoi(hStr);
                
                frames.push_back(frame);
                
                // Extract animation name (everything before last underscore)
                size_t lastUnderscore = name.find_last_of('_');
                std::string animName = (lastUnderscore != std::string::npos) ? 
                    name.substr(0, lastUnderscore) : name;
                
                animations[animName].push_back(frames.size() - 1);
            } catch (...) {
                // Skip invalid lines
            }
        }
    }
    
    file.close();
    
    // Calculate UV coordinates using provided texture dimensions
    if (spriteSheetWidth > 0 && spriteSheetHeight > 0) {
        for (auto& frame : frames) {
            frame.u1 = static_cast<float>(frame.x) / spriteSheetWidth;
            frame.v1 = 1.0f - static_cast<float>(frame.y + frame.height) / spriteSheetHeight;  // Flip Y
            frame.u2 = static_cast<float>(frame.x + frame.width) / spriteSheetWidth;
            frame.v2 = 1.0f - static_cast<float>(frame.y) / spriteSheetHeight;  // Flip Y
        }
        
        std::cout << "[SpriteElement] Using texture size: " << spriteSheetWidth << "x" << spriteSheetHeight << std::endl;
    } else {
        std::cerr << "[SpriteElement] Invalid texture dimensions: " << spriteSheetWidth << "x" << spriteSheetHeight << std::endl;
    }
    
    // Set default animation
    if (!defaultAnimation.empty() && animations.find(defaultAnimation) != animations.end()) {
        currentAnimation = defaultAnimation;
    } else if (!animations.empty()) {
        currentAnimation = animations.begin()->first;
    }
    
    std::cout << "[SpriteElement] Loaded " << frames.size() << " frames, " 
              << animations.size() << " animations" << std::endl;

}

// ... rest of the methods remain the same ...
void SpriteElement::render() {
    if (!visible || !renderContext || textureId == 0 || frames.empty()) return;

    // Draw border if enabled
    if (showBorder) {
        SDL_Color borderColor = style.getBorderColor();
        
        GraphicCommand borderCmd;
        borderCmd.type = GraphicCommand::BOX;
        borderCmd.x1 = static_cast<float>(x);
        borderCmd.y1 = static_cast<float>(y);
        borderCmd.x2 = static_cast<float>(x + width);
        borderCmd.y2 = static_cast<float>(y + height);
        borderCmd.color = glm::vec4(borderColor.r / 255.0f, borderColor.g / 255.0f, borderColor.b / 255.0f, borderColor.a / 255.0f);
        borderCmd.layer = layer;
        borderCmd.lineWidth = 1.0f;
        borderCmd.filled = false;
        
        renderContext->graphicQueue.push_back(borderCmd);
    }
    
    if (currentAnimation.empty() || !isPlaying){
        //std::cerr << "Im at  currentAnimation.empty() || !isPlaying" << std::endl;
        return;
    }
    
    const auto& animFrames = animations[currentAnimation];
    if (currentFrame >= animFrames.size()) return;
        
    
    const SpriteFrame& frame = frames[animFrames[currentFrame]];
    
    renderContext->drawImageUV(textureId,
                              static_cast<float>(x),
                              static_cast<float>(y),
                              static_cast<float>(width),
                              static_cast<float>(height),
                              frame.u1, frame.v1,
                              frame.u2, frame.v2,
                              layer);
}

void SpriteElement::update(float deltaTime) {
    if (!isPlaying) return;
    if (currentAnimation.empty()) isPlaying = false;
    
    const auto& animFrames = animations[currentAnimation];
    if (animFrames.empty()) return;
    
    currentTime += deltaTime;
    
    if (currentTime >= frameTime) {
        currentTime -= frameTime;
        currentFrame++;
        
        if (currentFrame >= animFrames.size()) {
            if (loop) {
                currentFrame = 0;
            } else {
                currentFrame = animFrames.size() - 1;
                isPlaying = false;
            }
        }
    }
    
    BaseElement::update(deltaTime);
}

void SpriteElement::play(const std::string& animationName, bool shouldLoop) {
    if (animations.find(animationName) == animations.end()) {
        std::cerr << "[SpriteElement] Animation not found: " << animationName << std::endl;
        return;
    }
    
    if (currentAnimation != animationName) {
        currentAnimation = animationName;
        currentFrame = 0;
        currentTime = 0.0f;
    }
    
    loop = shouldLoop;
    isPlaying = true;
}

int SpriteElement::getAnimationFrameCount(const std::string& animationName) const {
    auto it = animations.find(animationName);
    if (it == animations.end()) return 0;
    return static_cast<int>(it->second.size());
}

void SpriteElement::stop() {
    isPlaying = false;
    currentFrame = 0;
    currentTime = 0.0f;
}