#include "Camera.h"
#include "../entities/Player.h"
#include <cmath>
#include <algorithm>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
// On Windows, GLEW must be included before GL headers
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Camera::Camera() 
    : posX(0), posY(10), posZ(-20),
      lookX(0), lookY(10), lookZ(0),
      upX(0), upY(1), upZ(0),
      firstPerson(false),
      distance(18.0f),
      height(6.0f),
      smoothing(0.1f),
      targetPosX(0), targetPosY(10), targetPosZ(-20),
      orbitYaw(0.0f),
      orbitPitch(0.0f),
      mouseSensitivity(0.3f),
      lastMouseX(0),
      lastMouseY(0),
      mouseControlActive(false) {
}

void Camera::update(const Player* player, float deltaTime) {
    if (!player) return;
    
    float playerX = player->getX();
    float playerY = player->getY();
    float playerZ = player->getZ();
    float playerYaw = player->getYaw();
    float playerPitch = player->getPitch();
    
    float radYaw = playerYaw * M_PI / 180.0f;
    float radPitch = playerPitch * M_PI / 180.0f;
    
    if (firstPerson) {
        // First person: camera at player position, looking forward
        float offsetForward = 2.0f;
        
        targetPosX = playerX + std::sin(radYaw) * offsetForward;
        targetPosY = playerY;
        targetPosZ = playerZ + std::cos(radYaw) * offsetForward;
        
        float lookDist = 100.0f;
        lookX = playerX + std::sin(radYaw) * lookDist * std::cos(radPitch);
        lookY = playerY - std::sin(radPitch) * lookDist;
        lookZ = playerZ + std::cos(radYaw) * lookDist * std::cos(radPitch);
        
    } else {
        // Third person: camera behind and above player with mouse orbit support
        // Apply mouse orbit offset to the camera angle
        float totalYaw = playerYaw + orbitYaw;
        float radTotalYaw = totalYaw * M_PI / 180.0f;
        float radOrbitPitch = orbitPitch * M_PI / 180.0f;
        
        // Calculate camera position with orbit offset
        float adjustedHeight = height + std::sin(radOrbitPitch) * distance * 0.5f;
        float adjustedDistance = distance * std::cos(radOrbitPitch);
        
        // Position camera behind the player with orbit offset
        targetPosX = playerX - std::sin(radTotalYaw) * adjustedDistance;
        targetPosY = playerY + adjustedHeight;
        targetPosZ = playerZ - std::cos(radTotalYaw) * adjustedDistance;
        
        // Look at the player
        lookX = playerX;
        lookY = playerY;
        lookZ = playerZ;
    }
    
    // Smooth camera movement
    float lerpFactor = 1.0f - std::pow(1.0f - smoothing, deltaTime * 60.0f);
    lerpFactor = std::min(lerpFactor, 1.0f);
    
    posX += (targetPosX - posX) * lerpFactor;
    posY += (targetPosY - posY) * lerpFactor;
    posZ += (targetPosZ - posZ) * lerpFactor;
    
    // Calculate up vector based on player roll
    float playerRoll = player->getRoll();
    float radRoll = playerRoll * M_PI / 180.0f;
    
    if (firstPerson) {
        upX = std::sin(radRoll) * 0.3f;
        upY = std::cos(radRoll);
        upZ = 0;
    } else {
        // Keep camera mostly level in third person
        upX = std::sin(radRoll) * 0.1f;
        upY = 1.0f;
        upZ = 0;
    }
}

void Camera::apply() {
    gluLookAt(posX, posY, posZ,
              lookX, lookY, lookZ,
              upX, upY, upZ);
}

void Camera::toggle() {
    firstPerson = !firstPerson;
    // Reset orbit when toggling camera mode
    resetOrbit();
}

void Camera::setFirstPerson(bool fp) {
    firstPerson = fp;
}

bool Camera::isFirstPerson() const {
    return firstPerson;
}

void Camera::setDistance(float dist) {
    distance = dist;
}

void Camera::setHeight(float h) {
    height = h;
}

void Camera::handleMouseMotion(int x, int y) {
    if (!mouseControlActive || firstPerson) return;
    
    int deltaX = x - lastMouseX;
    int deltaY = y - lastMouseY;
    
    lastMouseX = x;
    lastMouseY = y;
    
    // Update orbit angles based on mouse movement
    orbitYaw += deltaX * mouseSensitivity;
    orbitPitch += deltaY * mouseSensitivity;
    
    // Clamp orbit pitch to prevent flipping
    if (orbitPitch > 60.0f) orbitPitch = 60.0f;
    if (orbitPitch < -30.0f) orbitPitch = -30.0f;
    
    // Keep orbit yaw in reasonable range
    while (orbitYaw > 180.0f) orbitYaw -= 360.0f;
    while (orbitYaw < -180.0f) orbitYaw += 360.0f;
}

void Camera::handleMouseButton(int button, bool pressed, int x, int y) {
    // Left mouse button (button 0) activates orbit control
    if (button == 0) {
        mouseControlActive = pressed;
        if (pressed) {
            lastMouseX = x;
            lastMouseY = y;
        }
    }
}

void Camera::resetOrbit() {
    orbitYaw = 0.0f;
    orbitPitch = 0.0f;
}
