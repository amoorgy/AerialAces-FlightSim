#include "Player.h"
#include <cmath>
#include <iostream>
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

Player::Player() 
    : x(0), y(80), z(0),  // Start high up
      pitch(0), yaw(0), roll(0),
      velocityX(0), velocityY(0), velocityZ(0),
      speed(0.3f),           // Very slow starting speed for better control
      maxSpeed(1.2f),        // Much slower maximum speed
      minSpeed(0.15f),       // Very slow minimum speed
      acceleration(0.4f),    // Smooth throttle response
      pitchSpeed(55.0f),     // Smooth pitch control
      yawSpeed(65.0f),       // Smooth yaw control
      rollSpeed(95.0f),      // Smooth roll control
      gravity(9.8f),
      lift(0),
      boundingRadius(3.0f),
      barrelRolling(false),
      barrelRollAngle(0),
      barrelRollSpeed(360.0f),
      spacePressed(false),
      alive(true),
      aircraftModel(nullptr),
      useModel(false) {
}

Player::Player(float startX, float startY, float startZ)
    : x(startX), y(startY), z(startZ),
      pitch(0), yaw(0), roll(0),
      velocityX(0), velocityY(0), velocityZ(0),
      speed(0.5f),           // Very slow starting speed for better control
      maxSpeed(1.2f),        // Much slower maximum speed
      minSpeed(0.15f),       // Very slow minimum speed
      acceleration(0.4f),    // Smooth throttle response
      pitchSpeed(55.0f),     // Smooth pitch control
      yawSpeed(65.0f),       // Smooth yaw control
      rollSpeed(95.0f),      // Smooth roll control
      gravity(9.8f),
      lift(0),
      boundingRadius(3.0f),
      barrelRolling(false),
      barrelRollAngle(0),
      barrelRollSpeed(360.0f),
      spacePressed(false),
      alive(true),
      aircraftModel(nullptr),
      useModel(false) {
}

Player::~Player() {
    if (aircraftModel != nullptr) {
        delete aircraftModel;
        aircraftModel = nullptr;
    }
}

bool Player::loadModel(const std::string& modelPath, float scale) {
    std::cout << "Player: Loading aircraft model from " << modelPath << std::endl;
    
    if (aircraftModel != nullptr) {
        delete aircraftModel;
    }
    
    aircraftModel = new Model();
    if (aircraftModel->load(modelPath)) {
        aircraftModel->setScale(scale);
        useModel = true;
        
        // Update bounding radius based on model size
        float minX, maxX, minY, maxY, minZ, maxZ;
        aircraftModel->getBounds(minX, maxX, minY, maxY, minZ, maxZ);
        float sizeX = maxX - minX;
        float sizeZ = maxZ - minZ;
        float sizeY = maxY - minY;
        
        // Use the largest dimension to ensure full coverage
        float maxSize = std::max({sizeX, sizeY, sizeZ});
        boundingRadius = maxSize / 2.0f;
        
        // Ensure minimum radius for collision detection reliability
        if (boundingRadius < 3.0f) {
            boundingRadius = 3.0f;
        }
        
        std::cout << "Player: Aircraft model loaded successfully!" << std::endl;
        std::cout << "Player: Model size: " << sizeX << " x " << sizeY << " x " << sizeZ << std::endl;
        std::cout << "Player: Bounding radius set to " << boundingRadius << std::endl;
        return true;
    } else {
        std::cerr << "Player: Failed to load aircraft model, will use primitives" << std::endl;
        delete aircraftModel;
        aircraftModel = nullptr;
        useModel = false;
        return false;
    }
}

