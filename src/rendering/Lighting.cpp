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
    : dayTime(17.0f),        // Start at 5pm (golden hour/sunset)
      daySpeed(0.02f),       // Slow speed - takes ~30 seconds for noticeable change
      nightMode(false),      // Always daytime (sunset)
      sunIntensity(0.7f),    // Start at sunset intensity
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
    // Perpetual sunset cycle - oscillates between golden hour and deep sunset
    // Range: 16.5 (4:30pm golden hour) to 18.5 (6:30pm sunset)
    // Never goes to night!
    
    dayTime += daySpeed * deltaTime;
    
    // Oscillate between 16.5 and 18.5 (2 hour range of sunset)
    // Use sine wave for smooth back-and-forth
    float cycleTime = dayTime * 0.1f;  // Slow cycle
    float sunsetPhase = std::sin(cycleTime);  // -1 to 1
    
    // Map to sunset time range: 16.5 to 18.5
    float effectiveTime = 17.5f + sunsetPhase;  // 16.5 to 18.5
    
    // Calculate sun intensity - always in the "sunset" range (0.5 to 0.85)
    // This keeps it bright enough to see but with warm sunset colors
    float timeFromNoon = std::abs(effectiveTime - 12.0f);  // 4.5 to 6.5 hours from noon
    sunIntensity = 0.85f - (timeFromNoon - 4.5f) * 0.175f;  // 0.85 at 4:30pm, 0.5 at 6:30pm
    sunIntensity = std::max(0.5f, std::min(0.85f, sunIntensity));
    
    // Never go to night mode
    nightMode = false;
    
    // Calculate sun position - low on horizon for sunset effect
    // Sun angle based on effective time (low angle = long shadows, warm light)
    float sunAngle = (effectiveTime - 6.0f) / 12.0f * M_PI;  // Angle in sky
    
    // Sun position - keep it low on the horizon
    sunY = std::sin(sunAngle) * sunDistance * 0.4f;  // Lower sun for sunset
    sunX = std::cos(sunAngle) * sunDistance * 0.6f;  // Moving across horizon
    sunZ = sunDistance * 0.4f;
    
    // Minimum sun height (never fully set)
    if (sunY < 40.0f) sunY = 40.0f;
    
    // Update flash decay
    if (flashIntensity > 0) {
        flashIntensity -= flashDecay * deltaTime;
        if (flashIntensity < 0) flashIntensity = 0;
    }
    
    // Warm sunset ambient - always warm, varies slightly
    float warmth = 1.0f - sunIntensity;  // More warm when sun is lower
    
    // Sunset ambient colors - warm oranges and golds
    ambientR = 0.35f + warmth * 0.15f;   // 0.35 to 0.50
    ambientG = 0.28f + warmth * 0.05f;   // 0.28 to 0.33
    ambientB = 0.22f - warmth * 0.05f;   // 0.22 to 0.17
    
    // Add flash effect to ambient
    ambientR = std::min(1.0f, ambientR + flashIntensity);
    ambientG = std::min(1.0f, ambientG + flashIntensity * 0.8f);
    ambientB = std::min(1.0f, ambientB + flashIntensity * 0.4f);
}

void Lighting::apply() {
    // Global ambient light - warm sunset tones
    GLfloat globalAmbient[] = {ambientR, ambientG, ambientB, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    
    // Sun light (GL_LIGHT0) - warm sunset colors
    GLfloat sunPos[] = {sunX, sunY, sunZ, 0.0f}; // Directional (w=0)
    
    // Sunset sun color - warm orange/gold tones
    // Lower sun = more orange/red, higher = more golden yellow
    float warmth = 1.0f - sunIntensity;  // More warm when intensity is lower
    
    GLfloat sunDiffuse[] = {
        sunIntensity * (1.0f + warmth * 0.4f),     // Strong red/orange
        sunIntensity * (0.75f - warmth * 0.15f),   // Less green for warm tone
        sunIntensity * (0.4f - warmth * 0.2f),     // Minimal blue
        1.0f
    };
    GLfloat sunAmbient[] = {
        sunIntensity * 0.25f, 
        sunIntensity * 0.18f, 
        sunIntensity * 0.1f, 
        1.0f
    };
    GLfloat sunSpecular[] = {
        sunIntensity * 1.1f, 
        sunIntensity * 0.9f, 
        sunIntensity * 0.5f, 
        1.0f
    };
    
    glLightfv(GL_LIGHT0, GL_POSITION, sunPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpecular);
    
    // Fill light (GL_LIGHT1) - soft blue/purple from opposite side for sunset atmosphere
    glEnable(GL_LIGHT1);
    
    GLfloat fillPos[] = {-sunX * 0.5f, sunY * 0.3f, -sunZ * 0.5f, 0.0f};
    GLfloat fillDiffuse[] = {
        0.15f * sunIntensity,   // Slight cool tint
        0.12f * sunIntensity,
        0.2f * sunIntensity,    // More blue for sky reflection
        1.0f
    };
    GLfloat fillAmbient[] = {0.05f, 0.05f, 0.08f, 1.0f};
    
    glLightfv(GL_LIGHT1, GL_POSITION, fillPos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, fillDiffuse);
    glLightfv(GL_LIGHT1, GL_AMBIENT, fillAmbient);
    glLightfv(GL_LIGHT1, GL_SPECULAR, fillDiffuse);
}

void Lighting::toggleDayNight() {
    // In sunset-only mode, this just shifts the sunset phase
    // Cycle between early golden hour and deeper sunset
    if (sunIntensity > 0.7f) {
        dayTime += 5.0f;  // Move toward deeper sunset
    } else {
        dayTime -= 5.0f;  // Move toward golden hour
    }
}

void Lighting::setNightMode(bool night) {
    // Ignore night mode - always stay in sunset
    nightMode = false;
    
    if (night) {
        // If "night" requested, go to deeper sunset
        dayTime = 18.0f;
    } else {
        // If "day" requested, go to golden hour
        dayTime = 16.5f;
    }
}

bool Lighting::isNightMode() const {
    return nightMode;  // Always false in sunset mode
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
