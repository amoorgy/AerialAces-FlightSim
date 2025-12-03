#include "Collectible.h"
#include <cmath>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
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
      bonusTime(5.0f) {
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
      bonusTime(5.0f) {
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
    
    glPopMatrix();
    
    // Draw collection sphere indicator (debug, can be removed)
    /*
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor4f(0.0f, 1.0f, 0.0f, 0.2f);
    glEnable(GL_BLEND);
    glutWireSphere(collisionRadius, 8, 8);
    glDisable(GL_BLEND);
    glPopMatrix();
    */
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
