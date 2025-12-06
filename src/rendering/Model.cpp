#include "Model.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <limits>
#include <cmath>
#include <algorithm>

Model::Model() : loaded(false), scaleFactor(1.0f), bvhRoot(nullptr),
                 vboVertices(0), vboNormals(0), vboTexCoords(0), vboInitialized(false) {
    for (int i = 0; i < 3; i++) {
        minBounds[i] = 0.0f;
        maxBounds[i] = 0.0f;
    }
}

Model::~Model() {
    cleanupVBOs();
    vertices.clear();
    normals.clear();
    texcoords.clear();
    indices.clear();
    allTriangles.clear();
    
    delete bvhRoot;
    bvhRoot = nullptr;
}

bool Model::load(const std::string& filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    
    bool ret = tinyobj::LoadObj(attrib, shapes, materials, &err, filepath.c_str());
    
    if (!err.empty()) {
        std::cerr << "TinyObjLoader Warning/Error: " << err << std::endl;
    }
    
    if (!ret) {
        std::cerr << "Failed to load model: " << filepath << std::endl;
        return false;
    }
    
    if (shapes.empty()) {
        std::cerr << "No shapes found in model: " << filepath << std::endl;
        return false;
    }
    
    std::cout << "Loading model: " << filepath << std::endl;
    std::cout << "  Vertices: " << attrib.vertices.size() / 3 << std::endl;
    std::cout << "  Normals: " << attrib.normals.size() / 3 << std::endl;
    std::cout << "  Texcoords: " << attrib.texcoords.size() / 2 << std::endl;
    std::cout << "  Shapes: " << shapes.size() << std::endl;
    
    // Process all shapes and combine into single vertex buffer
    for (const auto& shape : shapes) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
            for (size_t v = 0; v < 3; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                
                if (idx.vertex_index >= 0) {
                    vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                    vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                    vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
                } else {
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                }
                
                if (idx.normal_index >= 0 && (size_t)(idx.normal_index * 3 + 2) < attrib.normals.size()) {
                    normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
                    normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
                    normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
                } else {
                    normals.push_back(0.0f);
                    normals.push_back(1.0f);
                    normals.push_back(0.0f);
                }
                
                if (idx.texcoord_index >= 0 && (size_t)(idx.texcoord_index * 2 + 1) < attrib.texcoords.size()) {
                    texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                    texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                } else {
                    texcoords.push_back(0.0f);
                    texcoords.push_back(0.0f);
                }
                
                indices.push_back(static_cast<unsigned int>(indices.size()));
            }
            index_offset += 3;
        }
    }
    
    // Build triangles list for BVH
    std::cout << "Building collision BVH..." << std::endl;
    allTriangles.clear();
    for (size_t i = 0; i + 8 < vertices.size(); i += 9) {
        float a[3] = {vertices[i+0], vertices[i+1], vertices[i+2]};
        float b[3] = {vertices[i+3], vertices[i+4], vertices[i+5]};
        float c[3] = {vertices[i+6], vertices[i+7], vertices[i+8]};
        allTriangles.push_back(Triangle(a, b, c));
    }
    
    // Build BVH
    if (!allTriangles.empty()) {
        std::vector<Triangle> trisCopy = allTriangles;
        bvhRoot = buildBVH(trisCopy, 0);
        std::cout << "  BVH built with " << allTriangles.size() << " triangles" << std::endl;
    }
    
    calculateBounds();
    loaded = true;
    // NOTE: VBOs will be initialized lazily on first render when OpenGL context is ready
    // initVBOs();  // Can't call this here - OpenGL context not ready yet!
    
    std::cout << "Model loaded successfully!" << std::endl;
    std::cout << "  Final vertices: " << vertices.size() / 3 << std::endl;
    std::cout << "  Bounds: (" << minBounds[0] << ", " << minBounds[1] << ", " << minBounds[2] 
              << ") to (" << maxBounds[0] << ", " << maxBounds[1] << ", " << maxBounds[2] << ")" << std::endl;
    
    return true;
}

AABB Model::computeBounds(const std::vector<Triangle>& tris) const {
    if (tris.empty()) return AABB();
    
    AABB bounds = tris[0].getBounds();
    for (size_t i = 1; i < tris.size(); i++) {
        bounds.expand(tris[i].getBounds());
    }
    return bounds;
}

