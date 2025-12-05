#include "Level1.h"
#include "../physics/Collision.h"
#include <cmath>
#include <cstdio>
#include <iostream>

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

Level1::Level1()
    : state(Level1State::PLAYING),
      player(nullptr),
      camera(nullptr),
      lighting(nullptr),
      score(0),
      ringsCollected(0),
      totalRings(12),
      startTime(60.0f),
      bonusTimePerRing(5.0f),
      explosionActive(false),
      explosionTime(0),
      explosionX(0), explosionY(0), explosionZ(0),
      startX(0), startY(20), startZ(-50),
      levelWidth(400),
      levelLength(600) {
}

Level1::~Level1() {
    cleanup();
}

void Level1::init() {
    // Create player
    player = new Player(startX, startY, startZ);
    
    // Create camera
    camera = new Camera();
    camera->setFirstPerson(false);
    camera->setDistance(18.0f);
    camera->setHeight(6.0f);
    
    // Create lighting
    lighting = new Lighting();
    lighting->init();
    lighting->setNightMode(false);
    
    // Create terrain and rings
    createTerrain();
    createRings();
    
    // Load 3D models (with error handling/fallback to primitives)
    loadModels();
    
    // Start timer
    timer.start(startTime);
    
    // Reset score
    score = 0;
    ringsCollected = 0;
    state = Level1State::PLAYING;
    
    std::cout << "Level 1 initialized - Terrain Navigation Challenge" << std::endl;
    std::cout << "Collect " << totalRings << " rings before time runs out!" << std::endl;
}

