#ifndef TIMER_H
#define TIMER_H

/**
 * @class Timer
 * @brief Game timer for countdown and time-based mechanics
 * 
 * Manages the countdown timer for Level 1. Supports starting with
 * a set time, updating, adding bonus time, and checking expiration.
 */
class Timer {
private:
    float timeRemaining;  // Seconds remaining
    bool running;         // Whether timer is active
    
public:
    Timer();
    
    /**
     * Start the timer with specified seconds
     * @param seconds Initial countdown time
     */
    void start(float seconds);
    
    /**
     * Update timer (call each frame)
     * @param deltaTime Time elapsed since last frame
     */
    void update(float deltaTime);
    
    /**
     * Add bonus time (e.g., when collecting ring)
     * @param seconds Bonus seconds to add
     */
    void addTime(float seconds);
    
    /**
     * Stop the timer
     */
    void stop();
    
    /**
     * Reset timer to initial state
     */
    void reset();
    
    /**
     * Check if timer has expired
     * @return true if time ran out
     */
    bool isExpired() const;
    
    /**
     * Check if timer is currently running
     * @return true if running
     */
    bool isRunning() const;
    
    /**
     * Get remaining time
     * @return Seconds remaining (clamped to 0)
     */
    float getTime() const;
};

#endif // TIMER_H
