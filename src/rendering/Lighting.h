#ifndef LIGHTING_H
#define LIGHTING_H

/**
 * @class Lighting
 * @brief Manages scene lighting with animated effects
 * 
 * Handles sun light with day/night cycle, lighthouse beams,
 * lens flare effect, and special effects lighting (explosions).
 */
class Lighting {
private:
    // Day/night cycle
    float dayTime;          // 0-24 representing hour
    float daySpeed;         // How fast day progresses
    bool nightMode;         // True if in night mode
    
    // Sun properties
    float sunIntensity;     // Current sun brightness (0-1)
    float sunX, sunY, sunZ; // Sun position in world space
    float sunDistance;      // Distance from origin for sun position
    
    // Lighthouse properties (for night mode)
    float lighthouseAngle;  // Rotation angle of lighthouse beam
    float lighthouseSpeed;  // Rotation speed
    
    // Ambient light
    float ambientR, ambientG, ambientB;
    
    // Flash effect
    float flashIntensity;
    float flashDecay;
    
    // Lens flare
    float flareIntensity;   // Current lens flare intensity (0-1)
    
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
    
    /**
     * Get sun world position
     */
    float getSunX() const { return sunX; }
    float getSunY() const { return sunY; }
    float getSunZ() const { return sunZ; }
    
    /**
     * Calculate lens flare intensity based on camera looking at sun
     * @param camX, camY, camZ Camera position
     * @param lookX, lookY, lookZ Camera look direction (normalized)
     * @return flare intensity 0-1
     */
    float calculateFlareIntensity(float camX, float camY, float camZ,
                                   float lookX, float lookY, float lookZ) const;
    
    /**
     * Get current flare intensity
     */
    float getFlareIntensity() const { return flareIntensity; }
    
    /**
     * Set flare intensity (called after calculating)
     */
    void setFlareIntensity(float intensity) { flareIntensity = intensity; }
};

#endif // LIGHTING_H
