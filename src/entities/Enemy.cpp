#include "Enemy.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <ctime>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Enemy::Enemy()
    : x(0), y(100), z(-300),
      pitch(0), yaw(0), roll(0),
      velocityX(0), velocityY(0), velocityZ(0),
      speed(0.8f),
      state(EnemyState::FLY_STRAIGHT),
      stateTimer(0),
      stateDuration(3.0f),
      straightDuration(3.0f),
      bankDuration(2.5f),
      maxRoll(45.0f),
      turnSpeed(30.0f),
      boundingRadius(4.0f),
      alive(true),
      destroyed(false),
      destructionTimer(0),
      destructionDuration(2.0f),
      explosionScale(1.0f),
      aircraftModel(nullptr),
      useModel(false) {
    
    // Seed random number generator
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned int>(time(nullptr)));
        seeded = true;
    }
}

Enemy::Enemy(float startX, float startY, float startZ, float startYaw)
    : x(startX), y(startY), z(startZ),
      pitch(0), yaw(startYaw), roll(0),
      velocityX(0), velocityY(0), velocityZ(0),
      speed(0.8f),
      state(EnemyState::FLY_STRAIGHT),
      stateTimer(0),
      stateDuration(3.0f),
      straightDuration(3.0f),
      bankDuration(2.5f),
      maxRoll(45.0f),
      turnSpeed(30.0f),
      boundingRadius(4.0f),
      alive(true),
      destroyed(false),
      destructionTimer(0),
      destructionDuration(2.0f),
      explosionScale(1.0f),
      aircraftModel(nullptr),
      useModel(false) {
    
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned int>(time(nullptr)));
        seeded = true;
    }
}

Enemy::~Enemy() {
    if (aircraftModel != nullptr) {
        delete aircraftModel;
        aircraftModel = nullptr;
    }
}

bool Enemy::loadModel(const std::string& modelPath, float scale) {
    std::cout << "Enemy: Loading aircraft model from " << modelPath << std::endl;
    
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
        
        float maxSize = std::max({sizeX, sizeY, sizeZ});
        boundingRadius = maxSize / 2.0f;
        
        if (boundingRadius < 4.0f) {
            boundingRadius = 4.0f;
        }
        
        std::cout << "Enemy: Aircraft model loaded successfully!" << std::endl;
        std::cout << "Enemy: Bounding radius set to " << boundingRadius << std::endl;
        return true;
    } else {
        std::cerr << "Enemy: Failed to load aircraft model, will use primitives" << std::endl;
        delete aircraftModel;
        aircraftModel = nullptr;
        useModel = false;
        return false;
    }
}

void Enemy::update(float deltaTime) {
    if (!alive) {
        // Update destruction animation
        if (!destroyed) {
            destructionTimer += deltaTime;
            explosionScale = 1.0f + (destructionTimer / destructionDuration) * 3.0f;
            
            // Fall and rotate
            y -= 15.0f * deltaTime;
            pitch += 120.0f * deltaTime;
            roll += 180.0f * deltaTime;
            
            if (destructionTimer >= destructionDuration) {
                destroyed = true;
            }
        }
        return;
    }
    
    // Update FSM
    updateFSM(deltaTime);
    
    // Update movement
    updateMovement(deltaTime);
}

void Enemy::updateFSM(float deltaTime) {
    stateTimer += deltaTime;
    
    if (stateTimer >= stateDuration) {
        transitionState();
    }
    
    // Apply behavior based on current state
    switch (state) {
        case EnemyState::FLY_STRAIGHT:
            // Gradually level out roll
            if (roll > 0) {
                roll -= turnSpeed * deltaTime;
                if (roll < 0) roll = 0;
            } else if (roll < 0) {
                roll += turnSpeed * deltaTime;
                if (roll > 0) roll = 0;
            }
            break;
            
        case EnemyState::BANK_LEFT:
            // Bank left
            roll -= turnSpeed * deltaTime;
            if (roll < -maxRoll) roll = -maxRoll;
            
            // Turn based on roll
            yaw -= (roll / maxRoll) * 25.0f * deltaTime;
            break;
            
        case EnemyState::BANK_RIGHT:
            // Bank right
            roll += turnSpeed * deltaTime;
            if (roll > maxRoll) roll = maxRoll;
            
            // Turn based on roll
            yaw += (roll / maxRoll) * 25.0f * deltaTime;
            break;
            
        case EnemyState::DESTROYED:
            // Handled in main update
            break;
    }
    
    // Keep yaw in 0-360 range
    while (yaw >= 360.0f) yaw -= 360.0f;
    while (yaw < 0.0f) yaw += 360.0f;
}

