#include "Level2.h"
#include "../physics/Collision.h"
#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper function to find asset path
static std::string findAssetPath(const std::string& relativePath) {
    const char* basePaths[] = {
        "",
        "assets/",
        "../",
        "../../",
        "../../../",
        "../../../../",
        "../../../../../",
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
    
    std::cout << "Asset not found in any location: " << relativePath << std::endl;
    return relativePath;
}

Level2::Level2()
    : state(Level2State::PLAYING),
      player(nullptr),
      camera(nullptr),
      lighting(nullptr),
      score(0),
      enemiesDestroyed(0),
      totalEnemies(2),
      lockOnState(LockOnState::NONE),
      lockedTarget(nullptr),
      lockOnProgress(0),
      lockOnTime(2.0f),
      lockOnDistance(150.0f),
      lockOnAngle(30.0f),
      lockOnBeepTimer(0),
      missileFireCooldown(1.5f),
      missileFireTimer(0),
      leftMousePressed(false),
      enemyMissileSpawnTimer(0),
      enemyMissileSpawnInterval(5.0f),
      safeZoneRadius(100.0f),
      missileWarning(false),
      warningFlashTimer(0),
      nKeyWasPressed(false),
      startX(0), startY(120), startZ(0),
      startYaw(0),
      levelWidth(800),
      levelLength(800),
      finaleTarget(nullptr),
      finaleTriggered(false),
      finaleTimer(0) {
}

Level2::~Level2() {
    cleanup();
}

void Level2::init() {
    std::cout << "========================================" << std::endl;
    std::cout << "        LEVEL 2: AERIAL COMBAT          " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Objective: Destroy all enemy aircraft!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Left Mouse : Fire Missile (when locked)" << std::endl;
    std::cout << "  Aim at enemy to lock on" << std::endl;
    std::cout << std::endl;
    
    // Create player
    player = new Player(startX, startY, startZ);
    player->reset(startX, startY, startZ, startYaw);
    
    // Create camera
    camera = new Camera();
    camera->setFirstPerson(false);
    camera->setDistance(25.0f);
    camera->setHeight(8.0f);
    
    // Create lighting
    lighting = new Lighting();
    lighting->init();
    lighting->setNightMode(false);
    
    // Load 3D models
    loadModels();
    
    // Create sparse terrain
    createTerrain();
    
    // Create enemies
    createEnemies();
    
    // Timer not used in Level 2 (no time limit)
    // timer.start(999.0f);  // Commented out
    
    std::cout << "Level 2 initialized!" << std::endl;
}

void Level2::loadModels() {
    std::cout << "Level2: Loading models..." << std::endl;
    
    // Load player aircraft model
    std::string playerModelPath = findAssetPath("assets/Japan Plane/14082_WWII_Plane_Japan_Kawasaki_Ki-61_v1_L2.3ds");
    if (!player->loadModel(playerModelPath, 0.15f)) {
        std::cerr << "Level2: Could not load player model, using primitives" << std::endl;
    }
    
    std::cout << "Level2: Models loaded!" << std::endl;
}

void Level2::createTerrain() {
    std::cout << "Level2: Creating terrain..." << std::endl;
    
    // Create sparse mountains using mountain models
    std::string mountainModelPath = findAssetPath("assets/mountains/mountains/mountains.obj");
    
    // Fewer mountains than Level 1 for more open combat space
    float mountainPositions[][3] = {
        {-200, 0, -200},
        {200, 0, -200},
        {-200, 0, 200},
        {200, 0, 200},
        {-300, 0, 0},
        {300, 0, 0},
        {0, 0, -300},
        {0, 0, 300}
    };
    
    for (int i = 0; i < 8; i++) {
        Obstacle* mountain = new Obstacle(
            mountainPositions[i][0],
            mountainPositions[i][1],
            mountainPositions[i][2],
            50, 80, 50,  // Smaller mountains
            ObstacleType::MOUNTAIN
        );
        
        // Try to load mountain model
        if (!mountain->loadModel(mountainModelPath, 0.5f)) {
            std::cerr << "Level2: Could not load mountain model for obstacle " << i << std::endl;
        }
        
        terrain.push_back(mountain);
    }
    
    // Create ground plane
    Obstacle* ground = new Obstacle(0, -5, 0, 2000, 10, 2000, ObstacleType::GROUND);
    ground->setColor(0.3f, 0.5f, 0.3f);  // Green ground
    terrain.push_back(ground);
    
    std::cout << "Level2: Terrain created with " << terrain.size() << " obstacles" << std::endl;
}

void Level2::createEnemies() {
    std::cout << "Level2: Creating enemies..." << std::endl;
    
    // Create two enemy aircraft
    Enemy* enemy1 = new Enemy(-100, 100, -200, 0);
    Enemy* enemy2 = new Enemy(100, 110, -180, 180);
    
    // Try to load enemy models (DirectX .X format needs conversion, use primitives for now)
    // In future, convert enemy.X to OBJ format or use primitives
    std::string enemyModelPath = findAssetPath("assets/enemy/enemy/enemy.X");
    // enemy1->loadModel(enemyModelPath, 0.2f);  // Commented out as .X format not supported
    // enemy2->loadModel(enemyModelPath, 0.2f);
    
    enemy1->setSpeed(0.7f);
    enemy2->setSpeed(0.75f);
    
    enemies.push_back(enemy1);
    enemies.push_back(enemy2);
    
    std::cout << "Level2: Created " << enemies.size() << " enemies" << std::endl;
}

void Level2::update(float deltaTime, const bool* keys) {
    if (state != Level2State::PLAYING) {
        if (state == Level2State::WON || state == Level2State::LOST) {
            return;
        }
    }
    
    // Handle day/night toggle
    if ((keys['n'] || keys['N']) && !nKeyWasPressed) {
        toggleDayNight();
        nKeyWasPressed = true;
    } else if (!(keys['n'] || keys['N'])) {
        nKeyWasPressed = false;
    }
    
    // Update player
    if (player) {
        player->update(deltaTime, keys);
        
        // Update camera to follow player
        if (camera) {
            camera->update(player, deltaTime);
        }
    }
    
    // Update enemies
    updateEnemies(deltaTime);
    
    // Update missiles
    updateMissiles(deltaTime);
    
    // Update lock-on system
    updateLockOn(deltaTime);
    
    // Update explosions
    updateExplosions(deltaTime);
    
    // Check collisions
    checkCollisions();
    checkMissileCollisions();
    
    // Update missile firing cooldown
    if (missileFireTimer > 0) {
        missileFireTimer -= deltaTime;
    }
    
    // Spawn enemy missiles periodically (when player outside safe zone)
    enemyMissileSpawnTimer += deltaTime;
    if (enemyMissileSpawnTimer >= enemyMissileSpawnInterval) {
        float px, py, pz;
        player->getPosition(px, py, pz);
        if (!isInSafeZone(px, py, pz)) {
            spawnEnemyMissile();
        }
        enemyMissileSpawnTimer = 0;
    }
    
    // Check for level completion
    if (enemiesDestroyed >= totalEnemies && !finaleTriggered) {
        std::cout << "All enemies destroyed! Starting finale sequence..." << std::endl;
        state = Level2State::FINALE;
        finaleTriggered = true;
        
        // Create finale target (large structure to destroy)
        finaleTarget = new Obstacle(0, 50, -400, 40, 60, 40, ObstacleType::BUILDING);
        finaleTarget->setColor(0.5f, 0.1f, 0.1f);  // Red target
        terrain.push_back(finaleTarget);
    }
    
    // Finale sequence
    if (state == Level2State::FINALE) {
        finaleTimer += deltaTime;
        
        // Check if finale target is destroyed
        if (finaleTarget && !finaleTarget->isActive()) {
            state = Level2State::WON;
            std::cout << "MISSION ACCOMPLISHED!" << std::endl;
        }
    }
    
    // Check for player death
    if (player && !player->isAlive()) {
        state = Level2State::LOST;
    }
}

void Level2::updateEnemies(float deltaTime) {
    for (auto* enemy : enemies) {
        if (enemy && enemy->isAlive()) {
            enemy->update(deltaTime);
        }
    }
    
    // Remove fully destroyed enemies
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [this](Enemy* e) {
                if (e->isDestroyed()) {
                    delete e;
                    return true;
                }
                return false;
            }),
        enemies.end()
    );
}

