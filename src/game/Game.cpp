#include "Game.h"
#include "Level1.h"
#include "Level2.h"
#include "CoopMode.h"
#include <iostream>
#include <cstdlib>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
// On Windows, GLEW must be included before GL headers
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

Game::Game()
    : state(GameState::MENU),
      currentLevel(nullptr),
      currentLevelIndex(0),
      menuSystem(nullptr),
      deltaTime(0.016f),
      windowWidth(1280),
      windowHeight(720),
      pauseKeyPressed(false),
      lKeyPressed(false) {
}

Game::~Game() {
    cleanup();
}

void Game::init() {
    std::cout << "========================================" << std::endl;
    std::cout << "    TOP GUN MAVERICK FLIGHT SIMULATOR  " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // OpenGL initialization
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    
    // Set clear color (sky blue)
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    
    // Enable back-face culling for performance
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Create menu system
    menuSystem = new MenuSystem();
    state = GameState::MENU;
    
    std::cout << "Use UP/DOWN arrows to navigate menu, ENTER to select" << std::endl;
}

void Game::update(float dt) {
    deltaTime = dt;
    
    // Handle ESC key for quit
    if (input.isKeyPressed(27)) {  // ESC
        std::cout << "Exiting game..." << std::endl;
        cleanup();
        exit(0);
    }
    
    // Handle menu state
    if (state == GameState::MENU) {
        if (menuSystem) {
            menuSystem->update(dt, input.getKeys());
            
            if (menuSystem->isOptionConfirmed()) {
                MenuOption option = menuSystem->getSelectedOption();
                menuSystem->resetConfirmation();
                
                if (option == MenuOption::SINGLE_PLAYER) {
                    std::cout << "Starting Single Player mode..." << std::endl;
                    loadLevel(1);
                    state = GameState::PLAYING;
                } else if (option == MenuOption::COOP_MODE) {
                    std::cout << "Starting Co-op mode..." << std::endl;
                    loadCoopMode();
                    state = GameState::COOP_MODE;
                } else if (option == MenuOption::EXIT) {
                    std::cout << "Exiting game..." << std::endl;
                    cleanup();
                    exit(0);
                }
            }
        }
        return;
    }
    
    // Handle pause toggle
    if (input.isKeyPressed('p') || input.isKeyPressed('P')) {
        if (!pauseKeyPressed) {
            togglePause();
            pauseKeyPressed = true;
        }
    } else {
        pauseKeyPressed = false;
    }
    
    if (state == GameState::PAUSED) {
        return;  // Don't update game logic when paused
    }
    
    if ((state == GameState::PLAYING || state == GameState::COOP_MODE) && currentLevel) {
        currentLevel->update(dt, input.getKeys());
        
        // Check for level completion
        if (currentLevel->isWon()) {
            state = GameState::LEVEL_COMPLETE;
            std::cout << "Level " << currentLevelIndex << " complete!" << std::endl;
            std::cout << "Press 'L' to continue to next level..." << std::endl;
        }
        
        // Check for level loss
        if (currentLevel->isLost()) {
            state = GameState::GAME_OVER;
            std::cout << "Game Over! Press 'R' to restart level." << std::endl;
        }
    }
    
    // Handle level transition input with debouncing
    if (state == GameState::LEVEL_COMPLETE) {
        bool lPressed = input.isKeyPressed('l') || input.isKeyPressed('L');
        if (lPressed && !lKeyPressed) {
            std::cout << "L key detected! Current level: " << currentLevelIndex << ", MAX_LEVELS: " << MAX_LEVELS << std::endl;
            if (currentLevelIndex < MAX_LEVELS) {
                std::cout << "Loading level " << (currentLevelIndex + 1) << "..." << std::endl;
                loadLevel(currentLevelIndex + 1);
            } else {
                std::cout << "Congratulations! You've completed all levels!" << std::endl;
                state = GameState::GAME_OVER;
            }
            lKeyPressed = true;
        } else if (!lPressed) {
            lKeyPressed = false;
        }
    }
    
    // Handle restart on game over
    if (state == GameState::GAME_OVER && (input.isKeyPressed('r') || input.isKeyPressed('R'))) {
        loadLevel(currentLevelIndex);
    }
}

void Game::render() {
    if (currentLevel) {
        currentLevel->render();
    }
    
    // Render pause overlay if paused
    if (state == GameState::PAUSED) {
        renderPauseOverlay();
    }
}

