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
      startTime(90.0f),
      bonusTimePerRing(10.0f),
      explosionActive(false),
      explosionTime(0),
      explosionX(0), explosionY(0), explosionZ(0),
      startX(0), startY(80), startZ(0),  // Start HIGHER up (y=80)
      levelWidth(500),
      levelLength(500) {
}

Level1::~Level1() {
    cleanup();
}

void Level1::init() {
    // Create player at a good starting position HIGH above the terrain
    player = new Player(startX, startY, startZ);
    
    // Create camera - positioned behind and above the plane, looking forward and slightly down
    camera = new Camera();
    camera->setFirstPerson(false);
    camera->setDistance(15.0f);
    camera->setHeight(5.0f);
    
    // Create lighting
    lighting = new Lighting();
    lighting->init();
    lighting->setNightMode(false);
    
    // Load 3D models first
    loadModels();
    
    // Create rings positioned between plane and terrain
    createRings();
    
    // Start timer
    timer.start(startTime);
    
    // Reset score
    score = 0;
    ringsCollected = 0;
    state = Level1State::PLAYING;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Level 1 initialized - Mountain Valley Challenge" << std::endl;
    std::cout << "Fly through the valley and collect " << totalRings << " rings!" << std::endl;
    std::cout << "Avoid crashing into the mountains!" << std::endl;
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
    
    // Create rings at mid-altitude (between player at y=80 and terrain at y=-100)
    // Rings at around y=50-70 range
    
    rings.push_back(new Collectible(0, 60, 50));       // Ring 1
    rings.push_back(new Collectible(30, 65, 100));     // Ring 2
    rings.push_back(new Collectible(-25, 55, 150));    // Ring 3
    rings.push_back(new Collectible(15, 70, 200));     // Ring 4
    rings.push_back(new Collectible(-35, 60, 250));    // Ring 5
    rings.push_back(new Collectible(20, 75, 300));     // Ring 6
    rings.push_back(new Collectible(-10, 65, 350));    // Ring 7
    rings.push_back(new Collectible(0, 70, 400));      // Ring 8
    
    totalRings = rings.size();
    
    // Load ring models
    std::string ringModelPath = findAssetPath("assets/rings/Engagement Ring.obj");
    std::string ringTexturePath = findAssetPath("assets/rings/Engagement Ring.jpg");
    
    for (size_t i = 0; i < rings.size(); i++) {
        if (i % 2 == 0) {
            rings[i]->setColor(1.0f, 0.85f, 0.0f);   // Gold
        } else {
            rings[i]->setColor(0.0f, 0.9f, 1.0f);    // Cyan
        }
        rings[i]->setPointValue(100 + (i * 25));
        rings[i]->setBonusTime(bonusTimePerRing);
        rings[i]->loadModel(ringModelPath, ringTexturePath, 0.06f);
    }
    
    std::cout << "Created " << totalRings << " rings along flight path" << std::endl;
}