void Level2::updateMissiles(float deltaTime) {
    // Check for missile warnings
    missileWarning = false;
    float px, py, pz;
    player->getPosition(px, py, pz);
    
    for (auto* missile : missiles) {
        if (missile && missile->isActive()) {
            missile->update(deltaTime);
            
            // Check if enemy missile is close to player
            if (!missile->isPlayerOwned()) {
                float mx, my, mz;
                missile->getPosition(mx, my, mz);
                float dist = distanceToPlayer(mx, my, mz);
                if (dist < 50.0f) {
                    missileWarning = true;
                }
            }
        }
    }
    
    // Update warning flash
    if (missileWarning) {
        warningFlashTimer += deltaTime * 10.0f;
    }
    
    // Remove inactive missiles
    missiles.erase(
        std::remove_if(missiles.begin(), missiles.end(),
            [](Missile* m) {
                if (!m->isActive()) {
                    delete m;
                    return true;
                }
                return false;
            }),
        missiles.end()
    );
}

void Level2::updateLockOn(float deltaTime) {
    if (!player || !player->isAlive()) {
        lockOnState = LockOnState::NONE;
        lockedTarget = nullptr;
        lockOnProgress = 0;
        return;
    }
    
    // Find nearest enemy in sights
    Enemy* target = findNearestEnemy();
    
    if (target) {
        float distance, angle;
        if (isEnemyInSights(target, distance, angle)) {
            // Enemy in sights, start/continue locking
            if (lockOnState == LockOnState::NONE || lockedTarget != target) {
                lockOnState = LockOnState::ACQUIRING;
                lockedTarget = target;
                lockOnProgress = 0;
            }
            
            if (lockOnState == LockOnState::ACQUIRING) {
                lockOnProgress += deltaTime / lockOnTime;
                
                // Beep sound (faster as lock progresses)
                lockOnBeepTimer += deltaTime;
                float beepInterval = 0.5f - (lockOnProgress * 0.4f);
                if (lockOnBeepTimer >= beepInterval) {
                    // Play beep sound (TODO: add sound)
                    lockOnBeepTimer = 0;
                }
                
                if (lockOnProgress >= 1.0f) {
                    lockOnState = LockOnState::LOCKED;
                    lockOnProgress = 1.0f;
                    std::cout << "TARGET LOCKED!" << std::endl;
                }
            }
        } else {
            // Lost sight of target
            lockOnState = LockOnState::NONE;
            lockedTarget = nullptr;
            lockOnProgress = 0;
        }
    } else {
        // No target found
        lockOnState = LockOnState::NONE;
        lockedTarget = nullptr;
        lockOnProgress = 0;
    }
}

