#include "MenuSystem.h"
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

MenuSystem::MenuSystem()
    : selectedOption(MenuOption::SINGLE_PLAYER),
      enterPressed(false) {
}

MenuSystem::~MenuSystem() {
}

void MenuSystem::update(float deltaTime, const bool* keys) {
    // Handle up/down arrow keys for menu navigation
    static bool upWasPressed = false;
    static bool downWasPressed = false;
    
    if (keys[GLUT_KEY_UP] && !upWasPressed) {
        int opt = (int)selectedOption;
        opt--;
        if (opt < 0) opt = 2;  // Wrap to EXIT
        selectedOption = (MenuOption)opt;
        upWasPressed = true;
    } else if (!keys[GLUT_KEY_UP]) {
        upWasPressed = false;
    }
    
    if (keys[GLUT_KEY_DOWN] && !downWasPressed) {
        int opt = (int)selectedOption;
        opt++;
        if (opt > 2) opt = 0;  // Wrap to SINGLE_PLAYER
        selectedOption = (MenuOption)opt;
        downWasPressed = true;
    } else if (!keys[GLUT_KEY_DOWN]) {
        downWasPressed = false;
    }
}

void MenuSystem::render() {
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
    
    // Background
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Title
    glColor3f(1.0f, 1.0f, 0.0f);
    const char* title = "TOP GUN MAVERICK - FLIGHT SIMULATOR";
    glRasterPos2f(380, 600);
    for (const char* c = title; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
    
    // Menu options
    const char* options[] = {
        "Single Player",
        "Co-op Mode (Split-Screen)",
        "Exit"
    };
    
    for (int i = 0; i < 3; i++) {
        if ((int)selectedOption == i) {
            glColor3f(1.0f, 1.0f, 0.0f);  // Yellow for selected
        } else {
            glColor3f(1.0f, 1.0f, 1.0f);  // White for unselected
        }
        
        glRasterPos2f(500, 450 - i * 50);
        for (const char* c = options[i]; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
        // Selection indicator
        if ((int)selectedOption == i) {
            glColor3f(1.0f, 1.0f, 0.0f);
            glRasterPos2f(470, 450 - i * 50);
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '>');
        }
    }
    
    // Controls information
    glColor3f(0.7f, 0.7f, 0.7f);
    const char* controls[] = {
        "CONTROLS:",
        "W/S: Pitch Up/Down",
        "A/D: Roll Left/Right",
        "Q/E: Yaw Left/Right",
        "1/2: Speed Control",
        "Space: Barrel Roll",
        "C: Toggle Camera",
        "R: Restart Level",
        ""
    };
    
    int yPos = 250;
    for (int i = 0; controls[i][0] != '\0'; i++) {
        glRasterPos2f(100, yPos);
        for (const char* c = controls[i]; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        }
        yPos -= 20;
    }
    
    // Co-op controls
    glColor3f(0.7f, 0.7f, 0.7f);
    const char* coopControls[] = {
        "CO-OP CONTROLS:",
        "Player 1: WASD + QE",
        "Player 2: IJKL + UO",
        "Left-Click: Fire Missile",
        ""
    };
    
    yPos = 250;
    for (int i = 0; coopControls[i][0] != '\0'; i++) {
        glRasterPos2f(900, yPos);
        for (const char* c = coopControls[i]; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        }
        yPos -= 20;
    }
    
    // Navigation hint
    glColor3f(1.0f, 1.0f, 1.0f);
    const char* hint = "Use UP/DOWN arrows to navigate, ENTER to select";
    glRasterPos2f(420, 80);
    for (const char* c = hint; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void MenuSystem::handleKeyPress(unsigned char key, bool pressed) {
    if (key == '\r' && pressed) {  // Enter key
        enterPressed = true;
        std::cout << "Menu option selected: " << (int)selectedOption << std::endl;
    }
}

bool MenuSystem::isOptionConfirmed() const {
    return enterPressed;
}

void MenuSystem::resetConfirmation() {
    enterPressed = false;
}
