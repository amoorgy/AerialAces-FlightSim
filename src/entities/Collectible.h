#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

/**
 * @class Collectible
 * @brief Glowing rings that the player collects for points and bonus time
 * 
 * Rings have pulsing animation and glow effect.
 * Uses sphere collision detection for collection.
 */
class Collectible {
private:
    // Position
    float x, y, z;
    
    // Size
    float innerRadius;  // Inner radius of torus
    float outerRadius;  // Outer radius of torus
    float collisionRadius; // Sphere radius for collision
    
    // State
    bool collected;
    
    // Animation
    float animTime;
    float pulseScale;   // Current pulse scale (oscillates)
    float glowIntensity; // Current glow brightness
    float rotationAngle; // Rotation for visual effect
    
    // Visual properties
    float colorR, colorG, colorB;  // Base color
    
    // Points value
    int pointValue;
    float bonusTime;
    
public:
    Collectible();
    Collectible(float posX, float posY, float posZ);
    
    /**
     * Update animation
     * @param deltaTime Time since last frame
     */
    void update(float deltaTime);
    
    /**
     * Render the ring
     */
    void render() const;
    
    /**
     * Mark as collected
     */
    void collect();
    
    /**
     * Check if already collected
     */
    bool isCollected() const;
    
    /**
     * Reset to uncollected state
     */
    void reset();
    
    /**
     * Set ring color
     */
    void setColor(float r, float g, float b);
    
    /**
     * Set point value
     */
    void setPointValue(int points);
    
    /**
     * Set bonus time awarded on collection
     */
    void setBonusTime(float seconds);
    
    // Getters
    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }
    float getRadius() const { return collisionRadius; }
    int getPointValue() const { return pointValue; }
    float getBonusTime() const { return bonusTime; }
    float getGlowIntensity() const { return glowIntensity; }
};

#endif // COLLECTIBLE_H
