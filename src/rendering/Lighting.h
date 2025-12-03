#ifndef LIGHTING_H
#define LIGHTING_H

/**
 * @class Lighting
 * @brief Manages scene lighting with animated effects
 * 
 * Handles sun light with day/night cycle, lighthouse beams,
 * and special effects lighting (explosions).
 */
class Lighting {
private:
    // Day/night cycle
    float dayTime;          // 0-24 representing hour
    float daySpeed;         // How fast day progresses
    bool nightMode;         // True if in night mode
    
    // Sun properties
    float sunIntensity;     // Current sun brightness (0-1)
    float sunX, sunY, sunZ; // Sun position
    
    // Lighthouse properties (for night mode)
    float lighthouseAngle;  // Rotation angle of lighthouse beam
    float lighthouseSpeed;  // Rotation speed
    
    // Ambient light
    float ambientR, ambientG, ambientB;
    
public:
    Lighting();
    
    /**
     * Initialize OpenGL lighting
     */
    void init();
    
    /**
     * Update lighting based on time
     * @param deltaTime Time since last frame
     */
    void update(float deltaTime);
    
    /**
     * Apply lighting state to OpenGL
     */
    void apply();
    
    /**
     * Toggle between day and night mode
     */
    void toggleDayNight();
    
    /**
     * Set night mode directly
     * @param night true for night mode
     */
    void setNightMode(bool night);
    
    /**
     * Check if in night mode
     * @return true if night
     */
    bool isNightMode() const;
    
    /**
     * Create flash effect (for explosions)
     * @param intensity Flash brightness (0-1)
     */
    void flashEffect(float intensity);
    
    /**
     * Set day/night cycle speed
     * @param speed Speed multiplier (1.0 = normal)
     */
    void setDaySpeed(float speed);
    
    /**
     * Get current day time (0-24)
     */
    float getDayTime() const;
    
    /**
     * Get sun intensity (for rendering sun visual)
     */
    float getSunIntensity() const;
    
    /**
     * Get lighthouse beam angle (for rendering beam visual)
     */
    float getLighthouseAngle() const;
};

#endif // LIGHTING_H