BVHNode* Model::buildBVH(std::vector<Triangle>& tris, int depth) {
    BVHNode* node = new BVHNode();
    node->bounds = computeBounds(tris);
    
    // Leaf node conditions
    if (tris.size() <= 8 || depth > 20) {
        node->triangles = tris;
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    
    // Split along longest axis
    int axis = node->bounds.longestAxis();
    float cx, cy, cz;
    node->bounds.center(cx, cy, cz);
    float mid = (axis == 0) ? cx : (axis == 1) ? cy : cz;
    
    std::vector<Triangle> leftTris, rightTris;
    for (auto& tri : tris) {
        float triCenter = tri.centroid[axis];
        if (triCenter < mid) {
            leftTris.push_back(tri);
        } else {
            rightTris.push_back(tri);
        }
    }
    
    // If split didn't work, make leaf
    if (leftTris.empty() || rightTris.empty()) {
        node->triangles = tris;
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    
    node->left = buildBVH(leftTris, depth + 1);
    node->right = buildBVH(rightTris, depth + 1);
    return node;
}

void Model::closestPointOnTriangle(float px, float py, float pz,
                                    const Triangle& tri,
                                    float& outX, float& outY, float& outZ) const {
    // Compute vectors
    float v0[3] = {tri.v0[0], tri.v0[1], tri.v0[2]};
    float v1[3] = {tri.v1[0], tri.v1[1], tri.v1[2]};
    float v2[3] = {tri.v2[0], tri.v2[1], tri.v2[2]};
    
    float ab[3] = {v1[0]-v0[0], v1[1]-v0[1], v1[2]-v0[2]};
    float ac[3] = {v2[0]-v0[0], v2[1]-v0[1], v2[2]-v0[2]};
    float ap[3] = {px-v0[0], py-v0[1], pz-v0[2]};
    
    float d1 = ab[0]*ap[0] + ab[1]*ap[1] + ab[2]*ap[2];
    float d2 = ac[0]*ap[0] + ac[1]*ap[1] + ac[2]*ap[2];
    if (d1 <= 0.0f && d2 <= 0.0f) {
        outX = v0[0]; outY = v0[1]; outZ = v0[2];
        return;
    }
    
    float bp[3] = {px-v1[0], py-v1[1], pz-v1[2]};
    float d3 = ab[0]*bp[0] + ab[1]*bp[1] + ab[2]*bp[2];
    float d4 = ac[0]*bp[0] + ac[1]*bp[1] + ac[2]*bp[2];
    if (d3 >= 0.0f && d4 <= d3) {
        outX = v1[0]; outY = v1[1]; outZ = v1[2];
        return;
    }
    
    float vc = d1*d4 - d3*d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        float v = d1 / (d1 - d3);
        outX = v0[0] + v*ab[0];
        outY = v0[1] + v*ab[1];
        outZ = v0[2] + v*ab[2];
        return;
    }
    
    float cp[3] = {px-v2[0], py-v2[1], pz-v2[2]};
    float d5 = ab[0]*cp[0] + ab[1]*cp[1] + ab[2]*cp[2];
    float d6 = ac[0]*cp[0] + ac[1]*cp[1] + ac[2]*cp[2];
    if (d6 >= 0.0f && d5 <= d6) {
        outX = v2[0]; outY = v2[1]; outZ = v2[2];
        return;
    }
    
    float vb = d5*d2 - d1*d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        outX = v0[0] + w*ac[0];
        outY = v0[1] + w*ac[1];
        outZ = v0[2] + w*ac[2];
        return;
    }
    
    float va = d3*d6 - d5*d4;
    if (va <= 0.0f && (d4-d3) >= 0.0f && (d5-d6) >= 0.0f) {
        float w = (d4-d3) / ((d4-d3) + (d5-d6));
        outX = v1[0] + w*(v2[0]-v1[0]);
        outY = v1[1] + w*(v2[1]-v1[1]);
        outZ = v1[2] + w*(v2[2]-v1[2]);
        return;
    }
    
    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    outX = v0[0] + ab[0]*v + ac[0]*w;
    outY = v0[1] + ab[1]*v + ac[1]*w;
    outZ = v0[2] + ab[2]*v + ac[2]*w;
}

bool Model::sphereTriangleIntersect(float sx, float sy, float sz, float radius, const Triangle& tri) const {
    float closestX, closestY, closestZ;
    closestPointOnTriangle(sx, sy, sz, tri, closestX, closestY, closestZ);
    
    float dx = sx - closestX;
    float dy = sy - closestY;
    float dz = sz - closestZ;
    float distSq = dx*dx + dy*dy + dz*dz;
    
    // Use <= for inclusive check to catch edge cases
    return distSq <= (radius * radius);
}

bool Model::checkBVHCollision(BVHNode* node, float sx, float sy, float sz, float radius) const {
    if (!node) return false;
    
    if (!node->bounds.intersectsSphere(sx, sy, sz, radius)) {
        return false;
    }
    
    if (node->isLeaf()) {
        for (const auto& tri : node->triangles) {
            if (sphereTriangleIntersect(sx, sy, sz, radius, tri)) {
                return true;
            }
        }
        return false;
    }
    
    return checkBVHCollision(node->left, sx, sy, sz, radius) ||
           checkBVHCollision(node->right, sx, sy, sz, radius);
}

bool Model::checkCollision(float localX, float localY, float localZ, float radius) const {
    if (!loaded || !bvhRoot) {
        return false;
    }
    
    // Scale position to model space
    float mx = localX / scaleFactor;
    float my = localY / scaleFactor;
    float mz = localZ / scaleFactor;
    float mr = radius / scaleFactor;
    
    return checkBVHCollision(bvhRoot, mx, my, mz, mr);
}