void Level2::updateExplosions(float deltaTime) {
    for (auto& explosion : explosions) {
        explosion.timer += deltaTime;
        explosion.scale = 1.0f + (explosion.timer / explosion.duration) * 3.0f;
    }
    
    // Remove finished explosions
    explosions.erase(
        std::remove_if(explosions.begin(), explosions.end(),
            [](const ExplosionEffect& e) { return e.timer >= e.duration; }),
        explosions.end()
    );
}

void Level2::checkCollisions() {
    if (!player || !player->isAlive()) return;
    
    float px, py, pz;
    player->getPosition(px, py, pz);
    float pr = player->getBoundingRadius();
    
    // Check collision with terrain
    for (auto* obstacle : terrain) {
        if (obstacle && checkSphereAABBCollision(
            px, py, pz, pr,
            obstacle->getMinX(), obstacle->getMaxX(),
            obstacle->getMinY(), obstacle->getMaxY(),
            obstacle->getMinZ(), obstacle->getMaxZ())) {
            
            if (obstacle->getType() != ObstacleType::GROUND || py < 15.0f) {
                player->kill();
                triggerExplosion(px, py, pz);
                std::cout << "Player crashed into terrain!" << std::endl;
                return;
            }
        }
    }
}

void Level2::checkMissileCollisions() {
    for (auto* missile : missiles) {
        if (!missile || !missile->isActive()) continue;
        
        float mx, my, mz;
        missile->getPosition(mx, my, mz);
        float mr = missile->getBoundingRadius();
        
        if (missile->isPlayerOwned()) {
            // Player missile - check collision with enemies
            for (auto* enemy : enemies) {
                if (enemy && enemy->isAlive()) {
                    float ex, ey, ez;
                    enemy->getPosition(ex, ey, ez);
                    float er = enemy->getBoundingRadius();
                    
                    if (checkSphereCollision(mx, my, mz, mr, ex, ey, ez, er)) {
                        enemy->destroy();
                        missile->deactivate();
                        triggerExplosion(ex, ey, ez);
                        score += 100;
                        enemiesDestroyed++;
                        std::cout << "Enemy destroyed! (" << enemiesDestroyed << "/" << totalEnemies << ")" << std::endl;
                        break;
                    }
                }
            }
            
            // Check collision with finale target
            if (finaleTarget && state == Level2State::FINALE) {
                if (checkSphereAABBCollision(
                    mx, my, mz, mr,
                    finaleTarget->getMinX(), finaleTarget->getMaxX(),
                    finaleTarget->getMinY(), finaleTarget->getMaxY(),
                    finaleTarget->getMinZ(), finaleTarget->getMaxZ())) {
                    
                    finaleTarget->deactivate();
                    missile->deactivate();
                    triggerExplosion(finaleTarget->getX(), finaleTarget->getY(), finaleTarget->getZ());
                    std::cout << "Finale target destroyed!" << std::endl;
                }
            }
        } else {
            // Enemy missile - check collision with player
            if (player && player->isAlive()) {
                float px, py, pz;
                player->getPosition(px, py, pz);
                float pr = player->getBoundingRadius();
                
                if (checkSphereCollision(mx, my, mz, mr, px, py, pz, pr)) {
                    player->kill();
                    missile->deactivate();
                    triggerExplosion(px, py, pz);
                    std::cout << "Player hit by enemy missile!" << std::endl;
                    return;
                }
            }
        }
        
        // Check collision with terrain
        for (auto* obstacle : terrain) {
            if (obstacle && obstacle->getType() != ObstacleType::GROUND) {
                if (checkSphereAABBCollision(
                    mx, my, mz, mr,
                    obstacle->getMinX(), obstacle->getMaxX(),
                    obstacle->getMinY(), obstacle->getMaxY(),
                    obstacle->getMinZ(), obstacle->getMaxZ())) {
                    
                    missile->deactivate();
                    triggerExplosion(mx, my, mz);
                    break;
                }
            }
        }
    }
}

