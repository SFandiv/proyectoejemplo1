#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"

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

// --- SHADERS ---
const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    layout (location = 2) in vec3 aNormal;
    layout (location = 3) in vec3 aTangent;

    out vec3 FragPos;
    out vec2 TexCoord;
    out mat3 TBN;

    uniform mat4 model, view, projection;

    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        FragPos = vec3(model * vec4(aPos, 1.0));
        TexCoord = aTexCoord;

        vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
        vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T);
        TBN = mat3(T, B, N);
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core

    out vec4 FragColor;

    in vec2 TexCoord;
    in mat3 TBN;
    in vec3 FragPos;

    uniform sampler2D diffuseMap;
    uniform sampler2D normalMap;
    uniform vec3 lightPos;
    uniform vec3 lightColor;

    void main() {
        // 1. Extraer el mapa de normales de la imagen azul y ajustarlo
        vec3 normal = texture(normalMap, TexCoord).rgb;
        normal = normalize(normal * 2.0 - 1.0);

        // 2. Alinear la normal plana con la geometría 3D
        normal = normalize(TBN * normal);

        // 3. Calcular la luz direccional y ambiental
        vec3 ambient = 0.1 * lightColor;
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        vec4 texColor = texture(diffuseMap, TexCoord);
        FragColor = vec4((ambient + diffuse) * texColor.rgb, texColor.a);
        //FragColor = texture(normalMap, TexCoord);
    }
)glsl";

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

    // --- PRUEBA DE ASSIMP ---

    Assimp::Importer importer;

    std::cout << "Assimp funciona correctamente!" << std::endl;

    // --- COMPILAR SHADERS ---
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR DE COMPILACION VERTEX SHADER:\n" << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader); glDeleteShader(fragmentShader);

    // --- VÉRTICES Y UV ---
    float vertices[] = {
        // Posiciones          // Texturas (UV)   // Normales       // Tangentes
        // Cara Frontal
        -0.5f, 0.0f,  0.5f,    0.0f, 0.0f,  0.0f, 0.5f, 0.8f,   1.0f, 0.0f, 0.0f,
         0.5f, 0.0f,  0.5f,    1.0f, 0.0f,  0.0f, 0.5f, 0.8f,   1.0f, 0.0f, 0.0f,
         0.0f, 0.8f,  0.0f,    0.5f, 1.0f,  0.0f, 0.5f, 0.8f,   1.0f, 0.0f, 0.0f,
         // Cara Derecha
          0.5f, 0.0f,  0.5f,    0.0f, 0.0f,  0.8f, 0.5f, 0.0f,   0.0f, 0.0f, -1.0f,
          0.5f, 0.0f, -0.5f,    1.0f, 0.0f,  0.8f, 0.5f, 0.0f,   0.0f, 0.0f, -1.0f,
          0.0f, 0.8f,  0.0f,    0.5f, 1.0f,  0.8f, 0.5f, 0.0f,   0.0f, 0.0f, -1.0f,
          // Cara Trasera
           0.5f, 0.0f, -0.5f,    0.0f, 0.0f,  0.0f, 0.5f, -0.8f, -1.0f, 0.0f, 0.0f,
          -0.5f, 0.0f, -0.5f,    1.0f, 0.0f,  0.0f, 0.5f, -0.8f, -1.0f, 0.0f, 0.0f,
           0.0f, 0.8f,  0.0f,    0.5f, 1.0f,  0.0f, 0.5f, -0.8f, -1.0f, 0.0f, 0.0f,
           // Cara Izquierda
           -0.5f, 0.0f, -0.5f,    0.0f, 0.0f, -0.8f, 0.5f, 0.0f,   0.0f, 0.0f, 1.0f,
           -0.5f, 0.0f,  0.5f,    1.0f, 0.0f, -0.8f, 0.5f, 0.0f,   0.0f, 0.0f, 1.0f,
            0.0f, 0.8f,  0.0f,    0.5f, 1.0f, -0.8f, 0.5f, 0.0f,   0.0f, 0.0f, 1.0f,
            // Base 
            -0.5f, 0.0f, -0.5f,    0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
             0.5f, 0.0f, -0.5f,    1.0f, 1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
             0.5f, 0.0f,  0.5f,    1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
             0.5f, 0.0f,  0.5f,    1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
            -0.5f, 0.0f,  0.5f,    0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
            -0.5f, 0.0f, -0.5f,    0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f
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


    // --- CARGAR TEXTURAS ---
    GLuint textureDiffuse, textureNormal;
    glGenTextures(1, &textureDiffuse);
    glGenTextures(1, &textureNormal);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);

    // 1. TEXTURA ORIGINAL (LADRILLOS)
    glBindTexture(GL_TEXTURE_2D, textureDiffuse);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char* dataDiffuse = stbi_load("C:\\Users\\Santiago\\source\\repos\\proyectoejemplo1\\Resources\\image2.jpg", &width, &height, &nrChannels, 0);
    if (dataDiffuse) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, dataDiffuse);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Fallo al cargar la textura de ladrillos" << std::endl;
    }
    stbi_image_free(dataDiffuse);

    // 2. TEXTURA DE NORMALES (AZUL)
    glBindTexture(GL_TEXTURE_2D, textureNormal);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char* dataNormal = stbi_load("C:\\Users\\Santiago\\source\\repos\\proyectoejemplo1\\Resources\\image2_normal2.png", &width, &height, &nrChannels, 0);
    if (dataNormal) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, dataNormal);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Fallo al cargar el mapa de normales" << std::endl;
    }
    stbi_image_free(dataNormal);

    // --- CONFIGURACIÓN PREVIA AL BUCLE ---
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "diffuseMap"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "normalMap"), 1);

    // --- LOOP PRINCIPAL ---
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input de cámara
        float cameraSpeed = 2.5f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

        // Limpiar pantalla
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enlazar Texturas
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureNormal);

        // Posición y color de la luz dinámica
        glm::vec3 lightPos = cameraPos;
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

        // Enviar matrices y luz al Shader
        glUseProgram(shaderProgram);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), lightColor.x, lightColor.y, lightColor.z);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // Dibujar (Una sola vez por frame)
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 18);

        // Matriz del modelo del zorro
        glm::mat4 foxModel = glm::mat4(1.0f);

        // Posicionar el zorro
        foxModel = glm::translate(
            foxModel,
            glm::vec3(1.5f, -0.5f, 0.0f)
        );

        // Reducir su tamaño
        foxModel = glm::scale(
            foxModel,
            glm::vec3(0.01f)
        );

        // Enviar la matriz al shader
        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "model"),
            1,
            GL_FALSE,
            glm::value_ptr(foxModel)
        );  
        // Desvincular el normal map de la pirámide por seguridad antes de dibujar el zorro
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Dibujar el zorro pasando el shader
        fox.Draw(shaderProgram);

        // Restaurar la matriz original para otros objetos
        glm::mat4 identity = glm::mat4(1.0f);

        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "model"),
            1,
            GL_FALSE,
            glm::value_ptr(identity)
        );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}