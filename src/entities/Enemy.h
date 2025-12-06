#ifndef ENEMY_H
#define ENEMY_H

#include "../rendering/Model.h"

/**
 * @enum EnemyState
 * @brief FSM states for enemy AI
 */
enum class EnemyState {
    FLY_STRAIGHT,
    BANK_LEFT,
    BANK_RIGHT,
    DESTROYED
};

/**
 * @class Enemy
 * @brief Enemy fighter aircraft with FSM-based AI
 * 
 * Enemy aircraft that follows a finite state machine pattern:
 * - Flies straight for a duration
 * - Banks left or right in patterns
 * - Can be locked onto and destroyed
 */
class Enemy {
private:
    // Position in world space
    float x, y, z;
    
    // Rotation angles (degrees)
    float pitch;
    float yaw;
    float roll;
    
    // Velocity
    float velocityX, velocityY, velocityZ;
    float speed;
    
    // FSM state
    EnemyState state;
    float stateTimer;
    float stateDuration;
    
    // AI behavior parameters
    float straightDuration;
    float bankDuration;
    float maxRoll;
    float turnSpeed;
    
    // Collision
    float boundingRadius;
    
    // State
    bool alive;
    bool destroyed;
    
    // Destruction animation
    float destructionTimer;
    float destructionDuration;
    float explosionScale;
    
    // 3D Model
    Model* aircraftModel;
    bool useModel;
    
    // Methods
    void updateFSM(float deltaTime);
    void transitionState();
    void updateMovement(float deltaTime);
    
public:
    Enemy();
    Enemy(float startX, float startY, float startZ, float startYaw = 0);
    ~Enemy();
    
    /**
     * Load 3D model for the enemy aircraft
     */
    bool loadModel(const std::string& modelPath, float scale = 1.0f);
    
    /**
     * Update enemy AI and position
     */
    void update(float deltaTime);
    
    /**
     * Render the enemy aircraft
     */
    void render() const;
    
    /**
     * Destroy the enemy (trigger explosion animation)
     */
    void destroy();
    
    /**
     * Check if enemy is alive
     */
    bool isAlive() const { return alive; }
    
    /**
     * Check if enemy is destroyed (and animation complete)
     */
    bool isDestroyed() const { return destroyed; }
    
    /**
     * Get position
     */
    void getPosition(float& outX, float& outY, float& outZ) const {
        outX = x;
        outY = y;
        outZ = z;
    }
    
    /**
     * Get rotation
     */
    void getRotation(float& outPitch, float& outYaw, float& outRoll) const {
        outPitch = pitch;
        outYaw = yaw;
        outRoll = roll;
    }
    
    /**
     * Get bounding radius for collision detection
     */
    float getBoundingRadius() const { return boundingRadius; }
    
    /**
     * Get forward direction vector
     */
    void getForwardVector(float& outX, float& outY, float& outZ) const;
    
    /**
     * Get velocity
     */
    void getVelocity(float& outVX, float& outVY, float& outVZ) const {
        outVX = velocityX;
        outVY = velocityY;
        outVZ = velocityZ;
    }
    
    /**
     * Set speed
     */
    void setSpeed(float newSpeed) { speed = newSpeed; }
    
    /**
     * Set position
     */
    void setPosition(float newX, float newY, float newZ) {
        x = newX;
        y = newY;
        z = newZ;
    }
};

#endif // ENEMY_H
