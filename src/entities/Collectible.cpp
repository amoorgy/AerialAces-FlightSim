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
    : x(0), y(25), z(0),
      innerRadius(0.8f),
      outerRadius(5.0f),
      collisionRadius(6.0f),
      collected(false),
      animTime(0),
      pulseScale(1.0f),
      glowIntensity(1.0f),
      rotationAngle(0),
      colorR(1.0f), colorG(0.9f), colorB(0.0f),
      pointValue(100),
      bonusTime(10.0f),
      ringModel(nullptr),
      ringTexture(nullptr),
      useModel(false) {
}

Collectible::Collectible(float posX, float posY, float posZ)
    : x(posX), y(posY), z(posZ),
      innerRadius(0.8f),
      outerRadius(5.0f),
      collisionRadius(6.0f),
      collected(false),
      animTime(0),
      pulseScale(1.0f),
      glowIntensity(1.0f),
      rotationAngle(0),
      colorR(1.0f), colorG(0.9f), colorB(0.0f),
      pointValue(100),
      bonusTime(10.0f),
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
    
    if (ringModel != nullptr) {
        delete ringModel;
        ringModel = nullptr;
    }
    if (ringTexture != nullptr) {
        delete ringTexture;
        ringTexture = nullptr;
    }
    
    ringModel = new Model();
    if (!ringModel->load(modelPath)) {
        std::cerr << "Collectible: Failed to load ring model, will use primitives" << std::endl;
        delete ringModel;
        ringModel = nullptr;
        useModel = false;
        return false;
    }
    
    ringModel->setScale(scale);
    
    float minX, maxX, minY, maxY, minZ, maxZ;
    ringModel->getBounds(minX, maxX, minY, maxY, minZ, maxZ);
    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    collisionRadius = std::max(sizeX, sizeY) / 2.0f + 2.0f;
    
    if (!texturePath.empty()) {
        ringTexture = new Texture();
        if (!ringTexture->load(texturePath)) {
            std::cerr << "Collectible: Warning - failed to load texture" << std::endl;
            delete ringTexture;
            ringTexture = nullptr;
        }
    }
    
    useModel = true;
    std::cout << "Collectible: Ring model loaded! Collision radius: " << collisionRadius << std::endl;
    return true;
}

void Collectible::update(float deltaTime) {
    if (collected) return;
    
    animTime += deltaTime;
    // Enhanced pulsing effect for better visibility
    pulseScale = 1.0f + 0.15f * std::sin(animTime * 3.0f);
    glowIntensity = 0.8f + 0.2f * std::sin(animTime * 2.5f + 0.5f);
    rotationAngle += 40.0f * deltaTime;  // Slightly faster rotation
    if (rotationAngle >= 360.0f) {
        rotationAngle -= 360.0f;
    }
}

void Collectible::render() const {
    if (collected) return;
    
    glPushMatrix();
    
    glTranslatef(x, y, z);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);
    glScalef(pulseScale, pulseScale, pulseScale);
    
    if (useModel && ringModel != nullptr && ringModel->isLoaded()) {
        if (ringTexture != nullptr && ringTexture->isLoaded()) {
            ringTexture->bind();
        }
        
        glColor3f(colorR * glowIntensity, colorG * glowIntensity, colorB * glowIntensity);
        
        GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
        if (!lightingEnabled) glEnable(GL_LIGHTING);
        
        ringModel->render();
        
        if (!lightingEnabled) glDisable(GL_LIGHTING);
        
        if (ringTexture != nullptr && ringTexture->isLoaded()) {
            ringTexture->unbind();
        }
    } else {
        // Fallback: Draw using primitives
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        glColor4f(colorR, colorG, colorB, glowIntensity * 0.4f);
        glutSolidTorus(innerRadius * 1.5f, outerRadius * 1.3f, 16, 32);
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        
        glColor3f(colorR * glowIntensity, colorG * glowIntensity, colorB * glowIntensity);
        glutSolidTorus(innerRadius, outerRadius, 20, 40);
        
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, colorB * 0.5f + 0.5f);
        glutSolidTorus(innerRadius * 0.5f, outerRadius, 12, 32);
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