void Level1::createTerrain() {
    // Clear existing obstacles
    for (auto* obs : obstacles) {
        delete obs;
    }
    obstacles.clear();
    
    // Create ground plane
    obstacles.push_back(new Obstacle(0, 0, 0, levelWidth, 1, levelLength, ObstacleType::GROUND));
    
    // Create mountains in a pattern that creates a canyon/path
    // Left side mountains
    obstacles.push_back(new Obstacle(-80, 0, 0, 50, 60, 50, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(-90, 0, 80, 45, 55, 45, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(-75, 0, 160, 55, 70, 55, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(-85, 0, 240, 50, 50, 50, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(-70, 0, 320, 60, 65, 60, ObstacleType::MOUNTAIN));
    
    // Right side mountains
    obstacles.push_back(new Obstacle(80, 0, 20, 55, 55, 55, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(85, 0, 100, 50, 65, 50, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(75, 0, 180, 45, 50, 45, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(90, 0, 260, 55, 60, 55, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(70, 0, 340, 50, 70, 50, ObstacleType::MOUNTAIN));
    
    // Center obstacles (make it challenging)
    obstacles.push_back(new Obstacle(20, 0, 50, 35, 45, 35, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(-25, 0, 120, 30, 40, 30, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(30, 0, 200, 35, 50, 35, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(-15, 0, 280, 40, 45, 40, ObstacleType::MOUNTAIN));
    
    // Far mountains (background/boundaries)
    obstacles.push_back(new Obstacle(-150, 0, 150, 80, 100, 80, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(150, 0, 150, 75, 90, 75, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(-160, 0, 350, 85, 110, 85, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(160, 0, 350, 70, 95, 70, ObstacleType::MOUNTAIN));
    obstacles.push_back(new Obstacle(0, 0, 450, 100, 80, 100, ObstacleType::MOUNTAIN));
    
    // Some smaller rocks for detail
    obstacles.push_back(new Obstacle(-40, 0, 30, 15, 20, 15, ObstacleType::ROCK));
    obstacles.push_back(new Obstacle(50, 0, 150, 12, 15, 12, ObstacleType::ROCK));
    obstacles.push_back(new Obstacle(-50, 0, 220, 18, 22, 18, ObstacleType::ROCK));
}

void Level1::createRings() {
    // Clear existing rings
    for (auto* ring : rings) {
        delete ring;
    }
    rings.clear();
    
    // Create rings along a winding path through the canyon
    // Ring positions designed to guide player through safe path
    
    // Starting area rings (easier)
    rings.push_back(new Collectible(0, 20, 0));        // Ring 1 - Right at start
    rings.push_back(new Collectible(-10, 22, 40));    // Ring 2 - Slight left
    rings.push_back(new Collectible(15, 18, 80));     // Ring 3 - Go right
    
    // Middle section (more challenging navigation)
    rings.push_back(new Collectible(-20, 25, 130));   // Ring 4 - Left around obstacle
    rings.push_back(new Collectible(25, 20, 170));    // Ring 5 - Right side
    rings.push_back(new Collectible(0, 30, 210));     // Ring 6 - Center, higher
    rings.push_back(new Collectible(-30, 22, 250));   // Ring 7 - Left side
    
    // Final section
    rings.push_back(new Collectible(20, 25, 290));    // Ring 8
    rings.push_back(new Collectible(-10, 28, 330));   // Ring 9
    rings.push_back(new Collectible(10, 22, 370));    // Ring 10
    rings.push_back(new Collectible(-5, 30, 400));    // Ring 11
    rings.push_back(new Collectible(0, 25, 430));     // Ring 12 - Final ring
    
    totalRings = rings.size();
    
    // Set varied colors for visual interest
    for (size_t i = 0; i < rings.size(); i++) {
        // Alternate between gold and cyan colors
        if (i % 2 == 0) {
            rings[i]->setColor(1.0f, 0.9f, 0.0f);  // Gold
        } else {
            rings[i]->setColor(0.0f, 1.0f, 0.9f);  // Cyan
        }
    }
}

void Level1::loadModels() {
    std::cout << "\n=== Loading 3D Models ===" << std::endl;
    
    // Load aircraft model for player
    if (player != nullptr) {
        std::cout << "\nAttempting to load aircraft model..." << std::endl;
        bool success = player->loadModel("assets/plane 1.obj", 0.5f);
        if (!success) {
            std::cout << "Aircraft model failed to load, using primitive fallback" << std::endl;
        }
    }
    
    // Load ring model and texture for all collectibles
    std::cout << "\nAttempting to load ring models..." << std::endl;
    int ringLoadCount = 0;
    for (auto* ring : rings) {
        if (ring != nullptr) {
            bool success = ring->loadModel("assets/Engagement Ring.obj", "assets/Engagement Ring.jpg", 1.0f);
            if (success) ringLoadCount++;
        }
    }
    std::cout << "Loaded " << ringLoadCount << "/" << rings.size() << " ring models successfully" << std::endl;
    
    // Load terrain model for mountains (optional - only for specific obstacles)
    // For now, we'll keep mountains as primitives for variety
    // Uncomment below if you want to use iceland.obj for some mountains
    /*
    std::cout << "\nAttempting to load terrain models..." << std::endl;
    int terrainLoadCount = 0;
    for (auto* obs : obstacles) {
        if (obs != nullptr && obs->getType() == ObstacleType::MOUNTAIN) {
            // Only load for first few mountains as example
            if (terrainLoadCount < 3) {
                bool success = obs->loadModel("assets/iceland.obj", 5.0f);
                if (success) terrainLoadCount++;
            }
        }
    }
    std::cout << "Loaded " << terrainLoadCount << " terrain models successfully" << std::endl;
    */
    
    std::cout << "=== Model Loading Complete ===\n" << std::endl;
}

void Level1::update(float deltaTime, const bool* keys) {
    // Handle pause
    if (keys['p'] || keys['P']) {
        // Toggle pause handled elsewhere
    }
    
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
        std::cout << "VICTORY! All rings collected!" << std::endl;
        std::cout << "Final Score: " << score << std::endl;
    }
    
    // Check lose condition - time out
    if (timer.isExpired()) {
        state = Level1State::LOST;
        std::cout << "Time's up! Game Over!" << std::endl;
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
    
    // Check ring collection (sphere-sphere)
    for (auto* ring : rings) {
        if (!ring->isCollected()) {
            if (checkSphereCollision(px, py, pz, pr,
                                    ring->getX(), ring->getY(), ring->getZ(), ring->getRadius())) {
                // Collect the ring!
                ring->collect();
                ringsCollected++;
                score += ring->getPointValue();
                timer.addTime(ring->getBonusTime());
                
                std::cout << "Ring collected! " << ringsCollected << "/" << totalRings 
                          << " - Bonus time: +" << ring->getBonusTime() << "s" << std::endl;
            }
        }
    }
    
    // Check obstacle collisions (sphere-AABB)
    for (auto* obstacle : obstacles) {
        // Skip ground for now - handle separately
        if (obstacle->isGround()) {
            // Check if player is below ground level
            if (py - pr < obstacle->getY()) {
                // Crashed into ground!
                triggerCrash(px, py, pz);
                return;
            }
            continue;
        }
        
        // Check mountain collision using sphere-AABB
        if (checkSphereAABBCollision(px, py, pz, pr,
                                     obstacle->getMinX(), obstacle->getMaxX(),
                                     obstacle->getMinY(), obstacle->getMaxY(),
                                     obstacle->getMinZ(), obstacle->getMaxZ())) {
            // For mountains, do a more precise check using cone shape
            if (obstacle->getType() == ObstacleType::MOUNTAIN) {
                // Approximate: check if player is inside the cone
                float heightAboveBase = py - obstacle->getY();
                float coneRadius = obstacle->getRadiusAtHeight(py);
                
                if (heightAboveBase > 0 && heightAboveBase < obstacle->getHeight()) {
                    float distFromCenter = std::sqrt(
                        (px - obstacle->getX()) * (px - obstacle->getX()) +
                        (pz - obstacle->getZ()) * (pz - obstacle->getZ())
                    );
                    
                    if (distFromCenter < coneRadius + pr * 0.5f) {
                        triggerCrash(px, py, pz);
                        return;
                    }
                }
            } else {
                // Regular AABB collision
                triggerCrash(px, py, pz);
                return;
            }
        }
    }
    
    // Check level boundaries
    if (std::abs(px) > levelWidth / 2.0f || pz > levelLength / 2.0f || pz < -100) {
        // Out of bounds - could be lose condition or just push back
        // For now, just note it
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
    std::cout << "CRASH! Game Over!" << std::endl;
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
    
    // Render terrain/obstacles
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
        glColor3f(0.05f, 0.05f, 0.15f);
    } else {
        // Day sky - light blue
        float intensity = lighting->getSunIntensity();
        glColor3f(0.4f * intensity + 0.1f, 0.6f * intensity + 0.1f, 0.9f * intensity + 0.1f);
    }
    
    glPushMatrix();
    glTranslatef(player->getX(), player->getY(), player->getZ());
    glutSolidSphere(500.0, 20, 20);
    glPopMatrix();
    
    // Draw sun/moon
    glPushMatrix();
    float sunAngle = lighting->getDayTime() / 24.0f * 360.0f;
    float sunY = 300.0f * std::sin(sunAngle * M_PI / 180.0f);
    float sunX = 300.0f * std::cos(sunAngle * M_PI / 180.0f);
    
    glTranslatef(player->getX() + sunX, player->getY() + sunY, player->getZ() + 200);
    
    if (lighting->isNightMode()) {
        glColor3f(0.9f, 0.9f, 0.8f);  // Moon
    } else {
        glColor3f(1.0f, 1.0f, 0.8f);  // Sun
    }
    glutSolidSphere(20.0, 10, 10);
    glPopMatrix();
    
    glEnable(GL_LIGHTING);
}

void Level1::renderExplosion() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    int numParticles = 20;
    float maxRadius = 30.0f;
    
    for (int i = 0; i < numParticles; i++) {
        float angle = (float)i / numParticles * 360.0f;
        float radAngle = angle * M_PI / 180.0f;
        float elevation = ((i % 3) - 1) * 30.0f;  // Vary height
        
        float dist = explosionTime * maxRadius;
        float px = explosionX + std::cos(radAngle) * dist;
        float py = explosionY + std::sin(elevation * M_PI / 180.0f) * dist * 0.5f;
        float pz = explosionZ + std::sin(radAngle) * dist;
        
        float alpha = 1.0f - (explosionTime / 2.0f);
        float size = 2.0f + explosionTime * 3.0f;
        
        glPushMatrix();
        glTranslatef(px, py, pz);
        
        // Fire color gradient
        float colorPhase = explosionTime * 2.0f;
        glColor4f(1.0f, 0.5f - colorPhase * 0.3f, 0.0f, alpha);
        glutSolidSphere(size, 8, 8);
        
        glPopMatrix();
    }
    
    // Central flash
    if (explosionTime < 0.3f) {
        glPushMatrix();
        glTranslatef(explosionX, explosionY, explosionZ);
        float flash = 1.0f - (explosionTime / 0.3f);
        glColor4f(1.0f, 1.0f, 0.8f, flash);
        glutSolidSphere(10.0f + explosionTime * 50.0f, 12, 12);
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(10, 650);
    glVertex2f(250, 650);
    glVertex2f(250, 710);
    glVertex2f(10, 710);
    glEnd();
    
    // Top-right panel (timer)
    glBegin(GL_QUADS);
    glVertex2f(1030, 650);
    glVertex2f(1270, 650);
    glVertex2f(1270, 710);
    glVertex2f(1030, 710);
    glEnd();
    
    glDisable(GL_BLEND);
    
    // Draw text
    glColor3f(1.0f, 1.0f, 1.0f);
    
    // Rings collected
    char buffer[128];
    sprintf(buffer, "Rings: %d / %d", ringsCollected, totalRings);
    glRasterPos2f(20, 690);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Score
    sprintf(buffer, "Score: %d", score);
    glRasterPos2f(20, 665);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Timer with color based on urgency
    float timeLeft = timer.getTime();
    if (timeLeft < 10.0f) {
        glColor3f(1.0f, 0.2f, 0.2f);  // Red when low
    } else if (timeLeft < 20.0f) {
        glColor3f(1.0f, 1.0f, 0.2f);  // Yellow when medium
    } else {
        glColor3f(0.2f, 1.0f, 0.2f);  // Green when plenty
    }
    
    sprintf(buffer, "Time: %.1f", timeLeft);
    glRasterPos2f(1050, 690);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Speed indicator
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(buffer, "Speed: %.0f%%", (player->getSpeed() / 2.0f) * 100.0f);
    glRasterPos2f(1050, 665);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Controls hint at bottom
    glColor3f(0.7f, 0.7f, 0.7f);
    sprintf(buffer, "WASD: Fly | QE: Turn | C: Camera | N: Day/Night | R: Restart | ESC: Quit");
    glRasterPos2f(400, 20);
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
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
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
        glRasterPos2f(580, 400);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }
        
        glColor3f(1.0f, 1.0f, 1.0f);
        sprintf(buffer, "All rings collected!");
        glRasterPos2f(530, 360);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
        sprintf(buffer, "Final Score: %d", score);
        glRasterPos2f(550, 320);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
        sprintf(buffer, "Time Remaining: %.1f seconds", timer.getTime());
        glRasterPos2f(500, 280);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    } else if (state == Level1State::LOST) {
        // Game over message
        glColor3f(1.0f, 0.2f, 0.2f);
        sprintf(buffer, "GAME OVER");
        glRasterPos2f(560, 400);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }
        
        glColor3f(1.0f, 1.0f, 1.0f);
        if (timer.isExpired()) {
            sprintf(buffer, "Time ran out!");
        } else {
            sprintf(buffer, "You crashed!");
        }
        glRasterPos2f(560, 360);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
        sprintf(buffer, "Rings collected: %d / %d", ringsCollected, totalRings);
        glRasterPos2f(520, 320);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
        sprintf(buffer, "Score: %d", score);
        glRasterPos2f(570, 280);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }
    
    // Restart hint
    glColor3f(0.8f, 0.8f, 0.2f);
    sprintf(buffer, "Press R to restart");
    glRasterPos2f(550, 220);
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
    std::cout << "Restarting Level 1..." << std::endl;
    
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
    
    std::cout << "Level restarted! Collect " << totalRings << " rings!" << std::endl;
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
    return "Level 1: Terrain Navigation";
}

void Level1::toggleDayNight() {
    lighting->toggleDayNight();
    std::cout << "Mode: " << (lighting->isNightMode() ? "Night" : "Day") << std::endl;
}

bool Level1::isNightMode() const {
    return lighting->isNightMode();
}