void Level2::fireMissile() {
    if (!player || !player->isAlive()) return;
    if (lockOnState != LockOnState::LOCKED) return;
    if (missileFireTimer > 0) return;
    
    // Get player position and forward vector
    float px, py, pz, pitch, yaw, roll;
    player->getPosition(px, py, pz);
    player->getRotation(pitch, yaw, roll);
    
    // Calculate forward direction
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;
    float forwardX = std::sin(radYaw) * std::cos(radPitch);
    float forwardY = -std::sin(radPitch);
    float forwardZ = std::cos(radYaw) * std::cos(radPitch);
    
    // Spawn missile slightly in front of player
    float spawnDist = 5.0f;
    float missileStartX = px + forwardX * spawnDist;
    float missileStartY = py + forwardY * spawnDist;
    float missileStartZ = pz + forwardZ * spawnDist;
    
    Missile* missile = new Missile(missileStartX, missileStartY, missileStartZ, forwardX, forwardY, forwardZ, true);
    missile->setSpeed(3.0f);
    missiles.push_back(missile);
    
    missileFireTimer = missileFireCooldown;
    
    std::cout << "Missile fired!" << std::endl;
}

void Level2::spawnEnemyMissile() {
    if (enemies.empty() || !player || !player->isAlive()) return;
    
    // Pick random alive enemy
    std::vector<Enemy*> aliveEnemies;
    for (auto* enemy : enemies) {
        if (enemy && enemy->isAlive()) {
            aliveEnemies.push_back(enemy);
        }
    }
    
    if (aliveEnemies.empty()) return;
    
    Enemy* shooter = aliveEnemies[rand() % aliveEnemies.size()];
    
    // Get enemy position and aim at player
    float ex, ey, ez;
    shooter->getPosition(ex, ey, ez);
    
    float px, py, pz;
    player->getPosition(px, py, pz);
    
    // Calculate direction to player
    float dx = px - ex;
    float dy = py - ey;
    float dz = pz - ez;
    float length = std::sqrt(dx*dx + dy*dy + dz*dz);
    
    if (length > 0.001f) {
        dx /= length;
        dy /= length;
        dz /= length;
        
        Missile* missile = new Missile(ex, ey, ez, dx, dy, dz, false);
        missile->setSpeed(2.0f);
        missiles.push_back(missile);
        
        std::cout << "Enemy fired missile!" << std::endl;
    }
}

