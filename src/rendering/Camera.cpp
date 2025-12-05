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
      targetPosX(0), targetPosY(10), targetPosZ(-20) {
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
        // Third person: camera behind and above player
        // Position camera behind the player
        targetPosX = playerX - std::sin(radYaw) * distance;
        targetPosY = playerY + height;  // Above player
        targetPosZ = playerZ - std::cos(radYaw) * distance;
        
        // Look at a point ahead of and slightly below the player
        // This creates a view where you can see both the plane and the ground below
        float lookAhead = 20.0f;
        float lookDown = 5.0f;  // Look slightly below player level
        lookX = playerX + std::sin(radYaw) * lookAhead;
        lookY = playerY - lookDown;  // Look slightly downward
        lookZ = playerZ + std::cos(radYaw) * lookAhead;
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
