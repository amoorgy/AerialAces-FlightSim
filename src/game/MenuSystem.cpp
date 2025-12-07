#include "MenuSystem.h"
#include <iostream>
#include <cmath>

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

MenuSystem::MenuSystem()
    : selectedOption(MenuOption::SINGLE_PLAYER),
      enterPressed(false),
      animationTimer(0.0f),
      upKeyPressed(false),
      downKeyPressed(false),
      plane1X(-100.0f),
      plane1Y(600.0f),
      plane2X(1400.0f),
      plane2Y(200.0f),
      level2Unlocked(true) {  // Level 2 unlocked by default
    fadeAlpha[0] = 1.0f;
    fadeAlpha[1] = 0.3f;
    fadeAlpha[2] = 0.3f;
    fadeAlpha[3] = 0.3f;
}

MenuSystem::~MenuSystem() {
}

void MenuSystem::update(float deltaTime, const bool* keys, bool upPressed, bool downPressed) {
    // Update animation timer
    animationTimer += deltaTime;
    
    // Update background plane positions
    plane1X += 60.0f * deltaTime;  // Move right
    if (plane1X > 1400.0f) plane1X = -100.0f;  // Reset
    plane1Y = 600.0f + 30.0f * sin(animationTimer * 0.8f);
    
    plane2X -= 80.0f * deltaTime;  // Move left
    if (plane2X < -100.0f) plane2X = 1400.0f;  // Reset
    plane2Y = 200.0f + 40.0f * sin(animationTimer * 0.6f);
    
    // Update fade animations for buttons (now 4 options)
    float fadeSpeed = 3.0f;
    for (int i = 0; i < 4; i++) {
        float targetAlpha = ((int)selectedOption == i) ? 1.0f : 0.3f;
        // Level 2 is dimmer if locked
        if (i == 1 && !level2Unlocked) {
            targetAlpha = 0.15f;
        }
        if (fadeAlpha[i] < targetAlpha) {
            fadeAlpha[i] += fadeSpeed * deltaTime;
            if (fadeAlpha[i] > targetAlpha) fadeAlpha[i] = targetAlpha;
        } else if (fadeAlpha[i] > targetAlpha) {
            fadeAlpha[i] -= fadeSpeed * deltaTime;
            if (fadeAlpha[i] < targetAlpha) fadeAlpha[i] = targetAlpha;
        }
    }
    
    // Handle up/down arrow keys for menu navigation with proper debouncing
    bool upCurrentlyPressed = upPressed;
    bool downCurrentlyPressed = downPressed;
    
    // UP key - only trigger on press, not hold
    if (upCurrentlyPressed && !upKeyPressed) {
        int opt = (int)selectedOption;
        opt--;
        if (opt < 0) opt = 3;  // Wrap to EXIT (now index 3)
        // Skip Level 2 if locked
        if (opt == 1 && !level2Unlocked) opt--;
        if (opt < 0) opt = 3;
        selectedOption = (MenuOption)opt;
    }
    upKeyPressed = upCurrentlyPressed;
    
    // DOWN key - only trigger on press, not hold
    if (downCurrentlyPressed && !downKeyPressed) {
        int opt = (int)selectedOption;
        opt++;
        if (opt > 3) opt = 0;  // Wrap to SINGLE_PLAYER
        // Skip Level 2 if locked
        if (opt == 1 && !level2Unlocked) opt++;
        if (opt > 3) opt = 0;
        selectedOption = (MenuOption)opt;
    }
    downKeyPressed = downCurrentlyPressed;
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
    
    // Animated gradient background - Beautiful Sky Blue Theme
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float bgShift = sin(animationTimer * 0.3f) * 0.05f;
    
    // Sky gradient - vibrant sky blue
    glBegin(GL_QUADS);
    glColor3f(0.35f + bgShift, 0.65f + bgShift, 0.95f + bgShift);
    glVertex2f(0, 720);
    glVertex2f(1280, 720);
    glColor3f(0.45f + bgShift, 0.70f + bgShift, 1.0f);
    glVertex2f(1280, 400);
    glVertex2f(0, 400);
    glEnd();
    
    glBegin(GL_QUADS);
    glColor3f(0.45f + bgShift, 0.70f + bgShift, 1.0f);
    glVertex2f(0, 400);
    glVertex2f(1280, 400);
    glColor3f(0.60f + bgShift, 0.80f + bgShift, 0.98f + bgShift);
    glVertex2f(1280, 0);
    glVertex2f(0, 0);
    glEnd();
    
    // Fluffy animated clouds
    glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
    for (int i = 0; i < 5; i++) {
        float cloudX = fmod(animationTimer * 15.0f + i * 280.0f, 1500.0f);
        float cloudY = 600.0f - i * 80.0f;
        float wobble = sin(animationTimer * 0.7f + i) * 10.0f;
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cloudX + wobble, cloudY);
        for (int j = 0; j <= 12; j++) {
            float angle = j * M_PI * 2.0f / 12.0f;
            float radius = 35.0f + 15.0f * sin(j * 1.5f);
            glVertex2f(cloudX + wobble + cos(angle) * radius, cloudY + sin(angle) * radius * 0.6f);
        }
        glEnd();
    }
    
    // Sun rays effect
    float sunX = 1100.0f;
    float sunY = 600.0f;
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 0.98f, 0.5f, 0.3f);
    glVertex2f(sunX, sunY);
    glColor4f(1.0f, 0.98f, 0.7f, 0.0f);
    for (int i = 0; i <= 12; i++) {
        float angle = i * M_PI * 2.0f / 12.0f + animationTimer * 0.3f;
        glVertex2f(sunX + cos(angle) * 250.0f, sunY + sin(angle) * 250.0f);
    }
    glEnd();
    
    // Main title with glow effect - TOP GUN MAVERICK
    const char* mainTitle = "TOP GUN MAVERICK";
    const char* subtitle = "FLIGHT SIMULATOR";
    
    float titlePulse = 0.9f + 0.1f * sin(animationTimer * 2.0f);
    
    // Title glow layers
    for (int layer = 5; layer > 0; layer--) {
        float glowAlpha = 0.3f * titlePulse / layer;
        glColor4f(1.0f, 0.4f, 0.1f, glowAlpha);
        float offset = layer * 4.5f;
        for (float dx = -offset; dx <= offset; dx += offset * 0.5f) {
            for (float dy = -offset; dy <= offset; dy += offset * 0.5f) {
                glRasterPos2f(380 + dx, 590 + dy);
                for (const char* c = mainTitle; *c != '\0'; c++) {
                    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
                }
            }
        }
    }
    
    // Main title text
    glColor3f(1.0f, 0.98f * titlePulse, 0.9f * titlePulse);
    glRasterPos2f(380, 590);
    for (const char* c = mainTitle; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
    
    // Subtitle
    glColor4f(0.9f, 0.95f, 1.0f, 0.8f);
    glRasterPos2f(480, 550);
    for (const char* c = subtitle; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Menu options with modern button style (4 options now)
    const char* options[] = {
        "LEVEL 1: TERRAIN NAVIGATION",
        "LEVEL 2: AERIAL COMBAT",
        "CO-OP DOGFIGHT",
        "EXIT"
    };
    
    float buttonY[] = { 420, 360, 300, 240 };
    float buttonWidth = 350;
    float buttonHeight = 45;
    float buttonX = 640 - buttonWidth / 2;
    
    for (int i = 0; i < 4; i++) {
        bool isSelected = ((int)selectedOption == i);
        bool isLocked = (i == 1 && !level2Unlocked);
        float alpha = fadeAlpha[i];
        
        // Button background with gradient
        float pulse = isSelected ? (0.95f + 0.05f * sin(animationTimer * 4.0f)) : 1.0f;
        
        if (isLocked) {
            // Locked button - grayed out
            glColor4f(0.3f, 0.3f, 0.35f, 0.5f * alpha);
        } else if (isSelected) {
            // Selected button - bright gradient
            glColor4f(0.1f * pulse, 0.4f * pulse, 0.9f * pulse, 0.85f * alpha);
        } else {
            // Unselected button - darker
            glColor4f(0.15f, 0.2f, 0.4f, 0.6f * alpha);
        }
        
        // Draw rounded button background
        glBegin(GL_QUADS);
        glVertex2f(buttonX, buttonY[i]);
        glVertex2f(buttonX + buttonWidth, buttonY[i]);
        glVertex2f(buttonX + buttonWidth, buttonY[i] + buttonHeight);
        glVertex2f(buttonX, buttonY[i] + buttonHeight);
        glEnd();
        
        // Button border
        if (isSelected && !isLocked) {
            glColor4f(1.0f, 0.8f, 0.2f, alpha);
            glLineWidth(3.0f);
        } else {
            glColor4f(0.5f, 0.6f, 0.8f, 0.5f * alpha);
            glLineWidth(1.5f);
        }
        glBegin(GL_LINE_LOOP);
        glVertex2f(buttonX, buttonY[i]);
        glVertex2f(buttonX + buttonWidth, buttonY[i]);
        glVertex2f(buttonX + buttonWidth, buttonY[i] + buttonHeight);
        glVertex2f(buttonX, buttonY[i] + buttonHeight);
        glEnd();
        glLineWidth(1.0f);
        
        // Button text
        if (isLocked) {
            glColor4f(0.5f, 0.5f, 0.5f, alpha);
        } else if (isSelected) {
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
        } else {
            glColor4f(0.8f, 0.85f, 0.95f, alpha);
        }
        
        // Center text
        int textWidth = 0;
        for (const char* c = options[i]; *c != '\0'; c++) {
            textWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
        }
        float textX = buttonX + (buttonWidth - textWidth) / 2;
        float textY = buttonY[i] + 15;
        
        glRasterPos2f(textX, textY);
        for (const char* c = options[i]; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
        // Show locked indicator
        if (isLocked) {
            const char* lockText = "[COMPLETE LEVEL 1 TO UNLOCK]";
            int lockWidth = 0;
            for (const char* c = lockText; *c != '\0'; c++) {
                lockWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, *c);
            }
            glColor4f(1.0f, 0.5f, 0.2f, 0.7f);
            glRasterPos2f(640 - lockWidth / 2, buttonY[i] - 15);
            for (const char* c = lockText; *c != '\0'; c++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
            }
        }
    }
    
    // Selection indicator arrow
    int selIdx = (int)selectedOption;
    float arrowX = buttonX - 30;
    float arrowY = buttonY[selIdx] + buttonHeight / 2;
    float arrowPulse = sin(animationTimer * 5.0f) * 5.0f;
    
    glColor4f(1.0f, 0.8f, 0.2f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(arrowX + arrowPulse, arrowY);
    glVertex2f(arrowX - 15 + arrowPulse, arrowY + 10);
    glVertex2f(arrowX - 15 + arrowPulse, arrowY - 10);
    glEnd();
    
    // Controls hint at bottom
    glColor4f(0.7f, 0.8f, 0.95f, 0.6f);
    const char* hint = "Use UP/DOWN arrows to navigate, ENTER to select";
    glRasterPos2f(450, 80);
    for (const char* c = hint; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void MenuSystem::handleKeyPress(unsigned char key, bool pressed) {
    if (pressed && (key == 13 || key == ' ')) {  // Enter or Space
        // Don't allow selecting locked Level 2
        if (selectedOption == MenuOption::LEVEL_2 && !level2Unlocked) {
            return;
        }
        enterPressed = true;
    }
}

bool MenuSystem::isOptionConfirmed() const {
    return enterPressed;
}

void MenuSystem::resetConfirmation() {
    enterPressed = false;
}
