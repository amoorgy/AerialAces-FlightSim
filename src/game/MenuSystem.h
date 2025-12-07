#ifndef MENUSYSTEM_H
#define MENUSYSTEM_H

/**
 * @enum MenuOption
 * @brief Available menu options
 */
enum class MenuOption {
    SINGLE_PLAYER = 0,  // Level 1
    LEVEL_2 = 1,        // Level 2 (unlocked after completing Level 1)
    COOP_MODE = 2,
    EXIT = 3
};

/**
 * @class MenuSystem
 * @brief Main menu system for game mode selection
 * 
 * Displays options for Single Player, Level 2 (when unlocked), Co-op Mode, and Exit
 * Shows controls information
 */
class MenuSystem {
private:
    MenuOption selectedOption;
    bool enterPressed;
    float animationTimer;
    bool upKeyPressed;
    bool downKeyPressed;
    float fadeAlpha[4];  // Fade alpha for each button (now 4 options)
    float plane1X, plane1Y;  // Background plane 1 position
    float plane2X, plane2Y;  // Background plane 2 position
    
    // Level unlocking
    bool level2Unlocked;
    
public:
    MenuSystem();
    ~MenuSystem();
    
    /**
     * Update menu logic
     * @param deltaTime Time since last frame
     * @param keys Array of key states
     * @param upPressed Is up arrow pressed
     * @param downPressed Is down arrow pressed
     */
    void update(float deltaTime, const bool* keys, bool upPressed, bool downPressed);
    
    /**
     * Render the menu
     */
    void render();
    
    /**
     * Handle keyboard input
     * @param key ASCII key code
     * @param pressed true if pressed
     */
    void handleKeyPress(unsigned char key, bool pressed);
    
    /**
     * Get selected option
     */
    MenuOption getSelectedOption() const { return selectedOption; }
    
    /**
     * Check if option was confirmed
     */
    bool isOptionConfirmed() const;
    
    /**
     * Reset confirmation state
     */
    void resetConfirmation();
    
    /**
     * Unlock Level 2 (called when Level 1 is completed)
     */
    void unlockLevel2() { level2Unlocked = true; }
    
    /**
     * Check if Level 2 is unlocked
     */
    bool isLevel2Unlocked() const { return level2Unlocked; }
};

#endif // MENUSYSTEM_H
