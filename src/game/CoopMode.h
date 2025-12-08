#ifndef COOPMODE_H
#define COOPMODE_H

#include "Level.h"
#include "../entities/Player.h"
#include "../entities/Missile.h"
#include "../entities/Obstacle.h"
#include "../rendering/Camera.h"
#include "../rendering/Lighting.h"
#include <vector>

/**
 * @enum CoopState
 * @brief States for Co-op gameplay
 */
enum class CoopState {
    PLAYING,
    PLAYER1_WON,
    PLAYER2_WON,
    DRAW
};

/**
 * @class CoopMode
 * @brief Split-screen dogfight mode for two players
 * 
 * Features:
 * - Split-screen rendering (top/bottom)
 * - Player 1: WASD + QE controls
 * - Player 2: IJKL + UO controls
 * - Shoot each other with missiles
 * - Health bars and ammo counters
 * - Victory/defeat detection
 */
class CoopMode : public Level {
private:
    // Game state
    CoopState state;
    
    // Players
    Player* player1;
    Player* player2;
    
    // Health system
    int player1Health;
    int player2Health;
    int maxHealth;
    
    // Ammo system
    int player1Ammo;
    int player2Ammo;
    int maxAmmo;
    float reloadTime;
    float player1ReloadTimer;
    float player2ReloadTimer;
    
    // Missiles
    std::vector<Missile*> missiles;
    
    // Cooldown timers
    float player1FireCooldown;
    float player2FireCooldown;
    
    // Arena obstacles
    std::vector<Obstacle*> obstacles;
    
    // Cameras
    Camera* camera1;
    Camera* camera2;
    
    // Lighting
    Lighting* lighting;
    
    // Score tracking
    int player1Score;
    int player2Score;
    
    // Input tracking
    bool player1SpacePressed;
    bool player2SpacePressed;
    
    // Arena dimensions
    float arenaSize;
    
public:
    CoopMode();
    ~CoopMode();
    
    // Level interface
    void init() override;
    void update(float deltaTime, const bool* keys) override;
    void render() override;
    bool isWon() const override;
    bool isLost() const override;
    void cleanup() override;
    void restart() override;
    void handleMouse(int button, int state, int x, int y) override;
    void handleMouseMotion(int x, int y) override;
    int getScore() const override { return player1Score + player2Score; }
    float getTimeRemaining() const override { return 0.0f; }
    const char* getName() const override { return "Co-op Dogfight"; }
    
private:
    // Helper functions
    void createArena();
    void updatePlayer1(float deltaTime, const bool* keys);
    void updatePlayer2(float deltaTime, const bool* keys);
    void updateMissiles(float deltaTime);
    void checkCollisions();
    void fireMissilePlayer1();
    void fireMissilePlayer2();
    void renderSplitScreen();
    void renderPlayer1View();
    void renderPlayer2View();
    void renderHUD();
    void renderPlayer1HUD();
    void renderPlayer2HUD();
    void renderMessages();
    void renderArena();
    void renderHealthBar(float x, float y, int health, int maxHealth);
    void renderAmmoCounter(float x, float y, int ammo, int maxAmmo);
};

#endif // COOPMODE_H
