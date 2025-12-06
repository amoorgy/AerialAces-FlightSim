/**
 * @file main.cpp
 * @brief Top Gun Maverick Flight Simulator - Entry Point
 * 
 * A 3D flight simulator game for Computer Graphics course.
 * Level 1: Terrain Navigation - Fly through canyon collecting rings
 * 
 * Controls:
 *   W/S   - Pitch up/down
 *   A/D   - Roll left/right
 *   Q/E   - Yaw left/right
 *   1/2   - Decrease/Increase speed
 *   Space - Barrel roll
 *   C     - Toggle camera (first/third person)
 *   N     - Toggle day/night mode
 *   R     - Restart level
 *   P     - Pause
 *   ESC   - Quit
 *   Right-click - Toggle camera
 * 
 * @author AerialAces Team
 * @date December 2025
 */

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

#include "game/Game.h"

// Window settings
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const char* WINDOW_TITLE = "Top Gun Maverick - Flight Simulator";

// Global game instance
Game* game = nullptr;

// Timing
int previousTime = 0;

/**
 * Display callback - renders the game
 */
void display() {
    if (game) {
        game->render();
    }
}

/**
 * Timer callback - updates game logic at ~60 FPS
 */
void update(int value) {
    // Calculate delta time
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - previousTime) / 1000.0f;
    previousTime = currentTime;
    
    // Clamp delta time to prevent large jumps
    if (deltaTime > 0.1f) deltaTime = 0.1f;
    if (deltaTime < 0.001f) deltaTime = 0.016f;
    
    if (game) {
        game->update(deltaTime);
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);  // ~60 FPS
}

/**
 * Keyboard callback - handles key presses
 */
void keyboard(unsigned char key, int x, int y) {
    if (game) {
        game->handleKeyPress(key, true);
    }
}

/**
 * Keyboard up callback - handles key releases
 */
void keyboardUp(unsigned char key, int x, int y) {
    if (game) {
        game->handleKeyPress(key, false);
    }
}

/**
 * Special key callback - handles arrow keys, function keys, etc.
 */
void specialKeyboard(int key, int x, int y) {
    if (game) {
        game->handleSpecialKey(key, true);
    }
}

/**
 * Special key up callback
 */
void specialKeyboardUp(int key, int x, int y) {
    if (game) {
        game->handleSpecialKey(key, false);
    }
}

/**
 * Mouse button callback
 */
void mouse(int button, int state, int x, int y) {
    if (game) {
        game->handleMouse(button, state, x, y);
    }
}

/**
 * Mouse motion callback (when button is pressed)
 */
void mouseMotion(int x, int y) {
    if (game) {
        game->handleMouseMotion(x, y);
    }
}

/**
 * Passive mouse motion callback (no button pressed)
 */
void passiveMouseMotion(int x, int y) {
    if (game) {
        game->handleMouseMotion(x, y);
    }
}

/**
 * Window reshape callback
 */
void reshape(int width, int height) {
    if (game) {
        game->handleReshape(width, height);
    }
}

/**
 * Cleanup function called on exit
 */
void cleanup() {
    if (game) {
        game->cleanup();
        delete game;
        game = nullptr;
    }
}

/**
 * Main entry point
 */
int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 50);
    glutCreateWindow(WINDOW_TITLE);
    
    #ifndef __APPLE__
    // Initialize GLEW (must be done after creating window)
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "GLEW initialization failed: " << glewGetErrorString(glewStatus) << std::endl;
        return 1;
    }
    std::cout << "GLEW initialized successfully" << std::endl;
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    #endif
    
    // Create game instance
    game = new Game();
    
    // Initialize game
    game->init();
    
    // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeyboard);
    glutSpecialUpFunc(specialKeyboardUp);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(passiveMouseMotion);
    
    // Set up timer for game loop
    previousTime = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, update, 0);
    
    // Register cleanup
    atexit(cleanup);
    
    // Start main loop
    glutMainLoop();
    
    return 0;
}
