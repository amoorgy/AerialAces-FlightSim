#include "Lighting.h"
#include <cmath>
#include <algorithm>

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

Lighting::Lighting()
    : dayTime(12.0f),        // Start at noon
      daySpeed(0.02f),       // Slow speed - takes ~30 seconds for noticeable change
      nightMode(false),      // Start in day mode
      sunIntensity(1.0f),    // Start at full day intensity
      sunX(100.0f), sunY(80.0f), sunZ(100.0f),
      sunDistance(500.0f),
      lighthouseAngle(0.0f),
      lighthouseSpeed(45.0f),
      ambientR(0.4f), ambientG(0.35f), ambientB(0.3f),
      flashIntensity(0.0f),
      flashDecay(3.0f),
      flareIntensity(0.0f) {
}

void Lighting::init() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);  // Sun
    glEnable(GL_LIGHT1);  // Fill light for sunset
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    // Enable smooth shading
    glShadeModel(GL_SMOOTH);
    
    // Enable normalization for proper lighting when scaled
    glEnable(GL_NORMALIZE);
}

void Lighting::update(float deltaTime) {
    dayTime += daySpeed * deltaTime;
    
    if (nightMode) {
        // Night mode - very dark with minimal ambient light
        sunIntensity = 0.05f;  // Almost no sun
        
        // Very dark blue ambient for night
        ambientR = 0.02f;
        ambientG = 0.02f;
        ambientB = 0.08f;
        
        // Moon position (opposite of sun)
        float moonAngle = M_PI;  // Opposite of day sun
        sunY = std::sin(moonAngle) * sunDistance * 0.3f;
        sunX = std::cos(moonAngle) * sunDistance;
        sunZ = sunDistance * 0.3f;
        
        if (sunY < 20.0f) sunY = 20.0f;
        
    } else {
        // Day mode - bright and clear
        sunIntensity = 0.95f;
        
        // Bright ambient light for day
        ambientR = 0.5f;
        ambientG = 0.5f;
        ambientB = 0.55f;
        
        // Sun position high in sky
        float sunAngle = M_PI * 0.4f;  // 72 degrees up
        sunY = std::sin(sunAngle) * sunDistance;
        sunX = std::cos(sunAngle) * sunDistance * 0.5f;
        sunZ = sunDistance * 0.3f;
    }
    
    // Update flash decay
    if (flashIntensity > 0) {
        flashIntensity -= flashDecay * deltaTime;
        if (flashIntensity < 0) flashIntensity = 0;
    }
    
    // Add flash effect to ambient
    ambientR = std::min(1.0f, ambientR + flashIntensity);
    ambientG = std::min(1.0f, ambientG + flashIntensity * 0.8f);
    ambientB = std::min(1.0f, ambientB + flashIntensity * 0.4f);
}

