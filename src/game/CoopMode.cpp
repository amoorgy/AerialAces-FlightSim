#include "CoopMode.h"
#include "../physics/Collision.h"
#include <cmath>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CoopMode::CoopMode()
    : state(CoopState::PLAYING),
      player1(nullptr),
      player2(nullptr),
      player1Health(100),
      player2Health(100),
      maxHealth(100),
      player1Ammo(10),
      player2Ammo(10),
      maxAmmo(10),
      reloadTime(5.0f),
      player1ReloadTimer(0),
      player2ReloadTimer(0),
      player1FireCooldown(0),
      player2FireCooldown(0),
      camera1(nullptr),
      camera2(nullptr),
      lighting(nullptr),
      player1Score(0),
      player2Score(0),
      player1SpacePressed(false),
      player2SpacePressed(false),
      arenaSize(500.0f) {
}

CoopMode::~CoopMode() {
    cleanup();
}

void CoopMode::init() {
    std::cout << "========================================" << std::endl;
    std::cout << "      CO-OP DOGFIGHT MODE              " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Player 1 (Red): WASD + QE + F to fire" << std::endl;
    std::cout << "Player 2 (Blue): IJKL + UO + P to fire" << std::endl;
    std::cout << std::endl;
    
    // Create players at opposite ends of arena
    player1 = new Player(-200, 120, 0);
    player1->reset(-200, 120, 0, 90);  // Facing right
    
    player2 = new Player(200, 120, 0);
    player2->reset(200, 120, 0, 270);  // Facing left
    
    // Create cameras
    camera1 = new Camera();
    camera1->setFirstPerson(false);
    camera1->setDistance(25.0f);
    camera1->setHeight(8.0f);
    
    camera2 = new Camera();
    camera2->setFirstPerson(false);
    camera2->setDistance(25.0f);
    camera2->setHeight(8.0f);
    
    // Create lighting
    lighting = new Lighting();
    lighting->init();
    lighting->setNightMode(false);
    
    // Create arena
    createArena();
    
    std::cout << "Co-op mode initialized!" << std::endl;
}

void CoopMode::createArena() {
    // Ground plane
    Obstacle* ground = new Obstacle(0, -5, 0, arenaSize * 2, 10, arenaSize * 2, ObstacleType::GROUND);
    ground->setColor(0.3f, 0.5f, 0.3f);
    obstacles.push_back(ground);
    
    // Arena walls (invisible boundaries)
    // We'll check boundaries in collision detection instead
    
    // Add some obstacles for cover
    Obstacle* obstacle1 = new Obstacle(0, 50, 0, 30, 30, 30, ObstacleType::BUILDING);
    obstacle1->setColor(0.6f, 0.6f, 0.6f);
    obstacles.push_back(obstacle1);
    
    Obstacle* obstacle2 = new Obstacle(-100, 40, -100, 25, 40, 25, ObstacleType::BUILDING);
    obstacle2->setColor(0.5f, 0.5f, 0.6f);
    obstacles.push_back(obstacle2);
    
    Obstacle* obstacle3 = new Obstacle(100, 40, 100, 25, 40, 25, ObstacleType::BUILDING);
    obstacle3->setColor(0.6f, 0.5f, 0.5f);
    obstacles.push_back(obstacle3);
}

void CoopMode::update(float deltaTime, const bool* keys) {
    if (state != CoopState::PLAYING) {
        return;
    }
    
    // Update players
    updatePlayer1(deltaTime, keys);
    updatePlayer2(deltaTime, keys);
    
    // Update cameras
    if (camera1) camera1->update(player1, deltaTime);
    if (camera2) camera2->update(player2, deltaTime);
    
    // Update missiles
    updateMissiles(deltaTime);
    
    // Update reload timers
    if (player1Ammo <= 0 && player1ReloadTimer < reloadTime) {
        player1ReloadTimer += deltaTime;
        if (player1ReloadTimer >= reloadTime) {
            player1Ammo = maxAmmo;
            player1ReloadTimer = 0;
            std::cout << "Player 1 reloaded!" << std::endl;
        }
    }
    
    if (player2Ammo <= 0 && player2ReloadTimer < reloadTime) {
        player2ReloadTimer += deltaTime;
        if (player2ReloadTimer >= reloadTime) {
            player2Ammo = maxAmmo;
            player2ReloadTimer = 0;
            std::cout << "Player 2 reloaded!" << std::endl;
        }
    }
    
    // Update fire cooldowns
    if (player1FireCooldown > 0) player1FireCooldown -= deltaTime;
    if (player2FireCooldown > 0) player2FireCooldown -= deltaTime;
    
    // Check collisions
    checkCollisions();
    
    // Check win conditions
    if (player1Health <= 0) {
        state = CoopState::PLAYER2_WON;
        std::cout << "Player 2 wins!" << std::endl;
    } else if (player2Health <= 0) {
        state = CoopState::PLAYER1_WON;
        std::cout << "Player 1 wins!" << std::endl;
    }
}

