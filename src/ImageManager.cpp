#include "ImageManager.hpp"
#include <iostream>
#include <vector>

// Use stb_image for loading images (header-only library)
//#define STB_IMAGE_IMPLEMENTATION // DON'T define STB_IMAGE_IMPLEMENTATION - nanovg already does this!
#include "stb_image.h"

// Optional: for image resizing
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

ImageManager::ImageManager() {
    // Flip images vertically on load to match OpenGL's bottom-left origin
    stbi_set_flip_vertically_on_load(true);
}

ImageManager::~ImageManager() {
    clear();
}

GLuint ImageManager::loadImage(const std::string& filepath) {
    return loadImage(filepath, filepath);
}

GLuint ImageManager::loadImage(const std::string& filepath, const std::string& key) {
    // Check if already loaded
    if (hasImage(key)) {
        std::cout << "[ImageManager] Image '" << key << "' already loaded.\n";
        return getTexture(key);
    }
    
    // Load image data
    int width, height, channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4); // Force RGBA
    
    if (!data) {
        std::cerr << "[ImageManager] Failed to load image: " << filepath << "\n";
        std::cerr << "[ImageManager] Error: " << stbi_failure_reason() << "\n";
        return 0;
    }
    
    std::cout << "[ImageManager] Loaded image: " << filepath 
              << " (" << width << "x" << height << ", " << channels << " channels)\n";
    
    // Create OpenGL texture
    GLuint textureId = createTextureFromData(data, width, height);
    stbi_image_free(data);
    
    if (textureId == 0) {
        std::cerr << "[ImageManager] Failed to create texture for: " << filepath << "\n";
        return 0;
    }
    
    // Store image data
    ImageData imgData = {textureId, width, height};
    imagesByKey[key] = imgData;
    imagesByTexture[textureId] = imgData;
    
    return textureId;
}

GLuint ImageManager::getTexture(const std::string& key) const {
    auto it = imagesByKey.find(key);
    return (it != imagesByKey.end()) ? it->second.textureId : 0;
}

const ImageData* ImageManager::getImageData(const std::string& key) const {
    auto it = imagesByKey.find(key);
    return (it != imagesByKey.end()) ? &it->second : nullptr;
}

const ImageData* ImageManager::getImageData(GLuint textureId) const {
    auto it = imagesByTexture.find(textureId);
    return (it != imagesByTexture.end()) ? &it->second : nullptr;
}

GLuint ImageManager::createScaledImage(GLuint sourceTexId, int newWidth, int newHeight) {
    const ImageData* sourceData = getImageData(sourceTexId);
    if (!sourceData) {
        std::cerr << "[ImageManager] Source texture not found for scaling\n";
        return 0;
    }
    
    // Read back the source texture
    int srcWidth = sourceData->width;
    int srcHeight = sourceData->height;
    std::vector<unsigned char> srcPixels(srcWidth * srcHeight * 4);
    
    glBindTexture(GL_TEXTURE_2D, sourceTexId);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, srcPixels.data());
    
    // Resize using stb_image_resize
    std::vector<unsigned char> dstPixels(newWidth * newHeight * 4);
    stbir_resize_uint8_linear(
        srcPixels.data(), srcWidth, srcHeight, 0,
        dstPixels.data(), newWidth, newHeight, 0,
        STBIR_RGBA
    );
    
    // Create new texture
    GLuint newTexId = createTextureFromData(dstPixels.data(), newWidth, newHeight);
    
    if (newTexId != 0) {
        ImageData imgData = {newTexId, newWidth, newHeight};
        imagesByTexture[newTexId] = imgData;
    }
    
    return newTexId;
}

GLuint ImageManager::createScaledImage(const std::string& sourceKey, int newWidth, int newHeight, const std::string& destKey) {
    GLuint sourceTexId = getTexture(sourceKey);
    if (sourceTexId == 0) {
        std::cerr << "[ImageManager] Source image '" << sourceKey << "' not found\n";
        return 0;
    }
    
    GLuint newTexId = createScaledImage(sourceTexId, newWidth, newHeight);
    
    if (newTexId != 0 && !destKey.empty()) {
        const ImageData* data = getImageData(newTexId);
        if (data) {
            imagesByKey[destKey] = *data;
        }
    }
    
    return newTexId;
}