void Model::render() const {
    if (!loaded || vertices.empty()) {
        return;
    }
    
    glPushMatrix();
    glScalef(scaleFactor, scaleFactor, scaleFactor);
    
    // Initialize VBOs on first render if not already done
    if (!vboInitialized) {
        const_cast<Model*>(this)->initVBOs();
    }
    
    if (vboInitialized && vboVertices != 0) {
        // Modern VBO rendering - 3-5x faster than immediate mode
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        
        glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
        glNormalPointer(GL_FLOAT, 0, 0);
        
        if (vboTexCoords != 0 && !texcoords.empty()) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
            glTexCoordPointer(2, GL_FLOAT, 0, 0);
        }
        
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        if (vboTexCoords != 0) {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
    } else {
        // Fallback: immediate mode (slower)
        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < vertices.size() / 3; i++) {
            if (i * 3 + 2 < normals.size()) {
                glNormal3f(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
            }
            if (i * 2 + 1 < texcoords.size()) {
                glTexCoord2f(texcoords[i * 2], texcoords[i * 2 + 1]);
            }
            glVertex3f(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
        }
        glEnd();
    }
    
    glPopMatrix();
}

void Model::calculateBounds() {
    if (vertices.empty()) return;
    
    minBounds[0] = minBounds[1] = minBounds[2] = std::numeric_limits<float>::max();
    maxBounds[0] = maxBounds[1] = maxBounds[2] = std::numeric_limits<float>::lowest();
    
    for (size_t i = 0; i < vertices.size(); i += 3) {
        for (int j = 0; j < 3; j++) {
            float val = vertices[i + j];
            if (val < minBounds[j]) minBounds[j] = val;
            if (val > maxBounds[j]) maxBounds[j] = val;
        }
    }
}

void Model::getBounds(float& minX, float& maxX, float& minY, float& maxY, float& minZ, float& maxZ) const {
    minX = minBounds[0] * scaleFactor;
    maxX = maxBounds[0] * scaleFactor;
    minY = minBounds[1] * scaleFactor;
    maxY = maxBounds[1] * scaleFactor;
    minZ = minBounds[2] * scaleFactor;
    maxZ = maxBounds[2] * scaleFactor;
}

void Model::setScale(float scale) {
    scaleFactor = scale;
}

void Model::initVBOs() {
    if (vboInitialized || vertices.empty()) return;
    
    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    if (!normals.empty()) {
        glGenBuffers(1, &vboNormals);
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    }
    
    if (!texcoords.empty()) {
        glGenBuffers(1, &vboTexCoords);
        glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
        glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(float), texcoords.data(), GL_STATIC_DRAW);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    vboInitialized = true;
    std::cout << "VBOs initialized for model (vertices: " << vertices.size()/3 << ")" << std::endl;
}

void Model::cleanupVBOs() {
    if (vboVertices != 0) {
        glDeleteBuffers(1, &vboVertices);
        vboVertices = 0;
    }
    if (vboNormals != 0) {
        glDeleteBuffers(1, &vboNormals);
        vboNormals = 0;
    }
    if (vboTexCoords != 0) {
        glDeleteBuffers(1, &vboTexCoords);
        vboTexCoords = 0;
    }
    vboInitialized = false;
}

bool Model::getHeightAtPosition(float worldX, float worldZ, float modelX, float modelZ, float& outHeight) const {
    if (!loaded || vertices.empty()) return false;
    
    float localX = (worldX - modelX) / scaleFactor;
    float localZ = (worldZ - modelZ) / scaleFactor;
    
    float closestHeight = -999999.0f;
    bool found = false;
    
    for (size_t i = 0; i + 8 < vertices.size(); i += 9) {
        float v0x = vertices[i+0], v0y = vertices[i+1], v0z = vertices[i+2];
        float v1x = vertices[i+3], v1y = vertices[i+4], v1z = vertices[i+5];
        float v2x = vertices[i+6], v2y = vertices[i+7], v2z = vertices[i+8];
        
        float denom = (v1z - v2z) * (v0x - v2x) + (v2x - v1x) * (v0z - v2z);
        if (std::abs(denom) < 0.0001f) continue;
        
        float a = ((v1z - v2z) * (localX - v2x) + (v2x - v1x) * (localZ - v2z)) / denom;
        float b = ((v2z - v0z) * (localX - v2x) + (v0x - v2x) * (localZ - v2z)) / denom;
        float c = 1.0f - a - b;
        
        if (a >= -0.01f && a <= 1.01f && b >= -0.01f && b <= 1.01f && c >= -0.01f && c <= 1.01f) {
            float height = a * v0y + b * v1y + c * v2y;
            if (height > closestHeight) {
                closestHeight = height;
                found = true;
            }
        }
    }
    
    if (found) {
        outHeight = closestHeight * scaleFactor + modelX;
        return true;
    }
    return false;
}