void Player::update(float deltaTime, const bool* keys) {
    if (!alive) return;
    
    // Apply input
    applyInput(keys, deltaTime);
    
    // Update barrel roll animation
    if (barrelRolling) {
        barrelRollAngle += barrelRollSpeed * deltaTime;
        if (barrelRollAngle >= 360.0f) {
            barrelRolling = false;
            barrelRollAngle = 0;
        }
    }
    
    // Convert angles to radians
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;
    
    // Calculate forward direction based on yaw and pitch
    float forwardX = std::sin(radYaw) * std::cos(radPitch);
    float forwardY = -std::sin(radPitch);
    float forwardZ = std::cos(radYaw) * std::cos(radPitch);
    
    // Apply velocity based on speed and direction
    velocityX = forwardX * speed;
    velocityY = forwardY * speed;
    velocityZ = forwardZ * speed;
    
    // Update position
    x += velocityX;
    y += velocityY;
    z += velocityZ;
    
    // Apply roll-based turn (more realistic banking)
    float rollFactor = std::sin(roll * M_PI / 180.0f);
    yaw += rollFactor * 25.0f * deltaTime;
    
    // Natural roll recovery (roll back towards level)
    if (!keys['a'] && !keys['A'] && !keys['d'] && !keys['D']) {
        roll *= (1.0f - deltaTime * 2.0f);
    }
    
    // Natural pitch recovery
    if (!keys['w'] && !keys['W'] && !keys['s'] && !keys['S']) {
        pitch *= (1.0f - deltaTime * 1.5f);
    }
    
    // Clamp pitch to prevent extreme angles (increased for more dynamic flight)
    if (pitch > 60.0f) pitch = 60.0f;
    if (pitch < -60.0f) pitch = -60.0f;
    
    // Clamp roll (increased for more dramatic banking)
    if (roll > 75.0f) roll = 75.0f;
    if (roll < -75.0f) roll = -75.0f;
    
    // Keep yaw in 0-360 range
    while (yaw >= 360.0f) yaw -= 360.0f;
    while (yaw < 0.0f) yaw += 360.0f;
    
    // Minimum altitude enforcement (ground is at Y=0)
    if (y < 10.0f) {
        y = 10.0f;
        if (pitch > 0) pitch = 0;  // Force level or climb
    }
}

void Player::applyInput(const bool* keys, float deltaTime) {
    if (!alive) return;
    
    // Pitch control (W/S) - INVERTED Flight sim style: W = dive (nose down), S = climb (nose up)
    if (keys['w'] || keys['W']) {
        pitch += pitchSpeed * deltaTime;  // W = nose down (dive)
    }
    if (keys['s'] || keys['S']) {
        pitch -= pitchSpeed * deltaTime;  // S = nose up (climb)
    }
    
    // Roll control (A/D) - Normal: A = roll left, D = roll right
    if (keys['a'] || keys['A']) {
        roll -= rollSpeed * deltaTime;    // A = roll left
    }
    if (keys['d'] || keys['D']) {
        roll += rollSpeed * deltaTime;    // D = roll right
    }
    
    // Yaw control (Q/E) - Normal: Q = turn left, E = turn right
    if (keys['q'] || keys['Q']) {
        yaw -= yawSpeed * deltaTime;      // Q = turn left
    }
    if (keys['e'] || keys['E']) {
        yaw += yawSpeed * deltaTime;      // E = turn right
    }
    
    // Speed control (1/2)
    if (keys['1']) {
        speed -= acceleration * deltaTime;
        if (speed < minSpeed) speed = minSpeed;
    }
    if (keys['2']) {
        speed += acceleration * deltaTime;
        if (speed > maxSpeed) speed = maxSpeed;
    }
    
    // Barrel roll (Space or B) - only trigger on key press, not hold
    if (keys[' '] || keys['b'] || keys['B']) {
        if (!spacePressed && !barrelRolling) {
            int direction = (roll >= 0) ? 1 : -1;
            if (keys['a'] || keys['A']) direction = -1;
            if (keys['d'] || keys['D']) direction = 1;
            startBarrelRoll(direction);
        }
        spacePressed = true;
    } else {
        spacePressed = false;
    }
}

