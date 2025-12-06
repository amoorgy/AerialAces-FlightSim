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
    : dayTime(8.0f),         // Start at 8am (early morning)
      daySpeed(0.02f),       // Very slow - takes ~800 seconds (13+ min) for full day
      nightMode(false),
      sunIntensity(1.0f),
      sunX(100.0f), sunY(200.0f), sunZ(100.0f),
      sunDistance(500.0f),
      lighthouseAngle(0.0f),
      lighthouseSpeed(45.0f),
      ambientR(0.3f), ambientG(0.3f), ambientB(0.3f),
      flashIntensity(0.0f),
      flashDecay(3.0f),
      flareIntensity(0.0f) {
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
    }
    
    // Calculate sun intensity based on time of day
    // Dawn: ~6am, Noon: 12pm, Dusk: ~18pm, Night: 0-6, 18-24
    float timeAngle;
    if (dayTime >= 6.0f && dayTime <= 18.0f) {
        // Daytime: sun rises at 6, peaks at 12, sets at 18
        timeAngle = (dayTime - 6.0f) / 12.0f * M_PI;
        sunIntensity = std::sin(timeAngle);
    } else {
        // Nighttime
        sunIntensity = 0.0f;
    }
    
    // Clamp intensity and determine night mode
    sunIntensity = std::max(0.0f, std::min(1.0f, sunIntensity));
    nightMode = (sunIntensity < 0.2f);
    
    // Calculate sun position - sun moves in an arc across the sky
    // At noon (12), sun is directly overhead
    // At 6am, sun is on the horizon (east)
    // At 6pm, sun is on the horizon (west)
    float sunAngle = (dayTime - 6.0f) / 12.0f * M_PI;  // 0 at 6am, PI at 6pm
    
    // Sun position relative to world origin
    sunY = std::sin(sunAngle) * sunDistance;          // Height
    sunX = std::cos(sunAngle) * sunDistance * 0.5f;   // East-West movement
    sunZ = sunDistance * 0.3f;                         // Slightly in front
    
    // Keep sun above horizon during day
    if (sunY < 0) sunY = 0;
    
    // Update lighthouse beam rotation
    if (nightMode) {
        lighthouseAngle += lighthouseSpeed * deltaTime;
        if (lighthouseAngle >= 360.0f) lighthouseAngle -= 360.0f;
    }
    
    // Update flash decay
    if (flashIntensity > 0) {
        flashIntensity -= flashDecay * deltaTime;
        if (flashIntensity < 0) flashIntensity = 0;
    }
    
    // Adjust ambient based on sun intensity with smooth transition
    float dayAmbient = 0.25f + sunIntensity * 0.15f;
    float nightAmbient = 0.08f;
    
    if (nightMode) {
        ambientR = nightAmbient;
        ambientG = nightAmbient;
        ambientB = nightAmbient + 0.03f;  // Slightly blue at night
    } else {
        // Warm ambient during day, cooler at dawn/dusk
        float warmth = sunIntensity;
        ambientR = dayAmbient + warmth * 0.05f;
        ambientG = dayAmbient;
        ambientB = dayAmbient - warmth * 0.02f;
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
    
    // Sun light (GL_LIGHT0)
    GLfloat sunPos[] = {sunX, sunY, sunZ, 0.0f}; // Directional (w=0)
    
    // Sun color changes throughout day
    // Morning/evening: warm orange
    // Noon: bright white-yellow
    float warmth = 1.0f - sunIntensity;  // More warm when sun is low
    GLfloat sunDiffuse[] = {
        sunIntensity * (1.0f + warmth * 0.3f), 
        sunIntensity * (0.95f - warmth * 0.1f), 
        sunIntensity * (0.8f - warmth * 0.3f), 
        1.0f
    };
    GLfloat sunAmbient[] = {
        sunIntensity * 0.15f, 
        sunIntensity * 0.15f, 
        sunIntensity * 0.15f, 
        1.0f
    };
    GLfloat sunSpecular[] = {sunIntensity, sunIntensity, sunIntensity * 0.9f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, sunPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpecular);
    
    // Lighthouse light (GL_LIGHT1) - only in night mode
    if (nightMode) {
        glEnable(GL_LIGHT1);
        
        float radAngle = lighthouseAngle * M_PI / 180.0f;
        
        GLfloat lighthousePos[] = {50.0f, 30.0f, 50.0f, 1.0f};
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
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.001f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0001f);
    } else {
        glDisable(GL_LIGHT1);
    }
}

void Lighting::toggleDayNight() {
    if (nightMode) {
        dayTime = 12.0f;  // Jump to noon
    } else {
        dayTime = 0.0f;   // Jump to midnight
    }
    nightMode = !nightMode;
    
    if (nightMode) {
        sunIntensity = 0.0f;
    } else {
        sunIntensity = 1.0f;
    }
}

void Lighting::setNightMode(bool night) {
    nightMode = night;
    if (nightMode) {
        dayTime = 0.0f;
        sunIntensity = 0.0f;
    } else {
        dayTime = 12.0f;
        sunIntensity = 1.0f;
    }
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

float Lighting::calculateFlareIntensity(float camX, float camY, float camZ,
                                         float lookX, float lookY, float lookZ) const {
    // If it's night or sun is below horizon, no flare
    if (nightMode || sunIntensity < 0.1f) {
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
    
    // Normalize look direction (should already be normalized but just in case)
    float lookLen = std::sqrt(lookX*lookX + lookY*lookY + lookZ*lookZ);
    if (lookLen < 0.001f) return 0.0f;
    
    lookX /= lookLen;
    lookY /= lookLen;
    lookZ /= lookLen;
    
    // Dot product gives cosine of angle between camera look direction and sun direction
    float dot = toSunX * lookX + toSunY * lookY + toSunZ * lookZ;
    
    // Flare is strongest when looking directly at sun (dot = 1)
    // Start showing flare when dot > 0.85 (about 30 degrees from sun)
    if (dot < 0.85f) {
        return 0.0f;
    }
    
    // Map 0.85-1.0 to 0-1 intensity, with exponential falloff
    float normalizedAngle = (dot - 0.85f) / 0.15f;
    float flare = normalizedAngle * normalizedAngle * sunIntensity;
    
    return std::min(1.0f, flare);
}