void CoopMode::updatePlayer1(float deltaTime, const bool* keys) {
    if (!player1) return;
    
    // Player 1 controls: WASD + QE + Space
    bool p1Keys[256] = {false};
    
    // Map Player 1 controls
    p1Keys['w'] = keys['w'] || keys['W'];
    p1Keys['s'] = keys['s'] || keys['S'];
    p1Keys['a'] = keys['a'] || keys['A'];
    p1Keys['d'] = keys['d'] || keys['D'];
    p1Keys['q'] = keys['q'] || keys['Q'];
    p1Keys['e'] = keys['e'] || keys['E'];
    p1Keys['1'] = keys['1'];
    p1Keys['2'] = keys['2'];
    p1Keys[' '] = keys[' '];
    
    // Fire with F key
    if ((keys['f'] || keys['F']) && player1FireCooldown <= 0) {
        fireMissilePlayer1();
    }
    
    player1->update(deltaTime, p1Keys);
    
    // Keep player in bounds
    float px, py, pz;
    player1->getPosition(px, py, pz);
    if (std::abs(px) > arenaSize) player1->setX(px > 0 ? arenaSize : -arenaSize);
    if (std::abs(pz) > arenaSize) player1->setZ(pz > 0 ? arenaSize : -arenaSize);
    if (py < 10) player1->setY(10);
    if (py > 250) player1->setY(250);
}

void CoopMode::updatePlayer2(float deltaTime, const bool* keys) {
    if (!player2) return;
    
    // Player 2 controls: IJKL + UO + RightShift
    bool p2Keys[256] = {false};
    
    // Map Player 2 controls
    p2Keys['w'] = keys['i'] || keys['I'];  // I -> pitch up
    p2Keys['s'] = keys['k'] || keys['K'];  // K -> pitch down
    p2Keys['a'] = keys['j'] || keys['J'];  // J -> roll left
    p2Keys['d'] = keys['l'] || keys['L'];  // L -> roll right
    p2Keys['q'] = keys['u'] || keys['U'];  // U -> yaw left
    p2Keys['e'] = keys['o'] || keys['O'];  // O -> yaw right
    p2Keys['1'] = keys['8'];  // 8 -> decrease speed
    p2Keys['2'] = keys['9'];  // 9 -> increase speed
    p2Keys[' '] = keys[' '];  // Share space for barrel roll (or use different key)
    
    // Fire with P key
    if ((keys['p'] || keys['P']) && player2FireCooldown <= 0) {
        fireMissilePlayer2();
    }
    
    player2->update(deltaTime, p2Keys);
    
    // Keep player in bounds
    float px, py, pz;
    player2->getPosition(px, py, pz);
    if (std::abs(px) > arenaSize) player2->setX(px > 0 ? arenaSize : -arenaSize);
    if (std::abs(pz) > arenaSize) player2->setZ(pz > 0 ? arenaSize : -arenaSize);
    if (py < 10) player2->setY(10);
    if (py > 250) player2->setY(250);
}

