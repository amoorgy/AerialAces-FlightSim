#include <iostream>
#include <cmath>
#include <vector>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

// Window dimensions
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// Camera and aircraft state
struct Camera {
    float x, y, z;
    float pitch, yaw, roll;
    bool thirdPerson;
};

struct Aircraft {
    float x, y, z;
    float pitch, yaw, roll;
    float speed;
    float velocity[3];
};

Camera camera = {0.0f, 5.0f, -20.0f, 0.0f, 0.0f, 0.0f, true};
Aircraft player = {0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, {0.0f, 0.0f, 0.0f}};

// Input state
bool keys[256] = {false};

// Forward declarations
void init();
void display();
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void update(int value);
void drawAircraft();
void drawTerrain();
void drawSky();
void updatePhysics();
void updateCamera();

int main(int argc, char** argv) {
    std::cout << "=== Aerial Aces Flight Simulator ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  W/S: Pitch Up/Down" << std::endl;
    std::cout << "  A/D: Roll Left/Right" << std::endl;
    std::cout << "  Q/E: Yaw Left/Right" << std::endl;
    std::cout << "  C: Toggle Camera" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    std::cout << std::endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Aerial Aces - Flight Simulator");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat light_position[] = {1.0f, 1.0f, 1.0f, 0.0f};
    GLfloat light_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    updateCamera();

    drawSky();
    drawTerrain();

    glPushMatrix();
    glTranslatef(player.x, player.y, player.z);
    glRotatef(player.yaw, 0.0f, 1.0f, 0.0f);
    glRotatef(player.pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(player.roll, 0.0f, 0.0f, 1.0f);
    if (camera.thirdPerson) {
        drawAircraft();
    }
    glPopMatrix();

    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    keys[key] = true;

    if (key == 27) { // ESC
        exit(0);
    }
    if (key == 'c' || key == 'C') {
        camera.thirdPerson = !camera.thirdPerson;
        std::cout << "Camera: " << (camera.thirdPerson ? "Third Person" : "First Person") << std::endl;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

void update(int value) {
    updatePhysics();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void updatePhysics() {
    const float pitchSpeed = 2.0f;
    const float yawSpeed = 1.5f;
    const float rollSpeed = 3.0f;

    if (keys['w'] || keys['W']) player.pitch += pitchSpeed;
    if (keys['s'] || keys['S']) player.pitch -= pitchSpeed;
    if (keys['a'] || keys['A']) player.roll -= rollSpeed;
    if (keys['d'] || keys['D']) player.roll += rollSpeed;
    if (keys['q'] || keys['Q']) player.yaw -= yawSpeed;
    if (keys['e'] || keys['E']) player.yaw += yawSpeed;

    // Apply damping
    player.pitch *= 0.95f;
    player.yaw *= 0.95f;
    player.roll *= 0.95f;

    // Calculate forward direction
    float radYaw = player.yaw * M_PI / 180.0f;
    float radPitch = player.pitch * M_PI / 180.0f;

    player.velocity[0] = player.speed * sin(radYaw);
    player.velocity[1] = player.speed * sin(radPitch);
    player.velocity[2] = player.speed * cos(radYaw) * cos(radPitch);

    player.x += player.velocity[0];
    player.y += player.velocity[1];
    player.z += player.velocity[2];

    // Keep above ground
    if (player.y < 2.0f) player.y = 2.0f;
}

void updateCamera() {
    if (camera.thirdPerson) {
        float radYaw = player.yaw * M_PI / 180.0f;
        float distance = 15.0f;

        camera.x = player.x - distance * sin(radYaw);
        camera.y = player.y + 5.0f;
        camera.z = player.z - distance * cos(radYaw);

        gluLookAt(camera.x, camera.y, camera.z,
                  player.x, player.y, player.z,
                  0.0f, 1.0f, 0.0f);
    } else {
        float radYaw = player.yaw * M_PI / 180.0f;
        float radPitch = player.pitch * M_PI / 180.0f;

        float lookX = player.x + sin(radYaw);
        float lookY = player.y + sin(radPitch);
        float lookZ = player.z + cos(radYaw);

        gluLookAt(player.x, player.y, player.z,
                  lookX, lookY, lookZ,
                  0.0f, 1.0f, 0.0f);
    }
}

void drawAircraft() {
    glColor3f(0.2f, 0.2f, 0.8f);

    // Fuselage
    glPushMatrix();
    glScalef(1.0f, 0.5f, 3.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Wings
    glPushMatrix();
    glScalef(5.0f, 0.2f, 1.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Tail
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, -1.5f);
    glScalef(1.5f, 1.0f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawTerrain() {
    glDisable(GL_LIGHTING);

    // Ground plane
    glColor3f(0.2f, 0.6f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-500.0f, 0.0f, -500.0f);
    glVertex3f(500.0f, 0.0f, -500.0f);
    glVertex3f(500.0f, 0.0f, 500.0f);
    glVertex3f(-500.0f, 0.0f, 500.0f);
    glEnd();

    // Simple mountains
    glColor3f(0.5f, 0.4f, 0.3f);
    for (int i = -3; i <= 3; i++) {
        for (int j = -3; j <= 3; j++) {
            glPushMatrix();
            glTranslatef(i * 80.0f, 0.0f, j * 80.0f);
            glScalef(1.0f, 2.0f + (i + j) % 3, 1.0f);
            glutSolidCone(20.0, 30.0, 8, 8);
            glPopMatrix();
        }
    }

    // Navigation rings
    glColor3f(1.0f, 0.8f, 0.0f);
    for (int i = 1; i <= 5; i++) {
        glPushMatrix();
        glTranslatef(0.0f, 15.0f, i * 50.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glutWireTorus(1.0, 10.0, 10, 20);
        glPopMatrix();
    }

    glEnable(GL_LIGHTING);
}

void drawSky() {
    glDisable(GL_LIGHTING);
    glColor3f(0.5f, 0.7f, 1.0f);

    glPushMatrix();
    glTranslatef(player.x, player.y, player.z);
    glutSolidSphere(400.0, 20, 20);
    glPopMatrix();

    glEnable(GL_LIGHTING);
}