void Lighting::apply() {
    // Global ambient light
    GLfloat globalAmbient[] = {ambientR, ambientG, ambientB, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    
    // Sun/Moon light (GL_LIGHT0)
    GLfloat sunPos[] = {sunX, sunY, sunZ, 0.0f}; // Directional (w=0)
    
    GLfloat sunDiffuse[4], sunAmbient[4], sunSpecular[4];
    
    if (nightMode) {
        // Night - cool blue moonlight, very dim
        sunDiffuse[0] = sunIntensity * 0.3f;
        sunDiffuse[1] = sunIntensity * 0.3f;
        sunDiffuse[2] = sunIntensity * 0.5f;
        sunDiffuse[3] = 1.0f;
        
        sunAmbient[0] = sunIntensity * 0.05f;
        sunAmbient[1] = sunIntensity * 0.05f;
        sunAmbient[2] = sunIntensity * 0.1f;
        sunAmbient[3] = 1.0f;
        
        sunSpecular[0] = sunIntensity * 0.2f;
        sunSpecular[1] = sunIntensity * 0.2f;
        sunSpecular[2] = sunIntensity * 0.3f;
        sunSpecular[3] = 1.0f;
    } else {
        // Day - bright white/yellow sunlight
        sunDiffuse[0] = sunIntensity * 1.0f;
        sunDiffuse[1] = sunIntensity * 0.95f;
        sunDiffuse[2] = sunIntensity * 0.8f;
        sunDiffuse[3] = 1.0f;
        
        sunAmbient[0] = sunIntensity * 0.3f;
        sunAmbient[1] = sunIntensity * 0.3f;
        sunAmbient[2] = sunIntensity * 0.25f;
        sunAmbient[3] = 1.0f;
        
        sunSpecular[0] = sunIntensity * 1.0f;
        sunSpecular[1] = sunIntensity * 1.0f;
        sunSpecular[2] = sunIntensity * 1.0f;
        sunSpecular[3] = 1.0f;
    }
    
    glLightfv(GL_LIGHT0, GL_POSITION, sunPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpecular);
    
    // Fill light (GL_LIGHT1) - disabled in night mode
    if (nightMode) {
        glDisable(GL_LIGHT1);
    } else {
        glEnable(GL_LIGHT1);
        
        GLfloat fillPos[] = {-sunX * 0.5f, sunY * 0.3f, -sunZ * 0.5f, 0.0f};
        GLfloat fillDiffuse[] = {
            0.2f * sunIntensity,
            0.2f * sunIntensity,
            0.25f * sunIntensity,
            1.0f
        };
        GLfloat fillAmbient[] = {0.05f, 0.05f, 0.08f, 1.0f};
        
        glLightfv(GL_LIGHT1, GL_POSITION, fillPos);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, fillDiffuse);
        glLightfv(GL_LIGHT1, GL_AMBIENT, fillAmbient);
        glLightfv(GL_LIGHT1, GL_SPECULAR, fillDiffuse);
    }
}

void Lighting::toggleDayNight() {
    nightMode = !nightMode;
}

void Lighting::setNightMode(bool night) {
    nightMode = night;
}

bool Lighting::isNightMode() const {
    return nightMode;
}

void Lighting::flashEffect(float intensity) {
    flashIntensity = intensity;
}

void Lighting::setDaySpeed(float speed) {
    daySpeed = speed;
}

float Lighting::getDayTime() const {
    return dayTime;
}

float Lighting::getSunIntensity() const {
    return sunIntensity;
}

float Lighting::getLighthouseAngle() const {
    return lighthouseAngle;
}

void Lighting::updateLighthouseBeam(float deltaTime) {
    // Rotate lighthouse beam continuously
    lighthouseAngle += lighthouseSpeed * deltaTime;
    while (lighthouseAngle >= 360.0f) {
        lighthouseAngle -= 360.0f;
    }
}

float Lighting::calculateFlareIntensity(float camX, float camY, float camZ,
                                         float lookX, float lookY, float lookZ) const {
    // Sunset always has some sun to create flares
    if (sunIntensity < 0.3f) {
        return 0.0f;
    }
    
    // Calculate direction from camera to sun
    float toSunX = sunX - camX;
    float toSunY = sunY - camY;
    float toSunZ = sunZ - camZ;
    
    // Normalize
    float toSunLen = std::sqrt(toSunX*toSunX + toSunY*toSunY + toSunZ*toSunZ);
    if (toSunLen < 0.001f) return 0.0f;
    
    toSunX /= toSunLen;
    toSunY /= toSunLen;
    toSunZ /= toSunLen;
    
    // Normalize look direction
    float lookLen = std::sqrt(lookX*lookX + lookY*lookY + lookZ*lookZ);
    if (lookLen < 0.001f) return 0.0f;
    
    lookX /= lookLen;
    lookY /= lookLen;
    lookZ /= lookLen;
    
    // Dot product gives cosine of angle between camera look direction and sun direction
    float dot = toSunX * lookX + toSunY * lookY + toSunZ * lookZ;
    
    // Flare is strongest when looking directly at sun (dot = 1)
    // Sunset flares are more prominent - start showing earlier
    if (dot < 0.80f) {
        return 0.0f;
    }
    
    // Map 0.80-1.0 to 0-1 intensity, with exponential falloff
    // Sunset flares are warmer and more dramatic
    float normalizedAngle = (dot - 0.80f) / 0.20f;
    float flare = normalizedAngle * normalizedAngle * sunIntensity * 1.2f;
    
    return std::min(1.0f, flare);
}