void ImageManager::unloadImage(const std::string& key) {
    auto it = imagesByKey.find(key);
    if (it != imagesByKey.end()) {
        deleteTexture(it->second.textureId);
        imagesByTexture.erase(it->second.textureId);
        imagesByKey.erase(it);
    }
}

void ImageManager::unloadImage(GLuint textureId) {
    auto it = imagesByTexture.find(textureId);
    if (it != imagesByTexture.end()) {
        deleteTexture(textureId);
        
        // Remove from key map
        for (auto keyIt = imagesByKey.begin(); keyIt != imagesByKey.end(); ) {
            if (keyIt->second.textureId == textureId) {
                keyIt = imagesByKey.erase(keyIt);
            } else {
                ++keyIt;
            }
        }
        
        imagesByTexture.erase(it);
    }
}

void ImageManager::clear() {
    for (auto& pair : imagesByTexture) {
        deleteTexture(pair.first);
    }
    imagesByKey.clear();
    imagesByTexture.clear();
}

bool ImageManager::hasImage(const std::string& key) const {
    return imagesByKey.find(key) != imagesByKey.end();
}

bool ImageManager::hasImage(GLuint textureId) const {
    return imagesByTexture.find(textureId) != imagesByTexture.end();
}

GLuint ImageManager::createTextureFromData(unsigned char* data, int width, int height) {
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return textureId;
}

void ImageManager::deleteTexture(GLuint textureId) {
    if (textureId != 0) {
        glDeleteTextures(1, &textureId);
    }
}

#pragma region <Filters>
// Color Filters

GLuint ImageManager::applyGrayscale(const std::string& sourceKey, const std::string& destKey) {
    GLuint sourceTexId = getTexture(sourceKey);
    if (sourceTexId == 0) {
        std::cerr << "[ImageManager] Source image '" << sourceKey << "' not found\n";
        return 0;
    }
    
    GLuint resultTexId = applyGrayscale(sourceTexId);
    
    if (resultTexId != 0 && !destKey.empty()) {
        const ImageData* data = getImageData(resultTexId);
        if (data) {
            imagesByKey[destKey] = *data;
        }
    }
    
    return resultTexId;
}

GLuint ImageManager::applyGrayscale(GLuint sourceTexId) {
    const ImageData* sourceData = getImageData(sourceTexId);
    if (!sourceData) {
        std::cerr << "[ImageManager] Source texture not found for grayscale filter\n";
        return 0;
    }
    
    // Read back the source texture
    int width = sourceData->width;
    int height = sourceData->height;
    std::vector<unsigned char> srcPixels(width * height * 4);
    
    glBindTexture(GL_TEXTURE_2D, sourceTexId);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, srcPixels.data());
    
    // Apply grayscale filter
    std::vector<unsigned char> dstPixels(width * height * 4);
    
    for (int i = 0; i < width * height; ++i) {
        int srcIdx = i * 4;
        unsigned char r = srcPixels[srcIdx];
        unsigned char g = srcPixels[srcIdx + 1];
        unsigned char b = srcPixels[srcIdx + 2];
        unsigned char a = srcPixels[srcIdx + 3];
        
        // Luminosity method (recommended)
        unsigned char gray = static_cast<unsigned char>(0.21f * r + 0.72f * g + 0.07f * b);
        
        dstPixels[srcIdx] = gray;     // R
        dstPixels[srcIdx + 1] = gray; // G
        dstPixels[srcIdx + 2] = gray; // B
        dstPixels[srcIdx + 3] = a;    // A
    }
    
    // Create new texture
    GLuint newTexId = createTextureFromData(dstPixels.data(), width, height);
    
    if (newTexId != 0) {
        ImageData imgData = {newTexId, width, height};
        imagesByTexture[newTexId] = imgData;
    }
    
    return newTexId;
}

