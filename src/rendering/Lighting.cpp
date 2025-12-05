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
    : dayTime(12.0f),       // Start at noon
      daySpeed(0.0f),        // No automatic day progression (set > 0 to enable)
      nightMode(false),
      sunIntensity(1.0f),
      sunX(100.0f), sunY(200.0f), sunZ(100.0f),
      lighthouseAngle(0.0f),
      lighthouseSpeed(45.0f), // Degrees per second
      ambientR(0.3f), ambientG(0.3f), ambientB(0.3f) {
}

void Lighting::init() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);  // Sun
    glEnable(GL_LIGHT1);  // Lighthouse (optional)
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    // Enable smooth shading
    glShadeModel(GL_SMOOTH);
    
    // Enable normalization for proper lighting when scaled
    glEnable(GL_NORMALIZE);
}

void Lighting::update(float deltaTime) {
    // Update day/night cycle
    if (daySpeed > 0) {
        dayTime += daySpeed * deltaTime;
        if (dayTime >= 24.0f) dayTime -= 24.0f;
        
        // Calculate sun intensity based on time
        // Dawn: 6, Noon: 12, Dusk: 18, Midnight: 0
        float sunAngle = (dayTime - 6.0f) / 12.0f * M_PI;
        sunIntensity = std::max(0.0f, std::sin(sunAngle));
        
        // Auto-switch to night mode when dark
        nightMode = (sunIntensity < 0.3f);
    }
    
    // Update lighthouse beam rotation
    if (nightMode) {
        lighthouseAngle += lighthouseSpeed * deltaTime;
        if (lighthouseAngle >= 360.0f) lighthouseAngle -= 360.0f;
    }
    
    // Calculate sun position based on time
    float sunRotation = (dayTime / 24.0f) * 2.0f * M_PI;
    sunX = std::cos(sunRotation) * 200.0f;
    sunY = std::sin(sunRotation) * 200.0f + 50.0f;
    sunZ = 100.0f;
    
    // Adjust ambient based on day/night
    if (nightMode) {
        ambientR = 0.1f;
        ambientG = 0.1f;
        ambientB = 0.15f;
    } else {
        ambientR = 0.3f;
        ambientG = 0.3f;
        ambientB = 0.3f;
    }
}

void Lighting::apply() {
    // Global ambient light
    GLfloat globalAmbient[] = {ambientR, ambientG, ambientB, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    
    // Sun light (GL_LIGHT0)
    GLfloat sunPos[] = {sunX, sunY, sunZ, 0.0f}; // Directional (w=0)
    GLfloat sunDiffuse[] = {
        sunIntensity * 1.0f, 
        sunIntensity * 0.95f, 
        sunIntensity * 0.8f, 
        1.0f
    };
    GLfloat sunAmbient[] = {
        sunIntensity * 0.2f, 
        sunIntensity * 0.2f, 
        sunIntensity * 0.2f, 
        1.0f
    };
    GLfloat sunSpecular[] = {sunIntensity, sunIntensity, sunIntensity, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, sunPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpecular);
    
    // Lighthouse light (GL_LIGHT1) - only in night mode
    if (nightMode) {
        glEnable(GL_LIGHT1);
        
        float radAngle = lighthouseAngle * M_PI / 180.0f;
        
        // Lighthouse position (can be set per-level)
        GLfloat lighthousePos[] = {50.0f, 30.0f, 50.0f, 1.0f}; // Point light (w=1)
        
        // Spot direction based on rotating angle
        GLfloat spotDir[] = {
            std::cos(radAngle),
            -0.3f,
            std::sin(radAngle)
        };
        
        GLfloat lighthouseDiffuse[] = {1.0f, 1.0f, 0.8f, 1.0f};
        GLfloat lighthouseAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
        
        glLightfv(GL_LIGHT1, GL_POSITION, lighthousePos);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, lighthouseDiffuse);
        glLightfv(GL_LIGHT1, GL_AMBIENT, lighthouseAmbient);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0f);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 10.0f);
        
        // Attenuation
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.001f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0001f);
    } else {
        glDisable(GL_LIGHT1);
    }
}

void Lighting::toggleDayNight() {
    nightMode = !nightMode;
    if (nightMode) {
        sunIntensity = 0.2f;
    } else {
        sunIntensity = 1.0f;
    }
}

void Lighting::setNightMode(bool night) {
    nightMode = night;
    if (nightMode) {
        sunIntensity = 0.2f;
    } else {
        sunIntensity = 1.0f;
    }
}

bool Lighting::isNightMode() const {
    return nightMode;
}

void Lighting::flashEffect(float intensity) {
    // Temporarily boost ambient for flash effect
    // This would be called during explosion and reset after
    ambientR = std::min(1.0f, ambientR + intensity);
    ambientG = std::min(1.0f, ambientG + intensity * 0.8f);
    ambientB = std::min(1.0f, ambientB + intensity * 0.4f);
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