void CoopMode::updateMissiles(float deltaTime) {
    for (auto it = missiles.begin(); it != missiles.end(); ) {
        Missile* missile = *it;
        if (missile) {
            missile->update(deltaTime);
            
            // Remove out-of-bounds or expired missiles
            float mx, my, mz;
            missile->getPosition(mx, my, mz);
            if (std::abs(mx) > arenaSize * 2 || std::abs(mz) > arenaSize * 2 || 
                my < 0 || my > 300 || !missile->isActive()) {
                delete missile;
                it = missiles.erase(it);
            } else {
                ++it;
            }
        } else {
            it = missiles.erase(it);
        }
    }
}

void CoopMode::checkCollisions() {
    // Check missile collisions with players
    for (auto it = missiles.begin(); it != missiles.end(); ) {
        Missile* missile = *it;
        if (!missile || !missile->isActive()) {
            ++it;
            continue;
        }
        
        float mx, my, mz;
        missile->getPosition(mx, my, mz);
        
        // Check collision with Player 1 (if missile is from Player 2)
        if (missile->getOwner() == 1) {  // Player 2's missile
            float p1x, p1y, p1z;
            player1->getPosition(p1x, p1y, p1z);
            float dist = std::sqrt(std::pow(mx - p1x, 2) + std::pow(my - p1y, 2) + std::pow(mz - p1z, 2));
            
            float collisionDist = missile->getBoundingRadius() + player1->getBoundingRadius();
            if (dist < collisionDist) {  // Hit!
                player1Health -= 25;
                player2Score += 100;
                missile->deactivate();
                std::cout << "Player 2 hit Player 1! Health: " << player1Health << std::endl;
            }
        }
        
        // Check collision with Player 2 (if missile is from Player 1)
        if (missile->getOwner() == 0) {  // Player 1's missile
            float p2x, p2y, p2z;
            player2->getPosition(p2x, p2y, p2z);
            float dist = std::sqrt(std::pow(mx - p2x, 2) + std::pow(my - p2y, 2) + std::pow(mz - p2z, 2));
            
            float collisionDist = missile->getBoundingRadius() + player2->getBoundingRadius();
            if (dist < collisionDist) {  // Hit!
                player2Health -= 25;
                player1Score += 100;
                missile->deactivate();
                std::cout << "Player 1 hit Player 2! Health: " << player2Health << std::endl;
            }
        }
        
        // Check collision with obstacles
        for (Obstacle* obstacle : obstacles) {
            if (obstacle && obstacle->isActive()) {
                if (checkSphereAABBCollision(mx, my, mz, 2.0f,
                    obstacle->getMinX(), obstacle->getMaxX(),
                    obstacle->getMinY(), obstacle->getMaxY(),
                    obstacle->getMinZ(), obstacle->getMaxZ())) {
                    missile->deactivate();
                }
            }
        }
        
        if (!missile->isActive()) {
            delete missile;
            it = missiles.erase(it);
        } else {
            ++it;
        }
    }
}

void CoopMode::fireMissilePlayer1() {
    if (player1Ammo <= 0 || player1FireCooldown > 0) {
        return;
    }
    
    float px, py, pz;
    player1->getPosition(px, py, pz);
    float yaw = player1->getYaw();
    float pitch = player1->getPitch();
    
    // Calculate direction vector from yaw and pitch
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;
    float forwardX = std::sin(radYaw) * std::cos(radPitch);
    float forwardY = -std::sin(radPitch);
    float forwardZ = std::cos(radYaw) * std::cos(radPitch);
    
    Missile* missile = new Missile(px, py, pz, forwardX, forwardY, forwardZ, true);
    missile->setSpeed(5.0f);
    missile->setOwner(0);  // Player 1
    missiles.push_back(missile);
    
    player1Ammo--;
    player1FireCooldown = 1.0f;
    
    if (player1Ammo <= 0) {
        player1ReloadTimer = 0;
        std::cout << "Player 1 out of ammo! Reloading..." << std::endl;
    }
}

