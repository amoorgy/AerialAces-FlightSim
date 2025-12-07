#ifndef MENUSYSTEM_H
#define MENUSYSTEM_H

/**
 * @enum MenuOption
 * @brief Available menu options
 */
enum class MenuOption {
    SINGLE_PLAYER = 0,
    COOP_MODE = 1,
    EXIT = 2
};

/**
 * @class MenuSystem
 * @brief Main menu system for game mode selection
 * 
 * Displays options for Single Player, Co-op Mode, and Exit
 * Shows controls information
 */
class MenuSystem {
private:
    MenuOption selectedOption;
    bool enterPressed;
    
public:
    MenuSystem();
    ~MenuSystem();
    
    /**
     * Update menu logic
     * @param deltaTime Time since last frame
     * @param keys Array of key states
     */
    void update(float deltaTime, const bool* keys);
    
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
};

#endif // MENUSYSTEM_H
