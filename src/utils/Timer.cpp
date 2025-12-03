#include "Timer.h"

Timer::Timer() : timeRemaining(0.0f), running(false) {
}

void Timer::start(float seconds) {
    timeRemaining = seconds;
    running = true;
}

void Timer::update(float deltaTime) {
    if (running && timeRemaining > 0) {
        timeRemaining -= deltaTime;
        if (timeRemaining < 0) {
            timeRemaining = 0;
        }
    }
}

void Timer::addTime(float seconds) {
    timeRemaining += seconds;
}

void Timer::stop() {
    running = false;
}

void Timer::reset() {
    timeRemaining = 0.0f;
    running = false;
}

bool Timer::isExpired() const {
    return timeRemaining <= 0 && running;
}

bool Timer::isRunning() const {
    return running;
}

float Timer::getTime() const {
    return timeRemaining > 0 ? timeRemaining : 0.0f;
}