void CoopMode::fireMissilePlayer2() {
    if (player2Ammo <= 0 || player2FireCooldown > 0) {
        return;
    }
    
    float px, py, pz;
    player2->getPosition(px, py, pz);
    float yaw = player2->getYaw();
    float pitch = player2->getPitch();
    
    // Calculate direction vector from yaw and pitch
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;
    float forwardX = std::sin(radYaw) * std::cos(radPitch);
    float forwardY = -std::sin(radPitch);
    float forwardZ = std::cos(radYaw) * std::cos(radPitch);
    
    Missile* missile = new Missile(px, py, pz, forwardX, forwardY, forwardZ, false);
    missile->setSpeed(5.0f);
    missile->setOwner(1);  // Player 2
    missiles.push_back(missile);
    
    player2Ammo--;
    player2FireCooldown = 1.0f;
    
    if (player2Ammo <= 0) {
        player2ReloadTimer = 0;
        std::cout << "Player 2 out of ammo! Reloading..." << std::endl;
    }
}

void CoopMode::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render split screen
    renderSplitScreen();
    
    // Render HUD over everything
    renderHUD();
    
    // Render messages
    renderMessages();
    
    glutSwapBuffers();
}

void CoopMode::renderSplitScreen() {
    // Top half - Player 1 view
    renderPlayer1View();
    
    // Bottom half - Player 2 view
    renderPlayer2View();
    
    // Draw split line
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(0, 360);
    glVertex2f(1280, 360);
    glEnd();
    glLineWidth(1.0f);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void CoopMode::renderPlayer1View() {
    // Set viewport to top half
    glViewport(0, 360, 1280, 360);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1280.0/360.0, 0.1, 2000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Apply camera 1
    if (camera1 && player1) {
        camera1->apply();
    }
    
    // Apply lighting
    if (lighting) {
        lighting->apply();
    }
    
    // Render arena
    renderArena();
    
    // Render Player 2 (opponent)
    if (player2) {
        glColor3f(0.2f, 0.2f, 1.0f);  // Blue
        player2->render();
    }
    
    // Render missiles
    for (Missile* missile : missiles) {
        if (missile && missile->isActive()) {
            missile->render();
        }
    }
}

void CoopMode::renderPlayer2View() {
    // Set viewport to bottom half
    glViewport(0, 0, 1280, 360);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1280.0/360.0, 0.1, 2000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Apply camera 2
    if (camera2 && player2) {
        camera2->apply();
    }
    
    // Apply lighting
    if (lighting) {
        lighting->apply();
    }
    
    // Render arena
    renderArena();
    
    // Render Player 1 (opponent)
    if (player1) {
        glColor3f(1.0f, 0.2f, 0.2f);  // Red
        player1->render();
    }
    
    // Render missiles
    for (Missile* missile : missiles) {
        if (missile && missile->isActive()) {
            missile->render();
        }
    }
}

void CoopMode::renderArena() {
    // Render obstacles
    for (Obstacle* obstacle : obstacles) {
        if (obstacle && obstacle->isActive()) {
            obstacle->render();
        }
    }
}

void CoopMode::renderHUD() {
    // Reset viewport for HUD
    glViewport(0, 0, 1280, 720);
    
    renderPlayer1HUD();
    renderPlayer2HUD();
}

void CoopMode::renderPlayer1HUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // Player 1 label
    glColor3f(1.0f, 0.2f, 0.2f);
    const char* p1Label = "PLAYER 1";
    glRasterPos2f(20, 700);
    for (const char* c = p1Label; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Health bar
    renderHealthBar(20, 665, player1Health, maxHealth);
    
    // Ammo counter
    renderAmmoCounter(20, 635, player1Ammo, maxAmmo);
    
    // Score
    char buffer[64];
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(buffer, "Score: %d", player1Score);
    glRasterPos2f(20, 605);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    // Reload indicator
    if (player1Ammo <= 0) {
        glColor3f(1.0f, 1.0f, 0.0f);
        sprintf(buffer, "RELOADING: %.1fs", reloadTime - player1ReloadTimer);
        glRasterPos2f(20, 575);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        }
    }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void CoopMode::renderPlayer2HUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // Player 2 label
    glColor3f(0.2f, 0.2f, 1.0f);
    const char* p2Label = "PLAYER 2";
    glRasterPos2f(20, 340);
    for (const char* c = p2Label; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Health bar
    renderHealthBar(20, 305, player2Health, maxHealth);
    
    // Ammo counter
    renderAmmoCounter(20, 275, player2Ammo, maxAmmo);
    
    // Score
    char buffer[64];
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(buffer, "Score: %d", player2Score);
    glRasterPos2f(20, 245);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    // Reload indicator
    if (player2Ammo <= 0) {
        glColor3f(1.0f, 1.0f, 0.0f);
        sprintf(buffer, "RELOADING: %.1fs", reloadTime - player2ReloadTimer);
        glRasterPos2f(20, 215);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        }
    }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void CoopMode::renderHealthBar(float x, float y, int health, int maxHealth) {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 150, y);
    glVertex2f(x + 150, y + 20);
    glVertex2f(x, y + 20);
    glEnd();
    
    float healthPercent = (float)health / maxHealth;
    if (healthPercent < 0) healthPercent = 0;
    
    if (healthPercent > 0.5f) {
        glColor3f(0.2f, 1.0f, 0.2f);
    } else if (healthPercent > 0.25f) {
        glColor3f(1.0f, 1.0f, 0.0f);
    } else {
        glColor3f(1.0f, 0.2f, 0.2f);
    }
    
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 150 * healthPercent, y);
    glVertex2f(x + 150 * healthPercent, y + 20);
    glVertex2f(x, y + 20);
    glEnd();
    
    // Border
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + 150, y);
    glVertex2f(x + 150, y + 20);
    glVertex2f(x, y + 20);
    glEnd();
    
    // Text
    char buffer[32];
    sprintf(buffer, "HP: %d/%d", health, maxHealth);
    glRasterPos2f(x + 40, y + 6);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