void Level1::loadModels() {
    std::cout << "\n=== Loading 3D Models ===" << std::endl;
    
    // Load aircraft model
    if (player != nullptr) {
        std::cout << "\nLoading aircraft model..." << std::endl;
        std::string planePath = findAssetPath("assets/Japan Plane/14082_WWII_Plane_Japan_Kawasaki_Ki-61_v1_L2.obj");
        bool success = player->loadModel(planePath, 0.25f);
        if (!success) {
            std::cout << "Aircraft model not found, using primitive aircraft" << std::endl;
        }
    }
    
    // Load landscape model - position it as a ground plane below the player
    std::cout << "\nLoading landscape model..." << std::endl;
    std::string terrainPath = findAssetPath("assets/landscape/iceland.obj");
    
    // Position terrain as ground plane:
    // - Center at x=0, z=200 (ahead of player starting position)
    // - Y at 0 (ground level) - aircraft flies at y=80, ground at y=0
    // - This gives proper vertical separation between sky and ground
    Obstacle* landscape = new Obstacle(0, 0, 200, levelWidth, 1, levelLength, ObstacleType::GROUND);
    
    // Scale terrain large enough to cover the play area
    bool terrainLoaded = landscape->loadModel(terrainPath, 15.0f);
    
    if (terrainLoaded) {
        std::cout << "Landscape model loaded successfully!" << std::endl;
        std::cout << "Terrain positioned at ground level (Y=0), player at Y=80" << std::endl;
    } else {
        std::cout << "Landscape model not found, using flat ground" << std::endl;
    }
    
    obstacles.push_back(landscape);
    
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
    
    // Update player
    player->update(deltaTime, keys);
    
    // Update camera
    camera->update(player, deltaTime);
    
    // Update lighting
    lighting->update(deltaTime);
    
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
    
    // Check win condition
    if (ringsCollected >= totalRings) {
        state = Level1State::WON;
        timer.stop();
        std::cout << "\n*** VICTORY! All rings collected! ***" << std::endl;
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
}

void Level1::checkCollisions() {
    if (!player->isAlive()) return;
    
    float px = player->getX();
    float py = player->getY();
    float pz = player->getZ();
    float pr = player->getRadius();
    
    // Check ring collection
    for (auto* ring : rings) {
        if (!ring->isCollected()) {
            float dx = px - ring->getX();
            float dy = py - ring->getY();
            float dz = pz - ring->getZ();
            float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
            
            float collectRadius = pr + ring->getRadius() + 5.0f;
            
            if (dist < collectRadius) {
                ring->collect();
                ringsCollected++;
                score += ring->getPointValue();
                timer.addTime(ring->getBonusTime());
                
                std::cout << "Ring collected! " << ringsCollected << "/" << totalRings 
                          << " (+bonus time: " << ring->getBonusTime() << "s)" << std::endl;
            }
        }
    }
    
    // Crash if player goes too low (terrain is at y=0, crash when hitting ground)
    float groundLevel = 5.0f;  // Small buffer above actual ground
    
    if (py - pr < groundLevel) {
        triggerCrash(px, py, pz);
        return;
    }
    
    // Boundaries
    float boundaryX = levelWidth / 2.0f;
    float boundaryZ = 500.0f;
    
    if (std::abs(px) > boundaryX || pz < -50.0f || pz > boundaryZ) {
        std::cout << "Warning: Approaching level boundary!" << std::endl;
    }
    
    if (py > 200.0f) {
        std::cout << "Warning: Maximum altitude reached!" << std::endl;
    }
}

void Level1::triggerCrash(float x, float y, float z) {
    player->kill();
    state = Level1State::LOST;
    explosionActive = true;
    explosionTime = 0;
    explosionX = x;
    explosionY = y;
    explosionZ = z;
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
    
    // Render sky
    renderSky();
    
    // Render landscape/terrain
    for (auto* obstacle : obstacles) {
        obstacle->render();
    }
    
    // Render rings
    for (auto* ring : rings) {
        ring->render();
    }
    
    // Render player (only in third person)
    if (!camera->isFirstPerson() && player->isAlive()) {
        player->render();
    }
    
    // Render explosion if active
    if (explosionActive) {
        renderExplosion();
    }
    
    // Render HUD (2D overlay)
    renderHUD();
    
    // Render win/lose messages
    renderMessages();
    
    glutSwapBuffers();
}

void Level1::renderSky() {
    // Simple sky sphere
    glDisable(GL_LIGHTING);
    
    if (lighting->isNightMode()) {
        // Night sky - dark blue
        glColor3f(0.02f, 0.02f, 0.08f);
    } else {
        // Day sky - gradient blue
        float intensity = lighting->getSunIntensity();
        glColor3f(0.4f * intensity + 0.1f, 0.6f * intensity + 0.15f, 0.95f * intensity + 0.05f);
    }
    
    glPushMatrix();
    glTranslatef(player->getX(), player->getY(), player->getZ());
    glutSolidSphere(800.0, 24, 24);
    glPopMatrix();
    
    // Draw sun/moon
    glPushMatrix();
    float sunAngle = lighting->getDayTime() / 24.0f * 360.0f;
    float sunY = 400.0f * std::sin(sunAngle * M_PI / 180.0f);
    float sunX = 400.0f * std::cos(sunAngle * M_PI / 180.0f);
    
    glTranslatef(player->getX() + sunX, player->getY() + std::abs(sunY) + 100.0f, player->getZ() + 300);
    
    if (lighting->isNightMode()) {
        glColor3f(0.9f, 0.9f, 0.85f);  // Moon
        glutSolidSphere(25.0, 12, 12);
    } else {
        glColor3f(1.0f, 0.95f, 0.7f);  // Sun
        glutSolidSphere(30.0, 12, 12);
    }
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
    
    // Speed indicator
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(buffer, "Speed: %.0f%%", (player->getSpeed() / player->getSpeed()) * 100.0f);
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
    
    // Controls hint at bottom
    glColor3f(0.7f, 0.7f, 0.7f);
    sprintf(buffer, "W/S: Pitch | A/D: Roll | Q/E: Yaw | 1/2: Speed | C: Camera | N: Day/Night | R: Restart");
    glRasterPos2f(350, 20);
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
        
        glColor3f(1.0f, 1.0f, 1.0f);  // White color - fixed
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
        
        glColor3f(1.0f, 1.0f, 1.0f);  // White color - fixed
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

bool Level1::isWon() const {
    return state == Level1State::WON;
}

bool Level1::isLost() const {
    return state == Level1State::LOST;
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
}

void Level1::restart() {
    std::cout << "\nRestarting Level 1..." << std::endl;
    
    // Reset player
    player->reset(startX, startY, startZ);
    
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
    
    // Reset state
    state = Level1State::PLAYING;
    
    std::cout << "Level restarted! Collect " << totalRings << " rings!\n" << std::endl;
}

void Level1::handleMouse(int button, int state, int x, int y) {
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        // Toggle camera
        camera->toggle();
        std::cout << "Camera: " << (camera->isFirstPerson() ? "First Person" : "Third Person") << std::endl;
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
