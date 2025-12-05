#include "Collectible.h"
#include <cmath>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
// On Windows, GLEW must be included before GL headers
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Collectible::Collectible()
    : x(0), y(10), z(0),
      innerRadius(0.5f),
      outerRadius(4.0f),
      collisionRadius(5.0f),
      collected(false),
      animTime(0),
      pulseScale(1.0f),
      glowIntensity(1.0f),
      rotationAngle(0),
      colorR(1.0f), colorG(0.9f), colorB(0.0f),  // Golden yellow
      pointValue(100),
      bonusTime(5.0f),
      ringModel(nullptr),
      ringTexture(nullptr),
      useModel(false) {
}

Collectible::Collectible(float posX, float posY, float posZ)
    : x(posX), y(posY), z(posZ),
      innerRadius(0.5f),
      outerRadius(4.0f),
      collisionRadius(5.0f),
      collected(false),
      animTime(0),
      pulseScale(1.0f),
      glowIntensity(1.0f),
      rotationAngle(0),
      colorR(1.0f), colorG(0.9f), colorB(0.0f),  // Golden yellow
      pointValue(100),
      bonusTime(5.0f),
      ringModel(nullptr),
      ringTexture(nullptr),
      useModel(false) {
}

Collectible::~Collectible() {
    if (ringModel != nullptr) {
        delete ringModel;
        ringModel = nullptr;
    }
    if (ringTexture != nullptr) {
        delete ringTexture;
        ringTexture = nullptr;
    }
}

bool Collectible::loadModel(const std::string& modelPath, const std::string& texturePath, float scale) {
    std::cout << "Collectible: Loading ring model from " << modelPath << std::endl;
    
    // Clean up existing model/texture
    if (ringModel != nullptr) {
        delete ringModel;
        ringModel = nullptr;
    }
    if (ringTexture != nullptr) {
        delete ringTexture;
        ringTexture = nullptr;
    }
    
    // Load model
    ringModel = new Model();
    if (!ringModel->load(modelPath)) {
        std::cerr << "Collectible: Failed to load ring model, will use primitives" << std::endl;
        delete ringModel;
        ringModel = nullptr;
        useModel = false;
        return false;
    }
    
    ringModel->setScale(scale);
    
    // Load texture if provided
    if (!texturePath.empty()) {
        ringTexture = new Texture();
        if (!ringTexture->load(texturePath)) {
            std::cerr << "Collectible: Warning - failed to load texture, using model without texture" << std::endl;
            delete ringTexture;
            ringTexture = nullptr;
        } else {
            std::cout << "Collectible: Texture loaded successfully!" << std::endl;
        }
    }
    
    useModel = true;
    std::cout << "Collectible: Ring model loaded successfully!" << std::endl;
    return true;
}

void Collectible::update(float deltaTime) {
    if (collected) return;
    
    // Update animation time
    animTime += deltaTime;
    
    // Pulse scale animation (oscillates between 0.9 and 1.1)
    pulseScale = 1.0f + 0.1f * std::sin(animTime * 3.0f);
    
    // Glow intensity animation (oscillates between 0.6 and 1.0)
    glowIntensity = 0.8f + 0.2f * std::sin(animTime * 2.5f + 0.5f);
    
    // Rotation animation
    rotationAngle += 45.0f * deltaTime;  // 45 degrees per second
    if (rotationAngle >= 360.0f) {
        rotationAngle -= 360.0f;
    }
}

void Collectible::render() const {
    if (collected) return;
    
    glPushMatrix();
    
    // Position
    glTranslatef(x, y, z);
    
    // Rotate to face player direction (vertical ring)
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    
    // Add slow rotation for visual interest
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);
    
    // Apply pulse scale
    glScalef(pulseScale, pulseScale, pulseScale);
    
    // Use 3D model if loaded, otherwise use primitives
    if (useModel && ringModel != nullptr && ringModel->isLoaded()) {
        // Enable texture if available
        if (ringTexture != nullptr && ringTexture->isLoaded()) {
            ringTexture->bind();
        }
        
        // Set glowing color
        glColor3f(colorR * glowIntensity, colorG * glowIntensity, colorB * glowIntensity);
        
        // Enable lighting for proper model rendering
        GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
        if (!lightingEnabled) glEnable(GL_LIGHTING);
        
        // Render the 3D model
        ringModel->render();
        
        if (!lightingEnabled) glDisable(GL_LIGHTING);
        
        // Unbind texture
        if (ringTexture != nullptr && ringTexture->isLoaded()) {
            ringTexture->unbind();
        }
    } else {
        // Fallback: Draw using primitives
        // Draw outer glow (larger, semi-transparent)
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        glColor4f(colorR, colorG, colorB, glowIntensity * 0.3f);
        glutSolidTorus(innerRadius * 2.0f, outerRadius * 1.2f, 16, 32);
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        
        // Draw main ring
        glColor3f(colorR * glowIntensity, colorG * glowIntensity, colorB * glowIntensity);
        glutSolidTorus(innerRadius, outerRadius, 16, 32);
        
        // Draw inner bright core
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, colorB * 0.5f + 0.5f);  // Brighter core
        glutSolidTorus(innerRadius * 0.5f, outerRadius, 8, 32);
        glEnable(GL_LIGHTING);
    }
    
    glPopMatrix();
}

void Collectible::collect() {
    collected = true;
}

bool Collectible::isCollected() const {
    return collected;
}

void Collectible::reset() {
    collected = false;
    animTime = 0;
    pulseScale = 1.0f;
    glowIntensity = 1.0f;
    rotationAngle = 0;
}

void Collectible::setColor(float r, float g, float b) {
    colorR = r;
    colorG = g;
    colorB = b;
}

void Collectible::setPointValue(int points) {
    pointValue = points;
}

void Collectible::setBonusTime(float seconds) {
    bonusTime = seconds;
}
