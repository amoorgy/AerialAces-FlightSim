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
      totalEnemies(0),
      bullseyesDestroyed(0),
      totalBullseyes(3),
      ringsCollected(0),
      rocketFireCooldown(0.5f),
      rocketFireTimer(0),
      fKeyWasPressed(false),
      rocketsRemaining(5),
      maxRockets(10),
      punishmentMissile(nullptr),
      punishmentMissileActive(false),
      punishmentMissileDelay(2.0f),
      levelTimeLimit(2000.0f),
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
      lighthouseMissileSpawnTimer(0),
      lighthouseMissileSpawnInterval(999.0f),
      safeZoneRadius(100.0f),
      missileWarning(false),
      warningFlashTimer(0),
      nKeyWasPressed(false),
      cameraShakeIntensity(0),
      cameraShakeDuration(0),
      cameraShakeTimer(0),
      nearMissTimer(0),
      nearMissDetected(false),
      endScreenTimer(0),
      endScreenSelection(0),
      startX(-181.511f), startY(12.2729f), startZ(-350.922f),
      startYaw(12.2206f),
      levelWidth(800),
      levelLength(800),
      finaleTarget(nullptr),
      finaleTriggered(false),
      finaleTimer(0) {
    
    // Initialize sound paths
    explosionSoundPath = findAssetPath("assets/sounds/explosion.wav");
    lockOnSoundPath = findAssetPath("assets/sounds/lock_on.wav");
    missileLaunchSoundPath = findAssetPath("assets/sounds/missle_launch.wav");
    whooshSoundPath = findAssetPath("assets/sounds/whoosh.wav");
}

Level2::~Level2() {
    cleanup();
}

void Level2::init() {
    std::cout << "========================================" << std::endl;
    std::cout << "     LEVEL 2: TARGET PRACTICE           " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Objective: Destroy all 3 bullseye targets!" << std::endl;
    std::cout << "CHALLENGE MODE:" << std::endl;
    std::cout << "  - 30 SECOND TIME LIMIT!" << std::endl;
    std::cout << "  - Only 5 ROCKETS to start!" << std::endl;
    std::cout << "  - Collect BONUS RINGS for +3 rockets each" << std::endl;
    std::cout << "  - Run out of rockets = TRACKING MISSILE!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  F : Fire Rocket (straight line)" << std::endl;
    std::cout << "  W/S : Pitch up/down" << std::endl;
    std::cout << "  A/D : Roll left/right" << std::endl;
    std::cout << "  Q/E : Yaw left/right" << std::endl;
    std::cout << "  1/2 : Decrease/Increase speed" << std::endl;
    std::cout << "  N : Toggle Day/Night mode" << std::endl;
    std::cout << std::endl;
    
    // Create player at specified start position
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
    
    // Create lighthouses on mountain peaks (visual only)
    createLighthouses();
    
    // Create bullseye targets
    createBullseyes();
    
    // Create bonus rings for extra rockets
    createBonusRings();
    
    // Initialize rocket count
    rocketsRemaining = 5;
    ringsCollected = 0;
    bullseyesDestroyed = 0;
    
    // Initialize punishment missile
    punishmentMissile = nullptr;
    punishmentMissileActive = false;
    punishmentMissileDelay = 2.0f;
    
    // Start the level timer (30 seconds)
    levelTimer.start(levelTimeLimit);
    
    // No enemies in this version
    enemies.clear();
    totalEnemies = 0;
    enemiesDestroyed = 0;
    
    std::cout << "Level 2 initialized!" << std::endl;
    std::cout << "Player starts at: (" << startX << ", " << startY << ", " << startZ << ")" << std::endl;
    std::cout << "Bullseyes to destroy: " << totalBullseyes << std::endl;
    std::cout << "Starting rockets: " << rocketsRemaining << std::endl;
    std::cout << "Time limit: " << levelTimeLimit << " seconds" << std::endl;
}

void Level2::loadModels() {
    std::cout << "Level2: Loading models..." << std::endl;
    
    // Load player aircraft model with LARGE scale for visibility
    std::string playerModelPath = findAssetPath("assets/Japan Plane/14082_WWII_Plane_Japan_Kawasaki_Ki-61_v1_L2.obj");
    if (!player->loadModel(playerModelPath, 1.0f)) {  // Large scale for visibility
        std::cerr << "Level2: Could not load player model, using primitives" << std::endl;
    } else {
        std::cout << "Level2: Player model loaded successfully with scale 0.5" << std::endl;
    }
    
    std::cout << "Level2: Models loaded!" << std::endl;
}

