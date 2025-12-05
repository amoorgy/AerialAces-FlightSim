#include "Obstacle.h"
#include <cmath>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

Obstacle::Obstacle()
    : x(0), y(0), z(0),
      width(10), height(30), depth(10),
      type(ObstacleType::MOUNTAIN),
      colorR(0.5f), colorG(0.4f), colorB(0.3f),
      baseRadius(15.0f),
      obstacleModel(nullptr),
      useModel(false) {
}

Obstacle::Obstacle(float posX, float posY, float posZ, 
                   float w, float h, float d, 
                   ObstacleType obstacleType)
    : x(posX), y(posY), z(posZ),
      width(w), height(h), depth(d),
      type(obstacleType),
      colorR(0.5f), colorG(0.4f), colorB(0.3f),
      baseRadius(w / 2.0f),
      obstacleModel(nullptr),
      useModel(false) {
    
    // Set default colors based on type
    switch (type) {
        case ObstacleType::MOUNTAIN:
            colorR = 0.5f; colorG = 0.4f; colorB = 0.35f;  // Brown/rocky
            break;
        case ObstacleType::GROUND:
            colorR = 0.2f; colorG = 0.5f; colorB = 0.2f;   // Green grass
            break;
        case ObstacleType::BUILDING:
            colorR = 0.6f; colorG = 0.6f; colorB = 0.6f;   // Gray
            break;
        case ObstacleType::ROCK:
            colorR = 0.45f; colorG = 0.42f; colorB = 0.4f; // Dark gray
            break;
    }
}

Obstacle::~Obstacle() {
    if (obstacleModel != nullptr) {
        delete obstacleModel;
        obstacleModel = nullptr;
    }
}

bool Obstacle::loadModel(const std::string& modelPath, float scale) {
    std::cout << "Obstacle: Loading model from " << modelPath << std::endl;
    
    if (obstacleModel != nullptr) {
        delete obstacleModel;
    }
    
    obstacleModel = new Model();
    if (obstacleModel->load(modelPath)) {
        obstacleModel->setScale(scale);
        useModel = true;
        std::cout << "Obstacle: Model loaded successfully!" << std::endl;
        return true;
    } else {
        std::cerr << "Obstacle: Failed to load model, will use primitives" << std::endl;
        delete obstacleModel;
        obstacleModel = nullptr;
        useModel = false;
        return false;
    }
}

void Obstacle::render() const {
    glPushMatrix();
    
    glColor3f(colorR, colorG, colorB);
    
    // Use 3D model if loaded, otherwise use primitives
    if (useModel && obstacleModel != nullptr && obstacleModel->isLoaded()) {
        // Position the model
        glTranslatef(x, y, z);
        
        // Enable lighting for proper model rendering
        GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
        if (!lightingEnabled) glEnable(GL_LIGHTING);
        
        // Render the 3D model
        obstacleModel->render();
        
        if (!lightingEnabled) glDisable(GL_LIGHTING);
    } else {
        // Fallback: Use primitives based on type
        switch (type) {
            case ObstacleType::MOUNTAIN:
                // Render as cone
                glTranslatef(x, y, z);
                glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);  // Point cone upward
                
                // Main mountain cone
                glutSolidCone(baseRadius, height, 12, 8);
                
                // Add some detail - snow cap on tall mountains
                if (height > 40.0f) {
                    glColor3f(0.95f, 0.95f, 0.98f);  // White snow
                    glTranslatef(0.0f, 0.0f, height * 0.7f);
                    glutSolidCone(baseRadius * 0.3f, height * 0.3f, 8, 4);
                }
                break;
            
        case ObstacleType::GROUND:
            // Render as flat quad
            glTranslatef(x, y, z);
            
            // Main ground
            glBegin(GL_QUADS);
            glNormal3f(0.0f, 1.0f, 0.0f);
            glVertex3f(-width / 2.0f, 0.0f, -depth / 2.0f);
            glVertex3f(width / 2.0f, 0.0f, -depth / 2.0f);
            glVertex3f(width / 2.0f, 0.0f, depth / 2.0f);
            glVertex3f(-width / 2.0f, 0.0f, depth / 2.0f);
            glEnd();
            
            // Add grid lines for visual reference
            glDisable(GL_LIGHTING);
            glColor3f(0.15f, 0.4f, 0.15f);  // Darker green lines
            glBegin(GL_LINES);
            for (float i = -width / 2.0f; i <= width / 2.0f; i += 50.0f) {
                glVertex3f(i, 0.1f, -depth / 2.0f);
                glVertex3f(i, 0.1f, depth / 2.0f);
            }
            for (float i = -depth / 2.0f; i <= depth / 2.0f; i += 50.0f) {
                glVertex3f(-width / 2.0f, 0.1f, i);
                glVertex3f(width / 2.0f, 0.1f, i);
            }
            glEnd();
            glEnable(GL_LIGHTING);
            break;
            
        case ObstacleType::BUILDING:
            // Render as box
            glTranslatef(x, y + height / 2.0f, z);
            glScalef(width, height, depth);
            glutSolidCube(1.0);
            break;
            
        case ObstacleType::ROCK:
            // Render as irregular shape (use sphere + offset)
            glTranslatef(x, y + height / 2.0f, z);
            glScalef(width / 2.0f, height / 2.0f, depth / 2.0f);
            glutSolidSphere(1.0, 8, 6);
            break;
        }
    }
    
    glPopMatrix();
}

void Obstacle::setColor(float r, float g, float b) {
    colorR = r;
    colorG = g;
    colorB = b;
}

float Obstacle::getRadiusAtHeight(float h) const {
    if (type != ObstacleType::MOUNTAIN || h < y || h > y + height) {
        return 0.0f;
    }
    
    // Linear interpolation - cone narrows from base to tip
    float heightRatio = (h - y) / height;
    return baseRadius * (1.0f - heightRatio);
}
