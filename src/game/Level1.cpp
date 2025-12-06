#include "Level1.h"
#include "../physics/Collision.h"
#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
// On Windows, GLEW must be included before GL headers
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper function to find asset path (checks multiple locations)
static std::string findAssetPath(const std::string& relativePath) {
    // List of possible base paths to check
    // Executable runs from out/build/x64-Debug/bin/, assets are in source root
    const char* basePaths[] = {
        "",                     // Current directory
        "assets/",              // Assets subfolder in current dir
        "../",                  // Parent directory
        "../../",               // Two levels up
        "../../../",            // Three levels up
        "../../../../",         // Four levels up (out/build/x64-Debug/bin -> source root)
        "../../../../../",      // Five levels up
        nullptr
    };
    
    for (int i = 0; basePaths[i] != nullptr; i++) {
        std::string fullPath = std::string(basePaths[i]) + relativePath;
        std::ifstream file(fullPath);
        if (file.good()) {
            std::cout << "Found asset at: " << fullPath << std::endl;
            return fullPath;
        }
    }
    
    // Return original path if not found (let loading fail with proper error)
    std::cout << "Asset not found in any location: " << relativePath << std::endl;
    return relativePath;
}

Level1::Level1()
    : state(Level1State::PLAYING),
      player(nullptr),
      camera(nullptr),
      lighting(nullptr),
      score(0),
      ringsCollected(0),
      totalRings(8),
      startTime(180.0f),     // Much more time for very slow speeds
      bonusTimePerRing(20.0f),  // More bonus time per ring
      explosionActive(false),
      explosionTime(0),
      explosionX(0), explosionY(0), explosionZ(0),
      startX(26.5f), startY(64.6f), startZ(361.3f),  // New starting position
      startYaw(189.0f),  // Starting yaw rotation
      levelWidth(500),
      levelLength(500),
      spawnProtectionTime(3.5f) {  // Longer spawn protection for smoother start
}

Level1::~Level1() {
    cleanup();
}

