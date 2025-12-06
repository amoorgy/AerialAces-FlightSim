#ifndef PLAYER_H
#define PLAYER_H

#include "../rendering/Model.h"

/**
 * @class Player
 * @brief Fighter aircraft controlled by the player
 * 
 * Handles aircraft physics, movement, and rendering.
 * Uses sphere bounding volume for collision detection.
 */
class Player {
private:
    // Position in world space
    float x, y, z;
    
    // Rotation angles (degrees)
    float pitch;    // Up/down rotation
    float yaw;      // Left/right rotation
    float roll;     // Banking rotation
    
    // Velocity components
    float velocityX, velocityY, velocityZ;
    
    // Aircraft properties
    float speed;            // Forward speed
    float maxSpeed;         // Maximum speed
    float minSpeed;         // Minimum speed (stall)
    float acceleration;     // Speed change rate
    
    // Control sensitivities
    float pitchSpeed;
    float yawSpeed;
    float rollSpeed;
    
    // Physics
    float gravity;
    float lift;
    
    // Collision
    float boundingRadius;
    
    // Animation
    bool barrelRolling;
    float barrelRollAngle;
    float barrelRollSpeed;
    bool spacePressed;  // Track space key state to prevent multiple barrel rolls
    
    // State
    bool alive;
    
    // 3D Model
    Model* aircraftModel;
    bool useModel;  // Flag to use model vs primitives
    
public:
    Player();
    Player(float startX, float startY, float startZ);
    ~Player();  // Destructor to clean up model
    
    /**
     * Load 3D model for aircraft
     * @param modelPath Path to OBJ file
     * @param scale Scale factor for the model
     */
    bool loadModel(const std::string& modelPath, float scale = 1.0f);
    
    /**
     * Update player physics and position
     * @param deltaTime Time since last frame
     * @param keys Array of key states
     */
    void update(float deltaTime, const bool* keys);
    
    /**
     * Render the player aircraft
     */
    void render() const;
    
    /**
     * Apply input controls
     * @param keys Array of key states
     * @param deltaTime Time for smooth input
     */
    void applyInput(const bool* keys, float deltaTime);
    
    /**
     * Reset player to starting position
     * @param startX, startY, startZ New position
     * @param startYaw Initial yaw rotation (default 0)
     */
    void reset(float startX, float startY, float startZ, float startYaw = 0.0f);
    
    /**
     * Start barrel roll animation
     * @param direction 1 for right, -1 for left
     */
    void startBarrelRoll(int direction);
    
    /**
     * Check if barrel rolling
     */
    bool isBarrelRolling() const;
    
    /**
     * Set player as dead (crash)
     */
    void kill();
    
    /**
     * Check if player is alive
     */
    bool isAlive() const;
    
    // Getters for position
    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }
    
    /**
     * Get position (all components)
     */
    void getPosition(float& outX, float& outY, float& outZ) const {
        outX = x;
        outY = y;
        outZ = z;
    }
    
    // Getters for rotation
    float getPitch() const { return pitch; }
    float getYaw() const { return yaw; }
    float getRoll() const { return roll; }
    
    /**
     * Get rotation (all components)
     */
    void getRotation(float& outPitch, float& outYaw, float& outRoll) const {
        outPitch = pitch;
        outYaw = yaw;
        outRoll = roll;
    }
    
    /**
     * Get forward direction vector
     */
    void getForwardVector(float& outX, float& outY, float& outZ) const;
    
    // Getter for collision
    float getRadius() const { return boundingRadius; }
    float getBoundingRadius() const { return boundingRadius; }
    
    // Getter for speed
    float getSpeed() const { return speed; }
    
    // Bounding box (for AABB collision with obstacles)
    float getMinX() const { return x - boundingRadius; }
    float getMaxX() const { return x + boundingRadius; }
    float getMinY() const { return y - boundingRadius * 0.5f; }
    float getMaxY() const { return y + boundingRadius * 0.5f; }
    float getMinZ() const { return z - boundingRadius; }
    float getMaxZ() const { return z + boundingRadius; }
};

#endif // PLAYER_H