void Level2::createTerrain() {
    std::cout << "Level2: Creating terrain..." << std::endl;
    
    // Load mountains model for Level 2
    std::string terrainPath = findAssetPath("assets/mountains/mountains.obj");
    
    Obstacle* landscape = new Obstacle(0, -50, 0, 800, 1, 800, ObstacleType::MOUNTAIN);
    bool terrainLoaded = landscape->loadModel(terrainPath, 10.0f);
    
    if (terrainLoaded) {
        std::cout << "Level2: Mountains model loaded successfully!" << std::endl;
    } else {
        std::cout << "Level2: Mountains model not found, using flat ground" << std::endl;
        landscape->setColor(0.3f, 0.5f, 0.3f);
    }
    terrain.push_back(landscape);
    
    std::cout << "Level2: Terrain created with " << terrain.size() << " obstacles" << std::endl;
}

void Level2::createEnemies() {
    // No enemies in bullseye mode
    std::cout << "Level2: No enemies in target practice mode" << std::endl;
}

void Level2::createLighthouses() {
    std::cout << "\n=== Creating Lighthouses on Mountain Peaks ===" << std::endl;
    
    std::string lighthouseModelPath = findAssetPath("assets/lighthouse/lighthouse.obj");
    
    struct LighthousePos { float x, y, z; };
    LighthousePos positions[] = {
        {-280.762f, 62.3301f, 87.168f},
        {29.0708f, 69.4949f, -200.317f},
        {350.741f, 61.6798f, 267.019f},
        {289.58f, 73.0765f, 355.508f},
        {116.546f, 82.3485f, 156.181f}
    };
    
    // Increased lighthouse height for better visibility
    float lighthouseHeight = 60.0f;
    float lighthouseScale = 0.3f;
    
    for (int i = 0; i < 5; i++) {
        Lighthouse lh(positions[i].x, positions[i].y, positions[i].z, lighthouseHeight);
        
        // Larger collision box to match scaled model
        Obstacle* obs = new Obstacle(positions[i].x, positions[i].y, positions[i].z, 
                                      20, lighthouseHeight, 20, ObstacleType::BUILDING);
        if (!obs->loadModel(lighthouseModelPath, lighthouseScale)) {
            std::cout << "Lighthouse " << (i+1) << " model not found, using primitives" << std::endl;
        }
        lh.obstacle = obs;
        terrain.push_back(obs);
        
        lighthouses.push_back(lh);
        std::cout << "Lighthouse " << (i+1) << " created at (" 
                  << positions[i].x << ", " << positions[i].y << ", " << positions[i].z 
                  << ") with height " << lighthouseHeight << std::endl;
    }
    
    std::cout << "=== Lighthouses Created: " << lighthouses.size() << " ===\n" << std::endl;
}

void Level2::createBullseyes() {
    std::cout << "\n=== Creating Bullseye Targets ===" << std::endl;
    
    bullseyes.clear();
    bullseyesDestroyed = 0;
    
    bullseyes.push_back(Bullseye(300.324f, 25.0f, 127.959f, 10.0f));
    bullseyes.push_back(Bullseye(-212.699f, 25.0f, 248.028f, 10.0f));
    bullseyes.push_back(Bullseye(23.4846f, 25.0f, 104.306f, 10.0f));
    
    totalBullseyes = (int)bullseyes.size();
    
    for (size_t i = 0; i < bullseyes.size(); i++) {
        std::cout << "Bullseye " << (i+1) << " created at (" 
                  << bullseyes[i].x << ", " << bullseyes[i].y << ", " << bullseyes[i].z << ")" << std::endl;
    }
    
    std::cout << "=== Bullseyes Created: " << totalBullseyes << " ===\n" << std::endl;
}

void Level2::createBonusRings() {
    std::cout << "\n=== Creating Bonus Rings ===" << std::endl;
    
    bonusRings.clear();
    ringsCollected = 0;
    
    bonusRings.push_back(BonusRing(300.324f, 50.0f, 127.959f, 3));
    bonusRings.push_back(BonusRing(-212.699f, 50.0f, 248.028f, 3));
    bonusRings.push_back(BonusRing(23.4846f, 50.0f, 104.306f, 3));
    
    for (size_t i = 0; i < bonusRings.size(); i++) {
        std::cout << "Bonus Ring " << (i+1) << " created at (" 
                  << bonusRings[i].x << ", " << bonusRings[i].y << ", " << bonusRings[i].z 
                  << ") - Awards " << bonusRings[i].rocketBonus << " rockets" << std::endl;
    }
    
    std::cout << "=== Bonus Rings Created: " << bonusRings.size() << " ===\n" << std::endl;
}

