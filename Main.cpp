#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"
#include "Shader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION 
#include <stb/stb_image.h>

// --- VARIABLES GLOBALES Y CÁMARA ---
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
float lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;

glm::vec3 calculateCubicBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    glm::vec3 p = uuu * p0;
    p += 3.0f * uu * t * p1;
    p += 3.0f * u * tt * p2;
    p += ttt * p3;

    return p;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }

    float xoffset = (xpos - lastX) * 0.1f;
    float yoffset = (lastY - ypos) * 0.1f;
    lastX = xpos; lastY = ypos;

    yaw += xoffset; pitch += yoffset;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Piramide 3D Texturizada", NULL, NULL);
    if (!window) { std::cout << "Error GLFW\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGL()) { std::cout << "Error GLAD\n"; return -1; }
    glEnable(GL_DEPTH_TEST);

    Model fox("Models/Fox.glb");

    Assimp::Importer importer;
    std::cout << "Assimp funciona correctamente!" << std::endl;

    // --- COMPILAR SHADERS DESDE ARCHIVOS ---
    Shader ourShader("Shaders/default.vert", "Shaders/default.frag");

    // --- VÉRTICES Y UV ---
    float vertices[] = {
        // Posiciones          // Texturas (UV)   // Normales       // Tangentes
        // Cara Frontal
        -0.5f, 0.0f,  0.5f,    0.0f, 0.0f,   0.0f, 0.5f, 0.8f,    1.0f, 0.0f, 0.0f,
         0.5f, 0.0f,  0.5f,    1.0f, 0.0f,   0.0f, 0.5f, 0.8f,    1.0f, 0.0f, 0.0f,
         0.0f, 0.8f,  0.0f,    0.5f, 1.0f,   0.0f, 0.5f, 0.8f,    1.0f, 0.0f, 0.0f,
         // Cara Derecha
          0.5f, 0.0f,  0.5f,    0.0f, 0.0f,   0.8f, 0.5f, 0.0f,    0.0f, 0.0f, -1.0f,
          0.5f, 0.0f, -0.5f,    1.0f, 0.0f,   0.8f, 0.5f, 0.0f,    0.0f, 0.0f, -1.0f,
          0.0f, 0.8f,  0.0f,    0.5f, 1.0f,   0.8f, 0.5f, 0.0f,    0.0f, 0.0f, -1.0f,
          // Cara Trasera
           0.5f, 0.0f, -0.5f,    0.0f, 0.0f,   0.0f, 0.5f, -0.8f,  -1.0f, 0.0f, 0.0f,
          -0.5f, 0.0f, -0.5f,    1.0f, 0.0f,   0.0f, 0.5f, -0.8f,  -1.0f, 0.0f, 0.0f,
           0.0f, 0.8f,  0.0f,    0.5f, 1.0f,   0.0f, 0.5f, -0.8f,  -1.0f, 0.0f, 0.0f,
           // Cara Izquierda
           -0.5f, 0.0f, -0.5f,    0.0f, 0.0f,  -0.8f, 0.5f, 0.0f,    0.0f, 0.0f, 1.0f,
           -0.5f, 0.0f,  0.5f,    1.0f, 0.0f,  -0.8f, 0.5f, 0.0f,    0.0f, 0.0f, 1.0f,
            0.0f, 0.8f,  0.0f,    0.5f, 1.0f,  -0.8f, 0.5f, 0.0f,    0.0f, 0.0f, 1.0f,
            // Base 
            -0.5f, 0.0f, -0.5f,    0.0f, 1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
             0.5f, 0.0f, -0.5f,    1.0f, 1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
             0.5f, 0.0f,  0.5f,    1.0f, 0.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
             0.5f, 0.0f,  0.5f,    1.0f, 0.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
            -0.5f, 0.0f,  0.5f,    0.0f, 0.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
            -0.5f, 0.0f, -0.5f,    0.0f, 1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // --- CARGAR TEXTURAS DE LA PIRÁMIDE ---
    GLuint textureDiffuse, textureNormal;
    glGenTextures(1, &textureDiffuse);
    glGenTextures(1, &textureNormal);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);

    glBindTexture(GL_TEXTURE_2D, textureDiffuse);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    unsigned char* dataDiffuse = stbi_load("C:\\Users\\Santiago\\source\\repos\\proyectoejemplo1\\Resources\\image2.jpg", &width, &height, &nrChannels, 0);
    if (dataDiffuse) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, dataDiffuse);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else std::cout << "Fallo al cargar textura de ladrillos\n";
    stbi_image_free(dataDiffuse);

    glBindTexture(GL_TEXTURE_2D, textureNormal);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    unsigned char* dataNormal = stbi_load("C:\\Users\\Santiago\\source\\repos\\proyectoejemplo1\\Resources\\image2_normal2.png", &width, &height, &nrChannels, 0);
    if (dataNormal) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, dataNormal);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else std::cout << "Fallo al cargar normal map\n";
    stbi_image_free(dataNormal);

    // --- RUTAS DE CÁMARA (BÉZIER) ---
    glm::vec3 p0(-4.0f, 2.0f, 4.0f);
    glm::vec3 p1(0.0f, 4.0f, 5.0f);
    glm::vec3 p2(4.0f, 0.5f, 2.0f);
    glm::vec3 p3(1.5f, -0.2f, 1.5f);

    float bezierTime = 0.0f;
    bool playIntro = true;

    // --- LOOP PRINCIPAL ---
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Cámara
        if (playIntro) {
            bezierTime += 0.2f * deltaTime;

            if (bezierTime >= 1.0f) {
                bezierTime = 1.0f;
                playIntro = false;
            }

            cameraPos = calculateCubicBezier(p0, p1, p2, p3, bezierTime);

            glm::vec3 foxTarget(1.5f, -0.5f, 0.0f);
            cameraFront = glm::normalize(foxTarget - cameraPos);
        }
        else {
            float cameraSpeed = 2.5f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
        }

        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        // Actualizar luces y cámara
        glm::vec3 lightPos = cameraPos;
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
        ourShader.setVec3("lightPos", lightPos);
        ourShader.setVec3("lightColor", lightColor);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // ---------------------------------
        // DIBUJAR LA PIRÁMIDE
        // ---------------------------------
        glm::mat4 identity = glm::mat4(1.0f);
        ourShader.setMat4("model", identity);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureDiffuse);
        ourShader.setInt("diffuseMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureNormal);
        ourShader.setInt("normalMap", 1);
        ourShader.setBool("useNormalMap", true);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 18);

        // ---------------------------------
        // DIBUJAR EL ZORRO
        // ---------------------------------
        glm::mat4 foxModel = glm::mat4(1.0f);
        foxModel = glm::translate(foxModel, glm::vec3(1.5f, -0.5f, 0.0f));
        foxModel = glm::scale(foxModel, glm::vec3(0.01f));
        ourShader.setMat4("model", foxModel);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

        fox.Draw(ourShader.ID);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(ourShader.ID);
    glfwTerminate();
    return 0;
}