#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"
#include <iostream>

// Simple BMP loader (most compatible format)
struct BMPHeader {
    unsigned short type;
    unsigned int size;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;
    unsigned int header_size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bits;
    unsigned int compression;
    unsigned int imagesize;
    int xresolution;
    int yresolution;
    int ncolors;
    int importantcolors;
};

Texture::Texture() : loaded(false), textureID(0), width(0), height(0), channels(0), imageData(nullptr) {
}

Texture::~Texture() {
    if (loaded && textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
    if (imageData) {
        stbi_image_free(imageData);
        imageData = nullptr;
    }
}

bool Texture::load(const std::string& filepath) {
    // For now, implement a simple BMP loader
    // This can be expanded to support more formats or use stb_image
    
    std::cout << "Attempting to load texture: " << filepath << std::endl;
    
    // Check file extension
    std::string ext = filepath.substr(filepath.find_last_of(".") + 1);
    
    if (ext == "bmp" || ext == "BMP") {
        // Load BMP file
        FILE* file = fopen(filepath.c_str(), "rb");
        if (!file) {
            std::cerr << "Failed to open texture file: " << filepath << std::endl;
            return false;
        }
        
        // Read BMP header
        unsigned char header[54];
        if (fread(header, 1, 54, file) != 54) {
            std::cerr << "Not a valid BMP file" << std::endl;
            fclose(file);
            return false;
        }
        
        if (header[0] != 'B' || header[1] != 'M') {
            std::cerr << "Not a valid BMP file" << std::endl;
            fclose(file);
            return false;
        }
        
        // Extract information
        unsigned int dataPos = *(int*)&(header[0x0A]);
        unsigned int imageSize = *(int*)&(header[0x22]);
        width = *(int*)&(header[0x12]);
        height = *(int*)&(header[0x16]);
        
        if (imageSize == 0) imageSize = width * height * 3;
        if (dataPos == 0) dataPos = 54;
        
        // Read pixel data
        unsigned char* data = new unsigned char[imageSize];
        fseek(file, dataPos, SEEK_SET);
        fread(data, 1, imageSize, file);
        fclose(file);
        
        // BMP stores BGR, need to convert to RGB
        for (unsigned int i = 0; i < imageSize; i += 3) {
            unsigned char temp = data[i];
            data[i] = data[i + 2];
            data[i + 2] = temp;
        }
        
        // Check if OpenGL context is ready
        GLint currentTexture = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cout << "OpenGL context not ready yet - skipping texture creation" << std::endl;
            delete[] data;
            loaded = false;
            return false;
        }
        
        // Create OpenGL texture with mipmaps and anisotropic filtering
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        
        // Generate mipmaps for better quality at distance
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Enable anisotropic filtering if available
        GLfloat maxAnisotropy = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        if (maxAnisotropy > 1.0f) {
            maxAnisotropy = (maxAnisotropy < 16.0f) ? maxAnisotropy : 16.0f; // Cap at 16x
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
            std::cout << "Anisotropic filtering enabled: " << maxAnisotropy << "x" << std::endl;
        }
        
        delete[] data;
        
        loaded = true;
        channels = 3;
        
        std::cout << "Texture loaded successfully: " << width << "x" << height << std::endl;
        return true;
    }
    else if (ext == "jpg" || ext == "JPG" || ext == "jpeg" || ext == "JPEG" ||
             ext == "png" || ext == "PNG" || ext == "tga" || ext == "TGA") {
        // Use stb_image for JPEG, PNG, TGA, etc.
        std::cout << "Loading image with stb_image: " << filepath << std::endl;
        
        int imgWidth, imgHeight, imgChannels;
        // Load image data into memory (no OpenGL calls here)
        imageData = stbi_load(filepath.c_str(), &imgWidth, &imgHeight, &imgChannels, 0);
        
        if (!imageData) {
            std::cerr << "Failed to load image: " << filepath << std::endl;
            std::cerr << "stbi_image error: " << stbi_failure_reason() << std::endl;
            return false;
        }
        
        width = imgWidth;
        height = imgHeight;
        channels = imgChannels;
        
        std::cout << "Image loaded: " << width << "x" << height << " with " << channels << " channels" << std::endl;
        std::cout << "GL texture will be created on first bind()" << std::endl;
        
        loaded = true;
        return true;
    }
    else {
        std::cerr << "Unsupported texture format: " << ext << std::endl;
        return false;
    }
}

void Texture::bind() const {
    if (!loaded) return;
    
    // Lazy GL texture creation on first bind
    if (textureID == 0 && imageData != nullptr) {
        const_cast<Texture*>(this)->createGLTexture();
    }
    
    if (textureID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
}

void Texture::unbind() const {
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::createGLTexture() {
    if (textureID != 0 || !imageData) return;
    
    std::cout << "Creating OpenGL texture: " << width << "x" << height << std::endl;
    
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    GLenum format = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_RED;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
    
    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Enable anisotropic filtering if available
    if (GLEW_EXT_texture_filter_anisotropic) {
        GLfloat maxAnisotropy = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        if (maxAnisotropy > 1.0f) {
            maxAnisotropy = (maxAnisotropy < 16.0f) ? maxAnisotropy : 16.0f;
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        }
    }
    
    // Free image data now that GL texture is created
    stbi_image_free(imageData);
    imageData = nullptr;
    
    std::cout << "OpenGL texture created successfully" << std::endl;
}
