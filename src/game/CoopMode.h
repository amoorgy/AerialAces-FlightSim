#ifndef COOPMODE_H
#define COOPMODE_H

#include "Level.h"
#include "../entities/Player.h"
#include "../entities/Missile.h"
#include "../entities/Obstacle.h"
#include "../rendering/Camera.h"
#include "../rendering/Lighting.h"
#include "../utils/Timer.h"
#include <vector>
#include <string>

/**
 * @enum CoopState
 * @brief States for Co-op gameplay
 */
enum class CoopState {
    PLAYING,
    PLAYER1_WON,
    PLAYER2_WON,
    DRAW,
    PAUSED
};

/**
 * @struct CoopLighthouse
 * @brief Represents a lighthouse on a mountain peak (visual only)
 */
struct CoopLighthouse {
    float x, y, z;          // Position
    float height;           // Lighthouse height
    Obstacle* obstacle;     // Visual representation
    
    CoopLighthouse(float px, float py, float pz, float h)
        : x(px), y(py), z(pz), height(h), obstacle(nullptr) {}
};

/**
 * @struct CoopBullseye
 * @brief Represents a target bullseye that players must hit with rockets
 */
struct CoopBullseye {
    float x, y, z;          // Position
    float radius;           // Collision/visual radius
    bool destroyed;         // True if hit by rocket
    float rotationAngle;    // For animation
    int destroyedBy;        // 0 = none, 1 = player1, 2 = player2
    
    CoopBullseye(float px, float py, float pz, float r = 8.0f)
        : x(px), y(py), z(pz), radius(r), destroyed(false), 
          rotationAngle(0.0f), destroyedBy(0) {}
};

/**
 * @struct CoopBonusRing
 * @brief Rings that give bonus rockets when collected
 */
struct CoopBonusRing {
    float x, y, z;          // Position
    float radius;           // Collection radius
    bool collected;         // True if already collected
    float rotationAngle;    // For animation
    int rocketBonus;        // Number of rockets awarded
    int collectedBy;        // 0 = none, 1 = player1, 2 = player2
    
    CoopBonusRing(float px, float py, float pz, int bonus = 3)
        : x(px), y(py), z(pz), radius(5.0f), collected(false), 
          rotationAngle(0.0f), rocketBonus(bonus), collectedBy(0) {}
};

/**
 * @struct CoopRocket
 * @brief Simple rocket projectile that flies straight and explodes
 */
struct CoopRocket {
    float x, y, z;          // Position
    float dirX, dirY, dirZ; // Direction (normalized)
    float speed;            // Movement speed
    float lifetime;         // Time alive
    float maxLifetime;      // Max time before self-destruct
    bool active;            // Still flying
    int owner;              // 1 = player1, 2 = player2
    
    CoopRocket(float px, float py, float pz, float dx, float dy, float dz, int playerOwner)
        : x(px), y(py), z(pz), dirX(dx), dirY(dy), dirZ(dz),
          speed(4.0f), lifetime(0.0f), maxLifetime(3.0f), active(true), owner(playerOwner) {}
};

/**
 * @struct CoopExplosion
 * @brief Explosion visual effect
 */
struct CoopExplosion {
    float x, y, z;
    float timer;
    float duration;
    float scale;
    int lightId;
    
    CoopExplosion(float px, float py, float pz, int lid = -1)
        : x(px), y(py), z(pz), timer(0), duration(1.5f), scale(1.0f), lightId(lid) {}
};

/**
 * @struct CoopDebris
 * @brief Debris particle for explosions
 */
struct CoopDebris {
    float x, y, z;
    float vx, vy, vz;
    float rx, ry, rz;
    float rotSpeed;
    float life;
    float size;
    
    CoopDebris(float px, float py, float pz);
};

/**
 * @class CoopMode
 * @brief Split-screen Co-op Target Practice Mode
 * 
 * Based on Level 2's terrain and mechanics:
 * - Same mountains/lighthouses terrain as Level 2
 * - Split-screen (top/bottom) for two players
 * - Both players compete to destroy bullseye targets
 * - Limited rockets with bonus rings for extra ammo
 * - Score based on targets destroyed
 * - Punishment missiles for running out of rockets
 * 
 * Player 1: WASD + QE + F to fire rockets
 * Player 2: IJKL + UO + P to fire rockets
 * 
 * Win condition: Destroy more targets than opponent, or be last one alive
 */
class CoopMode : public Level {
private:
    // Game state
    CoopState state;
    
    // Players
    Player* player1;
    Player* player2;
    
    // Cameras (one per player)
    Camera* camera1;
    Camera* camera2;
    
    // Lighting
    Lighting* lighting;
    
    // Terrain (mountains)
    std::vector<Obstacle*> terrain;
    
