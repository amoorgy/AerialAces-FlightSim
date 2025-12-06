#include "Obstacle.h"
#include <cmath>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
// On Windows, GLEW must be included before GL headers
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

Obstacle::Obstacle()
    : x(0), y(0), z(0),
      width(100), height(100), depth(100),
      type(ObstacleType::GROUND),
      colorR(0.4f), colorG(0.5f), colorB(0.3f),
      baseRadius(50.0f),
      active(true),
      obstacleModel(nullptr),
      useModel(false) {
}

Obstacle::Obstacle(float posX, float posY, float posZ, 
                   float w, float h, float d, 
                   ObstacleType obstacleType)
    : x(posX), y(posY), z(posZ),
      width(w), height(h), depth(d),
      type(obstacleType),
      colorR(0.4f), colorG(0.5f), colorB(0.3f),
      baseRadius(w / 2.0f),
      active(true),
      obstacleModel(nullptr),
      useModel(false) {
    
    // Set default colors based on type
    switch (type) {
        case ObstacleType::MOUNTAIN:
            colorR = 0.5f; colorG = 0.45f; colorB = 0.4f;  // Brown/rocky
            break;
        case ObstacleType::GROUND:
            colorR = 0.35f; colorG = 0.5f; colorB = 0.3f;  // Green/terrain
            break;
        case ObstacleType::BUILDING:
            colorR = 0.6f; colorG = 0.6f; colorB = 0.6f;   // Gray
            break;
        case ObstacleType::ROCK:
            colorR = 0.45f; colorG = 0.42f; colorB = 0.4f; // Dark gray
            break;
    }
}

Obstacle::~Obstacle() {
    if (obstacleModel != nullptr) {
        delete obstacleModel;
        obstacleModel = nullptr;
    }
}

bool Obstacle::loadModel(const std::string& modelPath, float scale) {
    std::cout << "Obstacle: Loading model from " << modelPath << std::endl;
    
    if (obstacleModel != nullptr) {
        delete obstacleModel;
    }
    
    obstacleModel = new Model();
    if (obstacleModel->load(modelPath)) {
        obstacleModel->setScale(scale);
        useModel = true;
        
        // Get and print bounds for debugging
        float minX, maxX, minY, maxY, minZ, maxZ;
        obstacleModel->getBounds(minX, maxX, minY, maxY, minZ, maxZ);
        std::cout << "Obstacle: Model bounds after scaling:" << std::endl;
        std::cout << "  X: " << minX << " to " << maxX << " (size: " << (maxX - minX) << ")" << std::endl;
        std::cout << "  Y: " << minY << " to " << maxY << " (size: " << (maxY - minY) << ")" << std::endl;
        std::cout << "  Z: " << minZ << " to " << maxZ << " (size: " << (maxZ - minZ) << ")" << std::endl;
        
        // Update obstacle dimensions based on model
        width = maxX - minX;
        height = maxY - minY;
        depth = maxZ - minZ;
        
        std::cout << "Obstacle: Model loaded successfully!" << std::endl;
        return true;
    } else {
        std::cerr << "Obstacle: Failed to load model, will use primitives" << std::endl;
        delete obstacleModel;
        obstacleModel = nullptr;
        useModel = false;
        return false;
    }
}

