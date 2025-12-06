#include "Game.h"
#include "Level1.h"
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
      deltaTime(0.016f),
      windowWidth(1280),
      windowHeight(720),
      pauseKeyPressed(false) {
}

Game::~Game() {
    cleanup();
}

void Game::init() {
    std::cout << "========================================" << std::endl;
    std::cout << "    TOP GUN MAVERICK FLIGHT SIMULATOR  " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  W/S    : Pitch Up/Down" << std::endl;
    std::cout << "  A/D    : Roll Left/Right" << std::endl;
    std::cout << "  Q/E    : Yaw Left/Right" << std::endl;
    std::cout << "  1/2    : Decrease/Increase Speed" << std::endl;
    std::cout << "  SPACE  : Barrel Roll" << std::endl;
    std::cout << "  C      : Toggle Camera" << std::endl;
    std::cout << "  N      : Toggle Day/Night" << std::endl;
    std::cout << "  G      : Print Debug Position" << std::endl;
    std::cout << "  R      : Restart Level" << std::endl;
    std::cout << "  P      : Pause" << std::endl;
    std::cout << "  ESC    : Quit" << std::endl;
    std::cout << std::endl;
    std::cout << "Mouse:" << std::endl;
    std::cout << "  Left-click + drag : Orbit camera (3rd person)" << std::endl;
    std::cout << "  Right-click       : Toggle camera view" << std::endl;
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
    
    // Load Level 1
    loadLevel(1);
    state = GameState::PLAYING;
}

void Game::update(float dt) {
    deltaTime = dt;
    
    // Handle ESC key for quit
    if (input.isKeyPressed(27)) {  // ESC
        std::cout << "Exiting game..." << std::endl;
        cleanup();
        exit(0);
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
    
    if (state == GameState::PLAYING && currentLevel) {
        currentLevel->update(dt, input.getKeys());
        
        // Check for level completion
        if (currentLevel->isWon()) {
            std::cout << "Level " << currentLevelIndex << " complete!" << std::endl;
            // Could transition to next level here
            // For now, just stay on win screen
        }
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
}

void Game::handleKeyPress(unsigned char key, bool pressed) {
    input.setKey(key, pressed);
    
    // Handle camera toggle on key press
    if (pressed && (key == 'c' || key == 'C')) {
        if (currentLevel) {
            Level1* level1 = dynamic_cast<Level1*>(currentLevel);
            if (level1 && level1->getCamera()) {
                level1->getCamera()->toggle();
                std::cout << "Camera: " 
                          << (level1->getCamera()->isFirstPerson() ? "First Person" : "Third Person") 
                          << std::endl;
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
            currentLevel = new Level1();
            break;
        case 2:
            // Level 2 would go here
            std::cout << "Level 2 not yet implemented" << std::endl;
            currentLevel = new Level1();  // Fallback to Level 1
            break;
        default:
            std::cout << "Invalid level index: " << levelIndex << std::endl;
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
        state = GameState::PLAYING;
    }
}
