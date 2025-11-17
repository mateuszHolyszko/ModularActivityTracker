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