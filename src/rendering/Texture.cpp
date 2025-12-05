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

Texture::Texture() : loaded(false), textureID(0), width(0), height(0), channels(0) {
}

Texture::~Texture() {
    if (loaded && textureID != 0) {
        glDeleteTextures(1, &textureID);
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
        
        // Create OpenGL texture
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        
        delete[] data;
        
        loaded = true;
        channels = 3;
        
        std::cout << "Texture loaded successfully: " << width << "x" << height << std::endl;
        return true;
    }
    else if (ext == "jpg" || ext == "JPG" || ext == "jpeg" || ext == "JPEG") {
        // For JPEG files, we need a more complex loader
        // For now, just create a placeholder texture
        std::cout << "JPEG loading not fully implemented yet. Creating placeholder texture." << std::endl;
        
        // Create a simple procedural texture
        width = 256;
        height = 256;
        channels = 3;
        
        unsigned char* data = new unsigned char[width * height * 3];
        for (int i = 0; i < width * height * 3; i += 3) {
            // Create a checkerboard pattern
            int x = (i / 3) % width;
            int y = (i / 3) / width;
            unsigned char color = ((x / 32) + (y / 32)) % 2 == 0 ? 255 : 128;
            data[i] = color;
            data[i + 1] = color;
            data[i + 2] = color;
        }
        
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        
        delete[] data;
        
        loaded = true;
        
        std::cout << "Placeholder texture created: " << width << "x" << height << std::endl;
        return true;
    }
    else {
        std::cerr << "Unsupported texture format: " << ext << std::endl;
        return false;
    }
}

void Texture::bind() const {
    if (loaded && textureID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
}

void Texture::unbind() const {
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}
