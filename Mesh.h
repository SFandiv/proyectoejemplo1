
#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstddef>

#include <vector>
#include <string>

struct Vertex {
    glm::vec3 Position;
    glm::vec2 TexCoords;
    glm::vec3 Normal;
    glm::vec3 Tangent;
};

// Información de una textura
struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh
{
public:

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    unsigned int VAO, VBO, EBO;

    Mesh(
        std::vector<Vertex> vertices,
        std::vector<unsigned int> indices,
        std::vector<Texture> textures
    );

    void Draw(unsigned int shaderProgram);
};