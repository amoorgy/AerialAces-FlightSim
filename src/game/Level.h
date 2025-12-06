#ifndef LEVEL_H
#define LEVEL_H

/**
 * @class Level
 * @brief Abstract base class for game levels
 * 
 * Defines the interface that all levels must implement.
 * Supports win/lose conditions, update, and render.
 */
class Level {
public:
    virtual ~Level() {}
    
    /**
     * Initialize the level
     */
    virtual void init() = 0;
    
    /**
     * Update level logic
     * @param deltaTime Time since last frame
     * @param keys Array of key states
     */
    virtual void update(float deltaTime, const bool* keys) = 0;
    
    /**
     * Render the level
     */
    virtual void render() = 0;
    
    /**
     * Check if level is won
     * @return true if player has won
     */
    virtual bool isWon() const = 0;
    
    /**
     * Check if level is lost
     * @return true if player has lost
     */
    virtual bool isLost() const = 0;
    
    /**
     * Clean up level resources
     */
    virtual void cleanup() = 0;
    
    /**
     * Restart the level
     */
    virtual void restart() = 0;
    
    /**
     * Handle mouse button input
     * @param button Mouse button
     * @param state Button state
     * @param x Mouse X position
     * @param y Mouse Y position
     */
    virtual void handleMouse(int button, int state, int x, int y) = 0;
    
    /**
     * Handle mouse motion
     * @param x Mouse X position
     * @param y Mouse Y position
     */
    virtual void handleMouseMotion(int x, int y) = 0;
    
    /**
     * Get current score
     */
    virtual int getScore() const = 0;
    
    /**
     * Get time remaining
     */
    virtual float getTimeRemaining() const = 0;
    
    /**
     * Get level name/title
     */
    virtual const char* getName() const = 0;
};

#endif // LEVEL_H
