#include "Collision.h"
#include <cmath>
#include <algorithm>

bool checkSphereCollision(float x1, float y1, float z1, float r1,
                          float x2, float y2, float z2, float r2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    float distSq = dx * dx + dy * dy + dz * dz;
    float radiusSum = r1 + r2;
    return distSq < (radiusSum * radiusSum);
}

bool checkAABBCollision(float min1X, float max1X, float min1Y, float max1Y, float min1Z, float max1Z,
                        float min2X, float max2X, float min2Y, float max2Y, float min2Z, float max2Z) {
    return (min1X <= max2X && max1X >= min2X) &&
           (min1Y <= max2Y && max1Y >= min2Y) &&
           (min1Z <= max2Z && max1Z >= min2Z);
}

bool checkSphereAABBCollision(float sphereX, float sphereY, float sphereZ, float sphereR,
                              float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
    // Find the closest point on the AABB to the sphere center
    float closestX = std::max(minX, std::min(sphereX, maxX));
    float closestY = std::max(minY, std::min(sphereY, maxY));
    float closestZ = std::max(minZ, std::min(sphereZ, maxZ));
    
    // Calculate distance from sphere center to closest point
    float dx = sphereX - closestX;
    float dy = sphereY - closestY;
    float dz = sphereZ - closestZ;
    
    float distSq = dx * dx + dy * dy + dz * dz;
    
    return distSq < (sphereR * sphereR);
}

bool pointInSphere(float px, float py, float pz, float sx, float sy, float sz, float r) {
    float dx = px - sx;
    float dy = py - sy;
    float dz = pz - sz;
    float distSq = dx * dx + dy * dy + dz * dz;
    return distSq < (r * r);
}

bool pointInAABB(float px, float py, float pz,
                 float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
    return (px >= minX && px <= maxX) &&
           (py >= minY && py <= maxY) &&
           (pz >= minZ && pz <= maxZ);
}

float distanceSquared(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return dx * dx + dy * dy + dz * dz;
}

float distance(float x1, float y1, float z1, float x2, float y2, float z2) {
    return std::sqrt(distanceSquared(x1, y1, z1, x2, y2, z2));
}
