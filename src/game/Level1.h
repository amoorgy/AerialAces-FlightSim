#ifndef LEVEL1_H
#define LEVEL1_H

#include "Level.h"
#include "../entities/Player.h"
#include "../entities/Collectible.h"
#include "../entities/Obstacle.h"
#include "../rendering/Camera.h"
#include "../rendering/Lighting.h"
#include "../utils/Timer.h"
#include <vector>

/**
 * @enum Level1State
 * @brief States for Level 1 gameplay
 */
enum class Level1State {
    PLAYING,
    WON,
    LOST,
    PAUSED
};

/**
 * @class Level1
 * @brief Terrain Navigation Challenge
 * 
 * Players pilot through mountainous terrain, collecting rings
 * while avoiding obstacles. Time limit with bonus time for rings.
 * 
 * Win: Collect all rings
 * Lose: Crash or time runs out
 */
class Level1 : public Level {
private:
    // Game state
    Level1State state;
    
    // Entities
    Player* player;
    std::vector<Collectible*> rings;
    std::vector<Obstacle*> obstacles;
    
    // Systems
    Camera* camera;
    Lighting* lighting;
    Timer timer;
    
    // Score tracking
    int score;
    int ringsCollected;
    int totalRings;
    
    // Timer settings
    float startTime;
    float bonusTimePerRing;
    
    // Explosion effect
    bool explosionActive;
    float explosionTime;
    float explosionX, explosionY, explosionZ;
    
    // Player start position
    float startX, startY, startZ;
    float startYaw;  // Starting rotation
    
    // Level bounds
    float levelWidth;
    float levelLength;
    
    // Spawn protection
    float spawnProtectionTime;  // Seconds of invincibility after spawn
    
    // End screen animation
    float endScreenTimer;       // Timer for end screen animations
    
    // Lighthouses with rotating beams
    std::vector<Obstacle*> lighthouses;
    void createLighthouses();
    void renderLighthouses();
    void updateLighthouses(float deltaTime);
    
    // Color-based collision
    bool checkColorCollision();  // Check terrain collision using color sampling
    
    // Internal methods
    void createTerrain();
    void createRings();
    void loadModels();  // Load 3D models for entities
    void checkCollisions();
    void triggerCrash(float x, float y, float z);
    void renderHUD();
    void renderExplosion();
    void renderSky();
    void renderMessages();
    void renderLensFlare();  // New: render sun lens flare effect
    void updateLensFlare();  // New: calculate lens flare intensity
    
public:
    Level1();
    virtual ~Level1();
    
    // Level interface implementation
    void init() override;
    void update(float deltaTime, const bool* keys) override;
    void render() override;
    bool isWon() const override;
    bool isLost() const override;
    void cleanup() override;
    void restart() override;
    void handleMouse(int button, int state, int x, int y) override;
    void handleMouseMotion(int x, int y) override;
    int getScore() const override;
    float getTimeRemaining() const override;
    const char* getName() const override;
    
    // Level 1 specific methods
    void toggleDayNight();
    bool isNightMode() const;
    Player* getPlayer() const { return player; }
    Camera* getCamera() const { return camera; }
    Level1State getState() const { return state; }
};

#endif // LEVEL1_H
