#ifndef LEVEL2_H
#define LEVEL2_H

#include "Level.h"
#include "../entities/Player.h"
#include "../entities/Enemy.h"
#include "../entities/Missile.h"
#include "../entities/Obstacle.h"
#include "../entities/Collectible.h"
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
 * @struct Lighthouse
 * @brief Represents a lighthouse on a mountain peak
 */
struct Lighthouse {
    float x, y, z;          // Position
    float height;           // Lighthouse height
    Obstacle* obstacle;     // Visual representation
    
    Lighthouse(float px, float py, float pz, float h)
        : x(px), y(py), z(pz), height(h), obstacle(nullptr) {}
};

/**
 * @struct Bullseye
 * @brief Represents a target bullseye that player must hit with rockets
 */
struct Bullseye {
    float x, y, z;          // Position
    float radius;           // Collision/visual radius
    bool destroyed;         // True if hit by rocket
    float rotationAngle;    // For animation
    
    Bullseye(float px, float py, float pz, float r = 8.0f)
        : x(px), y(py), z(pz), radius(r), destroyed(false), rotationAngle(0.0f) {}
};

/**
 * @struct BonusRing
 * @brief Rings that give bonus rockets when collected
 */
struct BonusRing {
    float x, y, z;          // Position
    float radius;           // Collection radius
    bool collected;         // True if already collected
    float rotationAngle;    // For animation
    int rocketBonus;        // Number of rockets awarded
    
    BonusRing(float px, float py, float pz, int bonus = 3)
        : x(px), y(py), z(pz), radius(5.0f), collected(false), 
          rotationAngle(0.0f), rocketBonus(bonus) {}
};

/**
 * @struct Rocket
 * @brief Simple rocket projectile that flies straight and explodes
 */
struct Rocket {
    float x, y, z;          // Position
    float dirX, dirY, dirZ; // Direction (normalized)
    float speed;            // Movement speed
    float lifetime;         // Time alive
    float maxLifetime;      // Max time before self-destruct
    bool active;            // Still flying
    
    Rocket(float px, float py, float pz, float dx, float dy, float dz)
        : x(px), y(py), z(pz), dirX(dx), dirY(dy), dirZ(dz),
          speed(4.0f), lifetime(0.0f), maxLifetime(3.0f), active(true) {}
};

/**
 * @class Level2
 * @brief Aerial Combat Challenge - Bullseye Target Practice
 * 
 * Players must hit 3 bullseye targets with limited rockets:
 * - Press F to fire rockets in a straight line
 * - Limited rockets (start with 5)
 * - 30 second time limit
 * - Collect bonus rings near bullseyes for extra rockets
 * - If you run out of rockets, a tracking missile hunts you!
 * 
 * Win: Destroy all 3 bullseyes before time runs out
 * Lose: Run out of time or crash into terrain
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
    std::vector<Lighthouse> lighthouses;  // Lighthouses on mountain peaks
    
    // Bullseye targets
    std::vector<Bullseye> bullseyes;
    int bullseyesDestroyed;
    int totalBullseyes;
    
    // Bonus rings for extra rockets
    std::vector<BonusRing> bonusRings;
    int ringsCollected;
    
    // Player rockets
    std::vector<Rocket> rockets;
    float rocketFireCooldown;
    float rocketFireTimer;
    bool fKeyWasPressed;
    int rocketsRemaining;       // Limited rockets!
    int maxRockets;             // Max rockets player can have
    
    // Punishment missile (spawns when out of rockets)
    Missile* punishmentMissile;
    bool punishmentMissileActive;
    float punishmentMissileDelay;  // Delay before missile spawns
    
    // Timer for level
    Timer levelTimer;
    float levelTimeLimit;       // 30 seconds
    
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
    
    // Lighthouse missile spawning (replaces enemy missile spawning)
    float lighthouseMissileSpawnTimer;
    float lighthouseMissileSpawnInterval;
    
    // Safe zone (area near player start is safe from missiles)
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
    
    // End screen menu selection (0 = Restart, 1 = Main Menu)
    int endScreenSelection;
    
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
    void createLighthouses();
    void createBullseyes();
    void createBonusRings();
    void loadModels();
    void updateLockOn(float deltaTime);
    void updateMissiles(float deltaTime);
    void updateEnemies(float deltaTime);
    void updateExplosions(float deltaTime);
    void updateDebris(float deltaTime);
    void updateCameraShake(float deltaTime);
    void updateLighthouses(float deltaTime);
    void updateRockets(float deltaTime);
    void updateBullseyes(float deltaTime);
    void updateBonusRings(float deltaTime);
    void updatePunishmentMissile(float deltaTime);
    void checkCollisions();
    void checkMissileCollisions();
    void checkRocketCollisions();
    void checkBonusRingCollisions();
    void checkNearMisses(float deltaTime);
    void fireMissile();
    void fireRocket();
    void spawnPunishmentMissile();
    void spawnLighthouseMissile();
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
    void renderLighthouses();
    void renderBullseyes();
    void renderBonusRings();
    void renderRockets();
    void applyExplosionLights();
    
    // Helper methods
    bool isInSafeZone(float x, float y, float z);
    float distanceToPlayer(float x, float y, float z);
    Enemy* findNearestEnemy();
    bool isEnemyInSights(Enemy* enemy, float& outDistance, float& outAngle);
    int findClosestLighthouse();
    
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
    virtual float getTimeRemaining() const override { return levelTimer.getTime(); }
    virtual const char* getName() const override { return "Level 2: Target Practice"; }
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
    
    // Check if user selected main menu from end screen
    bool isMainMenuSelected() const { return (state == Level2State::LOST || state == Level2State::WON) && endScreenSelection == 1; }
    
    // Get end screen selection (0 = restart, 1 = menu)
    int getEndScreenSelection() const { return endScreenSelection; }
};

#endif // LEVEL2_H