void Level1::init() {
    // Create player at a good starting position HIGH above the terrain
    player = new Player(startX, startY, startZ);
    player->reset(startX, startY, startZ, startYaw);  // Set initial yaw
    
    // Create camera - positioned behind the plane with improved tracking
    camera = new Camera();
    camera->setFirstPerson(false);
    camera->setDistance(20.0f);  // Further back for larger plane visibility
    camera->setHeight(7.0f);     // Higher for better overview
    
    // Create lighting
    lighting = new Lighting();
    lighting->init();
    lighting->setNightMode(false);
    
    // Load 3D models first
    std::cout << "DEBUG: About to call loadModels()..." << std::endl;
    loadModels();
    std::cout << "DEBUG: loadModels() completed successfully!" << std::endl;
    
    // Create lighthouses with rotating beams
    std::cout << "DEBUG: About to call createLighthouses()..." << std::endl;
    createLighthouses();
    std::cout << "DEBUG: createLighthouses() completed successfully!" << std::endl;
    
    // Create rings positioned between plane and terrain
    std::cout << "DEBUG: About to call createRings()..." << std::endl;
    createRings();
    std::cout << "DEBUG: createRings() completed successfully!" << std::endl;
    
    // Start timer
    timer.start(startTime);
    
    // Reset score
    score = 0;
    ringsCollected = 0;
    state = Level1State::PLAYING;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Level 1 - Mountain Valley Challenge" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "OBJECTIVE: Collect " << totalRings << " rings and avoid terrain!" << std::endl;
    std::cout << "WIN: Collect final ring + at least 4 total rings" << std::endl;
    std::cout << "\nIMPROVED CONTROLS:" << std::endl;
    std::cout << "  - Very slow starting speed (0.3) for excellent control" << std::endl;
    std::cout << "  - Press '2' to accelerate up to max speed (1.2)" << std::endl;
    std::cout << "  - Press '1' to slow down to min speed (0.15)" << std::endl;
    std::cout << "  - Plane is 50% larger for better visibility" << std::endl;
    std::cout << "  - More responsive pitch/yaw/roll controls" << std::endl;
    std::cout << "  - 3.5 seconds spawn protection (invincibility)" << std::endl;
    std::cout << "\nTIME: " << startTime << " seconds (+" << bonusTimePerRing << "s per ring)" << std::endl;
    std::cout << "LIGHTHOUSES: Located at (-50,0,250) and (60,0,50)" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

void Level1::createTerrain() {
    // Clear existing obstacles
    for (auto* obs : obstacles) {
        delete obs;
    }
    obstacles.clear();
    
    // The terrain is now handled by the landscape model loaded in loadModels()
    // We keep this function for any additional obstacles if needed
}

void Level1::createRings() {
    // Clear existing rings
    for (auto* ring : rings) {
        delete ring;
    }
    rings.clear();
    
    // Create 8 rings along the flight path from start toward the final ring
    // Player starts at (26.5, 64.6, 361.3) facing yaw=189 (roughly toward negative Z)
    // Final ring is at (1.73, 127.72, -109.42)
    
    // Rings along trajectory toward final ring (spreading from Z=361 down to Z=-109)
    rings.push_back(new Collectible(20.0f, 65.0f, 300.0f));      // Ring 1 - near start
    rings.push_back(new Collectible(15.0f, 70.0f, 220.0f));      // Ring 2
    rings.push_back(new Collectible(10.0f, 68.0f, 140.0f));      // Ring 3
    rings.push_back(new Collectible(-26.1736f, 56.2772f, 80.5068f));  // Ring 4 - debug pos 3
    rings.push_back(new Collectible(11.2544f, 75.3101f, -28.5208f)); // Ring 5 - debug pos 2
    rings.push_back(new Collectible(5.0f, 95.0f, -60.0f));       // Ring 6 - climbing toward final
    rings.push_back(new Collectible(5.68f, 82.23f, -81.31f));    // Ring 7 - original user pos
    rings.push_back(new Collectible(1.73182f, 127.721f, -109.424f)); // Ring 8 - FINAL (must collect to win)
    
    totalRings = rings.size();  // 8 rings
    
    // Load ring models
    std::string ringModelPath = findAssetPath("assets/rings/Engagement Ring.obj");
    std::string ringTexturePath = findAssetPath("assets/rings/Engagement Ring.jpg");
    
    for (size_t i = 0; i < rings.size(); i++) {
        // Alternate colors - final ring is special
        if (i == rings.size() - 1) {
            // Final ring - special bright gold (MUST collect this one)
            rings[i]->setColor(1.0f, 0.9f, 0.2f);
            rings[i]->setPointValue(500);  // Bonus for final
        } else if (i % 2 == 0) {
            rings[i]->setColor(1.0f, 0.85f, 0.0f);   // Gold
            rings[i]->setPointValue(100 + (i * 25));
        } else {
            rings[i]->setColor(0.0f, 0.9f, 1.0f);    // Cyan
            rings[i]->setPointValue(100 + (i * 25));
        }
        rings[i]->setBonusTime(bonusTimePerRing);
        rings[i]->loadModel(ringModelPath, ringTexturePath, 0.06f);
    }
    
    std::cout << "Created " << totalRings << " rings along flight path" << std::endl;
    std::cout << "Win condition: Collect any 4 rings INCLUDING the FINAL ring!" << std::endl;
}

void Level1::loadModels() {
    std::cout << "\n=== Loading 3D Models ===" << std::endl;
    
    // Load aircraft model
    if (player != nullptr) {
        std::cout << "\nLoading aircraft model..." << std::endl;
        std::string planePath = findAssetPath("assets/Japan Plane/14082_WWII_Plane_Japan_Kawasaki_Ki-61_v1_L2.obj");
        std::cout << "DEBUG: About to call player->loadModel()..." << std::endl;
        bool success = player->loadModel(planePath, 0.75f);  // Much larger plane for better visibility
        std::cout << "DEBUG: player->loadModel() returned: " << success << std::endl;
        if (!success) {
            std::cout << "Aircraft model not found, using primitive aircraft" << std::endl;
        }
        std::cout << "DEBUG: Aircraft model loading complete!" << std::endl;
    }
    
    // Load landscape model - position it as a ground plane below the player
    std::cout << "\nLoading landscape model..." << std::endl;
    std::cout << "DEBUG: About to call findAssetPath for iceland.obj..." << std::endl;
    std::string terrainPath = findAssetPath("assets/landscape/iceland.obj");
    std::cout << "DEBUG: terrainPath = " << terrainPath << std::endl;
    
    // Position terrain as ground plane:
    // - Center at x=0, z=200 (ahead of player starting position)
    // - Y at 0 (ground level) - aircraft flies at y=80, ground at y=0
    // - This gives proper vertical separation between sky and ground
    std::cout << "DEBUG: Creating Obstacle object..." << std::endl;
    Obstacle* landscape = new Obstacle(0, 0, 200, levelWidth, 1, levelLength, ObstacleType::GROUND);
    std::cout << "DEBUG: Obstacle created, about to load model..." << std::endl;
    
    // Scale terrain large enough to cover the play area
    bool terrainLoaded = landscape->loadModel(terrainPath, 15.0f);
    std::cout << "DEBUG: loadModel returned, terrainLoaded = " << terrainLoaded << std::endl;
    
    if (terrainLoaded) {
        std::cout << "Landscape model loaded successfully!" << std::endl;
        std::cout << "Terrain positioned at ground level (Y=0), player at Y=80" << std::endl;
    } else {
        std::cout << "Landscape model not found, using flat ground" << std::endl;
    }
    
    std::cout << "DEBUG: Adding landscape to obstacles..." << std::endl;
    obstacles.push_back(landscape);
    std::cout << "DEBUG: Landscape added to obstacles!" << std::endl;
    
    std::cout << "=== Model Loading Complete ===\n" << std::endl;
}

void Level1::update(float deltaTime, const bool* keys) {
    if (state != Level1State::PLAYING) {
        // Check for restart
        if (keys['r'] || keys['R']) {
            restart();
        }
        return;
    }
    
    // Update spawn protection
    if (spawnProtectionTime > 0) {
        spawnProtectionTime -= deltaTime;
        if (spawnProtectionTime <= 0) {
            spawnProtectionTime = 0;
            std::cout << "Spawn protection ended - be careful!" << std::endl;
        }
    }
    
    // Update player
    player->update(deltaTime, keys);
    
    // Update camera
    camera->update(player, deltaTime);
    
    // Update lighting
    lighting->update(deltaTime);
    
    // Update lighthouses
    updateLighthouses(deltaTime);
    
    // Update timer
    timer.update(deltaTime);
    
    // Update rings animation
    for (auto* ring : rings) {
        ring->update(deltaTime);
    }
    
    // Check collisions
    checkCollisions();
    
    // Update explosion if active
    if (explosionActive) {
        explosionTime += deltaTime;
        if (explosionTime > 2.0f) {
            explosionActive = false;
        }
    }
    
    // Check win condition - must collect the FINAL ring (last ring) + at least 4 total
    bool finalRingCollected = rings.size() > 0 && rings[rings.size() - 1]->isCollected();
    if (finalRingCollected && ringsCollected >= 4) {
        state = Level1State::WON;
        timer.stop();
        std::cout << "\n*** VICTORY! Final ring collected! ***" << std::endl;
        std::cout << "Final Score: " << score << std::endl;
    }
    
    // Check lose condition - time out
    if (timer.isExpired()) {
        state = Level1State::LOST;
        std::cout << "\n*** Time's up! Game Over! ***" << std::endl;
    }
    
    // Toggle day/night with N key
    static bool nKeyWasPressed = false;
    if ((keys['n'] || keys['N']) && !nKeyWasPressed) {
        toggleDayNight();
        nKeyWasPressed = true;
    } else if (!(keys['n'] || keys['N'])) {
        nKeyWasPressed = false;
    }
    
    // Debug: Print current position with G key
    static bool gKeyWasPressed = false;
    if ((keys['g'] || keys['G']) && !gKeyWasPressed) {
        std::cout << "\n=== DEBUG POSITION ===" << std::endl;
        std::cout << "Player Position: X=" << player->getX() 
                  << ", Y=" << player->getY() 
                  << ", Z=" << player->getZ() << std::endl;
        std::cout << "Player Rotation: Pitch=" << player->getPitch()
                  << ", Yaw=" << player->getYaw()
                  << ", Roll=" << player->getRoll() << std::endl;
        std::cout << "Player Speed: " << player->getSpeed() << std::endl;
        std::cout << "=====================\n" << std::endl;
        gKeyWasPressed = true;
    } else if (!(keys['g'] || keys['G'])) {
        gKeyWasPressed = false;
    }
}

void Level1::checkCollisions() {
    if (!player->isAlive()) return;
    
    float px = player->getX();
    float py = player->getY();
    float pz = player->getZ();
    float pr = player->getRadius();
    
    // ========== RING COLLECTION (always active) ==========
    for (auto* ring : rings) {
        if (!ring->isCollected()) {
            float dx = px - ring->getX();
            float dy = py - ring->getY();
            float dz = pz - ring->getZ();
            float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
            
            // Generous hitbox for more satisfying collection at slower speeds
            float collectRadius = pr + ring->getRadius() + 4.5f;
            
            if (dist < collectRadius) {
                ring->collect();
                ringsCollected++;
                score += ring->getPointValue();
                timer.addTime(ring->getBonusTime());
                
                // Play collection sound
                PlaySound(TEXT("assets/sounds/collect.wav"), NULL, SND_FILENAME | SND_ASYNC);
                
                std::cout << "Ring collected! " << ringsCollected << "/" << totalRings 
                          << " (+bonus time: " << ring->getBonusTime() << "s)" << std::endl;
            }
        }
    }
    
    // ========== SPAWN PROTECTION ==========
    if (spawnProtectionTime > 0) {
        return;
    }
    
    // ========== TERRAIN CRASH DETECTION (BVH-based with swept collision) ==========
    // Use a slightly larger collision radius for reliable but not overly harsh detection
    float collisionRadius = pr * 1.3f;  // 30% larger radius for safety margin
    
    // Calculate player velocity for swept collision
    float radYaw = player->getYaw() * M_PI / 180.0f;
    float radPitch = player->getPitch() * M_PI / 180.0f;
    float speed = player->getSpeed();
    
    float velX = std::sin(radYaw) * std::cos(radPitch) * speed;
    float velY = -std::sin(radPitch) * speed;
    float velZ = std::cos(radYaw) * std::cos(radPitch) * speed;
    
    // Calculate how far we moved this frame
    float moveDistance = std::sqrt(velX*velX + velY*velY + velZ*velZ);
    
    // Determine number of sweep samples based on movement distance
    // Check at least every half-radius along the movement path
    int numSamples = std::max(1, (int)(moveDistance / (collisionRadius * 0.5f)) + 1);
    numSamples = std::min(numSamples, 10);  // Cap at 10 samples for performance
    
    for (auto* obstacle : obstacles) {
        if (obstacle->hasModel()) {
            // Check current position
            if (obstacle->checkModelCollision(px, py, pz, collisionRadius)) {
                std::cout << "COLLISION at current position (" << px << ", " << py << ", " << pz << ")" << std::endl;
                triggerCrash(px, py, pz);
                return;
            }
            
            // Swept collision: check positions along the movement path
            // This prevents tunneling through thin geometry
            if (numSamples > 1) {
                for (int i = 1; i <= numSamples; i++) {
                    float t = (float)i / (float)numSamples;
                    // Check positions slightly ahead (in direction of movement)
                    float checkX = px + velX * t * 0.5f;  // Check half-step ahead
                    float checkY = py + velY * t * 0.5f;
                    float checkZ = pz + velZ * t * 0.5f;
                    
                    if (obstacle->checkModelCollision(checkX, checkY, checkZ, collisionRadius)) {
                        std::cout << "SWEPT COLLISION detected at sample " << i << "/" << numSamples << std::endl;
                        std::cout << "  Position: (" << checkX << ", " << checkY << ", " << checkZ << ")" << std::endl;
                        triggerCrash(px, py, pz);
                        return;
                    }
                }
            }
            
            // Also check behind (in case we just passed through something)
            float behindX = px - velX * 0.5f;
            float behindY = py - velY * 0.5f;
            float behindZ = pz - velZ * 0.5f;
            if (obstacle->checkModelCollision(behindX, behindY, behindZ, collisionRadius)) {
                std::cout << "COLLISION detected behind player (tunneling prevention)" << std::endl;
                triggerCrash(px, py, pz);
                return;
            }
        }
    }
    
    // Ground floor check - absolute minimum altitude
    float absoluteFloor = 2.0f;
    if (py < absoluteFloor) {
        std::cout << "Below absolute floor!" << std::endl;
        triggerCrash(px, py, pz);
        return;
    }
}

bool Level1::checkColorCollision() {
    // Skip color collision in first-person mode (can't see terrain at player position)
    if (camera->isFirstPerson()) {
        return false;
    }
    
    // Get player position in screen space and sample color from framebuffer
    float px = player->getX();
    float py = player->getY();
    float pz = player->getZ();
    
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLdouble screenX, screenY, screenZ;
    
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    
    // Project player position to screen coordinates
    gluProject(px, py, pz, modelview, projection, viewport, &screenX, &screenY, &screenZ);
    
    // Only check if player is on screen
    if (screenX < 10 || screenX >= viewport[2] - 10 || 
        screenY < 10 || screenY >= viewport[3] - 10) {
        return false;
    }
    
    // Sample points AHEAD of the player (in the direction of travel)
    // This detects terrain before we fully collide
    int sampleOffsets[][2] = {
        {0, -10},    // Below center (terrain usually below)
        {-8, -8},    // Bottom-left
        {8, -8},     // Bottom-right
        {0, -15},    // Further below
    };
    
    int greenCount = 0;
    
    for (int i = 0; i < 4; i++) {
        int sx = (int)screenX + sampleOffsets[i][0];
        int sy = (int)screenY + sampleOffsets[i][1];
        
        // Make sure we're within screen bounds
        if (sx < 0 || sx >= viewport[2] || sy < 0 || sy >= viewport[3]) {
            continue;
        }
        
        // Read pixel color at this position
        unsigned char pixel[3];
        glReadPixels(sx, sy, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
        
        float r = pixel[0] / 255.0f;
        float g = pixel[1] / 255.0f;
        float b = pixel[2] / 255.0f;
        
        // Strict terrain green detection:
        // Must be clearly green (not gray, not sky, not plane)
        bool isTerrain = false;
        
        // Strong green terrain (grass/vegetation)
        // Green must be significantly higher than red and blue
        if (g > 0.3f && g > r * 1.2f && g > b * 1.5f && b < 0.4f) {
            isTerrain = true;
        }
        
        // Olive/brown-green terrain (mountainside)
        if (r > 0.25f && r < 0.55f && g > 0.3f && g < 0.6f && b < 0.25f) {
            if (g > r * 0.9f && g > b * 1.3f) {
                isTerrain = true;
            }
        }
        
        if (isTerrain) {
            greenCount++;
        }
    }
    
    // Require multiple green pixels to trigger collision (reduces false positives)
    if (greenCount >= 2) {
        std::cout << "Terrain detected: " << greenCount << " green pixels" << std::endl;
        return true;
    }
    
    return false;
}

void Level1::triggerCrash(float x, float y, float z) {
    player->kill();
    state = Level1State::LOST;
    explosionActive = true;
    explosionTime = 0;
    explosionX = x;
    explosionY = y;
    explosionZ = z;
    
    // Play explosion/crash sound
    PlaySound(TEXT("assets/sounds/explosion.wav"), NULL, SND_FILENAME | SND_ASYNC);
    lighting->flashEffect(0.5f);
    std::cout << "\n*** CRASH! Game Over! ***" << std::endl;
}

void Level1::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Apply camera
    camera->apply();
    
    // Apply lighting
    lighting->apply();
    
    // Update lens flare based on camera looking at sun
    updateLensFlare();
    
    // Render sky (no lighting)
    glDisable(GL_LIGHTING);
    renderSky();
    
    // ==== GROUP ALL LIT GEOMETRY TOGETHER FOR PERFORMANCE ====
    glEnable(GL_LIGHTING);
    
    // Render landscape/terrain
    for (auto* obstacle : obstacles) {
        obstacle->render();
    }
    
    // Render lighthouses with their rotating beams
    renderLighthouses();
    
    // Render rings
    for (auto* ring : rings) {
        ring->render();
    }
    
    // Render player (only in third person)
    if (!camera->isFirstPerson() && player->isAlive()) {
        player->render();
    }
    
    // ==== END LIT GEOMETRY ====
    glDisable(GL_LIGHTING);
    
    // Render explosion if active (unlit)
    if (explosionActive) {
        renderExplosion();
    }
    
    // Render lens flare effect (after 3D, before HUD)
    renderLensFlare();
    
    // Render HUD (2D overlay)
    renderHUD();
    
    // Render win/lose messages
    renderMessages();
    
    glutSwapBuffers();
}

void Level1::renderSky() {
    // Perpetual sunset sky
    glDisable(GL_LIGHTING);
    
    float intensity = lighting->getSunIntensity();
    
    // Sunset sky gradient - warm oranges, pinks, and purples
    // Never dark, always beautiful sunset colors
    float warmth = 1.0f - intensity;  // More warm colors when sun is lower
    
    // Base sky color - gradient from warm sunset
    float r = 0.6f + warmth * 0.3f;   // Orange-red: 0.6 to 0.9
    float g = 0.4f + warmth * 0.1f;   // Less green: 0.4 to 0.5
    float b = 0.5f - warmth * 0.2f;   // Purple-blue: 0.5 to 0.3
    
    glColor3f(r, g, b);
    
    // Sky dome follows player
    glPushMatrix();
    glTranslatef(player->getX(), player->getY(), player->getZ());
    glutSolidSphere(800.0, 24, 24);
    glPopMatrix();
    
    // Draw sun - large, warm sunset sun
    float sunX = lighting->getSunX();
    float sunY = lighting->getSunY();
    float sunZ = lighting->getSunZ();
    
    // Position sun relative to player
    glPushMatrix();
    glTranslatef(player->getX() + sunX * 0.8f, sunY, player->getZ() + sunZ);
    
    // Sunset sun - large orange/red glow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    // Outer glow - very large, soft orange
    glColor4f(1.0f, 0.5f + warmth * 0.2f, 0.2f, 0.15f * intensity);
    glutSolidSphere(80.0, 16, 16);
    
    // Middle glow - orange-yellow
    glColor4f(1.0f, 0.7f, 0.3f, 0.25f * intensity);
    glutSolidSphere(50.0, 16, 16);
    
    // Sun core - bright yellow-white
    glColor4f(1.0f, 0.95f, 0.7f, intensity);
    glutSolidSphere(30.0, 16, 16);
    
    glDisable(GL_BLEND);
    glPopMatrix();
    
    glEnable(GL_LIGHTING);
}

void Level1::renderExplosion() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    int numParticles = 30;
    float maxRadius = 40.0f;
    
    for (int i = 0; i < numParticles; i++) {
        float angle = (float)i / numParticles * 360.0f;
        float radAngle = angle * M_PI / 180.0f;
        float elevation = ((i % 5) - 2) * 20.0f;
        
        float dist = explosionTime * maxRadius;
        float px = explosionX + std::cos(radAngle) * dist;
        float py = explosionY + std::sin(elevation * M_PI / 180.0f) * dist * 0.5f + explosionTime * 10.0f;
        float pz = explosionZ + std::sin(radAngle) * dist;
        
        float alpha = 1.0f - (explosionTime / 2.0f);
        float size = 3.0f + explosionTime * 5.0f;
        
        glPushMatrix();
        glTranslatef(px, py, pz);
        
        // Fire color gradient
        float colorPhase = explosionTime * 2.0f;
        glColor4f(1.0f, 0.6f - colorPhase * 0.3f, 0.1f, alpha);
        glutSolidSphere(size, 8, 8);
        
        glPopMatrix();
    }
    
    // Central flash
    if (explosionTime < 0.3f) {
        glPushMatrix();
        glTranslatef(explosionX, explosionY, explosionZ);
        float flash = 1.0f - (explosionTime / 0.3f);
        glColor4f(1.0f, 1.0f, 0.9f, flash);
        glutSolidSphere(15.0f + explosionTime * 60.0f, 16, 16);
        glPopMatrix();
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void Level1::renderHUD() {
    // Switch to 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // Draw HUD background panels
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Top-left panel (rings and score)
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(10, 640);
    glVertex2f(260, 640);
    glVertex2f(260, 710);
    glVertex2f(10, 710);
    glEnd();
    
    // Top-right panel (timer and speed)
    glBegin(GL_QUADS);
    glVertex2f(1020, 640);
    glVertex2f(1270, 640);
    glVertex2f(1270, 710);
    glVertex2f(1020, 710);
    glEnd();
    
    // Altitude indicator (left side)
    glBegin(GL_QUADS);
    glVertex2f(10, 300);
    glVertex2f(60, 300);
    glVertex2f(60, 600);
    glVertex2f(10, 600);
    glEnd();
    
    glDisable(GL_BLEND);
    
    // Draw text
    glColor3f(1.0f, 1.0f, 1.0f);
    
    char buffer[128];
    
    // Rings collected
    sprintf(buffer, "Rings: %d / %d", ringsCollected, totalRings);
    glRasterPos2f(20, 685);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Score
    sprintf(buffer, "Score: %d", score);
    glRasterPos2f(20, 658);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Timer with color based on urgency
    float timeLeft = timer.getTime();
    if (timeLeft < 15.0f) {
        glColor3f(1.0f, 0.2f, 0.2f);  // Red when low
    } else if (timeLeft < 30.0f) {
        glColor3f(1.0f, 1.0f, 0.2f);  // Yellow when medium
    } else {
        glColor3f(0.2f, 1.0f, 0.2f);  // Green when plenty
    }
    
    sprintf(buffer, "Time: %.1f", timeLeft);
    glRasterPos2f(1040, 685);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Speed indicator (show actual speed value and percentage of max)
    glColor3f(1.0f, 1.0f, 1.0f);
    float speedPercent = (player->getSpeed() / 1.2f) * 100.0f;  // maxSpeed is 1.2
    sprintf(buffer, "Speed: %.2f (%.0f%%)", player->getSpeed(), speedPercent);
    glRasterPos2f(1040, 658);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Altitude indicator
    glColor3f(0.3f, 1.0f, 0.3f);
    sprintf(buffer, "ALT");
    glRasterPos2f(20, 580);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    // Altitude bar
    float altPercent = std::min(1.0f, player->getY() / 200.0f);
    glColor3f(0.2f, 0.8f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(20, 320);
    glVertex2f(50, 320);
    glVertex2f(50, 320 + altPercent * 240);
    glVertex2f(20, 320 + altPercent * 240);
    glEnd();
    
    // Altitude number
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(buffer, "%.0f", player->getY());
    glRasterPos2f(18, 305);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    // Spawn protection indicator
    if (spawnProtectionTime > 0) {
        glColor3f(0.2f, 1.0f, 0.2f);
        sprintf(buffer, "SPAWN PROTECTION: %.1fs", spawnProtectionTime);
        glRasterPos2f(540, 680);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }
    
    // Controls hint at bottom
    glColor3f(0.7f, 0.7f, 0.7f);
    sprintf(buffer, "W/S: Pitch | A/D: Roll | Q/E: Yaw | 1/2: Speed | Space: Barrel Roll | C: Camera | N: Day/Night");
    glRasterPos2f(320, 20);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Level1::renderMessages() {
    if (state == Level1State::PLAYING) return;
    
    // Switch to 2D
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // Semi-transparent overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1280, 0);
    glVertex2f(1280, 720);
    glVertex2f(0, 720);
    glEnd();
    glDisable(GL_BLEND);
    
    char buffer[128];
    
    if (state == Level1State::WON) {
        // Victory message
        glColor3f(0.2f, 1.0f, 0.2f);
        sprintf(buffer, "VICTORY!");
        glRasterPos2f(560, 420);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }
        
        glColor3f(1.0f, 1.0f, 1.0f);  // White color
        sprintf(buffer, "All rings collected!");
        glRasterPos2f(520, 380);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
        sprintf(buffer, "Final Score: %d", score);
        glRasterPos2f(540, 340);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
        sprintf(buffer, "Time Remaining: %.1f seconds", timer.getTime());
        glRasterPos2f(490, 300);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    } else if (state == Level1State::LOST) {
        // Game over message
        glColor3f(1.0f, 0.2f, 0.2f);
        sprintf(buffer, "GAME OVER");
        glRasterPos2f(540, 420);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }
        
        glColor3f(1.0f, 1.0f, 1.0f);  // White color
        if (timer.isExpired()) {
            sprintf(buffer, "Time ran out!");
        } else {
            sprintf(buffer, "You crashed into the terrain!");
        }
        glRasterPos2f(500, 380);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
        sprintf(buffer, "Rings collected: %d / %d", ringsCollected, totalRings);
        glRasterPos2f(510, 340);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
        sprintf(buffer, "Score: %d", score);
        glRasterPos2f(570, 300);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }
    
    // Restart hint
    glColor3f(1.0f, 0.9f, 0.2f);
    sprintf(buffer, "Press R to restart");
    glRasterPos2f(540, 240);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Level1::updateLensFlare() {
    // Calculate camera look direction
    float camX = camera->getX();
    float camY = camera->getY();
    float camZ = camera->getZ();
    
    // Get player position as approximate look target
    float lookX = player->getX() - camX;
    float lookY = player->getY() - camY;
    float lookZ = player->getZ() - camZ;
    
    // For first person, use player's forward direction
    if (camera->isFirstPerson()) {
        float yaw = player->getYaw() * M_PI / 180.0f;
        float pitch = player->getPitch() * M_PI / 180.0f;
        lookX = std::sin(yaw) * std::cos(pitch);
        lookY = -std::sin(pitch);
        lookZ = std::cos(yaw) * std::cos(pitch);
    }
    
    // Calculate flare intensity
    float flare = lighting->calculateFlareIntensity(camX, camY, camZ, lookX, lookY, lookZ);
    lighting->setFlareIntensity(flare);
}

void Level1::renderLensFlare() {
    float flareIntensity = lighting->getFlareIntensity();
    
    if (flareIntensity < 0.01f) return;  // No visible flare
    
    // Switch to 2D overlay mode
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending for glow
    
    // Screen center
    float cx = 640.0f;
    float cy = 360.0f;
    
    // Main flare glow - covers much of screen when looking at sun
    float glowSize = 400.0f + flareIntensity * 300.0f;
    
    // Draw radial gradient glow
    glBegin(GL_TRIANGLE_FAN);
    // Center is bright
    glColor4f(1.0f, 0.95f, 0.8f, flareIntensity * 0.6f);
    glVertex2f(cx, cy);
    // Edges fade out
    glColor4f(1.0f, 0.9f, 0.6f, 0.0f);
    int segments = 32;
    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * M_PI;
        glVertex2f(cx + std::cos(angle) * glowSize, cy + std::sin(angle) * glowSize);
    }
    glEnd();
    
    // Lens flare artifacts - circles along a line from center
    // These simulate internal lens reflections
    float artifactColors[][4] = {
        {1.0f, 0.8f, 0.4f, 0.3f},   // Orange
        {0.4f, 0.8f, 1.0f, 0.2f},   // Cyan
        {1.0f, 0.5f, 0.8f, 0.15f},  // Pink
        {0.6f, 1.0f, 0.6f, 0.1f},   // Green
        {1.0f, 1.0f, 0.5f, 0.25f},  // Yellow
    };
    float artifactPositions[] = {0.3f, 0.5f, 0.7f, 1.2f, 1.5f};
    float artifactSizes[] = {60.0f, 40.0f, 80.0f, 30.0f, 50.0f};
    
    // Flare line goes from sun position toward opposite side of screen
    // Simplified: just use center outward
    for (int i = 0; i < 5; i++) {
        float pos = artifactPositions[i];
        float size = artifactSizes[i] * (0.5f + flareIntensity * 0.5f);
        
        // Position along flare line (from top-right toward bottom-left)
        float fx = cx + (cx * 0.5f) * (1.0f - pos * 0.8f);
        float fy = cy + (cy * 0.3f) * (1.0f - pos * 0.8f);
        
        glColor4f(artifactColors[i][0], artifactColors[i][1], 
                  artifactColors[i][2], artifactColors[i][3] * flareIntensity);
        
        // Draw circle
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(fx, fy);
        for (int j = 0; j <= 16; j++) {
            float angle = (float)j / 16 * 2.0f * M_PI;
            glVertex2f(fx + std::cos(angle) * size, fy + std::sin(angle) * size);
        }
        glEnd();
    }
    
    // Horizontal streak (anamorphic flare)
    if (flareIntensity > 0.3f) {
        float streakAlpha = (flareIntensity - 0.3f) * 0.5f;
        glBegin(GL_QUADS);
        glColor4f(1.0f, 0.95f, 0.9f, 0.0f);
        glVertex2f(0, cy - 20);
        glColor4f(1.0f, 0.95f, 0.9f, streakAlpha);
        glVertex2f(cx, cy - 5);
        glColor4f(1.0f, 0.95f, 0.9f, streakAlpha);
        glVertex2f(cx, cy + 5);
        glColor4f(1.0f, 0.95f, 0.9f, 0.0f);
        glVertex2f(0, cy + 20);
        glEnd();
        
        glBegin(GL_QUADS);
        glColor4f(1.0f, 0.95f, 0.9f, streakAlpha);
        glVertex2f(cx, cy - 5);
        glColor4f(1.0f, 0.95f, 0.9f, 0.0f);
        glVertex2f(1280, cy - 20);
        glColor4f(1.0f, 0.95f, 0.9f, 0.0f);
        glVertex2f(1280, cy + 20);
        glColor4f(1.0f, 0.95f, 0.9f, streakAlpha);
        glVertex2f(cx, cy + 5);
        glEnd();
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Level1::handleMouse(int button, int state, int x, int y) {
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        // Toggle camera
        camera->toggle();
        std::cout << "Camera: " << (camera->isFirstPerson() ? "First Person" : "Third Person") << std::endl;
    }
    
    // Pass mouse button events to camera for orbit control
    // Map GLUT button to camera button index
    int buttonIndex = 0;
    if (button == GLUT_LEFT_BUTTON) buttonIndex = 0;
    else if (button == GLUT_MIDDLE_BUTTON) buttonIndex = 1;
    else if (button == GLUT_RIGHT_BUTTON) buttonIndex = 2;
    
    camera->handleMouseButton(buttonIndex, state == GLUT_DOWN, x, y);
}

void Level1::handleMouseMotion(int x, int y) {
    // Pass mouse motion to camera for orbit control
    if (camera) {
        camera->handleMouseMotion(x, y);
    }
}

int Level1::getScore() const {
    return score;
}

float Level1::getTimeRemaining() const {
    return timer.getTime();
}

const char* Level1::getName() const {
    return "Level 1: Mountain Valley Challenge";
}

void Level1::toggleDayNight() {
    lighting->toggleDayNight();
    std::cout << "Mode: " << (lighting->isNightMode() ? "Night" : "Day") << std::endl;
}

bool Level1::isNightMode() const {
    return lighting->isNightMode();
}

bool Level1::isWon() const {
    return state == Level1State::WON;
}

bool Level1::isLost() const {
    return state == Level1State::LOST;
}

void Level1::createLighthouses() {
    std::cout << "\n=== Creating Lighthouses ===" << std::endl;
    
    std::string lighthousePath = findAssetPath("assets/lighthouse/obj/obj/lighthouse.obj");
    
    // Create 2 lighthouses at strategic positions along the flight path
    // Lighthouse 1: PROMINENT on mountain peak - VERY LARGE AND HIGHLY VISIBLE
    Obstacle* lighthouse1 = new Obstacle(-50.0f, 65.0f, 250.0f, 15, 45, 15, ObstacleType::BUILDING);  // Lower but more visible
    if (lighthouse1->loadModel(lighthousePath, 8.0f)) {  // MUCH bigger scale for visibility
        lighthouses.push_back(lighthouse1);
        std::cout << "Lighthouse 1 created at (-50, 65, 250) on mountain - SCALE 8.0x" << std::endl;
    } else {
        delete lighthouse1;
        std::cout << "Lighthouse 1 model not found, using larger primitive cylinder" << std::endl;
        lighthouse1 = new Obstacle(-50.0f, 65.0f, 250.0f, 20, 60, 20, ObstacleType::BUILDING);
        lighthouses.push_back(lighthouse1);
    }
    
    // Lighthouse 2: PROMINENT on another mountain peak - VERY LARGE AND HIGHLY VISIBLE
    Obstacle* lighthouse2 = new Obstacle(60.0f, 55.0f, 50.0f, 15, 45, 15, ObstacleType::BUILDING);  // Lower but more visible
    if (lighthouse2->loadModel(lighthousePath, 8.0f)) {  // MUCH bigger scale for visibility
        lighthouses.push_back(lighthouse2);
        std::cout << "Lighthouse 2 created at (60, 55, 50) on mountain - SCALE 8.0x" << std::endl;
    } else {
        delete lighthouse2;
        std::cout << "Lighthouse 2 model not found, using larger primitive cylinder" << std::endl;
        lighthouse2 = new Obstacle(60.0f, 55.0f, 50.0f, 20, 60, 20, ObstacleType::BUILDING);
        lighthouses.push_back(lighthouse2);
    }
    
    std::cout << "=== Lighthouses Created: " << lighthouses.size() << " ===\n" << std::endl;
}

void Level1::updateLighthouses(float deltaTime) {
    // Update lighthouse beam rotation in lighting system
    lighting->updateLighthouseBeam(deltaTime);
}

void Level1::renderLighthouses() {
    glEnable(GL_LIGHTING);
    
    // Enable lighthouse lights GL_LIGHT2 and GL_LIGHT3
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);
    
    float angle1 = lighting->getLighthouseAngle();
    float angle2 = angle1 + 180.0f;  // Second lighthouse 180° out of phase
    
    // Lighthouse 1 at (-50, 65, 250) on mountain
    float lh1X = -50.0f;
    float lh1Y = 65.0f;  // On mountain at visible height
    float lh1Z = 250.0f;
    
    // Lighthouse 2 at (60, 55, 50) on mountain
    float lh2X = 60.0f;
    float lh2Y = 55.0f;  // On mountain at visible height
    float lh2Z = 50.0f;
    
    // Configure GL_LIGHT2 for lighthouse 1 beam - POWERFUL AND REALISTIC
    float rad1 = angle1 * M_PI / 180.0f;
    GLfloat lh1Pos[] = {lh1X, lh1Y + 72.0f, lh1Z, 1.0f};  // Top of much taller lighthouse
    GLfloat lh1Dir[] = {std::sin(rad1), -0.2f, std::cos(rad1)};  // Realistic beam angle
    GLfloat lh1Diffuse[] = {2.5f, 2.3f, 2.0f, 1.0f};  // Very bright warm lighthouse beam
    GLfloat lh1Specular[] = {2.5f, 2.5f, 2.2f, 1.0f};  // Strong specular highlights
    
    glLightfv(GL_LIGHT2, GL_POSITION, lh1Pos);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, lh1Dir);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, lh1Diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, lh1Specular);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 28.0f);  // Wide powerful beam
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 15.0f);  // Realistic falloff
    glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 0.5f);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.0015f);
    glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.00002f);
    
    // Configure GL_LIGHT3 for lighthouse 2 beam - POWERFUL AND REALISTIC
    float rad2 = angle2 * M_PI / 180.0f;
    GLfloat lh2Pos[] = {lh2X, lh2Y + 72.0f, lh2Z, 1.0f};  // Top of lighthouse
    GLfloat lh2Dir[] = {std::sin(rad2), -0.2f, std::cos(rad2)};
    GLfloat lh2Diffuse[] = {2.2f, 2.5f, 2.3f, 1.0f};  // Very bright cool white beam
    GLfloat lh2Specular[] = {2.2f, 2.5f, 2.5f, 1.0f};
    
    glLightfv(GL_LIGHT3, GL_POSITION, lh2Pos);
    glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, lh2Dir);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, lh2Diffuse);
    glLightfv(GL_LIGHT3, GL_SPECULAR, lh2Specular);
    glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 28.0f);  // Wide powerful beam
    glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, 15.0f);  // Realistic falloff
    glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, 0.5f);
    glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.0015f);
    glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.00002f);
    
    // Render lighthouse structures
    for (auto* lighthouse : lighthouses) {
        lighthouse->render();
    }
    
    // Render visible light beams (volumetric effect)
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    
    // Lighthouse 1 beam visual - POWERFUL SEARCHLIGHT CONE
    glPushMatrix();
    glTranslatef(lh1X, lh1Y + 72.0f, lh1Z);  // From top of tall lighthouse
    glRotatef(angle1, 0.0f, 1.0f, 0.0f);
    glRotatef(-12.0f, 1.0f, 0.0f, 0.0f);  // Slight downward angle
    
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 0.95f, 0.8f, 1.0f);  // Intense warm light at source
    glVertex3f(0.0f, 0.0f, 0.0f);
    glColor4f(1.0f, 0.9f, 0.7f, 0.0f);  // Fade to transparent
    for (int i = 0; i <= 32; i++) {  // Very smooth circular beam
        float a = (float)i / 32 * 2.0f * M_PI;
        glVertex3f(std::sin(a) * 85.0f, 0.0f, std::cos(a) * 85.0f + 300.0f);  // Large powerful beam
    }
    glEnd();
    glPopMatrix();
    
    // Lighthouse 2 beam visual - POWERFUL SEARCHLIGHT CONE
    glPushMatrix();
    glTranslatef(lh2X, lh2Y + 72.0f, lh2Z);  // From top of tall lighthouse
    glRotatef(angle2, 0.0f, 1.0f, 0.0f);
    glRotatef(-12.0f, 1.0f, 0.0f, 0.0f);  // Slight downward angle
    
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.85f, 0.95f, 1.0f, 1.0f);  // Intense cool white at source
    glVertex3f(0.0f, 0.0f, 0.0f);
    glColor4f(0.8f, 0.9f, 1.0f, 0.0f);  // Fade to transparent
    for (int i = 0; i <= 32; i++) {  // Very smooth circular beam
        float a = (float)i / 32 * 2.0f * M_PI;
        glVertex3f(std::sin(a) * 85.0f, 0.0f, std::cos(a) * 85.0f + 300.0f);  // Large powerful beam
    }
    glEnd();
    glPopMatrix();
    
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void Level1::cleanup() {
    delete player;
    player = nullptr;
    
    delete camera;
    camera = nullptr;
    
    delete lighting;
    lighting = nullptr;
    
    for (auto* ring : rings) {
        delete ring;
    }
    rings.clear();
    
    for (auto* obstacle : obstacles) {
        delete obstacle;
    }
    obstacles.clear();
    
    for (auto* lighthouse : lighthouses) {
        delete lighthouse;
    }
    lighthouses.clear();
}

void Level1::restart() {
    std::cout << "\nRestarting Level 1..." << std::endl;
    
    // Reset player
    player->reset(startX, startY, startZ, startYaw);
    
    // Reset rings
    for (auto* ring : rings) {
        ring->reset();
    }
    
    // Reset score and timer
    score = 0;
    ringsCollected = 0;
    timer.start(startTime);
    
    // Reset explosion
    explosionActive = false;
    explosionTime = 0;
    
    // Reset camera orbit
    camera->resetOrbit();
    
    // Reset spawn protection
    spawnProtectionTime = 2.0f;
    
    // Reset state
    state = Level1State::PLAYING;
    
    std::cout << "Level restarted! Collect " << totalRings << " rings!\n" << std::endl;
}