void Level2::update(float deltaTime, const bool* keys) {
    if (state != Level2State::PLAYING) {
        if (state == Level2State::WON || state == Level2State::LOST) {
            endScreenTimer += deltaTime;
            
            if (endScreenTimer > 1.0f) {
                if (keys['r'] || keys['R']) {
                    reset();
                    return;
                }
            }
            return;
        }
    }
    
    // Update level timer
    levelTimer.update(deltaTime);
    
    // Check if time ran out
    if (levelTimer.isExpired() && state == Level2State::PLAYING) {
        state = Level2State::LOST;
        endScreenTimer = 0;
        std::cout << "TIME'S UP! Mission Failed!" << std::endl;
        return;
    }
    
    // Toggle day/night with N key
    if ((keys['n'] || keys['N']) && !nKeyWasPressed) {
        toggleDayNight();
        nKeyWasPressed = true;
    } else if (!(keys['n'] || keys['N'])) {
        nKeyWasPressed = false;
    }
    
    // Fire rocket with F key
    if ((keys['f'] || keys['F']) && !fKeyWasPressed) {
        fireRocket();
        fKeyWasPressed = true;
    } else if (!(keys['f'] || keys['F'])) {
        fKeyWasPressed = false;
    }
    
    // Update player
    if (player) {
        player->update(deltaTime, keys);
        if (camera) {
            camera->update(player, deltaTime);
        }
    }
    
    // Update lighting
    if (lighting) {
        lighting->update(deltaTime);
    }
    
    // Update lighthouses
    updateLighthouses(deltaTime);
    
    // Update rockets
    updateRockets(deltaTime);
    
    // Update bullseyes animation
    updateBullseyes(deltaTime);
    
    // Update bonus rings
    updateBonusRings(deltaTime);
    
    // Check bonus ring collection
    checkBonusRingCollisions();
    
    // Check rocket collisions with bullseyes
    checkRocketCollisions();
    
    // Update punishment missile if active
    updatePunishmentMissile(deltaTime);
    
    // Update explosions
    updateExplosions(deltaTime);
    
    // Update debris
    updateDebris(deltaTime);
    
    // Update camera shake
    updateCameraShake(deltaTime);
    
    // Check collisions (ground)
    checkCollisions();
    
    // Update rocket firing cooldown
    if (rocketFireTimer > 0) {
        rocketFireTimer -= deltaTime;
    }
    
    // Check for level completion (WIN condition)
    if (bullseyesDestroyed >= totalBullseyes && state == Level2State::PLAYING) {
        state = Level2State::WON;
        endScreenTimer = 0;
        std::cout << "ALL BULLSEYES DESTROYED! MISSION ACCOMPLISHED!" << std::endl;
        return;
    }
    
    // Check for player death
    if (player && !player->isAlive()) {
        state = Level2State::LOST;
        endScreenTimer = 0;
        return;
    }
    
    // Check if TRULY out of rockets (no rockets remaining AND no rockets in flight)
    // Only then spawn punishment missile or show warning
    bool hasRocketsInFlight = !rockets.empty();
    bool outOfAllRockets = (rocketsRemaining <= 0 && !hasRocketsInFlight);
    bool bullseyesRemain = (bullseyesDestroyed < totalBullseyes);
    
    if (outOfAllRockets && bullseyesRemain && state == Level2State::PLAYING) {
        // Player is truly out of rockets and still has targets - spawn punishment missile
        if (!punishmentMissileActive) {
            punishmentMissileDelay -= deltaTime;
            if (punishmentMissileDelay <= 0) {
                spawnPunishmentMissile();
                std::cout << "OUT OF ROCKETS! Punishment missile launched!" << std::endl;
            }
        }
        missileWarning = true;
        warningFlashTimer += deltaTime * 5.0f;
    } else {
        // Reset warning if player still has options
        if (!punishmentMissileActive) {
            missileWarning = false;
        }
    }
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

void Level2::updateExplosions(float deltaTime) {
    for (auto& explosion : explosions) {
        explosion.timer += deltaTime;
        explosion.scale = 1.0f + (explosion.timer / explosion.duration) * 3.0f;
    }
    
    // Remove finished explosions and disable their lights
    explosions.erase(
        std::remove_if(explosions.begin(), explosions.end(),
            [](const ExplosionEffect& e) {
                if (e.timer >= e.duration && e.lightId >= 0) {
                    glDisable(GL_LIGHT2 + e.lightId);
                }
                return e.timer >= e.duration;
            }),
        explosions.end()
    );
}

void Level2::updateDebris(float deltaTime) {
    for (auto& particle : debris) {
        // Apply gravity
        particle.vy -= 9.8f * deltaTime;
        
        // Update position
        particle.x += particle.vx * deltaTime * 10.0f;
        particle.y += particle.vy * deltaTime * 10.0f;
        particle.z += particle.vz * deltaTime * 10.0f;
        
        // Update rotation
        particle.rx += particle.rotSpeed * deltaTime;
        particle.ry += particle.rotSpeed * deltaTime * 1.3f;
        particle.rz += particle.rotSpeed * deltaTime * 0.7f;
        
        // Decay life
        particle.life -= deltaTime * 0.8f;
    }
    
    // Remove dead debris
    debris.erase(
        std::remove_if(debris.begin(), debris.end(),
            [](const DebrisParticle& p) { return p.life <= 0.0f || p.y < -10.0f; }),
        debris.end()
    );
}

void Level2::updateCameraShake(float deltaTime) {
    if (cameraShakeTimer > 0) {
        cameraShakeTimer -= deltaTime;
        if (cameraShakeTimer <= 0) {
            cameraShakeIntensity = 0;
            cameraShakeTimer = 0;
        }
    }
}

void Level2::updateLighthouses(float deltaTime) {
    if (lighting) {
        lighting->updateLighthouseBeam(deltaTime);
    }
}

void Level2::updateRockets(float deltaTime) {
    for (auto& rocket : rockets) {
        if (!rocket.active) continue;
        rocket.x += rocket.dirX * rocket.speed * deltaTime * 60.0f;
        rocket.y += rocket.dirY * rocket.speed * deltaTime * 60.0f;
        rocket.z += rocket.dirZ * rocket.speed * deltaTime * 60.0f;
        rocket.lifetime += deltaTime;
        if (rocket.lifetime >= rocket.maxLifetime) {
            rocket.active = false;
            triggerExplosion(rocket.x, rocket.y, rocket.z);
            spawnDebris(rocket.x, rocket.y, rocket.z, 5);
            playSound(explosionSoundPath);
        }
    }
    rockets.erase(std::remove_if(rockets.begin(), rockets.end(),
        [](const Rocket& r) { return !r.active; }), rockets.end());
}

void Level2::updateBullseyes(float deltaTime) {
    for (auto& bullseye : bullseyes) {
        if (!bullseye.destroyed) {
            bullseye.rotationAngle += deltaTime * 45.0f;
            if (bullseye.rotationAngle > 360.0f) bullseye.rotationAngle -= 360.0f;
        }
    }
}

void Level2::updateBonusRings(float deltaTime) {
    for (auto& ring : bonusRings) {
        if (!ring.collected) {
            ring.rotationAngle += deltaTime * 90.0f;
            if (ring.rotationAngle > 360.0f) ring.rotationAngle -= 360.0f;
        }
    }
}

void Level2::checkBonusRingCollisions() {
    if (!player || !player->isAlive()) return;
    float px, py, pz;
    player->getPosition(px, py, pz);
    float playerRadius = player->getBoundingRadius();
    for (auto& ring : bonusRings) {
        if (ring.collected) continue;
        float dx = px - ring.x, dy = py - ring.y, dz = pz - ring.z;
        float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
        if (distance < ring.radius + playerRadius + 5.0f) {
            ring.collected = true;
            ringsCollected++;
            rocketsRemaining += ring.rocketBonus;
            if (rocketsRemaining > maxRockets) rocketsRemaining = maxRockets;
            score += 200;
            punishmentMissileDelay = 2.0f;
            missileWarning = false;
            playSound(explosionSoundPath);
            std::cout << "BONUS RING COLLECTED! +" << ring.rocketBonus << " rockets!" << std::endl;
        }
    }
}

void Level2::checkRocketCollisions() {
    for (auto& rocket : rockets) {
        if (!rocket.active) continue;
        for (auto& bullseye : bullseyes) {
            if (bullseye.destroyed) continue;
            float dx = rocket.x - bullseye.x, dy = rocket.y - bullseye.y, dz = rocket.z - bullseye.z;
            float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (distance < bullseye.radius + 2.0f) {
                bullseye.destroyed = true;
                rocket.active = false;
                bullseyesDestroyed++;
                score += 500;
                triggerExplosion(bullseye.x, bullseye.y, bullseye.z);
                spawnDebris(bullseye.x, bullseye.y, bullseye.z, 15);
                playSound(explosionSoundPath);
                triggerCameraShake(5.0f, 0.8f);
                if (lighting) lighting->flashEffect(0.5f);
                std::cout << "BULLSEYE HIT! (" << bullseyesDestroyed << "/" << totalBullseyes << ")" << std::endl;
                break;
            }
        }
    }
}

void Level2::updatePunishmentMissile(float deltaTime) {
    if (!punishmentMissileActive || !punishmentMissile) return;
    if (!player || !player->isAlive()) {
        delete punishmentMissile;
        punishmentMissile = nullptr;
        punishmentMissileActive = false;
        return;
    }
    punishmentMissile->setTargetPlayer(player);
    punishmentMissile->update(deltaTime);
    float mx, my, mz;
    punishmentMissile->getPosition(mx, my, mz);
    float px, py, pz;
    player->getPosition(px, py, pz);
    float dx = mx - px, dy = my - py, dz = mz - pz;
    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (distance < 5.0f) {
        player->kill();
        triggerExplosion(px, py, pz);
        spawnDebris(px, py, pz, 30);
        playSound(explosionSoundPath);
        triggerCameraShake(10.0f, 1.5f);
        if (lighting) lighting->flashEffect(0.8f);
        delete punishmentMissile;
        punishmentMissile = nullptr;
        punishmentMissileActive = false;
    }
}

void Level2::spawnPunishmentMissile() {
    if (punishmentMissileActive) return;
    float px, py, pz;
    player->getPosition(px, py, pz);
    float spawnX = px - 100.0f, spawnY = py + 50.0f, spawnZ = pz - 100.0f;
    float dx = px - spawnX, dy = py - spawnY, dz = pz - spawnZ;
    float len = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (len > 0) { dx /= len; dy /= len; dz /= len; }
    punishmentMissile = new Missile(spawnX, spawnY, spawnZ, dx, dy, dz, false);
    punishmentMissile->setSpeed(2.5f);
    punishmentMissile->setHoming(true);
    punishmentMissile->setTurnRate(60.0f);
    punishmentMissile->setTargetPlayer(player);
    std::string missileModelPath = findAssetPath("assets/missle/mk82snak_obj/Mk 82 Snakeye.obj");
    punishmentMissile->loadModel(missileModelPath, 0.5f);
    punishmentMissileActive = true;
    playSound(missileLaunchSoundPath);
}

void Level2::fireRocket() {
    if (!player || !player->isAlive()) return;
    if (rocketFireTimer > 0) return;
    if (rocketsRemaining <= 0) { std::cout << "OUT OF ROCKETS!" << std::endl; return; }
    float px, py, pz, pitch, yaw, roll;
    player->getPosition(px, py, pz);
    player->getRotation(pitch, yaw, roll);
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;
    float forwardX = std::sin(radYaw) * std::cos(radPitch);
    float forwardY = -std::sin(radPitch);
    float forwardZ = std::cos(radYaw) * std::cos(radPitch);
    float spawnDist = 8.0f;
    Rocket rocket(px + forwardX * spawnDist, py + forwardY * spawnDist, pz + forwardZ * spawnDist, forwardX, forwardY, forwardZ);
    rocket.speed = 4.0f;
    rocket.maxLifetime = 3.0f;
    rockets.push_back(rocket);
    rocketsRemaining--;
    rocketFireTimer = rocketFireCooldown;
    playSound(missileLaunchSoundPath);
}

void Level2::spawnLighthouseMissile() { }
int Level2::findClosestLighthouse() { return -1; }

void Level2::checkCollisions() {
    if (!player || !player->isAlive()) return;
    float px, py, pz;
    player->getPosition(px, py, pz);
    if (py < -20.0f) {
        player->kill();
        triggerExplosion(px, py, pz);
        spawnDebris(px, py, pz, 20);
        playSound(explosionSoundPath);
        triggerCameraShake(8.0f, 1.0f);
        if (lighting) lighting->flashEffect(0.6f);
    }
}

void Level2::checkMissileCollisions() { }
void Level2::checkNearMisses(float deltaTime) { }
void Level2::fireMissile() { }
void Level2::updateLockOn(float deltaTime) { }
void Level2::updateEnemies(float deltaTime) { }

void Level2::triggerExplosion(float x, float y, float z) {
    explosions.push_back(ExplosionEffect(x, y, z));
}

void Level2::triggerCameraShake(float intensity, float duration) {
    cameraShakeIntensity = intensity;
    cameraShakeDuration = duration;
    cameraShakeTimer = duration;
}

void Level2::spawnDebris(float x, float y, float z, int count) {
    for (int i = 0; i < count; i++) debris.push_back(DebrisParticle(x, y, z));
}

void Level2::playSound(const std::string& soundPath) {
#ifdef _WIN32
    PlaySoundA(soundPath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
#endif
}

float Level2::distanceToPlayer(float x, float y, float z) {
    if (!player) return 999999.0f;
    float px, py, pz;
    player->getPosition(px, py, pz);
    return std::sqrt((x-px)*(x-px) + (y-py)*(y-py) + (z-pz)*(z-pz));
}

bool Level2::isInSafeZone(float x, float y, float z) { return false; }
Enemy* Level2::findNearestEnemy() { return nullptr; }
bool Level2::isEnemyInSights(Enemy* e, float& d, float& a) { return false; }

bool Level2::isWon() const { return state == Level2State::WON; }
bool Level2::isLost() const { return state == Level2State::LOST; }
void Level2::reset() { cleanup(); init(); }
void Level2::toggleDayNight() { if (lighting) lighting->toggleDayNight(); }
void Level2::handleMouse(int button, int st, int x, int y) { 
    handleMouseButton(button, st, x, y); 
}

void Level2::handleMouseMotion(int x, int y) { 
    // Pass mouse motion to camera for orbit control
    if (camera) {
        camera->handleMouseMotion(x, y);
    }
}

void Level2::handleKeyPress(unsigned char key, bool pressed) { 
    if ((key == 'r' || key == 'R') && pressed) reset();
    // C key toggles camera view
    if ((key == 'c' || key == 'C') && pressed && camera) {
        camera->toggle();
        std::cout << "Camera: " << (camera->isFirstPerson() ? "First Person" : "Third Person") << std::endl;
    }
}

void Level2::handleMouseButton(int button, int st, int x, int y) {
    // Right click toggles camera view
    if (button == GLUT_RIGHT_BUTTON && st == GLUT_DOWN && camera) {
        camera->toggle();
        std::cout << "Camera: " << (camera->isFirstPerson() ? "First Person" : "Third Person") << std::endl;
    }
    
    // Pass mouse button events to camera for orbit control
    if (camera) {
        int buttonIndex = 0;
        if (button == GLUT_LEFT_BUTTON) buttonIndex = 0;
        else if (button == GLUT_MIDDLE_BUTTON) buttonIndex = 1;
        else if (button == GLUT_RIGHT_BUTTON) buttonIndex = 2;
        
        camera->handleMouseButton(buttonIndex, st == GLUT_DOWN, x, y);
    }
}

void Level2::handleMouseMove(int x, int y) {
    // Pass mouse motion to camera for orbit control (passive motion)
    if (camera) {
        camera->handleMouseMotion(x, y);
    }
}

void Level2::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1280.0/720.0, 0.1, 1000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    if (camera) camera->apply();
    
    if (cameraShakeTimer > 0) {
        float shakeX = ((rand() % 200 - 100) / 100.0f) * cameraShakeIntensity * 0.1f;
        float shakeY = ((rand() % 200 - 100) / 100.0f) * cameraShakeIntensity * 0.1f;
        float shakeZ = ((rand() % 200 - 100) / 100.0f) * cameraShakeIntensity * 0.1f;
        glTranslatef(shakeX, shakeY, shakeZ);
    }
    
    if (lighting) lighting->apply();
    applyExplosionLights();
    
    renderSky();
    
    for (auto* obstacle : terrain) {
        if (obstacle) obstacle->render();
    }
    
    renderLighthouses();
    renderBullseyes();
    renderBonusRings();
    renderRockets();
    
    // Render player only in third-person view (not in first-person cockpit view)
    if (player && player->isAlive() && camera && !camera->isFirstPerson()) {
        player->render();
    }
    
    for (auto* missile : missiles) {
        if (missile) missile->render();
    }
    
    if (punishmentMissileActive && punishmentMissile) {
        punishmentMissile->render();
    }
    
    renderExplosions();
    renderDebris();
    renderHUD();
    
    if (lockOnState != LockOnState::NONE) renderLockOnReticle();
    if (missileWarning) renderMissileWarning();
    
    renderMessages();
    
    glutSwapBuffers();
}

void Level2::renderLighthouses() {
    if (lighthouses.empty() || !lighting) return;
    if (!lighting->isNightMode()) return;
    
    float baseAngle = lighting->getLighthouseAngle();
    
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    
    for (size_t i = 0; i < lighthouses.size(); i++) {
        const Lighthouse& lh = lighthouses[i];
        float angle = baseAngle + (i * 72.0f);
        
        // Position beam lower - subtract offset from total height
        float beamY = lh.y + lh.height - 10.0f;
        
        glPushMatrix();
        glTranslatef(lh.x, beamY, lh.z);
        glRotatef(angle, 0.0f, 1.0f, 0.0f);
        glRotatef(-8.0f, 1.0f, 0.0f, 0.0f);
        
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 0.95f, 0.8f, 0.7f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glColor4f(1.0f, 0.9f, 0.7f, 0.0f);
        for (int j = 0; j <= 32; j++) {
            float a = (float)j / 32 * 2.0f * M_PI;
            glVertex3f(std::sin(a) * 200.0f, 0.0f, std::cos(a) * 200.0f + 500.0f);
        }
        glEnd();
        
        // Glowing orb at light source
        glColor4f(1.0f, 0.95f, 0.7f, 0.9f);
        glutSolidSphere(4.0f, 16, 16);
        
        glPopMatrix();
    }
    
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void Level2::renderBullseyes() {
    glEnable(GL_LIGHTING);
    
    for (const auto& bullseye : bullseyes) {
        if (bullseye.destroyed) continue;
        
        glPushMatrix();
        glTranslatef(bullseye.x, bullseye.y, bullseye.z);
        glRotatef(bullseye.rotationAngle, 0.0f, 1.0f, 0.0f);
        
        GLfloat matWhite[] = {1.0f, 1.0f, 1.0f, 1.0f};
        GLfloat matRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
        
        float ringRadius = bullseye.radius;
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matWhite);
        glPushMatrix();
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidTorus(1.0f, ringRadius, 16, 32);
        glPopMatrix();
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matRed);
        glPushMatrix();
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidTorus(1.0f, ringRadius * 0.7f, 16, 32);
        glPopMatrix();
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matWhite);
        glPushMatrix();
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidTorus(1.0f, ringRadius * 0.4f, 16, 32);
        glPopMatrix();
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matRed);
        glutSolidSphere(ringRadius * 0.15f, 16, 16);
        
        glPopMatrix();
    }
}

