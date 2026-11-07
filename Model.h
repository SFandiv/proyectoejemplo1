#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

#include <string>
#include <vector>

class Model
{
public:

    std::vector<Mesh> meshes;

    Model(const std::string& path);

    void Draw(unsigned int shaderProgram);

private:

    std::string directory;

    void loadModel(const std::string& path);

    void processNode(aiNode* node, const aiScene* scene);

    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    std::vector<Texture> loadMaterialTextures(
        aiMaterial* mat,
        aiTextureType type,
        const std::string& typeName,
        const aiScene* scene
    );

    unsigned int TextureFromFile(
        const std::string& path,
        const std::string& directory
    );

    unsigned int TextureFromEmbedded(
        const aiTexture* texture
    );
};