void Obstacle::render() const {
    // Don't render if inactive (destroyed)
    if (!active) return;
    
    glPushMatrix();
    
    // Use 3D model if loaded, otherwise use primitives
    if (useModel && obstacleModel != nullptr && obstacleModel->isLoaded()) {
        // Position the model at obstacle location
        glTranslatef(x, y, z);
        
        // For ground/landscape type, orient as a horizontal ground plane
        if (type == ObstacleType::GROUND) {
            // The terrain model needs to be oriented as a flat ground surface
            // facing upward (Y-up). Most landscape models are already Y-up,
            // so we only apply rotation if needed based on model orientation.
            // Scale uniformly on XZ plane for proper ground coverage
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);  // Rotate to lay flat if model is vertical
        }
        
        // Set terrain color (brownish-green for landscape)
        glColor3f(0.45f, 0.55f, 0.35f);
        
        GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
        if (!lightingEnabled) glEnable(GL_LIGHTING);
        
        // Set material properties
        GLfloat matAmbient[] = { 0.2f, 0.25f, 0.15f, 1.0f };
        GLfloat matDiffuse[] = { 0.45f, 0.55f, 0.35f, 1.0f };
        GLfloat matSpecular[] = { 0.05f, 0.05f, 0.05f, 1.0f };
        GLfloat matShininess[] = { 5.0f };
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);
        
        obstacleModel->render();
        
        if (!lightingEnabled) glDisable(GL_LIGHTING);
    } else {
        // Fallback: Use primitives based on type
        glColor3f(colorR, colorG, colorB);
        
        switch (type) {
            case ObstacleType::MOUNTAIN:
                glTranslatef(x, y, z);
                glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                glutSolidCone(baseRadius, height, 16, 12);
                if (height > 40.0f) {
                    glColor3f(0.95f, 0.95f, 0.98f);
                    glTranslatef(0.0f, 0.0f, height * 0.7f);
                    glutSolidCone(baseRadius * 0.3f, height * 0.3f, 12, 6);
                }
                break;
            
            case ObstacleType::GROUND:
                glTranslatef(x, y, z);
                // Render as large flat ground plane
                glColor3f(0.3f, 0.5f, 0.25f);  // Green ground color
                glBegin(GL_QUADS);
                glNormal3f(0.0f, 1.0f, 0.0f);
                glVertex3f(-width / 2.0f, 0.0f, -depth / 2.0f);
                glVertex3f(width / 2.0f, 0.0f, -depth / 2.0f);
                glVertex3f(width / 2.0f, 0.0f, depth / 2.0f);
                glVertex3f(-width / 2.0f, 0.0f, depth / 2.0f);
                glEnd();
                
                // Grid lines
                {
                    glDisable(GL_LIGHTING);
                    glColor3f(0.25f, 0.4f, 0.2f);
                    glBegin(GL_LINES);
                    float gridSpacing = 50.0f;
                    for (float i = -width / 2.0f; i <= width / 2.0f; i += gridSpacing) {
                        glVertex3f(i, 0.5f, -depth / 2.0f);
                        glVertex3f(i, 0.5f, depth / 2.0f);
                    }
                    for (float i = -depth / 2.0f; i <= depth / 2.0f; i += gridSpacing) {
                        glVertex3f(-width / 2.0f, 0.5f, i);
                        glVertex3f(width / 2.0f, 0.5f, i);
                    }
                    glEnd();
                    glEnable(GL_LIGHTING);
                }
                break;
            
            case ObstacleType::BUILDING: {
                // Render HUGE VISIBLE LIGHTHOUSE with bright colors and lights
                glTranslatef(x, y, z);
                
                // Enable lighting for the lighthouse structure
                GLboolean wasLit = glIsEnabled(GL_LIGHTING);
                glEnable(GL_LIGHTING);
                
                // Main tower - WHITE with RED stripes (classic lighthouse)
                GLfloat matWhite[] = {1.0f, 1.0f, 1.0f, 1.0f};
                GLfloat matRed[] = {1.0f, 0.1f, 0.1f, 1.0f};
                GLfloat matSpecular[] = {0.8f, 0.8f, 0.8f, 1.0f};
                GLfloat matShine[] = {32.0f};
                
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShine);
                
                // Draw 3 alternating white/red sections
                for (int section = 0; section < 3; section++) {
                    glPushMatrix();
                    glTranslatef(0, section * height / 3.0f, 0);
                    
                    if (section % 2 == 0) {
                        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matWhite);
                    } else {
                        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matRed);
                    }
                    
                    glutSolidCylinder(width / 2.0f, height / 3.0f, 20, 8);
                    glPopMatrix();
                }
                
                // Top dome/light housing - BRIGHT YELLOW (glowing)
                GLfloat matYellow[] = {1.0f, 1.0f, 0.3f, 1.0f};
                GLfloat matEmissive[] = {0.5f, 0.5f, 0.2f, 1.0f};  // Makes it glow!
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matYellow);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, matEmissive);
                
                glPushMatrix();
                glTranslatef(0, height, 0);
                glutSolidSphere(width * 0.7f, 16, 16);  // Big glowing sphere
                glPopMatrix();
                
                // Reset emission so it doesn't affect other objects
                GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);
                
                if (!wasLit) glDisable(GL_LIGHTING);
                break;
            }
            
            case ObstacleType::ROCK:
                glTranslatef(x, y + height / 2.0f, z);
                glScalef(width / 2.0f, height / 2.0f, depth / 2.0f);
                glutSolidSphere(1.0, 10, 8);
                break;
        }
    }
    
    glPopMatrix();
}