void Enemy::transitionState() {
    stateTimer = 0;
    
    // Random state transition
    int nextState = rand() % 3;
    
    switch (nextState) {
        case 0:
            state = EnemyState::FLY_STRAIGHT;
            stateDuration = straightDuration + (rand() % 100) / 50.0f; // 3-5 seconds
            break;
        case 1:
            state = EnemyState::BANK_LEFT;
            stateDuration = bankDuration + (rand() % 100) / 50.0f; // 2.5-4.5 seconds
            break;
        case 2:
            state = EnemyState::BANK_RIGHT;
            stateDuration = bankDuration + (rand() % 100) / 50.0f; // 2.5-4.5 seconds
            break;
    }
}

void Enemy::updateMovement(float deltaTime) {
    // Convert angles to radians
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;
    
    // Calculate forward direction
    float forwardX = std::sin(radYaw) * std::cos(radPitch);
    float forwardY = -std::sin(radPitch);
    float forwardZ = std::cos(radYaw) * std::cos(radPitch);
    
    // Apply velocity
    velocityX = forwardX * speed;
    velocityY = forwardY * speed;
    velocityZ = forwardZ * speed;
    
    // Update position
    x += velocityX;
    y += velocityY;
    z += velocityZ;
    
    // Keep above minimum altitude
    if (y < 50.0f) {
        y = 50.0f;
        pitch = -10.0f; // Climb
    }
    
    // Keep below maximum altitude
    if (y > 200.0f) {
        y = 200.0f;
        pitch = 10.0f; // Descend
    }
}

void Enemy::render() const {
    glPushMatrix();
    
    // Apply transformations
    glTranslatef(x, y, z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    glRotatef(pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(roll, 0.0f, 0.0f, 1.0f);
    
    if (!alive) {
        // Render explosion effect
        glPushMatrix();
        glScalef(explosionScale, explosionScale, explosionScale);
        
        // Explosion sphere
        glColor4f(1.0f, 0.5f, 0.0f, 1.0f - (destructionTimer / destructionDuration));
        glutSolidSphere(3.0f, 12, 12);
        
        // Inner bright core
        glColor4f(1.0f, 1.0f, 0.0f, 1.0f - (destructionTimer / destructionDuration));
        glutSolidSphere(1.5f, 12, 12);
        
        glPopMatrix();
    } else {
        // Use 3D model if loaded, otherwise use primitives
        if (useModel && aircraftModel != nullptr && aircraftModel->isLoaded()) {
            glColor3f(0.8f, 0.8f, 0.8f);
            
            GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
            if (!lightingEnabled) glEnable(GL_LIGHTING);
            
            // Enemy aircraft orientation (same as player)
            glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            
            aircraftModel->render();
            
            if (!lightingEnabled) glDisable(GL_LIGHTING);
        } else {
            // Fallback: Draw enemy aircraft using primitives
            glPushMatrix();
            glScalef(1.2f, 1.2f, 1.2f);  // Slightly smaller than player
            
            // Fuselage (main body) - Red enemy color
            glColor3f(0.8f, 0.1f, 0.1f);  // Red
            glPushMatrix();
            glScalef(1.8f, 1.0f, 7.0f);
            glutSolidCube(1.0);
            glPopMatrix();
            
            // Cockpit
            glColor3f(0.2f, 0.2f, 0.2f);  // Dark glass
            glPushMatrix();
            glTranslatef(0.0f, 0.7f, 0.5f);
            glScalef(1.0f, 0.7f, 1.5f);
            glutSolidSphere(0.5, 10, 10);
            glPopMatrix();
            
            // Main wings
            glColor3f(0.7f, 0.1f, 0.1f);  // Darker red
            glPushMatrix();
            glScalef(10.0f, 0.25f, 2.5f);
            glutSolidCube(1.0);
            glPopMatrix();
            
            // Tail wings
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, -3.2f);
            glScalef(4.0f, 0.2f, 1.0f);
            glutSolidCube(1.0);
            glPopMatrix();
            
            // Vertical tail fin
            glColor3f(0.75f, 0.15f, 0.15f);
            glPushMatrix();
            glTranslatef(0.0f, 1.0f, -3.2f);
            glScalef(0.2f, 2.0f, 1.0f);
            glutSolidCube(1.0);
            glPopMatrix();
            
            // Engine exhaust
            glColor3f(1.0f, 0.3f, 0.0f);  // Orange glow
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, -3.8f);
            glutSolidSphere(0.4, 8, 8);
            glPopMatrix();
            
            glPopMatrix();  // End enemy scale
        }
    }
    
    glPopMatrix();
}

void Enemy::destroy() {
    if (alive) {
        alive = false;
        destructionTimer = 0;
        std::cout << "Enemy destroyed!" << std::endl;
    }
}

void Enemy::getForwardVector(float& outX, float& outY, float& outZ) const {
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;
    
    outX = std::sin(radYaw) * std::cos(radPitch);
    outY = -std::sin(radPitch);
    outZ = std::cos(radYaw) * std::cos(radPitch);
}
