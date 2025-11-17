#pragma once
#include <GL/glew.h>
#include <string>
#include <unordered_map>
#include <memory>

struct ImageData {
    GLuint textureId = 0;
    int width = 0;
    int height = 0;
};

class ImageManager {
public:
    ImageManager();
    ~ImageManager();

    // Load an image from file and return its ID
    // Returns 0 if loading fails
    GLuint loadImage(const std::string& filepath);
    
    // Load an image with a custom key for easier reference
    GLuint loadImage(const std::string& filepath, const std::string& key);
    
    // Get texture by key
    GLuint getTexture(const std::string& key) const;
    
    // Get image data (includes dimensions)
    const ImageData* getImageData(const std::string& key) const;
    const ImageData* getImageData(GLuint textureId) const;
    
    // Create a scaled version of an existing image
    // Returns new texture ID
    GLuint createScaledImage(GLuint sourceTexId, int newWidth, int newHeight);
    GLuint createScaledImage(const std::string& sourceKey, int newWidth, int newHeight, const std::string& destKey = "");
    
    // Unload specific image
    void unloadImage(const std::string& key);
    void unloadImage(GLuint textureId);
    
    // Unload all images
    void clear();
    
    // Check if an image exists
    bool hasImage(const std::string& key) const;
    bool hasImage(GLuint textureId) const;

    // Color Filters
    GLuint applyGrayscale(const std::string& sourceKey, const std::string& destKey = "");
    GLuint applyGrayscale(GLuint sourceTexId);
    GLuint applyColorFilter(const std::string& sourceKey, const float rgbMultipliers[3], const std::string& destKey = "");
    GLuint applyColorFilter(GLuint sourceTexId, const float rgbMultipliers[3]);
    GLuint applySepia(const std::string& sourceKey, const std::string& destKey = "");
    GLuint applyInvert(const std::string& sourceKey, const std::string& destKey = "");

private:
    std::unordered_map<std::string, ImageData> imagesByKey;
    std::unordered_map<GLuint, ImageData> imagesByTexture;
    
    GLuint createTextureFromData(unsigned char* data, int width, int height);
    void deleteTexture(GLuint textureId);
};