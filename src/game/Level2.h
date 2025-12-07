#ifndef LEVEL2_H
#define LEVEL2_H

#include "Level.h"
#include "../entities/Player.h"
#include "../entities/Enemy.h"
#include "../entities/Missile.h"
#include "../entities/Obstacle.h"
#include "../rendering/Camera.h"
#include "../rendering/Lighting.h"
#include "../utils/Timer.h"
#include <vector>

/**
 * @enum Level2State
 * @brief States for Level 2 gameplay
 */
enum class Level2State {
    PLAYING,
    FINALE,     // Final cinematic target sequence
    WON,
    LOST,
    PAUSED
};

/**
 * @enum LockOnState
 * @brief States for lock-on targeting system
 */
enum class LockOnState {
    NONE,           // No target
    ACQUIRING,      // Locking onto target
    LOCKED          // Fully locked
};

/**
 * @class Level2
 * @brief Aerial Combat Challenge
 * 
 * Players engage in dogfight with enemy aircraft:
 * - Evade incoming missiles from enemies
 * - Lock onto enemy aircraft using FSM-based AI
 * - Destroy enemies with player missiles
 * - Complete finale sequence to win
 * 
 * Win: Destroy all enemies and final target
 * Lose: Get hit by enemy missile or crash
 */
class Level2 : public Level {
private:
    // Game state
    Level2State state;
    
    // Entities
    Player* player;
    std::vector<Enemy*> enemies;
    std::vector<Missile*> missiles;
    std::vector<Obstacle*> terrain;  // Sparse mountains
    
    // Systems
    Camera* camera;
    Lighting* lighting;
    Timer timer;
    
    // Score tracking
    int score;
    int enemiesDestroyed;
    int totalEnemies;
    
    // Lock-on system
    LockOnState lockOnState;
    Enemy* lockedTarget;
    float lockOnProgress;      // 0.0 to 1.0
    float lockOnTime;          // Time to fully lock
    float lockOnDistance;      // Max distance for lock
    float lockOnAngle;         // Max angle from center for lock
    float lockOnBeepTimer;     // For audio feedback
    
    // Missile firing
    float missileFireCooldown;
    float missileFireTimer;
    bool leftMousePressed;
    
    // Enemy missile spawning
    float enemyMissileSpawnTimer;
    float enemyMissileSpawnInterval;
    
    // Safe zone (area near player is safe from enemy missiles)
    float safeZoneRadius;
    
    // Warning effects
    bool missileWarning;
    float warningFlashTimer;
    
    // Key press tracking
    bool nKeyWasPressed;
    
    // Explosion effects
    struct ExplosionEffect {
        float x, y, z;
        float timer;
        float duration;
        float scale;
        int lightId;  // Dynamic light ID for this explosion
        
        ExplosionEffect(float px, float py, float pz, int lid = -1)
            : x(px), y(py), z(pz), timer(0), duration(1.5f), scale(1.0f), lightId(lid) {}
    };
    std::vector<ExplosionEffect> explosions;
    
    // Debris particles
    struct DebrisParticle {
        float x, y, z;
        float vx, vy, vz;  // Velocity
        float rx, ry, rz;  // Rotation
        float rotSpeed;
        float life;
        float size;
        
        DebrisParticle(float px, float py, float pz)
            : x(px), y(py), z(pz), 
              vx((rand()%200-100)/100.0f), 
              vy((rand()%150+50)/100.0f), 
              vz((rand()%200-100)/100.0f),
              rx(rand()%360), ry(rand()%360), rz(rand()%360),
              rotSpeed((rand()%200+100)/10.0f),
              life(1.0f), size(0.5f + (rand()%10)/10.0f) {}
    };
    std::vector<DebrisParticle> debris;
    
    // Camera shake
    float cameraShakeIntensity;
    float cameraShakeDuration;
    float cameraShakeTimer;
    
    // Near-miss detection
    float nearMissTimer;
    bool nearMissDetected;
    
    // End screen animation
    float endScreenTimer;
    
    // Audio paths
    std::string explosionSoundPath;
    std::string lockOnSoundPath;
    std::string missileLaunchSoundPath;
    std::string whooshSoundPath;
    
    // Player start position
    float startX, startY, startZ;
    float startYaw;
    
    // Level bounds
    float levelWidth;
    float levelLength;
    
    // Finale target
    Obstacle* finaleTarget;
    bool finaleTriggered;
    float finaleTimer;
    
    // Internal methods
    void createTerrain();
    void createEnemies();
    void loadModels();
    void updateLockOn(float deltaTime);
    void updateMissiles(float deltaTime);
    void updateEnemies(float deltaTime);
    void updateExplosions(float deltaTime);
    void updateDebris(float deltaTime);
    void updateCameraShake(float deltaTime);
    void checkCollisions();
    void checkMissileCollisions();
    void checkNearMisses(float deltaTime);
    void fireMissile();
    void spawnEnemyMissile();
    void triggerExplosion(float x, float y, float z);
    void triggerCameraShake(float intensity, float duration);
    void spawnDebris(float x, float y, float z, int count);
    void playSound(const std::string& soundPath);
    void renderHUD();
    void renderLockOnReticle();
    void renderExplosions();
    void renderDebris();
    void renderSky();
    void renderMessages();
    void renderMissileWarning();
    void applyExplosionLights();
    
    // Helper methods
    bool isInSafeZone(float x, float y, float z);
    float distanceToPlayer(float x, float y, float z);
    Enemy* findNearestEnemy();
    bool isEnemyInSights(Enemy* enemy, float& outDistance, float& outAngle);
    
public:
    Level2();
    virtual ~Level2();
    
    // Level interface implementation
    virtual void init() override;
    virtual void update(float deltaTime, const bool* keys) override;
    virtual void render() override;
    virtual void cleanup() override;
    virtual bool isWon() const override;
    virtual bool isLost() const override;
    
    // Level interface
    virtual void handleMouse(int button, int state, int x, int y) override;
    virtual void handleMouseMotion(int x, int y) override;
    virtual int getScore() const override { return score; }
    virtual float getTimeRemaining() const override { return 999.0f; }  // No time limit in Level 2
    virtual const char* getName() const override { return "Level 2: Aerial Combat"; }
    virtual void restart() override { reset(); }
    
    // Reset method (non-virtual)
    void reset();
    
    // Input handling
    void handleKeyPress(unsigned char key, bool pressed);
    void handleMouseButton(int button, int state, int x, int y);
    void handleMouseMove(int x, int y);
    
    // Day/night toggle
    void toggleDayNight();
    
    // Camera access
    Camera* getCamera() const { return camera; }
};

#endif // LEVEL2_H
