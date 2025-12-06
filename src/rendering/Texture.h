#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
// On Windows, GLEW must be included before GL headers
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

class Texture {
public:
    Texture();
    ~Texture();
    
    // Load a texture from file (JPEG, PNG, BMP, etc.)
    bool load(const std::string& filepath);
    
    // Bind texture for rendering
    void bind() const;
    
    // Unbind texture
    void unbind() const;
    
    // Check if texture is loaded
    bool isLoaded() const { return loaded; }
    
    // Get OpenGL texture ID
    GLuint getID() const { return textureID; }

private:
    void createGLTexture(); // Helper to create GL texture from imageData
    bool loaded;
    GLuint textureID;
    int width, height, channels;
    unsigned char* imageData; // Store image data until GL texture is created
};

#endif // TEXTURE_H
