#include "Input.h"
#include <cstring>
#include <cctype>

Input::Input() : mouseX(0), mouseY(0) {
    reset();
}

void Input::setKey(unsigned char key, bool pressed) {
    keys[key] = pressed;
    // Also set opposite case for convenience
    if (std::isalpha(key)) {
        keys[std::toupper(key)] = pressed;
        keys[std::tolower(key)] = pressed;
    }
}

void Input::setSpecialKey(int key, bool pressed) {
    if (key >= 0 && key < 256) {
        specialKeys[key] = pressed;
    }
}

void Input::setMousePosition(int x, int y) {
    mouseX = x;
    mouseY = y;
}

void Input::setMouseButton(int button, bool pressed) {
    if (button >= 0 && button < 3) {
        mouseButtons[button] = pressed;
    }
}

bool Input::isKeyPressed(unsigned char key) const {
    return keys[key];
}

bool Input::isSpecialKeyPressed(int key) const {
    if (key >= 0 && key < 256) {
        return specialKeys[key];
    }
    return false;
}

bool Input::isMouseButtonPressed(int button) const {
    if (button >= 0 && button < 3) {
        return mouseButtons[button];
    }
    return false;
}

int Input::getMouseX() const {
    return mouseX;
}

int Input::getMouseY() const {
    return mouseY;
}

void Input::reset() {
    std::memset(keys, 0, sizeof(keys));
    std::memset(specialKeys, 0, sizeof(specialKeys));
    std::memset(mouseButtons, 0, sizeof(mouseButtons));
}

const bool* Input::getKeys() const {
    return keys;
}