GLuint ImageManager::applyColorFilter(GLuint sourceTexId, const float rgbMultipliers[3]) {
    const ImageData* sourceData = getImageData(sourceTexId);
    if (!sourceData) {
        std::cerr << "[ImageManager] Source texture not found for color filter\n";
        return 0;
    }
    
    int width = sourceData->width;
    int height = sourceData->height;
    std::vector<unsigned char> srcPixels(width * height * 4);
    
    glBindTexture(GL_TEXTURE_2D, sourceTexId);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, srcPixels.data());
    
    std::vector<unsigned char> dstPixels(width * height * 4);
    
    for (int i = 0; i < width * height; ++i) {
        int srcIdx = i * 4;
        
        for (int channel = 0; channel < 3; ++channel) { // RGB only
            float multiplied = srcPixels[srcIdx + channel] * rgbMultipliers[channel];
            dstPixels[srcIdx + channel] = static_cast<unsigned char>(
                std::min(255.0f, std::max(0.0f, multiplied))
            );
        }
        dstPixels[srcIdx + 3] = srcPixels[srcIdx + 3]; // Keep alpha
    }
    
    GLuint newTexId = createTextureFromData(dstPixels.data(), width, height);
    
    if (newTexId != 0) {
        ImageData imgData = {newTexId, width, height};
        imagesByTexture[newTexId] = imgData;
    }
    
    return newTexId;
}

GLuint ImageManager::applySepia(const std::string& sourceKey, const std::string& destKey) {
    GLuint sourceTexId = getTexture(sourceKey);
    if (sourceTexId == 0) return 0;
    
    const ImageData* sourceData = getImageData(sourceTexId);
    if (!sourceData) return 0;
    
    int width = sourceData->width;
    int height = sourceData->height;
    std::vector<unsigned char> srcPixels(width * height * 4);
    
    glBindTexture(GL_TEXTURE_2D, sourceTexId);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, srcPixels.data());
    
    std::vector<unsigned char> dstPixels(width * height * 4);
    
    for (int i = 0; i < width * height; ++i) {
        int srcIdx = i * 4;
        unsigned char r = srcPixels[srcIdx];
        unsigned char g = srcPixels[srcIdx + 1];
        unsigned char b = srcPixels[srcIdx + 2];
        
        // Sepia tone formula
        dstPixels[srcIdx] = std::min(255, (int)(0.393f * r + 0.769f * g + 0.189f * b));     // R
        dstPixels[srcIdx + 1] = std::min(255, (int)(0.349f * r + 0.686f * g + 0.168f * b)); // G
        dstPixels[srcIdx + 2] = std::min(255, (int)(0.272f * r + 0.534f * g + 0.131f * b)); // B
        dstPixels[srcIdx + 3] = srcPixels[srcIdx + 3]; // Alpha
    }
    
    GLuint newTexId = createTextureFromData(dstPixels.data(), width, height);
    
    if (newTexId != 0 && !destKey.empty()) {
        ImageData imgData = {newTexId, width, height};
        imagesByKey[destKey] = imgData;
        imagesByTexture[newTexId] = imgData;
    }
    
    return newTexId;
}

GLuint ImageManager::applyInvert(const std::string& sourceKey, const std::string& destKey) {
    GLuint sourceTexId = getTexture(sourceKey);
    if (sourceTexId == 0) return 0;
    
    const ImageData* sourceData = getImageData(sourceTexId);
    if (!sourceData) return 0;
    
    int width = sourceData->width;
    int height = sourceData->height;
    std::vector<unsigned char> srcPixels(width * height * 4);
    
    glBindTexture(GL_TEXTURE_2D, sourceTexId);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, srcPixels.data());
    
    std::vector<unsigned char> dstPixels(width * height * 4);
    
    for (int i = 0; i < width * height * 4; ++i) {
        dstPixels[i] = 255 - srcPixels[i]; // Invert all channels including alpha
    }
    
    GLuint newTexId = createTextureFromData(dstPixels.data(), width, height);
    
    if (newTexId != 0 && !destKey.empty()) {
        ImageData imgData = {newTexId, width, height};
        imagesByKey[destKey] = imgData;
        imagesByTexture[newTexId] = imgData;
    }
    
    return newTexId;
}
#pragma endregion <Filters>