void Level2::triggerExplosion(float x, float y, float z) {
    explosions.push_back(ExplosionEffect(x, y, z));
}

Enemy* Level2::findNearestEnemy() {
    if (!player) return nullptr;
    
    Enemy* nearest = nullptr;
    float nearestDist = lockOnDistance;
    
    for (auto* enemy : enemies) {
        if (enemy && enemy->isAlive()) {
            float ex, ey, ez;
            enemy->getPosition(ex, ey, ez);
            float dist = distanceToPlayer(ex, ey, ez);
            
            if (dist < nearestDist) {
                nearestDist = dist;
                nearest = enemy;
            }
        }
    }
    
    return nearest;
}

bool Level2::isEnemyInSights(Enemy* enemy, float& outDistance, float& outAngle) {
    if (!enemy || !player) return false;
    
    float px, py, pz, pitch, yaw, roll;
    player->getPosition(px, py, pz);
    player->getRotation(pitch, yaw, roll);
    
    float ex, ey, ez;
    enemy->getPosition(ex, ey, ez);
    
    // Calculate direction to enemy
    float dx = ex - px;
    float dy = ey - py;
    float dz = ez - pz;
    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
    
    if (distance < 0.001f) return false;
    
    dx /= distance;
    dy /= distance;
    dz /= distance;
    
    // Calculate player forward vector
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;
    float forwardX = std::sin(radYaw) * std::cos(radPitch);
    float forwardY = -std::sin(radPitch);
    float forwardZ = std::cos(radYaw) * std::cos(radPitch);
    
    // Calculate angle between forward and enemy direction
    float dot = forwardX * dx + forwardY * dy + forwardZ * dz;
    float angle = std::acos(std::max(-1.0f, std::min(1.0f, dot))) * 180.0f / M_PI;
    
    outDistance = distance;
    outAngle = angle;
    
    return (distance <= lockOnDistance && angle <= lockOnAngle);
}

float Level2::distanceToPlayer(float x, float y, float z) {
    if (!player) return 999999.0f;
    
    float px, py, pz;
    player->getPosition(px, py, pz);
    
    float dx = x - px;
    float dy = y - py;
    float dz = z - pz;
    
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

bool Level2::isInSafeZone(float x, float y, float z) {
    // Safe zone is area near starting position
    float dx = x - startX;
    float dy = y - startY;
    float dz = z - startZ;
    float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
    
    return (dist < safeZoneRadius);
}

void Level2::render() {
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set up camera
    if (camera) {
        camera->apply();
    }
    
    // Apply lighting
    if (lighting) {
        lighting->apply();
    }
    
    // Render sky
    renderSky();
    
    // Render terrain
    for (auto* obstacle : terrain) {
        if (obstacle) {
            obstacle->render();
        }
    }
    
    // Render player with glow in night mode
    if (player && player->isAlive()) {
        if (lighting && lighting->isNightMode()) {
            GLfloat emission[] = {0.2f, 0.2f, 0.3f, 1.0f};
            GLfloat shininess = 60.0f;
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        }
        player->render();
        if (lighting && lighting->isNightMode()) {
            GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);
        }
    }
    
    // Render enemies with red glow in night mode
    if (lighting && lighting->isNightMode()) {
        GLfloat emission[] = {0.5f, 0.1f, 0.1f, 1.0f};  // Red glow
        GLfloat shininess = 80.0f;
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }
    for (auto* enemy : enemies) {
        if (enemy) {
            enemy->render();
        }
    }
    if (lighting && lighting->isNightMode()) {
        GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);
    }
    
    // Render missiles with bright glow in night mode
    if (lighting && lighting->isNightMode()) {
        GLfloat emission[] = {0.8f, 0.5f, 0.0f, 1.0f};  // Orange glow
        GLfloat shininess = 100.0f;
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }
    for (auto* missile : missiles) {
        if (missile) {
            missile->render();
        }
    }
    if (lighting && lighting->isNightMode()) {
        GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);
    }
    
    // Render explosions
    renderExplosions();
    
    // Render HUD
    renderHUD();
    
    // Render lock-on reticle
    if (lockOnState != LockOnState::NONE) {
        renderLockOnReticle();
    }
    
    // Render missile warning
    if (missileWarning) {
        renderMissileWarning();
    }
    
    // Render messages
    renderMessages();
    
    glutSwapBuffers();
}

