#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <memory>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

/**
 * @struct AABB
 * @brief Axis-Aligned Bounding Box for spatial queries
 */
struct AABB {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
    
    AABB() : minX(0), minY(0), minZ(0), maxX(0), maxY(0), maxZ(0) {}
    
    AABB(float x0, float y0, float z0, float x1, float y1, float z1)
        : minX(x0), minY(y0), minZ(z0), maxX(x1), maxY(y1), maxZ(z1) {}
    
    // Get center of the box
    void center(float& cx, float& cy, float& cz) const {
        cx = (minX + maxX) * 0.5f;
        cy = (minY + maxY) * 0.5f;
        cz = (minZ + maxZ) * 0.5f;
    }
    
    // Get longest axis (0=X, 1=Y, 2=Z)
    int longestAxis() const {
        float dx = maxX - minX;
        float dy = maxY - minY;
        float dz = maxZ - minZ;
        if (dx >= dy && dx >= dz) return 0;
        if (dy >= dx && dy >= dz) return 1;
        return 2;
    }
    
    // Check if sphere intersects this AABB
    bool intersectsSphere(float sx, float sy, float sz, float radius) const {
        // Find closest point on AABB to sphere center
        float closestX = (sx < minX) ? minX : (sx > maxX) ? maxX : sx;
        float closestY = (sy < minY) ? minY : (sy > maxY) ? maxY : sy;
        float closestZ = (sz < minZ) ? minZ : (sz > maxZ) ? maxZ : sz;
        
        // Check if that point is within radius (use <= for inclusive check)
        float dx = sx - closestX;
        float dy = sy - closestY;
        float dz = sz - closestZ;
        return (dx*dx + dy*dy + dz*dz) <= (radius * radius);
    }
    
    // Expand to include another AABB
    void expand(const AABB& other) {
        if (other.minX < minX) minX = other.minX;
        if (other.minY < minY) minY = other.minY;
        if (other.minZ < minZ) minZ = other.minZ;
        if (other.maxX > maxX) maxX = other.maxX;
        if (other.maxY > maxY) maxY = other.maxY;
        if (other.maxZ > maxZ) maxZ = other.maxZ;
    }
};

/**
 * @struct Triangle
 * @brief Triangle for collision detection
 */
struct Triangle {
    float v0[3], v1[3], v2[3];
    float centroid[3];
    
    Triangle() {
        for (int i = 0; i < 3; i++) {
            v0[i] = v1[i] = v2[i] = centroid[i] = 0;
        }
    }
    
    Triangle(float* a, float* b, float* c) {
        for (int i = 0; i < 3; i++) {
            v0[i] = a[i];
            v1[i] = b[i];
            v2[i] = c[i];
            centroid[i] = (a[i] + b[i] + c[i]) / 3.0f;
        }
    }
    
    AABB getBounds() const {
        return AABB(
            std::min({v0[0], v1[0], v2[0]}),
            std::min({v0[1], v1[1], v2[1]}),
            std::min({v0[2], v1[2], v2[2]}),
            std::max({v0[0], v1[0], v2[0]}),
            std::max({v0[1], v1[1], v2[1]}),
            std::max({v0[2], v1[2], v2[2]})
        );
    }
};

/**
 * @struct BVHNode
 * @brief Node in the Bounding Volume Hierarchy tree
 */
struct BVHNode {
    AABB bounds;
    BVHNode* left;
    BVHNode* right;
    std::vector<Triangle> triangles;  // Only leaf nodes store triangles
    
    BVHNode() : left(nullptr), right(nullptr) {}
    ~BVHNode() {
        delete left;
        delete right;
    }
    
    bool isLeaf() const { return left == nullptr && right == nullptr; }
};

/**
 * @class Model
 * @brief Loads and renders 3D models from OBJ files with BVH collision
 */
class Model {
private:
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<unsigned int> indices;
    
    bool loaded;
    float scaleFactor;
    float minBounds[3];
    float maxBounds[3];
    
    // BVH for collision detection
    BVHNode* bvhRoot;
    std::vector<Triangle> allTriangles;
    
    void calculateBounds();
    BVHNode* buildBVH(std::vector<Triangle>& tris, int depth = 0);
    AABB computeBounds(const std::vector<Triangle>& tris) const;
    bool checkBVHCollision(BVHNode* node, float sx, float sy, float sz, float radius) const;
    bool sphereTriangleIntersect(float sx, float sy, float sz, float radius, const Triangle& tri) const;
    void closestPointOnTriangle(float px, float py, float pz,
                                const Triangle& tri,
                                float& outX, float& outY, float& outZ) const;
    
public:
    Model();
    ~Model();
    
    bool load(const std::string& filepath);
    void render() const;
    
    bool isLoaded() const { return loaded; }
    
    void getBounds(float& minX, float& maxX, float& minY, float& maxY, float& minZ, float& maxZ) const;
    void setScale(float scale);
    float getScale() const { return scaleFactor; }
    
    /**
     * Check if a sphere collides with the model using BVH
     * @param localX, localY, localZ Position relative to model origin
     * @param radius Collision sphere radius
     * @return true if collision detected
     */
    bool checkCollision(float localX, float localY, float localZ, float radius) const;
    
    /**
     * Get approximate terrain height at a given X,Z position
     */
    bool getHeightAtPosition(float worldX, float worldZ, float modelX, float modelZ, float& outHeight) const;
    
    // Access to raw vertex data
    const std::vector<float>& getVertices() const { return vertices; }
    size_t getVertexCount() const { return vertices.size() / 3; }
};

#endif // MODEL_H