void CoopMode::renderAmmoCounter(float x, float y, int ammo, int maxAmmo) {
    glColor3f(1.0f, 1.0f, 1.0f);
    char buffer[32];
    sprintf(buffer, "Ammo: %d/%d", ammo, maxAmmo);
    glRasterPos2f(x, y);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

void CoopMode::renderMessages() {
    if (state == CoopState::PLAYING) return;
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // Overlay
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
    
    if (state == CoopState::PLAYER1_WON) {
        glColor3f(1.0f, 0.2f, 0.2f);
        sprintf(buffer, "PLAYER 1 WINS!");
        glRasterPos2f(520, 400);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }
    } else if (state == CoopState::PLAYER2_WON) {
        glColor3f(0.2f, 0.2f, 1.0f);
        sprintf(buffer, "PLAYER 2 WINS!");
        glRasterPos2f(520, 400);
        for (char* c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }
    }
    
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(buffer, "Final Scores - P1: %d | P2: %d", player1Score, player2Score);
    glRasterPos2f(480, 350);
    for (char* c = buffer; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    glColor3f(1.0f, 1.0f, 0.0f);
    sprintf(buffer, "Press R to restart");
    glRasterPos2f(540, 300);
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

void CoopMode::cleanup() {
    if (player1) {
        delete player1;
        player1 = nullptr;
    }
    
    if (player2) {
        delete player2;
        player2 = nullptr;
    }
    
    for (Missile* missile : missiles) {
        delete missile;
    }
    missiles.clear();
    
    for (Obstacle* obstacle : obstacles) {
        delete obstacle;
    }
    obstacles.clear();
    
    if (camera1) {
        delete camera1;
        camera1 = nullptr;
    }
    
    if (camera2) {
        delete camera2;
        camera2 = nullptr;
    }
    
    if (lighting) {
        delete lighting;
        lighting = nullptr;
    }
}

bool CoopMode::isWon() const {
    return (state == CoopState::PLAYER1_WON || state == CoopState::PLAYER2_WON);
}

bool CoopMode::isLost() const {
    return false;  // No "lose" in co-op, only win/lose per player
}

void CoopMode::restart() {
    cleanup();
    state = CoopState::PLAYING;
    player1Health = maxHealth;
    player2Health = maxHealth;
    player1Ammo = maxAmmo;
    player2Ammo = maxAmmo;
    player1Score = 0;
    player2Score = 0;
    init();
}

void CoopMode::handleMouse(int button, int buttonState, int x, int y) {
    // Mouse controls disabled - using keyboard for firing
    // Players use F and P keys to fire
}

void CoopMode::handleMouseMotion(int x, int y) {
    // Not used in co-op mode
}

