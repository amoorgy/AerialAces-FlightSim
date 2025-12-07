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
      enterPressed(false),
      animationTimer(0.0f),
      upKeyPressed(false),
      downKeyPressed(false),
      plane1X(-100.0f),
      plane1Y(600.0f),
      plane2X(1400.0f),
      plane2Y(200.0f) {
    fadeAlpha[0] = 1.0f;
    fadeAlpha[1] = 0.3f;
    fadeAlpha[2] = 0.3f;
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
    
    // Update fade animations for buttons
    float fadeSpeed = 3.0f;  // Speed of fade in/out
    for (int i = 0; i < 3; i++) {
        float targetAlpha = ((int)selectedOption == i) ? 1.0f : 0.3f;
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
        if (opt < 0) opt = 2;  // Wrap to EXIT
        selectedOption = (MenuOption)opt;
    }
    upKeyPressed = upCurrentlyPressed;
    
    // DOWN key - only trigger on press, not hold
    if (downCurrentlyPressed && !downKeyPressed) {
        int opt = (int)selectedOption;
        opt++;
        if (opt > 2) opt = 0;  // Wrap to SINGLE_PLAYER
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
    
    // Animated gradient background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float bgShift = sin(animationTimer * 0.5f) * 0.08f;
    glBegin(GL_QUADS);
    // Lighter sky gradient - sunset/dusk theme
    glColor3f(0.15f + bgShift, 0.25f + bgShift, 0.45f + bgShift * 0.5f);  // Lighter blue top
    glVertex2f(0, 720);
    glVertex2f(1280, 720);
    glColor3f(0.25f + bgShift, 0.20f + bgShift, 0.40f + bgShift * 0.5f);  // Purple-pink middle
    glVertex2f(1280, 360);
    glVertex2f(0, 360);
    glEnd();
    
    glBegin(GL_QUADS);
    glColor3f(0.25f + bgShift, 0.20f + bgShift, 0.40f + bgShift * 0.5f);  // Purple-pink middle
    glVertex2f(0, 360);
    glVertex2f(1280, 360);
    glColor3f(0.35f + bgShift, 0.15f + bgShift, 0.25f + bgShift * 0.5f);  // Warmer bottom
    glVertex2f(1280, 0);
    glVertex2f(0, 0);
    glEnd();
    
    // Animated stars/particles in background
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 80; i++) {
        float x = (i * 127 % 1280);
        float y = (i * 73 % 720);
        float twinkle = 0.4f + 0.6f * sin(animationTimer * 2.0f + i * 0.5f);
        glColor4f(1.0f, 1.0f, 0.9f, twinkle * 0.7f);
        glVertex2f(x, y);
    }
    glEnd();
    glPointSize(1.0f);
    
    // Animated background planes - simple triangular silhouettes
    // Plane 1 - moving right
    float plane1Alpha = 0.3f + 0.1f * sin(animationTimer * 1.5f);
    glColor4f(0.8f, 0.9f, 1.0f, plane1Alpha);
    glBegin(GL_TRIANGLES);
    // Fuselage
    glVertex2f(plane1X, plane1Y);
    glVertex2f(plane1X + 60, plane1Y + 8);
    glVertex2f(plane1X + 60, plane1Y - 8);
    // Wings
    glVertex2f(plane1X + 20, plane1Y);
    glVertex2f(plane1X + 40, plane1Y + 25);
    glVertex2f(plane1X + 45, plane1Y);
    glVertex2f(plane1X + 20, plane1Y);
    glVertex2f(plane1X + 40, plane1Y - 25);
    glVertex2f(plane1X + 45, plane1Y);
    // Tail
    glVertex2f(plane1X, plane1Y);
    glVertex2f(plane1X + 5, plane1Y + 15);
    glVertex2f(plane1X + 10, plane1Y);
    glEnd();
    
    // Plane 2 - moving left (mirrored)
    float plane2Alpha = 0.25f + 0.08f * sin(animationTimer * 1.2f);
    glColor4f(0.9f, 0.85f, 1.0f, plane2Alpha);
    glBegin(GL_TRIANGLES);
    // Fuselage
    glVertex2f(plane2X, plane2Y);
    glVertex2f(plane2X - 60, plane2Y + 8);
    glVertex2f(plane2X - 60, plane2Y - 8);
    // Wings
    glVertex2f(plane2X - 20, plane2Y);
    glVertex2f(plane2X - 40, plane2Y + 25);
    glVertex2f(plane2X - 45, plane2Y);
    glVertex2f(plane2X - 20, plane2Y);
    glVertex2f(plane2X - 40, plane2Y - 25);
    glVertex2f(plane2X - 45, plane2Y);
    // Tail
    glVertex2f(plane2X, plane2Y);
    glVertex2f(plane2X - 5, plane2Y + 15);
    glVertex2f(plane2X - 10, plane2Y);
    glEnd();
    
    // Vapor trails
    glLineWidth(1.5f);
    glColor4f(0.9f, 0.95f, 1.0f, plane1Alpha * 0.5f);
    glBegin(GL_LINES);
    glVertex2f(plane1X, plane1Y + 3);
    glVertex2f(plane1X - 40, plane1Y + 3);
    glVertex2f(plane1X, plane1Y - 3);
    glVertex2f(plane1X - 40, plane1Y - 3);
    glEnd();
    glColor4f(0.9f, 0.95f, 1.0f, plane2Alpha * 0.5f);
    glBegin(GL_LINES);
    glVertex2f(plane2X, plane2Y + 3);
    glVertex2f(plane2X + 40, plane2Y + 3);
    glVertex2f(plane2X, plane2Y - 3);
    glVertex2f(plane2X + 40, plane2Y - 3);
    glEnd();
    glLineWidth(1.0f);
    
    // Main title with glow effect - TOP GUN MAVERICK
    const char* mainTitle = "TOP GUN MAVERICK";
    const char* subtitle = "FLIGHT SIMULATOR";
    
    float titlePulse = 0.9f + 0.1f * sin(animationTimer * 2.0f);
    
    // Title glow layers - vibrant orange-red glow
    for (int layer = 5; layer > 0; layer--) {
        float glowAlpha = 0.3f * titlePulse / layer;
        glColor4f(1.0f, 0.4f, 0.1f, glowAlpha);  // Bright orange-red glow
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
    
    // Main title text - brilliant white-gold
    glColor3f(1.0f, 0.98f * titlePulse, 0.9f * titlePulse);  // Brilliant white-gold
    glRasterPos2f(380, 590);
    for (const char* c = mainTitle; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
    
    // Subtitle with shimmer - bright cyan
    float shimmer = 0.9f + 0.1f * sin(animationTimer * 3.0f);
    glColor3f(0.95f * shimmer, 0.98f * shimmer, 1.0f * shimmer);  // Brilliant cyan-white
    glRasterPos2f(475, 550);
    for (const char* c = subtitle; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // Menu options with beautiful styled boxes
    const char* options[] = {
        "SINGLE PLAYER",
        "CO-OP DOGFIGHT",
        "EXIT"
    };
    
    float baseY = 400.0f;
    float spacing = 80.0f;
    float boxWidth = 400.0f;
    float boxHeight = 60.0f;
    
    for (int i = 0; i < 3; i++) {
        float yPos = baseY - i * spacing;
        float xCenter = 640.0f;
        
        bool isSelected = ((int)selectedOption == i);
        float currentAlpha = fadeAlpha[i];  // Use smooth fade alpha
        
        // Animated hover effect
        float hoverScale = isSelected ? (1.0f + 0.05f * sin(animationTimer * 5.0f)) : 1.0f;
        float boxW = boxWidth * hoverScale;
        float boxH = boxHeight * hoverScale;
        
        // Selection box with gradient
        if (isSelected) {
            // Outer glow - pulsing
            float glowPulse = 0.4f + 0.3f * sin(animationTimer * 4.0f);
            glColor4f(1.0f, 0.75f, 0.2f, glowPulse);
            glBegin(GL_QUADS);
            glVertex2f(xCenter - boxW * 0.55f, yPos - boxH * 0.55f);
            glVertex2f(xCenter + boxW * 0.55f, yPos - boxH * 0.55f);
            glVertex2f(xCenter + boxW * 0.55f, yPos + boxH * 0.55f);
            glVertex2f(xCenter - boxW * 0.55f, yPos + boxH * 0.55f);
            glEnd();
        }
        
        // Main box with gradient effect - fades based on selection
        if (isSelected) {
            // Selected box - vibrant lighter gradient
            glBegin(GL_QUADS);
            glColor4f(0.35f, 0.30f, 0.55f, 0.95f * currentAlpha);  // Top - brighter purple
            glVertex2f(xCenter - boxW * 0.5f, yPos + boxH * 0.5f);
            glVertex2f(xCenter + boxW * 0.5f, yPos + boxH * 0.5f);
            glColor4f(0.25f, 0.35f, 0.60f, 0.95f * currentAlpha);  // Bottom - brighter blue
            glVertex2f(xCenter + boxW * 0.5f, yPos - boxH * 0.5f);
            glVertex2f(xCenter - boxW * 0.5f, yPos - boxH * 0.5f);
            glEnd();
        } else {
            // Unselected box - fades to subtle transparency
            glColor4f(0.18f, 0.18f, 0.25f, 0.6f * currentAlpha);
            glBegin(GL_QUADS);
            glVertex2f(xCenter - boxW * 0.5f, yPos - boxH * 0.5f);
            glVertex2f(xCenter + boxW * 0.5f, yPos - boxH * 0.5f);
            glVertex2f(xCenter + boxW * 0.5f, yPos + boxH * 0.5f);
            glVertex2f(xCenter - boxW * 0.5f, yPos + boxH * 0.5f);
            glEnd();
        }
        
        // Box border - vibrant colors with fade
        glLineWidth(isSelected ? 3.5f : 1.5f);
        if (isSelected) {
            float borderPulse = 0.85f + 0.15f * sin(animationTimer * 6.0f);
            glColor4f(1.0f * borderPulse, 0.80f * borderPulse, 0.30f * borderPulse, currentAlpha);  // Brighter gold
        } else {
            glColor4f(0.50f, 0.60f, 0.80f, 0.5f * currentAlpha);  // Lighter blue-grey with fade
        }
        glBegin(GL_LINE_LOOP);
        glVertex2f(xCenter - boxW * 0.5f, yPos - boxH * 0.5f);
        glVertex2f(xCenter + boxW * 0.5f, yPos - boxH * 0.5f);
        glVertex2f(xCenter + boxW * 0.5f, yPos + boxH * 0.5f);
        glVertex2f(xCenter - boxW * 0.5f, yPos + boxH * 0.5f);
        glEnd();
        glLineWidth(1.0f);
        
        // Option text - bright and readable with fade
        if (isSelected) {
            float textPulse = 0.95f + 0.05f * sin(animationTimer * 4.0f);
            glColor4f(1.0f * textPulse, 0.97f * textPulse, 0.5f * textPulse, currentAlpha);  // Brighter gold
        } else {
            glColor4f(0.85f, 0.88f, 0.95f, 0.5f + 0.5f * currentAlpha);  // Soft white with fade
        }
        
        // Center text in box
        int textWidth = 0;
        for (const char* c = options[i]; *c != '\0'; c++) {
            textWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
        glRasterPos2f(xCenter - textWidth * 0.5f, yPos - 6);
        for (const char* c = options[i]; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        
        // Animated arrow for selected - bright and visible with fade
        if (isSelected) {
            float arrowBounce = sin(animationTimer * 8.0f) * 5.0f;
            float arrowPulse = 0.85f + 0.15f * sin(animationTimer * 10.0f);
            glColor4f(1.0f * arrowPulse, 0.90f * arrowPulse, 0.2f * arrowPulse, currentAlpha);  // Brighter gold
            glRasterPos2f(xCenter - boxW * 0.5f - 30 + arrowBounce, yPos - 6);
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '>');
            glRasterPos2f(xCenter + boxW * 0.5f + 20 - arrowBounce, yPos - 6);
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '<');
        }
    }
    
    // Elegant navigation hint - brighter
    float hintPulse = 0.75f + 0.25f * sin(animationTimer * 3.0f);
    glColor3f(0.85f * hintPulse, 0.92f * hintPulse, 1.0f * hintPulse);
    const char* hint = "[ UP / DOWN ]  Navigate    [ ENTER ]  Select";
    int hintWidth = 0;
    for (const char* c = hint; *c != '\0'; c++) {
        hintWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, *c);
    }
    glRasterPos2f(640 - hintWidth * 0.5f, 80);
    for (const char* c = hint; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    // Credits at bottom - lighter
    glColor3f(0.65f, 0.68f, 0.75f);
    const char* credits = "Made by Amr Ramadan & Daniel Michael";
    int creditsWidth = 0;
    for (const char* c = credits; *c != '\0'; c++) {
        creditsWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, *c);
    }
    glRasterPos2f(640 - creditsWidth * 0.5f, 30);
    for (const char* c = credits; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    // Decorative lines - lighter and more visible
    glLineWidth(2.0f);
    glColor4f(0.60f, 0.65f, 0.80f, 0.6f);
    glBegin(GL_LINES);
    glVertex2f(200, 500);
    glVertex2f(1080, 500);
    glVertex2f(200, 100);
    glVertex2f(1080, 100);
    glEnd();
    glLineWidth(1.0f);
    
    glDisable(GL_BLEND);
    
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