    // Lighthouses on mountain peaks
    std::vector<CoopLighthouse> lighthouses;
    
    // Bullseye targets
    std::vector<CoopBullseye> bullseyes;
    int totalBullseyes;
    int player1BullseyesDestroyed;
    int player2BullseyesDestroyed;
    
    // Bonus rings for extra rockets
    std::vector<CoopBonusRing> bonusRings;
    int player1RingsCollected;
    int player2RingsCollected;
    
    // Player rockets
    std::vector<CoopRocket> rockets;
    float rocketFireCooldown;
    float player1RocketTimer;
    float player2RocketTimer;
    bool player1FKeyPressed;
    bool player2PKeyPressed;
    int player1RocketsRemaining;
    int player2RocketsRemaining;
    int maxRockets;
    
    // Punishment missiles
    Missile* player1PunishmentMissile;
    Missile* player2PunishmentMissile;
    bool player1PunishmentActive;
    bool player2PunishmentActive;
    float player1PunishmentDelay;
    float player2PunishmentDelay;
    
    // Timer
    Timer levelTimer;
    float levelTimeLimit;
    
    // Score tracking
    int player1Score;
    int player2Score;
    
    // Health (optional: collision with ground/obstacles)
    int player1Health;
    int player2Health;
    int maxHealth;
    
    // Explosions and debris
    std::vector<CoopExplosion> explosions;
    std::vector<CoopDebris> debris;
    
    // Camera shake
    float cameraShakeIntensity;
    float cameraShakeDuration;
    float cameraShakeTimer;
    
    // Warning effects
    bool player1MissileWarning;
    bool player2MissileWarning;
    float warningFlashTimer;
    
    // Key press tracking
    bool nKeyWasPressed;
    
    // End screen
    float endScreenTimer;
    
    // Player start positions
    float player1StartX, player1StartY, player1StartZ, player1StartYaw;
    float player2StartX, player2StartY, player2StartZ, player2StartYaw;
    
    // Level bounds
    float levelWidth;
    float levelLength;
    
    // Sound paths
    std::string explosionSoundPath;
    std::string missileLaunchSoundPath;
    
    // Arena size
    float arenaSize;
    
public:
    CoopMode();
    virtual ~CoopMode();
    
    // Level interface implementation
    virtual void init() override;
    virtual void update(float deltaTime, const bool* keys) override;
    virtual void render() override;
    virtual void cleanup() override;
    virtual bool isWon() const override;
    virtual bool isLost() const override;
    virtual void restart() override;
    virtual void handleMouse(int button, int state, int x, int y) override;
    virtual void handleMouseMotion(int x, int y) override;
    virtual int getScore() const override { return player1Score + player2Score; }
    virtual float getTimeRemaining() const override { return levelTimer.getTime(); }
    virtual const char* getName() const override { return "Co-op Target Practice"; }
    
    // Day/night toggle
    void toggleDayNight();
    
private:
    // Initialization
    void loadModels();
    void createTerrain();
    void createLighthouses();
    void createBullseyes();
    void createBonusRings();
    
    // Update functions
    void updatePlayer1(float deltaTime, const bool* keys);
    void updatePlayer2(float deltaTime, const bool* keys);
    void updateRockets(float deltaTime);
    void updateBullseyes(float deltaTime);
    void updateBonusRings(float deltaTime);
    void updateLighthouses(float deltaTime);
    void updateExplosions(float deltaTime);
    void updateDebris(float deltaTime);
    void updateCameraShake(float deltaTime);
    void updatePunishmentMissiles(float deltaTime);
    
    // Collision checks
    void checkCollisions();
    void checkRocketCollisions();
    void checkBonusRingCollisions();
    void checkGroundCollisions();
    
    // Actions
    void fireRocketPlayer1();
    void fireRocketPlayer2();
    void spawnPunishmentMissile(int playerNum);
    void triggerExplosion(float x, float y, float z);
    void triggerCameraShake(float intensity, float duration);
    void spawnDebris(float x, float y, float z, int count);
    void playSound(const std::string& soundPath);
    
    // Rendering
    void renderSplitScreen();
    void renderPlayer1View();
    void renderPlayer2View();
    void renderScene();
    void renderSky();
    void renderLighthouses();
    void renderBullseyes();
    void renderBonusRings();
    void renderRockets();
    void renderExplosions();
    void renderDebris();
    void renderHUD();
    void renderPlayer1HUD();
    void renderPlayer2HUD();
    void renderMissileWarning(int playerNum);
    void renderMessages();
    void applyExplosionLights();
    
    // Helper functions
    float distanceToPlayer(float x, float y, float z, int playerNum);
};

#endif // COOPMODE_H
