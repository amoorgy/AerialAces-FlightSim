#include "Model.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <limits>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>

Model::Model() : loaded(false), scaleFactor(1.0f), bvhRoot(nullptr),
                 vboVertices(0), vboNormals(0), vboTexCoords(0), vboInitialized(false),
                 texture(nullptr), hasTexture(false),
                 heightmapResolution(0), heightmapMinX(0), heightmapMaxX(0),
                 heightmapMinZ(0), heightmapMaxZ(0), heightmapCellSize(1.0f),
                 heightmapBuilt(false) {
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
    
    if (texture) {
        delete texture;
        texture = nullptr;
    }
}

bool Model::loadMaterialTexture(const std::string& mtlPath) {
    std::ifstream file(mtlPath);
    if (!file.is_open()) {
        std::cout << "Could not open MTL file: " << mtlPath << std::endl;
        return false;
    }
    
    std::string line;
    std::string textureFile;
    
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        // Look for map_Kd (diffuse texture) or map_Ka (ambient texture)
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "map_Kd" || prefix == "map_Ka") {
            iss >> textureFile;
            if (!textureFile.empty()) {
                break;  // Found a texture, use it
            }
        }
    }
    file.close();
    
    if (textureFile.empty()) {
        std::cout << "No texture reference found in MTL file" << std::endl;
        return false;
    }
    
    // Build full texture path (texture file is usually in same directory as MTL)
    std::string texturePath = basePath + textureFile;
    
    // Also try without base path (in case texture path is already relative to assets)
    std::ifstream testFile(texturePath);
    if (!testFile.good()) {
        // Try just the texture filename in the same directory
        texturePath = basePath + textureFile;
        testFile.open(texturePath);
        if (!testFile.good()) {
            // Try various asset paths
            const char* assetPaths[] = {
                "",
                "assets/",
                "../",
                "../../",
                "../../../",
                "../../../../",
                "../../../../../",
                nullptr
            };
            
            for (int i = 0; assetPaths[i] != nullptr; i++) {
                texturePath = std::string(assetPaths[i]) + basePath + textureFile;
                testFile.open(texturePath);
                if (testFile.good()) {
                    testFile.close();
                    break;
                }
            }
        }
    }
    testFile.close();
    
    std::cout << "Loading texture: " << texturePath << std::endl;
    
    texture = new Texture();
    if (texture->load(texturePath)) {
        hasTexture = true;
        std::cout << "Texture loaded successfully!" << std::endl;
        return true;
    } else {
        delete texture;
        texture = nullptr;
        std::cout << "Failed to load texture: " << texturePath << std::endl;
        return false;
    }
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
    
    // Extract base path for texture loading
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        basePath = filepath.substr(0, lastSlash + 1);
    } else {
        basePath = "";
    }
    
    // Try to load texture from MTL file
    std::string mtlPath = filepath.substr(0, filepath.find_last_of('.')) + ".mtl";
    loadMaterialTexture(mtlPath);
    
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
    
    // Build heightmap for collision
    buildHeightmap(256);  // 256x256 resolution
    
    loaded = true;
    
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
    
    // Bind texture if available
    bool textureEnabled = false;
    if (hasTexture && texture && texture->isLoaded()) {
        texture->bind();
        textureEnabled = true;
    }
    
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
    
    // Unbind texture
    if (textureEnabled) {
        texture->unbind();
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
    
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    
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
    // This is a simplified height query - a proper implementation would ray cast
    // For now, return false to indicate no height data available
    return false;
}