void Obstacle::setColor(float r, float g, float b) {
    colorR = r;
    colorG = g;
    colorB = b;
}

float Obstacle::getRadiusAtHeight(float h) const {
    if (type != ObstacleType::MOUNTAIN || h < y || h > y + height) {
        return 0.0f;
    }
    
    // Linear interpolation - cone narrows from base to tip
    float heightRatio = (h - y) / height;
    return baseRadius * (1.0f - heightRatio);
}

bool Obstacle::checkModelCollision(float px, float py, float pz, float radius) const {
    if (!useModel || obstacleModel == nullptr || !obstacleModel->isLoaded()) {
        return false;
    }
    
    // Convert player world position to local model coordinates
    // The obstacle is positioned at (x, y, z) in world space
    float localX = px - x;
    float localY = py - y;
    float localZ = pz - z;
    
    // The terrain is rendered with glRotatef(-90, 1, 0, 0) which rotates around X axis
    // Rotation matrix for -90� around X:
    //   [1,  0,   0 ]   [mx]   [ mx ]
    //   [0,  0,   1 ] * [my] = [ mz ]   (model Y -> world Z)
    //   [0, -1,   0 ]   [mz]   [-my ]   (model Z -> world -Y)
    //
    // So model coords (mx, my, mz) appear at world (mx, mz, -my)
    //
    // INVERSE (world to model):
    //   [1,  0,  0]   [wx]   [ wx ]
    //   [0,  0, -1] * [wy] = [-wz ]   (world Z -> model -Y)
    //   [0,  1,  0]   [wz]   [ wy ]   (world Y -> model Z)
    
    if (type == ObstacleType::GROUND) {
        float modelX = localX;
        float modelY = -localZ;     // World Z becomes negative model Y
        float modelZ = localY;      // World Y becomes model Z
        localX = modelX;
        localY = modelY;
        localZ = modelZ;
    }
    
    // Debug output - more frequent for testing
    static int debugCounter = 0;
    debugCounter++;
    if (debugCounter % 60 == 0) {  // Every second at 60fps
        float minX, maxX, minY, maxY, minZ, maxZ;
        obstacleModel->getBounds(minX, maxX, minY, maxY, minZ, maxZ);
        std::cout << "=== Collision Debug ===" << std::endl;
        std::cout << "Player world: (" << px << ", " << py << ", " << pz << ")" << std::endl;
        std::cout << "Obstacle pos: (" << x << ", " << y << ", " << z << ")" << std::endl;
        std::cout << "Local (transformed): (" << localX << ", " << localY << ", " << localZ << ")" << std::endl;
        std::cout << "Model bounds: X[" << minX << "," << maxX << "] Y[" << minY << "," << maxY << "] Z[" << minZ << "," << maxZ << "]" << std::endl;
        std::cout << "Radius: " << radius << std::endl;
        
        // Check if player is even within model bounds
        bool inBounds = (localX >= minX && localX <= maxX &&
                         localY >= minY && localY <= maxY &&
                         localZ >= minZ && localZ <= maxZ);
        std::cout << "Within model bounds: " << (inBounds ? "YES" : "NO") << std::endl;
        std::cout << "======================" << std::endl;
    }
    
    return obstacleModel->checkCollision(localX, localY, localZ, radius);
}
