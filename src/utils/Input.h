#ifndef INPUT_H
#define INPUT_H

/**
 * @class Input
 * @brief Handles keyboard and mouse input state
 * 
 * Provides clean interface for checking key states and mouse buttons.
 * Call setKey/setMouse from GLUT callbacks, then query states in game logic.
 */
class Input {
private:
    bool keys[256];           // Regular keyboard keys
    bool specialKeys[256];    // Special keys (arrows, F1-F12)
    int mouseX, mouseY;       // Current mouse position
    bool mouseButtons[3];     // Left, middle, right buttons
    
public:
    Input();
    
    /**
     * Set regular key state
     * @param key ASCII key code
     * @param pressed true if key was pressed, false if released
     */
    void setKey(unsigned char key, bool pressed);
    
    /**
     * Set special key state (arrows, function keys)
     * @param key GLUT special key code
     * @param pressed true if key was pressed, false if released
     */
    void setSpecialKey(int key, bool pressed);
    
    /**
     * Update mouse position
     * @param x Mouse X coordinate
     * @param y Mouse Y coordinate
     */
    void setMousePosition(int x, int y);
    
    /**
     * Set mouse button state
     * @param button Button index (0=left, 1=middle, 2=right)
     * @param pressed true if pressed
     */
    void setMouseButton(int button, bool pressed);
    
    /**
     * Check if key is currently pressed
     * @param key ASCII key code (handles both upper/lowercase)
     * @return true if pressed
     */
    bool isKeyPressed(unsigned char key) const;
    
    /**
     * Check if special key is pressed
     * @param key GLUT special key code
     * @return true if pressed
     */
    bool isSpecialKeyPressed(int key) const;
    
    /**
     * Check if mouse button is pressed
     * @param button Button index
     * @return true if pressed
     */
    bool isMouseButtonPressed(int button) const;
    
    /**
     * Get mouse X coordinate
     * @return Mouse X position
     */
    int getMouseX() const;
    
    /**
     * Get mouse Y coordinate
     * @return Mouse Y position
     */
    int getMouseY() const;
    
    /**
     * Reset all input states to unpressed
     */
    void reset();
    
    /**
     * Get raw keys array (for direct access)
     * @return Pointer to keys array
     */
    const bool* getKeys() const;
};

#endif // INPUT_H
