#include "Model.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <limits>
#include <cmath>

Model::Model() : loaded(false), scaleFactor(1.0f) {
    for (int i = 0; i < 3; i++) {
        minBounds[i] = 0.0f;
        maxBounds[i] = 0.0f;
    }
}

Model::~Model() {
    vertices.clear();
    normals.clear();
    texcoords.clear();
    indices.clear();
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
        // Process each face
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
            // Process each triangle (3 vertices)
            for (size_t v = 0; v < 3; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                
                // Add vertex position
                if (idx.vertex_index >= 0) {
                    vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                    vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                    vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
                } else {
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                }
                
                // Add normal
                if (idx.normal_index >= 0 && idx.normal_index * 3 + 2 < attrib.normals.size()) {
                    normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
                    normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
                    normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
                } else {
                    // Calculate face normal if not provided
                    normals.push_back(0.0f);
                    normals.push_back(1.0f);
                    normals.push_back(0.0f);
                }
                
                // Add texture coordinate
                if (idx.texcoord_index >= 0 && idx.texcoord_index * 2 + 1 < attrib.texcoords.size()) {
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
    
    // Calculate normals if missing
    if (normals.empty() || normals.size() != vertices.size()) {
        normals.clear();
        normals.resize(vertices.size(), 0.0f);
        
        // Calculate face normals
        for (size_t i = 0; i < indices.size(); i += 3) {
            unsigned int i0 = indices[i];
            unsigned int i1 = indices[i + 1];
            unsigned int i2 = indices[i + 2];
            
            float v0[3] = {vertices[i0 * 3], vertices[i0 * 3 + 1], vertices[i0 * 3 + 2]};
            float v1[3] = {vertices[i1 * 3], vertices[i1 * 3 + 1], vertices[i1 * 3 + 2]};
            float v2[3] = {vertices[i2 * 3], vertices[i2 * 3 + 1], vertices[i2 * 3 + 2]};
            
            float edge1[3] = {v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]};
            float edge2[3] = {v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]};
            
            float normal[3] = {
                edge1[1] * edge2[2] - edge1[2] * edge2[1],
                edge1[2] * edge2[0] - edge1[0] * edge2[2],
                edge1[0] * edge2[1] - edge1[1] * edge2[0]
            };
            
            float length = std::sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
            if (length > 0.0f) {
                normal[0] /= length;
                normal[1] /= length;
                normal[2] /= length;
            }
            
            // Add to all three vertices
            for (int j = 0; j < 3; j++) {
                unsigned int idx = indices[i + j];
                normals[idx * 3 + 0] += normal[0];
                normals[idx * 3 + 1] += normal[1];
                normals[idx * 3 + 2] += normal[2];
            }
        }
        
        // Normalize all normals
        for (size_t i = 0; i < normals.size(); i += 3) {
            float length = std::sqrt(normals[i] * normals[i] + normals[i + 1] * normals[i + 1] + normals[i + 2] * normals[i + 2]);
            if (length > 0.0f) {
                normals[i] /= length;
                normals[i + 1] /= length;
                normals[i + 2] /= length;
            }
        }
    }
    
    calculateBounds();
    loaded = true;
    
    std::cout << "Model loaded successfully!" << std::endl;
    std::cout << "  Final vertices: " << vertices.size() / 3 << std::endl;
    std::cout << "  Bounds: (" << minBounds[0] << ", " << minBounds[1] << ", " << minBounds[2] 
              << ") to (" << maxBounds[0] << ", " << maxBounds[1] << ", " << maxBounds[2] << ")" << std::endl;
    
    return true;
}

void Model::render() const {
    if (!loaded || vertices.empty()) {
        return;
    }
    
    glPushMatrix();
    glScalef(scaleFactor, scaleFactor, scaleFactor);
    
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < vertices.size() / 3; i++) {
        // Set normal
        if (i * 3 + 2 < normals.size()) {
            glNormal3f(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
        }
        
        // Set texture coordinate
        if (i * 2 + 1 < texcoords.size()) {
            glTexCoord2f(texcoords[i * 2], texcoords[i * 2 + 1]);
        }
        
        // Set vertex position
        glVertex3f(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
    }
    glEnd();
    
    glPopMatrix();
}

void Model::calculateBounds() {
    if (vertices.empty()) {
        return;
    }
    
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
