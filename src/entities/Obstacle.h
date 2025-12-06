#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "../rendering/Model.h"
#include <string>

/**
 * @enum ObstacleType
 * @brief Types of obstacles in the game
 */
enum class ObstacleType {
    MOUNTAIN,   // Conical mountain obstacle
    GROUND,     // Ground plane
    BUILDING,   // Rectangular building
    ROCK        // Boulder/rock formation
};

/**
 * @class Obstacle
 * @brief Static obstacles that the player must avoid
 * 
 * Uses AABB (Axis-Aligned Bounding Box) for collision detection.
 * Can represent mountains, ground, buildings, etc.
 */
class Obstacle {
private:
    // Position (center of obstacle)
    float x, y, z;
    
    // Dimensions
    float width;    // X dimension
    float height;   // Y dimension  
    float depth;    // Z dimension
    
    // Type determines rendering
    ObstacleType type;
    
    // Visual properties
    float colorR, colorG, colorB;
    
    // For mountains specifically
    float baseRadius;  // Base radius of cone
    
    // State
    bool active;  // For destroyable obstacles in Level 2
    
    // 3D Model support
    Model* obstacleModel;
    bool useModel;  // Flag to use model vs primitives
    
public:
    Obstacle();
    Obstacle(float posX, float posY, float posZ, 
             float w, float h, float d, 
             ObstacleType obstacleType);
    ~Obstacle();  // Destructor to clean up
    
    /**
     * Load 3D model for obstacle (mountains, rocks, etc.)
     * @param modelPath Path to OBJ file
     * @param scale Scale factor for the model
     */
    bool loadModel(const std::string& modelPath, float scale = 1.0f);
    
    /**
     * Render the obstacle
     */
    void render() const;
    
    /**
     * Set obstacle color
     */
    void setColor(float r, float g, float b);
    
    /**
     * Get obstacle type
     */
    ObstacleType getType() const { return type; }
    
    // Position getters
    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }
    
    // Dimension getters
    float getWidth() const { return width; }
    float getHeight() const { return height; }
    float getDepth() const { return depth; }
    
    // AABB bounds for collision detection
    float getMinX() const { return x - width / 2.0f; }
    float getMaxX() const { return x + width / 2.0f; }
    float getMinY() const { return y; }  // y is base for mountains
    float getMaxY() const { return y + height; }
    float getMinZ() const { return z - depth / 2.0f; }
    float getMaxZ() const { return z + depth / 2.0f; }
    
    /**
     * Check if this is a ground plane
     */
    bool isGround() const { return type == ObstacleType::GROUND; }
    
    /**
     * For mountains: get approximate collision radius at given height
     * (cones narrow at top)
     */
    float getRadiusAtHeight(float h) const;
    
    /**
     * Check collision with loaded model
     * @param px Player X position
     * @param py Player Y position
     * @param pz Player Z position
     * @param radius Player collision radius
     * @return true if collision detected
     */
    bool checkModelCollision(float px, float py, float pz, float radius) const;
    
    /**
     * Check if model is loaded
     */
    bool hasModel() const { return useModel && obstacleModel != nullptr; }
    
    /**
     * Check if obstacle is active (for destroyable targets in Level 2)
     */
    bool isActive() const { return active; }
    
    /**
     * Deactivate obstacle (mark as destroyed)
     */
    void deactivate() { active = false; }
    
    /**
     * Activate obstacle
     */
    void activate() { active = true; }
};

#endif // OBSTACLE_H