void Model::buildHeightmap(int resolution) {
    if (!loaded || vertices.empty()) {
        heightmapBuilt = false;
        return;
    }
    
    heightmapResolution = resolution;
    heightmap.resize(resolution * resolution, -99999.0f);  // Initialize to very low
    
    // Use model bounds for heightmap extents
    heightmapMinX = minBounds[0];
    heightmapMaxX = maxBounds[0];
    heightmapMinZ = minBounds[2];
    heightmapMaxZ = maxBounds[2];
    
    float rangeX = heightmapMaxX - heightmapMinX;
    float rangeZ = heightmapMaxZ - heightmapMinZ;
    heightmapCellSize = std::max(rangeX, rangeZ) / (resolution - 1);
    
    std::cout << "Building heightmap " << resolution << "x" << resolution << std::endl;
    std::cout << "  X range: " << heightmapMinX << " to " << heightmapMaxX << std::endl;
    std::cout << "  Z range: " << heightmapMinZ << " to " << heightmapMaxZ << std::endl;
    
    // For each triangle, rasterize it into the heightmap
    for (size_t i = 0; i + 8 < vertices.size(); i += 9) {
        float v0x = vertices[i+0], v0y = vertices[i+1], v0z = vertices[i+2];
        float v1x = vertices[i+3], v1y = vertices[i+4], v1z = vertices[i+5];
        float v2x = vertices[i+6], v2y = vertices[i+7], v2z = vertices[i+8];
        
        // Find bounding box of triangle in grid coords
        float triMinX = std::min({v0x, v1x, v2x});
        float triMaxX = std::max({v0x, v1x, v2x});
        float triMinZ = std::min({v0z, v1z, v2z});
        float triMaxZ = std::max({v0z, v1z, v2z});
        
        int gridMinX = std::max(0, (int)((triMinX - heightmapMinX) / heightmapCellSize));
        int gridMaxX = std::min(resolution - 1, (int)((triMaxX - heightmapMinX) / heightmapCellSize) + 1);
        int gridMinZ = std::max(0, (int)((triMinZ - heightmapMinZ) / heightmapCellSize));
        int gridMaxZ = std::min(resolution - 1, (int)((triMaxZ - heightmapMinZ) / heightmapCellSize) + 1);
        
        // Rasterize triangle
        for (int gz = gridMinZ; gz <= gridMaxZ; gz++) {
            for (int gx = gridMinX; gx <= gridMaxX; gx++) {
                float worldX = heightmapMinX + gx * heightmapCellSize;
                float worldZ = heightmapMinZ + gz * heightmapCellSize;
                
                // Barycentric coordinates to check if point is in triangle
                float denom = (v1z - v2z) * (v0x - v2x) + (v2x - v1x) * (v0z - v2z);
                if (std::abs(denom) < 0.0001f) continue;
                
                float a = ((v1z - v2z) * (worldX - v2x) + (v2x - v1x) * (worldZ - v2z)) / denom;
                float b = ((v2z - v0z) * (worldX - v2x) + (v0x - v2x) * (worldZ - v2z)) / denom;
                float c = 1.0f - a - b;
                
                // Check if inside triangle (with small tolerance)
                if (a >= -0.01f && b >= -0.01f && c >= -0.01f) {
                    float height = a * v0y + b * v1y + c * v2y;
                    int idx = gz * resolution + gx;
                    if (height > heightmap[idx]) {
                        heightmap[idx] = height;
                    }
                }
            }
        }
    }
    
    // Fill holes by averaging neighbors (simple gap filling)
    std::vector<float> smoothed = heightmap;
    for (int z = 1; z < resolution - 1; z++) {
        for (int x = 1; x < resolution - 1; x++) {
            int idx = z * resolution + x;
            if (heightmap[idx] < -99000.0f) {
                float sum = 0;
                int count = 0;
                for (int dz = -1; dz <= 1; dz++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int nidx = (z + dz) * resolution + (x + dx);
                        if (heightmap[nidx] > -99000.0f) {
                            sum += heightmap[nidx];
                            count++;
                        }
                    }
                }
                if (count > 0) {
                    smoothed[idx] = sum / count;
                }
            }
        }
    }
    heightmap = smoothed;
    
    heightmapBuilt = true;
    std::cout << "Heightmap built successfully!" << std::endl;
}

float Model::sampleHeightmapBilinear(float x, float z) const {
    if (!heightmapBuilt) return -99999.0f;
    
    // Convert to grid coordinates
    float gx = (x - heightmapMinX) / heightmapCellSize;
    float gz = (z - heightmapMinZ) / heightmapCellSize;
    
    // Clamp to valid range
    gx = std::max(0.0f, std::min(gx, (float)(heightmapResolution - 2)));
    gz = std::max(0.0f, std::min(gz, (float)(heightmapResolution - 2)));
    
    int x0 = (int)gx;
    int z0 = (int)gz;
    int x1 = x0 + 1;
    int z1 = z0 + 1;
    
    float fx = gx - x0;
    float fz = gz - z0;
    
    float h00 = heightmap[z0 * heightmapResolution + x0];
    float h10 = heightmap[z0 * heightmapResolution + x1];
    float h01 = heightmap[z1 * heightmapResolution + x0];
    float h11 = heightmap[z1 * heightmapResolution + x1];
    
    // Skip invalid samples
    if (h00 < -99000.0f || h10 < -99000.0f || h01 < -99000.0f || h11 < -99000.0f) {
        return -99999.0f;
    }
    
    // Bilinear interpolation
    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;
    return h0 * (1.0f - fz) + h1 * fz;
}

bool Model::checkHeightmapCollision(float localX, float localY, float localZ, float radius) const {
    if (!heightmapBuilt) return false;
    
    // Scale to model space
    float mx = localX / scaleFactor;
    float my = localY / scaleFactor;
    float mz = localZ / scaleFactor;
    float mr = radius / scaleFactor;
    
    // Sample terrain height at this XZ position
    float terrainHeight = sampleHeightmapBilinear(mx, mz);
    
    if (terrainHeight < -99000.0f) {
        return false;  // Outside heightmap bounds
    }
    
    // Collision if player's bottom is below terrain
    return (my - mr) < terrainHeight;
}

bool Model::getTerrainHeightAt(float localX, float localZ, float& outHeight) const {
    if (!heightmapBuilt) return false;
    
    float mx = localX / scaleFactor;
    float mz = localZ / scaleFactor;
    
    float h = sampleHeightmapBilinear(mx, mz);
    if (h < -99000.0f) return false;
    
    outHeight = h * scaleFactor;
    return true;
}
