#ifndef GAME_H
#define GAME_H

#include "Level.h"
#include "../utils/Input.h"

/**
 * @enum GameState
 * @brief Main game states
 */
enum class GameState {
    MENU,       // Main menu
    PLAYING,    // In level
    PAUSED,     // Game paused
    GAME_OVER,  // Game ended (win or lose)
    LEVEL_COMPLETE // Level completed, transition to next
};

/**
 * @class Game
 * @brief Main game manager
 * 
 * Handles game states, level transitions, and main loop callbacks.
 * Singleton-like usage through pointer passed to GLUT callbacks.
 */
class Game {
private:
    // Current state
    GameState state;
    
    // Current level
    Level* currentLevel;
    int currentLevelIndex;
    static const int MAX_LEVELS = 2;
    
    // Input handler
    Input input;
    
    // Timing
    float deltaTime;
    
    // Window dimensions
    int windowWidth;
    int windowHeight;
    
    // Key state tracking
    bool pauseKeyPressed;
    bool lKeyPressed;
    
public:
    Game();
    ~Game();
    
    /**
     * Initialize the game
     */
    void init();
    
    /**
     * Update game logic
     * @param dt Delta time since last frame
     */
    void update(float dt);
    
    /**
     * Render the game
     */
    void render();
    
    /**
     * Clean up resources
     */
    void cleanup();
    
    /**
     * Handle keyboard input (key press)
     * @param key ASCII key code
     * @param pressed true if key was pressed, false if released
     */
    void handleKeyPress(unsigned char key, bool pressed);
    
    /**
     * Handle special keys (arrows, function keys)
     * @param key GLUT special key code
     * @param pressed true if pressed
     */
    void handleSpecialKey(int key, bool pressed);
    
    /**
     * Handle mouse input
     * @param button Mouse button
     * @param state Button state
     * @param x Mouse X position
     * @param y Mouse Y position
     */
    void handleMouse(int button, int state, int x, int y);
    
    /**
     * Handle mouse motion
     * @param x Mouse X position
     * @param y Mouse Y position
     */
    void handleMouseMotion(int x, int y);
    
    /**
     * Handle window resize
     * @param width New window width
     * @param height New window height
     */
    void handleReshape(int width, int height);
    
    /**
     * Load a specific level
     * @param levelIndex Level number (1, 2, etc.)
     */
    void loadLevel(int levelIndex);
    
    /**
     * Advance to next level
     */
    void nextLevel();
    
    /**
     * Toggle pause
     */
    void togglePause();
    
    /**
     * Restart current level
     */
    void restartLevel();
    
    /**
     * Render pause overlay
     */
    void renderPauseOverlay();
    
    /**
     * Get current game state
     */
    GameState getState() const { return state; }
    
    /**
     * Check if game is running
     */
    bool isRunning() const { return state == GameState::PLAYING; }
};

#endif // GAME_H
