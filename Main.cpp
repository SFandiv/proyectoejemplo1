#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Define esto antes del include si stb_image.h no está precompilado en otro lado
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

    out vec3 FragPos;
    out vec3 Normal;
    out vec2 TexCoord;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        TexCoord = aTexCoord;
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core

    out vec4 FragColor;

    in vec2 TexCoord;
    in vec3 Normal;
    in vec3 FragPos;

    uniform sampler2D texture1;
    uniform vec3 lightPos;
    uniform vec3 lightColor;

    void main() {
    
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse);
    
    vec4 texColor = texture(texture1, TexCoord);
        FragColor = vec4(result * texColor.rgb, texColor.a);
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

    // --- VÉRTICES Y UV (Sin EBO) ---
    float vertices[] = {
        // Posiciones          // Texturas (UV)
        // Cara Frontal
        -0.5f, 0.0f,  0.5f,    0.0f, 0.0f,  0.0f, 0.5f, 0.8f,
         0.5f, 0.0f,  0.5f,    1.0f, 0.0f,  0.0f, 0.5f, 0.8f,
         0.0f, 0.8f,  0.0f,    0.5f, 1.0f,  0.0f, 0.5f, 0.8f,
         // Cara Derecha
          0.5f, 0.0f,  0.5f,    0.0f, 0.0f,  0.8f, 0.5f, 0.0f,
          0.5f, 0.0f, -0.5f,    1.0f, 0.0f,  0.8f, 0.5f, 0.0f,
          0.0f, 0.8f,  0.0f,    0.5f, 1.0f,  0.8f, 0.5f, 0.0f,
          // Cara Trasera
           0.5f, 0.0f, -0.5f,    0.0f, 0.0f,  0.0f, 0.5f, -0.8f,
          -0.5f, 0.0f, -0.5f,    1.0f, 0.0f,  0.0f, 0.5f, -0.8f,
           0.0f, 0.8f,  0.0f,    0.5f, 1.0f,  0.0f, 0.5f, -0.8f,
           // Cara Izquierda
           -0.5f, 0.0f, -0.5f,    0.0f, 0.0f,   -0.8f, 0.5f, 0.0f,
           -0.5f, 0.0f,  0.5f,    1.0f, 0.0f,   -0.8f, 0.5f, 0.0f,
            0.0f, 0.8f,  0.0f,    0.5f, 1.0f,   -0.8f, 0.5f, 0.0f,
            // Base (2 triángulos) 
            -0.5f, 0.0f, -0.5f,    0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
             0.5f, 0.0f, -0.5f,    1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
             0.5f, 0.0f,  0.5f,    1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
             0.5f, 0.0f,  0.5f,    1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
            -0.5f, 0.0f,  0.5f,    0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
            -0.5f, 0.0f, -0.5f,    0.0f, 1.0f,   0.0f, -1.0f, 0.0f

    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Atributo 0: Posición (X, Y, Z) - Salto de 8 floats, empieza en 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atributo 1: Textura (U, V) - Salto de 8 floats, empieza después de 3 floats
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Atributo 2: Normales (NX, NY, NZ) - Salto de 8 floats, empieza después de 5 floats (Pos + Tex)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // --- CARGAR TEXTURA ---
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true); // Evita que la textura salga de cabeza
    int width, height, nrChannels;
    unsigned char* data = stbi_load("C:\\Users\\Santiago\\source\\repos\\proyectoejemplo1\\Resources\\image2.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        // Si tu imagen fuera PNG, cambia GL_RGB por GL_RGBA
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Fallo al cargar la textura" << std::endl;
    }
    stbi_image_free(data);

    // --- LOOP PRINCIPAL ---
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

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

        // Render
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glUseProgram(shaderProgram);

        // Define la posición de la luz (ej. un poco a la derecha, arriba y hacia adelante)
        // Calcular una posición que gira en círculo a lo largo del tiempo
        float time = glfwGetTime();
        glm::vec3 lightPos(2.0f * sin(time), 1.0f, 2.0f * cos(time));
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // Luz completamente blanca

        GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");

        glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(lightColorLoc, lightColor.x, lightColor.y, lightColor.z);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 18);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}