void Level2::renderSky() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(60.0, 1280.0/720.0, 0.1, 1000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    bool isNight = lighting && lighting->isNightMode();
    
    // Sky gradient
    glBegin(GL_QUADS);
    if (isNight) {
        // Night sky
        glColor3f(0.01f, 0.01f, 0.05f);  // Very dark blue top
        glVertex3f(-500, 200, -500);
        glVertex3f(500, 200, -500);
        glColor3f(0.02f, 0.02f, 0.08f);  // Slightly lighter bottom
        glVertex3f(500, -50, -500);
        glVertex3f(-500, -50, -500);
    } else {
        // Day sky
        glColor3f(0.3f, 0.5f, 0.8f);  // Darker blue top
        glVertex3f(-500, 200, -500);
        glVertex3f(500, 200, -500);
        glColor3f(0.6f, 0.8f, 1.0f);  // Lighter blue bottom
        glVertex3f(500, -50, -500);
        glVertex3f(-500, -50, -500);
    }
    glEnd();
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void Level2::renderExplosions() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (const auto& explosion : explosions) {
        glPushMatrix();
        glTranslatef(explosion.x, explosion.y, explosion.z);
        
        float progress = explosion.timer / explosion.duration;
        float alpha = 1.0f - progress;
        
        glScalef(explosion.scale, explosion.scale, explosion.scale);
        
        // Outer sphere
        glColor4f(1.0f, 0.5f, 0.0f, alpha * 0.8f);
        glutSolidSphere(5.0f, 16, 16);
        
        // Inner bright core
        glColor4f(1.0f, 1.0f, 0.3f, alpha);
        glutSolidSphere(3.0f, 12, 12);
        
        glPopMatrix();
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void Level2::renderHUD() {
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
    
    // Draw score
    glColor3f(1.0f, 1.0f, 1.0f);
    char scoreText[64];
    sprintf(scoreText, "Score: %d", score);
    glRasterPos2f(20, 690);
    for (char* c = scoreText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Draw enemies remaining
    char enemyText[64];
    sprintf(enemyText, "Enemies: %d/%d", totalEnemies - enemiesDestroyed, totalEnemies);
    glRasterPos2f(20, 660);
    for (char* c = enemyText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Draw lock-on status
    if (lockOnState != LockOnState::NONE) {
        if (lockOnState == LockOnState::ACQUIRING) {
            glColor3f(1.0f, 1.0f, 0.0f);  // Yellow
            sprintf(scoreText, "LOCKING... %d%%", (int)(lockOnProgress * 100));
        } else {
            glColor3f(0.0f, 1.0f, 0.0f);  // Green
            sprintf(scoreText, "LOCKED");
        }
        glRasterPos2f(540, 50);
        for (char* c = scoreText; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }
    
    // Crosshair
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(635, 360);
    glVertex2f(645, 360);
    glVertex2f(640, 355);
    glVertex2f(640, 365);
    glEnd();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Level2::renderLockOnReticle() {
    if (!lockedTarget || !camera) return;
    
    float ex, ey, ez;
    lockedTarget->getPosition(ex, ey, ez);
    
    // Project enemy position to screen space (simplified)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // Draw reticle at screen center (simplified - enemy is in sights)
    float centerX = 640;
    float centerY = 360;
    float size = 40.0f - (lockOnProgress * 20.0f);  // Closes in as lock progresses
    
    if (lockOnState == LockOnState::ACQUIRING) {
        glColor3f(1.0f, 1.0f, 0.0f);  // Yellow
    } else {
        glColor3f(1.0f, 0.0f, 0.0f);  // Red when locked
    }
    
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(centerX - size, centerY - size);
    glVertex2f(centerX + size, centerY - size);
    glVertex2f(centerX + size, centerY + size);
    glVertex2f(centerX - size, centerY + size);
    glEnd();
    
    // Draw corner brackets
    float bracketSize = 10.0f;
    glBegin(GL_LINES);
    // Top-left
    glVertex2f(centerX - size, centerY - size);
    glVertex2f(centerX - size + bracketSize, centerY - size);
    glVertex2f(centerX - size, centerY - size);
    glVertex2f(centerX - size, centerY - size + bracketSize);
    // Top-right
    glVertex2f(centerX + size, centerY - size);
    glVertex2f(centerX + size - bracketSize, centerY - size);
    glVertex2f(centerX + size, centerY - size);
    glVertex2f(centerX + size, centerY - size + bracketSize);
    // Bottom-left
    glVertex2f(centerX - size, centerY + size);
    glVertex2f(centerX - size + bracketSize, centerY + size);
    glVertex2f(centerX - size, centerY + size);
    glVertex2f(centerX - size, centerY + size - bracketSize);
    // Bottom-right
    glVertex2f(centerX + size, centerY + size);
    glVertex2f(centerX + size - bracketSize, centerY + size);
    glVertex2f(centerX + size, centerY + size);
    glVertex2f(centerX + size, centerY + size - bracketSize);
    glEnd();
    glLineWidth(1.0f);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Level2::renderMissileWarning() {
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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Flash red at edges
    float flash = std::abs(std::sin(warningFlashTimer));
    glColor4f(1.0f, 0.0f, 0.0f, flash * 0.3f);
    
    glBegin(GL_QUADS);
    // Left edge
    glVertex2f(0, 0);
    glVertex2f(50, 0);
    glVertex2f(50, 720);
    glVertex2f(0, 720);
    // Right edge
    glVertex2f(1230, 0);
    glVertex2f(1280, 0);
    glVertex2f(1280, 720);
    glVertex2f(1230, 720);
    glEnd();
    
    // Warning text
    glColor3f(1.0f, 0.0f, 0.0f);
    const char* warning = "MISSILE WARNING!";
    glRasterPos2f(550, 650);
    for (const char* c = warning; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Level2::renderMessages() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    if (state == Level2State::WON) {
        glColor3f(0.0f, 1.0f, 0.0f);
        const char* winText = "MISSION ACCOMPLISHED!";
        glRasterPos2f(480, 360);
        for (const char* c = winText; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }
    } else if (state == Level2State::LOST) {
        glColor3f(1.0f, 0.0f, 0.0f);
        const char* loseText = "MISSION FAILED";
        glRasterPos2f(520, 360);
        for (const char* c = loseText; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }
    } else if (state == Level2State::FINALE) {
        glColor3f(1.0f, 1.0f, 0.0f);
        const char* finaleText = "DESTROY THE FINAL TARGET!";
        glRasterPos2f(450, 650);
        for (const char* c = finaleText; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Level2::cleanup() {
    if (player) {
        delete player;
        player = nullptr;
    }
    
    for (auto* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
    
    for (auto* missile : missiles) {
        delete missile;
    }
    missiles.clear();
    
    for (auto* obstacle : terrain) {
        delete obstacle;
    }
    terrain.clear();
    
    if (camera) {
        delete camera;
        camera = nullptr;
    }
    
    if (lighting) {
        delete lighting;
        lighting = nullptr;
    }
    
    finaleTarget = nullptr;  // Already in terrain vector
}

bool Level2::isWon() const {
    return (state == Level2State::WON);
}

bool Level2::isLost() const {
    return (state == Level2State::LOST);
}

void Level2::reset() {
    cleanup();
    init();
}

void Level2::handleKeyPress(unsigned char key, bool pressed) {
    // Reset on R key
    if ((key == 'r' || key == 'R') && pressed) {
        reset();
    }
}

void Level2::handleMouse(int button, int buttonState, int x, int y) {
    handleMouseButton(button, buttonState, x, y);
}

void Level2::handleMouseMotion(int x, int y) {
    handleMouseMove(x, y);
}

void Level2::handleMouseButton(int button, int buttonState, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (buttonState == GLUT_DOWN) {
            leftMousePressed = true;
            // Try to fire missile
            if (lockOnState == LockOnState::LOCKED) {
                fireMissile();
            }
        } else {
            leftMousePressed = false;
        }
    } else if (button == GLUT_RIGHT_BUTTON && buttonState == GLUT_DOWN) {
        // Toggle camera view
        if (camera) {
            camera->toggle();
        }
    }
}

void Level2::handleMouseMove(int x, int y) {
    // Mouse movement handled by camera in Level base class if needed
}

void Level2::toggleDayNight() {
    if (lighting) {
        lighting->toggleDayNight();
        std::cout << "Mode: " << (lighting->isNightMode() ? "Night" : "Day") << std::endl;
    }
}
