#include "Player.h"
#include <cmath>
#include <iostream>

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

Player::Player() 
    : x(0), y(15), z(0),
      pitch(0), yaw(0), roll(0),
      velocityX(0), velocityY(0), velocityZ(0),
      speed(0.8f),
      maxSpeed(2.0f),
      minSpeed(0.3f),
      acceleration(0.5f),
      pitchSpeed(80.0f),
      yawSpeed(60.0f),
      rollSpeed(120.0f),
      gravity(9.8f),
      lift(0),
      boundingRadius(3.0f),
      barrelRolling(false),
      barrelRollAngle(0),
      barrelRollSpeed(360.0f),
      alive(true),
      aircraftModel(nullptr),
      useModel(false) {
}

Player::Player(float startX, float startY, float startZ)
    : x(startX), y(startY), z(startZ),
      pitch(0), yaw(0), roll(0),
      velocityX(0), velocityY(0), velocityZ(0),
      speed(0.8f),
      maxSpeed(2.0f),
      minSpeed(0.3f),
      acceleration(0.5f),
      pitchSpeed(80.0f),
      yawSpeed(60.0f),
      rollSpeed(120.0f),
      gravity(9.8f),
      lift(0),
      boundingRadius(3.0f),
      barrelRolling(false),
      barrelRollAngle(0),
      barrelRollSpeed(360.0f),
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
        std::cout << "Player: Aircraft model loaded successfully!" << std::endl;
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
    yaw += rollFactor * 30.0f * deltaTime;
    
    // Natural roll recovery (roll back towards level)
    if (!keys['a'] && !keys['A'] && !keys['d'] && !keys['D']) {
        roll *= (1.0f - deltaTime * 2.0f);
    }
    
    // Natural pitch recovery
    if (!keys['w'] && !keys['W'] && !keys['s'] && !keys['S']) {
        pitch *= (1.0f - deltaTime * 1.5f);
    }
    
    // Clamp pitch to prevent loops (optional, can remove for arcade feel)
    if (pitch > 60.0f) pitch = 60.0f;
    if (pitch < -60.0f) pitch = -60.0f;
    
    // Clamp roll
    if (roll > 70.0f) roll = 70.0f;
    if (roll < -70.0f) roll = -70.0f;
    
    // Keep yaw in 0-360 range
    while (yaw >= 360.0f) yaw -= 360.0f;
    while (yaw < 0.0f) yaw += 360.0f;
}

void Player::applyInput(const bool* keys, float deltaTime) {
    if (!alive) return;
    
    // Pitch control (W/S)
    if (keys['w'] || keys['W']) {
        pitch += pitchSpeed * deltaTime;
    }
    if (keys['s'] || keys['S']) {
        pitch -= pitchSpeed * deltaTime;
    }
    
    // Roll control (A/D)
    if (keys['a'] || keys['A']) {
        roll -= rollSpeed * deltaTime;
    }
    if (keys['d'] || keys['D']) {
        roll += rollSpeed * deltaTime;
    }
    
    // Yaw control (Q/E)
    if (keys['q'] || keys['Q']) {
        yaw -= yawSpeed * deltaTime;
    }
    if (keys['e'] || keys['E']) {
        yaw += yawSpeed * deltaTime;
    }
    
    // Speed control (Shift to boost, Ctrl to slow)
    // Using numeric keys as alternative: 1 = slow, 2 = fast
    if (keys['1']) {
        speed -= acceleration * deltaTime;
        if (speed < minSpeed) speed = minSpeed;
    }
    if (keys['2']) {
        speed += acceleration * deltaTime;
        if (speed > maxSpeed) speed = maxSpeed;
    }
    
    // Barrel roll (Space or B)
    if ((keys[' '] || keys['b'] || keys['B']) && !barrelRolling) {
        // Direction based on current roll
        int direction = (roll >= 0) ? 1 : -1;
        if (keys['a'] || keys['A']) direction = -1;
        if (keys['d'] || keys['D']) direction = 1;
        startBarrelRoll(direction);
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
        // Set material properties for the model
        glColor3f(0.7f, 0.7f, 0.7f);  // Light gray base color
        
        // Enable lighting for proper model rendering
        GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
        if (!lightingEnabled) glEnable(GL_LIGHTING);
        
        // Render the 3D model
        aircraftModel->render();
        
        if (!lightingEnabled) glDisable(GL_LIGHTING);
    } else {
        // Fallback: Draw aircraft using primitives
        // Fuselage (main body)
        glColor3f(0.2f, 0.3f, 0.8f);  // Navy blue
        glPushMatrix();
        glScalef(1.0f, 0.6f, 4.0f);
        glutSolidCube(1.0);
        glPopMatrix();
        
        // Cockpit
        glColor3f(0.3f, 0.7f, 0.9f);  // Light blue (glass)
        glPushMatrix();
        glTranslatef(0.0f, 0.4f, 0.5f);
        glScalef(0.6f, 0.4f, 1.0f);
        glutSolidSphere(0.5, 10, 10);
        glPopMatrix();
        
        // Main wings
        glColor3f(0.3f, 0.4f, 0.7f);  // Lighter blue
        glPushMatrix();
        glScalef(6.0f, 0.15f, 1.5f);
        glutSolidCube(1.0);
        glPopMatrix();
        
        // Tail wings (horizontal stabilizers)
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -1.8f);
        glScalef(2.5f, 0.1f, 0.6f);
        glutSolidCube(1.0);
        glPopMatrix();
        
        // Vertical tail fin
        glColor3f(0.25f, 0.35f, 0.75f);
        glPushMatrix();
        glTranslatef(0.0f, 0.6f, -1.8f);
        glScalef(0.1f, 1.2f, 0.6f);
        glutSolidCube(1.0);
        glPopMatrix();
        
        // Engine exhaust (small sphere at back)
        glColor3f(0.9f, 0.5f, 0.1f);  // Orange glow
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -2.2f);
        glutSolidSphere(0.25, 8, 8);
        glPopMatrix();
        
        // Wing tips
        glColor3f(1.0f, 0.0f, 0.0f);  // Red
        glPushMatrix();
        glTranslatef(3.0f, 0.0f, 0.0f);
        glutSolidSphere(0.15, 6, 6);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-3.0f, 0.0f, 0.0f);
        glutSolidSphere(0.15, 6, 6);
        glPopMatrix();
    }
    
    glPopMatrix();
}

void Player::reset(float startX, float startY, float startZ) {
    x = startX;
    y = startY;
    z = startZ;
    pitch = 0;
    yaw = 0;
    roll = 0;
    velocityX = velocityY = velocityZ = 0;
    speed = 0.8f;
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
