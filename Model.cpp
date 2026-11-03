
#include "Model.h"

#include <iostream>
#include <filesystem>
#include <cmath>

#include <stb/stb_image.h>

// Constructor
Model::Model(const std::string& path)
{
    loadModel(path);
}

// Cargar el modelo usando Assimp
void Model::loadModel(const std::string& path)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace
    );

    // Comprobar errores
    if (!scene || !scene->mRootNode ||
        (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
    {
        std::cout << "ERROR::ASSIMP::"
            << importer.GetErrorString()
            << std::endl;
        return;
    }

    // Guardar el directorio del modelo
    size_t slash = path.find_last_of("/\\");
    directory = (slash == std::string::npos)
        ? "."
        : path.substr(0, slash);

    // Procesar los nodos
    processNode(scene->mRootNode, scene);

    std::cout << "Modelo cargado correctamente: "
        << path << std::endl;
}

// Procesar los nodos de la escena
void Model::processNode(aiNode* node, const aiScene* scene)
{
    // Procesar las mallas del nodo actual
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    // Procesar los nodos hijos
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

// Procesar una malla individual
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Recorrer los vértices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        // Posición
        vertex.Position = glm::vec3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );

        // Normales
        if (mesh->HasNormals())
        {
            vertex.Normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        }
        else
        {
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        // Coordenadas UV
        if (mesh->mTextureCoords[0])
        {
            vertex.TexCoords = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        // Tangentes
        if (mesh->HasTangentsAndBitangents())
        {
            vertex.Tangent = glm::vec3(
                mesh->mTangents[i].x,
                mesh->mTangents[i].y,
                mesh->mTangents[i].z
            );
        }
        else
        {
            glm::vec3 n = glm::normalize(vertex.Normal);

            glm::vec3 helper =
                (std::abs(n.y) < 0.999f)
                ? glm::vec3(0.0f, 1.0f, 0.0f)
                : glm::vec3(1.0f, 0.0f, 0.0f);

            vertex.Tangent = glm::normalize(
                glm::cross(helper, n)
            );
        }

        vertices.push_back(vertex);
    }

    // Obtener índices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Obtener materiales y texturas
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material =
            scene->mMaterials[mesh->mMaterialIndex];

        // Textura de color base (GLTF)
        textures = loadMaterialTextures(
            material,
            aiTextureType_BASE_COLOR,
            "texture_diffuse",
            scene
        );

        // Si no existe, buscar textura diffuse
        if (textures.empty())
        {
            textures = loadMaterialTextures(
                material,
                aiTextureType_DIFFUSE,
                "texture_diffuse",
                scene
            );
        }

        // Buscar mapa de normales
        std::vector<Texture> normalTextures =
            loadMaterialTextures(
                material,
                aiTextureType_NORMALS,
                "texture_normal",
                scene
            );

        // Agregar las normales
        textures.insert(
            textures.end(),
            normalTextures.begin(),
            normalTextures.end()
        );
    }

    // Crear la malla con sus texturas
    return Mesh(vertices, indices, textures);
}

// Cargar las texturas de un material
std::vector<Texture> Model::loadMaterialTextures(
    aiMaterial* mat,
    aiTextureType type,
    const std::string& typeName,
    const aiScene* scene)
{
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;

        if (mat->GetTexture(type, i, &str) != AI_SUCCESS)
            continue;

        Texture texture;

        texture.id = 0;
        texture.type = typeName;
        texture.path = str.C_Str();

        // Comprobar si la textura está incrustada en el GLB
        const aiTexture* embedded =
            scene->GetEmbeddedTexture(str.C_Str());

        if (embedded)
        {
            texture.id = TextureFromEmbedded(embedded);
        }
        else
        {
            texture.id = TextureFromFile(
                str.C_Str(),
                directory
            );
        }

        if (texture.id != 0)
        {
            textures.push_back(texture);
        }
    }

    return textures;
}

// Cargar una textura externa
unsigned int Model::TextureFromFile(
    const std::string& path,
    const std::string& directory)
{
    std::string filename = directory + "/" + path;

    int width, height, channels;

    stbi_set_flip_vertically_on_load(false);

    unsigned char* data = stbi_load(
        filename.c_str(),
        &width,
        &height,
        &channels,
        0
    );

    if (!data)
    {
        std::cout << "ERROR::TEXTURE:: No se pudo cargar: "
            << filename << std::endl;
        return 0;
    }

    GLenum format = GL_RGB;

    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    unsigned int textureID;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    stbi_image_free(data);

    return textureID;
}

// Cargar una textura incrustada dentro del GLB
unsigned int Model::TextureFromEmbedded(
    const aiTexture* texture)
{
    int width, height, channels;

    unsigned char* data = nullptr;

    GLenum format = GL_RGBA;

    if (texture->mHeight == 0)
    {
        // Imagen comprimida: PNG, JPG, etc.
        data = stbi_load_from_memory(
            reinterpret_cast<const unsigned char*>(
                texture->pcData
                ),
            static_cast<int>(texture->mWidth),
            &width,
            &height,
            &channels,
            0
        );

        if (!data)
        {
            std::cout << "ERROR::TEXTURE:: "
                << "No se pudo leer la textura incrustada."
                << std::endl;
            return 0;
        }

        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;
    }
    else
    {
        // Imagen almacenada como píxeles aiTexel
        width = static_cast<int>(texture->mWidth);
        height = static_cast<int>(texture->mHeight);

        data = new unsigned char[width * height * 4];

        for (int i = 0; i < width * height; i++)
        {
            data[i * 4 + 0] = texture->pcData[i].r;
            data[i * 4 + 1] = texture->pcData[i].g;
            data[i * 4 + 2] = texture->pcData[i].b;
            data[i * 4 + 3] = texture->pcData[i].a;
        }

        format = GL_RGBA;
    }

    unsigned int textureID;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR
    );
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    if (texture->mHeight == 0) {
        stbi_image_free(data); // Liberar memoria alojada por stb_image
    }
    else {
        delete[] data;         // Liberar memoria alojada por new[]
    }
    return textureID;
}

// Dibujar todas las mallas
void Model::Draw(unsigned int shaderProgram)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].Draw(shaderProgram);
    }
}