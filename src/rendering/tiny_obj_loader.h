// tinyobjloader - Minimal OBJ loader implementation
// Full library: https://github.com/tinyobjloader/tinyobjloader

#ifndef TINY_OBJ_LOADER_H
#define TINY_OBJ_LOADER_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

namespace tinyobj {

struct vec3 {
    float x, y, z;
    vec3() : x(0), y(0), z(0) {}
    vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct vec2 {
    float x, y;
    vec2() : x(0), y(0) {}
    vec2(float x_, float y_) : x(x_), y(y_) {}
};

struct attrib_t {
    std::vector<float> vertices;  // 3 floats per vertex (x, y, z)
    std::vector<float> normals;   // 3 floats per normal (x, y, z)
    std::vector<float> texcoords; // 2 floats per texcoord (u, v)
};

struct index_t {
    int vertex_index;
    int normal_index;
    int texcoord_index;
    
    index_t() : vertex_index(-1), normal_index(-1), texcoord_index(-1) {}
};

struct mesh_t {
    std::vector<index_t> indices;
    std::vector<unsigned char> num_face_vertices; // Number of vertices per face
};

struct shape_t {
    std::string name;
    mesh_t mesh;
};

struct material_t {
    std::string name;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    std::string ambient_texname;
    std::string diffuse_texname;
    
    material_t() : shininess(0.0f) {}
};

// Parse vertex data (v, vn, vt lines)
inline void ParseVertex(const std::string& line, std::vector<float>& data, int components) {
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix; // Skip 'v', 'vn', or 'vt'
    
    for (int i = 0; i < components; i++) {
        float val;
        if (iss >> val) {
            data.push_back(val);
        }
    }
}

// Parse face data (f lines)
inline void ParseFace(const std::string& line, mesh_t& mesh) {
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix; // Skip 'f'
    
    std::vector<index_t> face_indices;
    std::string vertex_str;
    
    while (iss >> vertex_str) {
        index_t idx;
        
        // Parse format: v/vt/vn or v//vn or v/vt or v
        size_t pos1 = vertex_str.find('/');
        if (pos1 == std::string::npos) {
            // Format: v
            idx.vertex_index = std::stoi(vertex_str) - 1;
        } else {
            idx.vertex_index = std::stoi(vertex_str.substr(0, pos1)) - 1;
            
            size_t pos2 = vertex_str.find('/', pos1 + 1);
            if (pos2 == std::string::npos) {
                // Format: v/vt
                if (pos1 + 1 < vertex_str.length()) {
                    idx.texcoord_index = std::stoi(vertex_str.substr(pos1 + 1)) - 1;
                }
            } else {
                // Format: v/vt/vn or v//vn
                if (pos2 > pos1 + 1) {
                    idx.texcoord_index = std::stoi(vertex_str.substr(pos1 + 1, pos2 - pos1 - 1)) - 1;
                }
                if (pos2 + 1 < vertex_str.length()) {
                    idx.normal_index = std::stoi(vertex_str.substr(pos2 + 1)) - 1;
                }
            }
        }
        
        face_indices.push_back(idx);
    }
    
    // Triangulate if necessary (simple fan triangulation)
    if (face_indices.size() >= 3) {
        for (size_t i = 1; i < face_indices.size() - 1; i++) {
            mesh.indices.push_back(face_indices[0]);
            mesh.indices.push_back(face_indices[i]);
            mesh.indices.push_back(face_indices[i + 1]);
        }
    }
}

// Main loading function
inline bool LoadObj(attrib_t& attrib, std::vector<shape_t>& shapes, std::vector<material_t>& materials,
                   std::string* err, const char* filename) {
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        if (err) *err = std::string("Failed to open file: ") + filename;
        return false;
    }
    
    shape_t current_shape;
    current_shape.name = "default";
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start);
        
        // Parse based on prefix
        if (line.substr(0, 2) == "v ") {
            ParseVertex(line, attrib.vertices, 3);
        }
        else if (line.substr(0, 3) == "vn ") {
            ParseVertex(line, attrib.normals, 3);
        }
        else if (line.substr(0, 3) == "vt ") {
            ParseVertex(line, attrib.texcoords, 2);
        }
        else if (line.substr(0, 2) == "f ") {
            ParseFace(line, current_shape.mesh);
        }
        else if (line.substr(0, 2) == "o " || line.substr(0, 2) == "g ") {
            // New object/group
            if (!current_shape.mesh.indices.empty()) {
                shapes.push_back(current_shape);
                current_shape = shape_t();
            }
            current_shape.name = line.substr(2);
        }
    }
    
    // Add final shape
    if (!current_shape.mesh.indices.empty()) {
        shapes.push_back(current_shape);
    }
    
    file.close();
    return true;
}

} // namespace tinyobj

#endif // TINY_OBJ_LOADER_H