void Level2::renderBonusRings() {
    glEnable(GL_LIGHTING);
    
    for (const auto& ring : bonusRings) {
        if (ring.collected) continue;
        
        glPushMatrix();
        glTranslatef(ring.x, ring.y, ring.z);
        glRotatef(ring.rotationAngle, 0.0f, 1.0f, 0.0f);
        
        GLfloat matGold[] = {1.0f, 0.85f, 0.0f, 1.0f};
        GLfloat matEmission[] = {0.5f, 0.4f, 0.0f, 1.0f};
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matGold);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, matEmission);
        
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidTorus(1.5f, ring.radius, 16, 32);
        
        GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);
        
        glPopMatrix();
    }
}

void Level2::renderRockets() {
    glDisable(GL_LIGHTING);
    
    for (const auto& rocket : rockets) {
        if (!rocket.active) continue;
        
        glPushMatrix();
        glTranslatef(rocket.x, rocket.y, rocket.z);
        
        float yaw = std::atan2(rocket.dirX, rocket.dirZ) * 180.0f / M_PI;
        float pitch = std::asin(-rocket.dirY) * 180.0f / M_PI;
        
        glRotatef(yaw, 0, 1, 0);
        glRotatef(pitch, 1, 0, 0);
        
        glColor3f(0.3f, 0.3f, 0.3f);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(0.5f, 3.0f, 8, 4);
        glPopMatrix();
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(1.0f, 1.0f, 0.3f, 0.9f);
        glPushMatrix();
        glTranslatef(0, 0, -1.5f);
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(0.3f, 1.5f, 8, 2);
        glPopMatrix();
        glDisable(GL_BLEND);
        
        glPopMatrix();
    }
    
    glEnable(GL_LIGHTING);
}

