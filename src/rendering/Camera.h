#ifndef CAMERA_H
#define CAMERA_H

// Forward declaration
class Player;

/**
 * @class Camera
 * @brief Manages first-person and third-person camera views
 * 
 * Follows the player aircraft in both camera modes.
 * First-person: Cockpit view, looking forward
 * Third-person: Behind and above the aircraft, with mouse orbit support
 */
class Camera {
private:
    // Camera position
    float posX, posY, posZ;
    
    // Look target
    float lookX, lookY, lookZ;
    
    // Up vector
    float upX, upY, upZ;
    
    // Camera mode
    bool firstPerson;
    
    // Third person settings
    float distance;     // Distance behind player
    float height;       // Height above player
    float smoothing;    // Camera smoothing factor (0-1)
    
    // Smooth transition variables
    float targetPosX, targetPosY, targetPosZ;
    
    // Mouse orbit control
    float orbitYaw;       // Horizontal orbit angle offset (degrees)
    float orbitPitch;     // Vertical orbit angle offset (degrees)
    float mouseSensitivity;
    int lastMouseX;
    int lastMouseY;
    bool mouseControlActive;
    
public:
    Camera();
    
    /**
     * Update camera to follow player
     * @param player The player to follow
     * @param deltaTime Time since last frame for smooth transitions
     */
    void update(const Player* player, float deltaTime);
    
    /**
     * Apply camera transformation (call before rendering scene)
     * Sets up gluLookAt with current camera state
     */
    void apply();
    
    /**
     * Toggle between first and third person
     */
    void toggle();
    
    /**
     * Set camera mode directly
     * @param fp true for first-person, false for third-person
     */
    void setFirstPerson(bool fp);
    
    /**
     * Check current camera mode
     * @return true if in first-person mode
     */
    bool isFirstPerson() const;
    
    /**
     * Set third-person camera distance
     * @param dist Distance behind player
     */
    void setDistance(float dist);
    
    /**
     * Set third-person camera height
     * @param h Height above player
     */
    void setHeight(float h);
    
    /**
     * Handle mouse motion for orbit control
     * @param x Current mouse X position
     * @param y Current mouse Y position
     */
    void handleMouseMotion(int x, int y);
    
    /**
     * Handle mouse button for starting/stopping orbit
     * @param button Mouse button (0=left, 1=middle, 2=right)
     * @param pressed True if button was pressed, false if released
     * @param x Mouse X position
     * @param y Mouse Y position
     */
    void handleMouseButton(int button, bool pressed, int x, int y);
    
    /**
     * Reset camera orbit angles to default
     */
    void resetOrbit();
    
    /**
     * Get camera position
     */
    float getX() const { return posX; }
    float getY() const { return posY; }
    float getZ() const { return posZ; }
};

#endif // CAMERA_H