void Player::render() const {
    glPushMatrix();
    
    // Apply transformations
    glTranslatef(x, y, z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);      // Yaw around Y axis
    glRotatef(pitch, 1.0f, 0.0f, 0.0f);    // Pitch around X axis
    glRotatef(roll, 0.0f, 0.0f, 1.0f);     // Roll around Z axis
    
    // Apply barrel roll animation on top
    if (barrelRolling) {
        glRotatef(barrelRollAngle, 0.0f, 0.0f, 1.0f);
    }
    
    // Use 3D model if loaded, otherwise use primitives
    if (useModel && aircraftModel != nullptr && aircraftModel->isLoaded()) {
        glColor3f(0.8f, 0.8f, 0.8f);
        
        GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
        if (!lightingEnabled) glEnable(GL_LIGHTING);
        
        // Correct model orientation for Japanese WWII plane
        // Camera is behind the plane, plane nose should point FORWARD (away from camera)
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);   // Rotate to face away from camera
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);   // Stand model upright
        
        aircraftModel->render();
        
        if (!lightingEnabled) glDisable(GL_LIGHTING);
    } else {
        // Fallback: Draw aircraft using primitives - SCALED UP for better visibility
        glPushMatrix();
        glScalef(1.5f, 1.5f, 1.5f);  // Scale entire plane up by 50%
        
        // Fuselage (main body)
        glColor3f(0.2f, 0.3f, 0.8f);  // Navy blue
        glPushMatrix();
        glScalef(2.0f, 1.2f, 8.0f);
        glutSolidCube(1.0);
        glPopMatrix();
        
        // Cockpit
        glColor3f(0.3f, 0.7f, 0.9f);  // Light blue (glass)
        glPushMatrix();
        glTranslatef(0.0f, 0.8f, 1.0f);
        glScalef(1.2f, 0.8f, 2.0f);
        glutSolidSphere(0.5, 10, 10);
        glPopMatrix();
        
        // Main wings
        glColor3f(0.3f, 0.4f, 0.7f);  // Lighter blue
        glPushMatrix();
        glScalef(12.0f, 0.3f, 3.0f);
        glutSolidCube(1.0);
        glPopMatrix();
        
        // Tail wings (horizontal stabilizers)
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -3.6f);
        glScalef(5.0f, 0.2f, 1.2f);
        glutSolidCube(1.0);
        glPopMatrix();
        
        // Vertical tail fin
        glColor3f(0.25f, 0.35f, 0.75f);
        glPushMatrix();
        glTranslatef(0.0f, 1.2f, -3.6f);
        glScalef(0.2f, 2.4f, 1.2f);
        glutSolidCube(1.0);
        glPopMatrix();
        
        // Engine exhaust
        glColor3f(1.0f, 0.5f, 0.1f);  // Orange glow
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -4.4f);
        glutSolidSphere(0.5, 8, 8);
        glPopMatrix();
        
        // Wing tips
        glColor3f(1.0f, 0.0f, 0.0f);  // Red
        glPushMatrix();
        glTranslatef(6.0f, 0.0f, 0.0f);
        glutSolidSphere(0.3, 6, 6);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-6.0f, 0.0f, 0.0f);
        glutSolidSphere(0.3, 6, 6);
        glPopMatrix();
        
        glPopMatrix();  // End plane scale
    }
    
    glPopMatrix();
}

void Player::reset(float startX, float startY, float startZ, float startYaw) {
    x = startX;
    y = startY;
    z = startZ;
    pitch = 0;
    yaw = startYaw;
    roll = 0;
    velocityX = velocityY = velocityZ = 0;
    speed = 0.3f;  // Match new default speed
    alive = true;
    barrelRolling = false;
    barrelRollAngle = 0;
}

void Player::startBarrelRoll(int direction) {
    if (!barrelRolling && alive) {
        barrelRolling = true;
        barrelRollAngle = 0;
        barrelRollSpeed = 360.0f * direction;
    }
}

bool Player::isBarrelRolling() const {
    return barrelRolling;
}

void Player::kill() {
    alive = false;
    speed = 0;
}

bool Player::isAlive() const {
    return alive;
}

void Player::getForwardVector(float& outX, float& outY, float& outZ) const {
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;
    
    outX = std::sin(radYaw) * std::cos(radPitch);
    outY = -std::sin(radPitch);
    outZ = std::cos(radYaw) * std::cos(radPitch);
}
