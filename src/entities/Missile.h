#ifndef MISSILE_H
#define MISSILE_H

#include "../rendering/Model.h"
#include <vector>

/**
 * @struct ParticleTrail
 * @brief Particle for missile trail effect
 */
struct ParticleTrail {
    float x, y, z;
    float life;  // 0.0 to 1.0
    float size;
    
    ParticleTrail(float px, float py, float pz, float psize = 1.0f)
        : x(px), y(py), z(pz), life(1.0f), size(psize) {}
};

/**
 * @class Missile
 * @brief Projectile fired by player or enemies
 * 
 * Features:
 * - Travels in straight line from launch point
 * - Animated trail effect
 * - Collision detection
 * - Timed lifetime
 */
class Missile {
private:
    // Position
    float x, y, z;
    
    // Direction (normalized)
    float dirX, dirY, dirZ;
    
    // Velocity
    float speed;
    
    // State
    bool active;
    bool playerOwned;  // True if fired by player, false if enemy
    
    // Lifetime
    float lifetime;
    float maxLifetime;
    
    // Collision
    float boundingRadius;
    
    // Trail particles
    std::vector<ParticleTrail> trail;
    float trailSpawnTimer;
    float trailSpawnInterval;
    int maxTrailParticles;
    
    // 3D Model
    Model* missileModel;
    bool useModel;
    
    // Visual
    float rotationAngle;  // For spin effect
    
public:
    Missile();
    Missile(float startX, float startY, float startZ,
            float forwardX, float forwardY, float forwardZ,
            bool fromPlayer = true);
    ~Missile();
    
    /**
     * Load 3D model for the missile
     */
    bool loadModel(const std::string& modelPath, float scale = 1.0f);
    
    /**
     * Update missile position and trail
     */
    void update(float deltaTime);
    
    /**
     * Render missile and trail
     */
    void render() const;
    
    /**
     * Check if missile is active
     */
    bool isActive() const { return active; }
    
    /**
     * Deactivate missile (after collision or timeout)
     */
    void deactivate() { active = false; }
    
    /**
     * Check if missile is owned by player
     */
    bool isPlayerOwned() const { return playerOwned; }
    
    /**
     * Get position
     */
    void getPosition(float& outX, float& outY, float& outZ) const {
        outX = x;
        outY = y;
        outZ = z;
    }
    
    /**
     * Get bounding radius for collision
     */
    float getBoundingRadius() const { return boundingRadius; }
    
    /**
     * Set speed
     */
    void setSpeed(float newSpeed) { speed = newSpeed; }
    
    /**
     * Set owner (0 = player1, 1 = player2, etc.)
     */
    void setOwner(int owner) { ownerID = owner; }
    
    /**
     * Get owner ID
     */
    int getOwner() const { return ownerID; }
    
private:
    int ownerID;  // For multiplayer tracking
    
    /**
     * Spawn a new trail particle
     */
    void spawnTrailParticle();
    
    /**
     * Update trail particles
     */
    void updateTrail(float deltaTime);
    
    /**
     * Render trail effect
     */
    void renderTrail() const;
};

#endif // MISSILE_H
