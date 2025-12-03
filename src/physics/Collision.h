#ifndef COLLISION_H
#define COLLISION_H

/**
 * @file Collision.h
 * @brief Collision detection utilities
 * 
 * Provides functions for detecting collisions between different
 * geometric shapes used in the game: spheres (rings, player bounding)
 * and AABBs (mountains, ground).
 */

/**
 * Check sphere-sphere collision
 * Used for: ring collection, player vs ring
 * 
 * @param x1, y1, z1 Center of first sphere
 * @param r1 Radius of first sphere
 * @param x2, y2, z2 Center of second sphere
 * @param r2 Radius of second sphere
 * @return true if spheres are overlapping
 */
bool checkSphereCollision(float x1, float y1, float z1, float r1,
                          float x2, float y2, float z2, float r2);

/**
 * Check AABB (Axis-Aligned Bounding Box) collision
 * Used for: player vs mountains, player vs ground
 * 
 * @param min1X, max1X X bounds of first box
 * @param min1Y, max1Y Y bounds of first box
 * @param min1Z, max1Z Z bounds of first box
 * @param min2X, max2X X bounds of second box
 * @param min2Y, max2Y Y bounds of second box
 * @param min2Z, max2Z Z bounds of second box
 * @return true if boxes are overlapping
 */
bool checkAABBCollision(float min1X, float max1X, float min1Y, float max1Y, float min1Z, float max1Z,
                        float min2X, float max2X, float min2Y, float max2Y, float min2Z, float max2Z);

/**
 * Check sphere vs AABB collision
 * Used for: player (sphere) vs obstacle (AABB)
 * 
 * @param sphereX, sphereY, sphereZ Sphere center
 * @param sphereR Sphere radius
 * @param minX, maxX X bounds of box
 * @param minY, maxY Y bounds of box
 * @param minZ, maxZ Z bounds of box
 * @return true if sphere and box are overlapping
 */
bool checkSphereAABBCollision(float sphereX, float sphereY, float sphereZ, float sphereR,
                              float minX, float maxX, float minY, float maxY, float minZ, float maxZ);

/**
 * Check if point is inside sphere
 * 
 * @param px, py, pz Point position
 * @param sx, sy, sz Sphere center
 * @param r Sphere radius
 * @return true if point is inside sphere
 */
bool pointInSphere(float px, float py, float pz, float sx, float sy, float sz, float r);

/**
 * Check if point is inside AABB
 * 
 * @param px, py, pz Point position
 * @param minX, maxX, minY, maxY, minZ, maxZ Box bounds
 * @return true if point is inside box
 */
bool pointInAABB(float px, float py, float pz,
                 float minX, float maxX, float minY, float maxY, float minZ, float maxZ);

/**
 * Calculate squared distance between two 3D points
 * Useful for avoiding sqrt when only comparing distances
 * 
 * @param x1, y1, z1 First point
 * @param x2, y2, z2 Second point
 * @return Squared distance
 */
float distanceSquared(float x1, float y1, float z1, float x2, float y2, float z2);

/**
 * Calculate actual distance between two 3D points
 * 
 * @param x1, y1, z1 First point
 * @param x2, y2, z2 Second point
 * @return Distance
 */
float distance(float x1, float y1, float z1, float x2, float y2, float z2);

#endif // COLLISION_H
