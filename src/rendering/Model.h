#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
// On Windows, GLEW must be included before GL headers
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

class Model {
public:
    Model();
    ~Model();
    
    // Load an OBJ file
    bool load(const std::string& filepath);
    
    // Render the model
    void render() const;
    
    // Check if model is loaded successfully
    bool isLoaded() const { return loaded; }
    
    // Get model bounds for collision detection
    void getBounds(float& minX, float& maxX, float& minY, float& maxY, float& minZ, float& maxZ) const;
    
    // Scale factor for adjusting model size
    void setScale(float scale);
    float getScale() const { return scaleFactor; }

private:
    bool loaded;
    float scaleFactor;
    
    // Model data
    std::vector<float> vertices;   // x, y, z positions
    std::vector<float> normals;    // nx, ny, nz
    std::vector<float> texcoords;  // u, v
    std::vector<unsigned int> indices; // Vertex indices for triangles
    
    // Bounding box
    float minBounds[3], maxBounds[3];
    
    void calculateBounds();
};

#endif // MODEL_H
