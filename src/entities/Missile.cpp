#include "Missile.h"
#include <cmath>
#include <iostream>
#include <algorithm>

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

Missile::Missile()
    : x(0), y(0), z(0),
      dirX(0), dirY(0), dirZ(1),
      speed(2.5f),
      active(false),
      playerOwned(true),
      lifetime(0),
      maxLifetime(10.0f),
      boundingRadius(0.8f),
      trailSpawnTimer(0),
      trailSpawnInterval(0.05f),
      maxTrailParticles(30),
      missileModel(nullptr),
      useModel(false),
      rotationAngle(0) {
}

Missile::Missile(float startX, float startY, float startZ,
                 float forwardX, float forwardY, float forwardZ,
                 bool fromPlayer)
    : x(startX), y(startY), z(startZ),
      dirX(forwardX), dirY(forwardY), dirZ(forwardZ),
      speed(2.5f),
      active(true),
      playerOwned(fromPlayer),
      lifetime(0),
      maxLifetime(10.0f),
      boundingRadius(0.8f),
      trailSpawnTimer(0),
      trailSpawnInterval(0.05f),
      maxTrailParticles(30),
      missileModel(nullptr),
      useModel(false),
      rotationAngle(0) {
    
    // Normalize direction vector
    float length = std::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
    if (length > 0.001f) {
        dirX /= length;
        dirY /= length;
        dirZ /= length;
    }
}

Missile::~Missile() {
    if (missileModel != nullptr) {
        delete missileModel;
        missileModel = nullptr;
    }
}

bool Missile::loadModel(const std::string& modelPath, float scale) {
    std::cout << "Missile: Loading model from " << modelPath << std::endl;
    
    if (missileModel != nullptr) {
        delete missileModel;
    }
    
    missileModel = new Model();
    if (missileModel->load(modelPath)) {
        missileModel->setScale(scale);
        useModel = true;
        
        std::cout << "Missile: Model loaded successfully!" << std::endl;
        return true;
    } else {
        std::cerr << "Missile: Failed to load model, will use primitives" << std::endl;
        delete missileModel;
        missileModel = nullptr;
        useModel = false;
        return false;
    }
}

void Missile::update(float deltaTime) {
    if (!active) return;
    
    // Update lifetime
    lifetime += deltaTime;
    if (lifetime >= maxLifetime) {
        active = false;
        return;
    }
    
    // Update position
    x += dirX * speed;
    y += dirY * speed;
    z += dirZ * speed;
    
    // Update rotation for visual effect
    rotationAngle += 360.0f * deltaTime;
    if (rotationAngle >= 360.0f) rotationAngle -= 360.0f;
    
    // Update trail
    updateTrail(deltaTime);
    
    // Spawn trail particles
    trailSpawnTimer += deltaTime;
    if (trailSpawnTimer >= trailSpawnInterval) {
        spawnTrailParticle();
        trailSpawnTimer = 0;
    }
}

void Missile::spawnTrailParticle() {
    if (trail.size() >= static_cast<size_t>(maxTrailParticles)) {
        trail.erase(trail.begin());
    }
    
    trail.push_back(ParticleTrail(x, y, z, 1.2f));
}

void Missile::updateTrail(float deltaTime) {
    // Update existing particles
    for (auto& particle : trail) {
        particle.life -= deltaTime * 1.5f;  // Fade rate
        particle.size *= 0.98f;  // Shrink over time
    }
    
    // Remove dead particles
    trail.erase(
        std::remove_if(trail.begin(), trail.end(),
            [](const ParticleTrail& p) { return p.life <= 0.0f; }),
        trail.end()
    );
}

void Missile::render() const {
    if (!active) return;
    
    // Render trail first (behind missile)
    renderTrail();
    
    glPushMatrix();
    
    // Position
    glTranslatef(x, y, z);
    
    // Orient missile in direction of travel
    float yaw = std::atan2(dirX, dirZ) * 180.0f / M_PI;
    float pitch = std::asin(-dirY) * 180.0f / M_PI;
    
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    glRotatef(pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationAngle, 0.0f, 0.0f, 1.0f);  // Spin effect
    
    if (useModel && missileModel != nullptr && missileModel->isLoaded()) {
        glColor3f(0.8f, 0.8f, 0.8f);
        
        GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
        if (!lightingEnabled) glEnable(GL_LIGHTING);
        
        missileModel->render();
        
        if (!lightingEnabled) glDisable(GL_LIGHTING);
    } else {
        // Fallback: Draw missile using primitives
        glDisable(GL_LIGHTING);
        
        // Missile body (cylinder)
        if (playerOwned) {
            glColor3f(0.3f, 0.3f, 0.8f);  // Blue for player
        } else {
            glColor3f(0.8f, 0.1f, 0.1f);  // Red for enemy
        }
        
        glPushMatrix();
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        GLUquadric* quad = gluNewQuadric();
        gluCylinder(quad, 0.3, 0.3, 2.5, 12, 1);
        gluDeleteQuadric(quad);
        glPopMatrix();
        
        // Nose cone
        glPushMatrix();
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidCone(0.3, 0.8, 12, 1);
        glPopMatrix();
        
        // Tail fins
        glColor3f(0.5f, 0.5f, 0.5f);
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glRotatef(i * 90.0f, 0.0f, 0.0f, 1.0f);
            glTranslatef(0.3f, 0.0f, 2.0f);
            glScalef(0.5f, 0.05f, 0.6f);
            glutSolidCube(1.0);
            glPopMatrix();
        }
        
        glEnable(GL_LIGHTING);
    }
    
    glPopMatrix();
}

void Missile::renderTrail() const {
    if (trail.empty()) return;
    
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    
    for (const auto& particle : trail) {
        glPushMatrix();
        glTranslatef(particle.x, particle.y, particle.z);
        
        // Color based on owner and life
        float alpha = particle.life * 0.6f;
        if (playerOwned) {
            glColor4f(0.8f, 0.8f, 1.0f, alpha);  // Blue-white trail for player
        } else {
            glColor4f(1.0f, 0.5f, 0.2f, alpha);  // Orange trail for enemy
        }
        
        glutSolidSphere(particle.size, 8, 8);
        
        // Inner bright core
        glColor4f(1.0f, 1.0f, 0.8f, alpha * 0.5f);
        glutSolidSphere(particle.size * 0.5f, 6, 6);
        
        glPopMatrix();
    }
    
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}