void Game::renderPauseOverlay() {
    // Switch to 2D
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
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
    glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(0, windowHeight);
    glEnd();
    glDisable(GL_BLEND);
    
    // Pause text
    glColor3f(1.0f, 1.0f, 1.0f);
    const char* pauseText = "PAUSED";
    glRasterPos2f(windowWidth / 2 - 40, windowHeight / 2);
    for (const char* c = pauseText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
    
    const char* resumeText = "Press P to resume";
    glRasterPos2f(windowWidth / 2 - 70, windowHeight / 2 - 40);
    for (const char* c = resumeText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    glutSwapBuffers();
}

void Game::cleanup() {
    if (currentLevel) {
        currentLevel->cleanup();
        delete currentLevel;
        currentLevel = nullptr;
    }
    
    if (menuSystem) {
        delete menuSystem;
        menuSystem = nullptr;
    }
}

void Game::handleKeyPress(unsigned char key, bool pressed) {
    input.setKey(key, pressed);
    
    // Handle menu input
    if (state == GameState::MENU && menuSystem) {
        menuSystem->handleKeyPress(key, pressed);
        return;
    }
    
    // Handle camera toggle on key press
    if (pressed && (key == 'c' || key == 'C')) {
        if (currentLevel) {
            // Try Level1 first
            Level1* level1 = dynamic_cast<Level1*>(currentLevel);
            if (level1 && level1->getCamera()) {
                level1->getCamera()->toggle();
                std::cout << "Camera: " 
                          << (level1->getCamera()->isFirstPerson() ? "First Person" : "Third Person") 
                          << std::endl;
            } else {
                // Try Level2
                Level2* level2 = dynamic_cast<Level2*>(currentLevel);
                if (level2 && level2->getCamera()) {
                    level2->getCamera()->toggle();
                    std::cout << "Camera: " 
                              << (level2->getCamera()->isFirstPerson() ? "First Person" : "Third Person") 
                              << std::endl;
                }
            }
        }
    }
}

void Game::handleSpecialKey(int key, bool pressed) {
    input.setSpecialKey(key, pressed);
}

void Game::handleMouse(int button, int buttonState, int x, int y) {
    input.setMousePosition(x, y);
    
    // Map GLUT button constants
    int buttonIndex = 0;
    if (button == GLUT_LEFT_BUTTON) buttonIndex = 0;
    else if (button == GLUT_MIDDLE_BUTTON) buttonIndex = 1;
    else if (button == GLUT_RIGHT_BUTTON) buttonIndex = 2;
    
    input.setMouseButton(buttonIndex, buttonState == GLUT_DOWN);
    
    // Forward to current level
    if (currentLevel) {
        currentLevel->handleMouse(button, buttonState, x, y);
    }
}

void Game::handleMouseMotion(int x, int y) {
    input.setMousePosition(x, y);
    
    // Forward mouse motion to current level for camera orbit control
    if (currentLevel) {
        currentLevel->handleMouseMotion(x, y);
    }
}

void Game::handleReshape(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

void Game::loadLevel(int levelIndex) {
    // Clean up existing level
    if (currentLevel) {
        currentLevel->cleanup();
        delete currentLevel;
        currentLevel = nullptr;
    }
    
    currentLevelIndex = levelIndex;
    
    switch (levelIndex) {
        case 1:
            std::cout << "Loading Level 1: Terrain Navigation..." << std::endl;
            currentLevel = new Level1();
            break;
        case 2:
            std::cout << "Loading Level 2: Aerial Combat..." << std::endl;
            currentLevel = new Level2();
            break;
        default:
            std::cout << "Invalid level index: " << levelIndex << ", loading Level 1" << std::endl;
            currentLevel = new Level1();
            break;
    }
    
    if (currentLevel) {
        currentLevel->init();
        std::cout << "Loaded: " << currentLevel->getName() << std::endl;
    }
    
    state = GameState::PLAYING;
}

void Game::nextLevel() {
    if (currentLevelIndex < MAX_LEVELS) {
        loadLevel(currentLevelIndex + 1);
    } else {
        std::cout << "Congratulations! You've completed all levels!" << std::endl;
        state = GameState::GAME_OVER;
    }
}

void Game::togglePause() {
    if (state == GameState::PLAYING) {
        state = GameState::PAUSED;
        std::cout << "Game Paused" << std::endl;
    } else if (state == GameState::PAUSED) {
        state = GameState::PLAYING;
        std::cout << "Game Resumed" << std::endl;
    }
}

void Game::restartLevel() {
    if (currentLevel) {
        currentLevel->restart();
        if (state == GameState::COOP_MODE) {
            state = GameState::COOP_MODE;
        } else {
            state = GameState::PLAYING;
        }
    }
}

void Game::loadCoopMode() {
    // Clean up existing level
    if (currentLevel) {
        currentLevel->cleanup();
        delete currentLevel;
        currentLevel = nullptr;
    }
    
    std::cout << "Loading Co-op Dogfight Mode..." << std::endl;
    currentLevel = new CoopMode();
    
    if (currentLevel) {
        currentLevel->init();
        std::cout << "Loaded: " << currentLevel->getName() << std::endl;
    }
    
    state = GameState::COOP_MODE;
}