void Level2::renderHUD() {
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
    
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(10, 640); glVertex2f(280, 640);
    glVertex2f(280, 710); glVertex2f(10, 710);
    glEnd();
    
    glBegin(GL_QUADS);
    glVertex2f(1020, 640); glVertex2f(1270, 640);
    glVertex2f(1270, 710); glVertex2f(1020, 710);
    glEnd();
    
    glDisable(GL_BLEND);
    
    char buffer[128];
    
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(buffer, "Score: %d", score);
    glRasterPos2f(20, 685);
    for (char* c = buffer; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    
    glColor3f(1.0f, 0.3f, 0.3f);
    sprintf(buffer, "TARGETS: %d/%d", totalBullseyes - bullseyesDestroyed, totalBullseyes);
    glRasterPos2f(20, 658);
    for (char* c = buffer; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    
    float timeLeft = levelTimer.getTime();
    if (timeLeft < 10.0f) glColor3f(1.0f, 0.0f, 0.0f);
    else if (timeLeft < 20.0f) glColor3f(1.0f, 1.0f, 0.0f);
    else glColor3f(0.0f, 1.0f, 0.0f);
    sprintf(buffer, "TIME: %.1f", timeLeft);
    glRasterPos2f(1040, 685);
    for (char* c = buffer; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    
    if (rocketsRemaining <= 0) glColor3f(1.0f, 0.0f, 0.0f);
    else if (rocketsRemaining <= 2) glColor3f(1.0f, 1.0f, 0.0f);
    else glColor3f(0.0f, 1.0f, 0.0f);
    sprintf(buffer, "ROCKETS: %d", rocketsRemaining);
    glRasterPos2f(1040, 658);
    for (char* c = buffer; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    
    if (rocketsRemaining <= 0) {
        float pulse = 0.5f + 0.5f * std::sin(warningFlashTimer);
        glColor3f(pulse, 0.0f, 0.0f);
        sprintf(buffer, "OUT OF ROCKETS! COLLECT RINGS!");
        glRasterPos2f(450, 680);
        for (char* c = buffer; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    glColor3f(0.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(630, 360); glVertex2f(650, 360);
    glVertex2f(640, 350); glVertex2f(640, 370);
    glEnd();
    glLineWidth(1.0f);
    
    glColor3f(0.7f, 0.7f, 0.7f);
    sprintf(buffer, "F: Fire Rocket | N: Day/Night | Collect RINGS for more rockets!");
    glRasterPos2f(380, 20);
    for (char* c = buffer; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Level2::renderSky() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(60.0, 1280.0/720.0, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    GLfloat m[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, m);
    m[12] = 0; m[13] = 0; m[14] = 0;
    glLoadMatrixf(m);
    
    bool isNight = lighting && lighting->isNightMode();
    glBegin(GL_QUADS);
    if (isNight) {
        glColor3f(0.01f, 0.01f, 0.05f);
        glVertex3f(-500, 200, -500); glVertex3f(500, 200, -500);
        glColor3f(0.02f, 0.02f, 0.08f);
        glVertex3f(500, -50, -500); glVertex3f(-500, -50, -500);
    } else {
        glColor3f(0.3f, 0.5f, 0.8f);
        glVertex3f(-500, 200, -500); glVertex3f(500, 200, -500);
        glColor3f(0.6f, 0.8f, 1.0f);
        glVertex3f(500, -50, -500); glVertex3f(-500, -50, -500);
    }
    glEnd();
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Level2::renderExplosions() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto& e : explosions) {
        glPushMatrix();
        glTranslatef(e.x, e.y, e.z);
        float progress = e.timer / e.duration;
        float alpha = 1.0f - progress;
        glScalef(e.scale, e.scale, e.scale);
        glColor4f(1.0f, 0.5f, 0.0f, alpha * 0.8f);
        glutSolidSphere(5.0f, 16, 16);
        glColor4f(1.0f, 1.0f, 0.3f, alpha);
        glutSolidSphere(3.0f, 12, 12);
        glPopMatrix();
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void Level2::renderDebris() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto& p : debris) {
        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        glRotatef(p.rx, 1, 0, 0);
        glRotatef(p.ry, 0, 1, 0);
        glRotatef(p.rz, 0, 0, 1);
        glScalef(p.size, p.size, p.size);
        glColor4f(0.3f, 0.3f, 0.3f, p.life * 0.8f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void Level2::renderLockOnReticle() { }

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
    float flash = std::abs(std::sin(warningFlashTimer));
    glColor4f(1.0f, 0.0f, 0.0f, flash * 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(50, 0); glVertex2f(50, 720); glVertex2f(0, 720);
    glVertex2f(1230, 0); glVertex2f(1280, 0); glVertex2f(1280, 720); glVertex2f(1230, 720);
    glEnd();
    glColor3f(1.0f, 0.0f, 0.0f);
    const char* warning = "MISSILE WARNING!";
    glRasterPos2f(550, 650);
    for (const char* c = warning; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Level2::renderMessages() {
    if (state == Level2State::PLAYING) return;
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
    
    float overlayAlpha = std::min(0.85f, endScreenTimer);
    glColor4f(0.0f, 0.0f, 0.0f, overlayAlpha);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(1280, 0); glVertex2f(1280, 720); glVertex2f(0, 720);
    glEnd();
    
    char buffer[128];
    float pulse = 0.8f + 0.2f * std::sin(endScreenTimer * 3.0f);
    
    if (state == Level2State::WON) {
        glColor3f(0.2f * pulse, 1.0f * pulse, 0.2f * pulse);
        sprintf(buffer, "ALL TARGETS DESTROYED!");
        glRasterPos2f(460, 420);
        for (char* c = buffer; *c; c++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        glColor3f(1.0f, 1.0f, 1.0f);
        sprintf(buffer, "Score: %d", score);
        glRasterPos2f(560, 340);
        for (char* c = buffer; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        glColor3f(1.0f, 0.9f, 0.2f);
        sprintf(buffer, "Press R to restart");
        glRasterPos2f(540, 240);
        for (char* c = buffer; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    } else if (state == Level2State::LOST) {
        glColor3f(1.0f * pulse, 0.2f * pulse, 0.2f * pulse);
        sprintf(buffer, "MISSION FAILED");
        glRasterPos2f(520, 420);
        for (char* c = buffer; *c; c++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        glColor3f(1.0f, 1.0f, 1.0f);
        sprintf(buffer, "Score: %d", score);
        glRasterPos2f(570, 340);
        for (char* c = buffer; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        glColor3f(1.0f, 0.9f, 0.2f);
        sprintf(buffer, "Press R to restart");
        glRasterPos2f(540, 240);
        for (char* c = buffer; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Level2::applyExplosionLights() {
    int lightIndex = 0;
    for (auto& e : explosions) {
        if (lightIndex >= 6) break;
        GLenum lightId = GL_LIGHT2 + lightIndex;
        e.lightId = lightIndex;
        glEnable(lightId);
        float progress = e.timer / e.duration;
        float intensity = (1.0f - progress) * 0.8f;
        GLfloat position[] = {e.x, e.y, e.z, 1.0f};
        GLfloat diffuse[] = {intensity, intensity * 0.5f, intensity * 0.1f, 1.0f};
        glLightfv(lightId, GL_POSITION, position);
        glLightfv(lightId, GL_DIFFUSE, diffuse);
        lightIndex++;
    }
}

void Level2::cleanup() {
    if (player) { delete player; player = nullptr; }
    
    for (auto* enemy : enemies) delete enemy;
    enemies.clear();
    
    for (auto* missile : missiles) delete missile;
    missiles.clear();
    
    if (punishmentMissile) { delete punishmentMissile; punishmentMissile = nullptr; }
    punishmentMissileActive = false;
    
    rockets.clear();
    bullseyes.clear();
    bonusRings.clear();
    lighthouses.clear();
    
    for (auto* obstacle : terrain) delete obstacle;
    terrain.clear();
    
    explosions.clear();
    debris.clear();
    
    if (camera) { delete camera; camera = nullptr; }
    if (lighting) { delete lighting; lighting = nullptr; }
    
    finaleTarget = nullptr;
}
