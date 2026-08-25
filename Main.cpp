#include<iostream>
// GLAD es un cargador de funciones que gestiona los punteros de OpenGL modernos para nuestro hardware específico.
#include<glad/glad.h>
// GLFW es la biblioteca que nos ayuda a crear la ventana, manejar el teclado, ratón y el contexto de renderizado.
#include<GLFW/glfw3.h>

// Código fuente del Vertex Shader (Shader de Vértices) escrito en GLSL.
// Su propósito principal es procesar cada uno de los vértices y establecer su posición final en la pantalla.
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n" // Recibe la posición del vértice como atributo de entrada en el canal 0.
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n" // Transforma el vector 3D a coordenadas homogéneas de 4D (w = 1.0).
"}\0";

// Código fuente del Fragment Shader (Shader de Fragmentos/Píxeles) escrito en GLSL.
// Su propósito es calcular y pintar el color final de cada píxel individual de nuestras formas geométricas.
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n" // Variable de salida que enviará el color final al búfer de pantalla.
"void main()\n"
"{\n"
"  FragColor = vec4(0.8f, 0.3f, 0.03f, 1.0f);\n" // Define el color de dibujo (Naranja/Rojizo en formato RGBA).
"}\n\0";

int main()
{
	// Inicializa la biblioteca GLFW para poder usar sus funciones de ventanas y eventos.
	glfwInit();

	// Configura los "hints" de GLFW para indicarle a nuestro sistema operativo qué versión de OpenGL queremos.
	// Definimos que usaremos OpenGL versión 3.3 (Versión Mayor 3).
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// Definimos que usaremos OpenGL versión 3.3 (Versión Menor 3).
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Le decimos a GLFW que use el "Profile Core" de OpenGL.
	// El perfil Core nos obliga a usar el pipeline moderno eliminando todas las funciones obsoletas del pipeline clásico.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Arreglo unidimensional que contiene las coordenadas (X, Y, Z) de nuestra geometría en el espacio afín.
	// Hemos añadido los tres cuadrados de las esquinas. Cada cuadrado mide 0.1 de ancho/alto (0.05 de radio a cada lado).
	GLfloat vertices[] =
	{
		// === 1. TRIÁNGULO PRINCIPAL (3 Vértices) ===
		// Esquina inferior izquierda (V1): (-0.5f, -0.288675f, 0.0f)
		-0.5f, -0.288675f, 0.0f,
		// Esquina inferior derecha (V2):   (0.5f, -0.288675f, 0.0f)
		 0.5f, -0.288675f, 0.0f,
		 // Esquina superior (V3):            (0.0f,  0.577350f, 0.0f)
		  0.0f,  0.577350f, 0.0f,

		  // === 2. CUADRADO EN LA ESQUINA INFERIOR IZQUIERDA (Centrado en V1) ===
		  // Primer triángulo del cuadrado
		  -0.55f, -0.338675f, 0.0f, // Inferior Izquierda (BL)
		  -0.45f, -0.338675f, 0.0f, // Inferior Derecha (BR)
		  -0.45f, -0.238675f, 0.0f, // Superior Derecha (TR)
		  // Segundo triángulo del cuadrado
		  -0.55f, -0.338675f, 0.0f, // Inferior Izquierda (BL)
		  -0.45f, -0.238675f, 0.0f, // Superior Derecha (TR)
		  -0.55f, -0.238675f, 0.0f, // Superior Izquierda (TL)

		  // === 3. CUADRADO EN LA ESQUINA INFERIOR DERECHA (Centrado en V2) ===
		  // Primer triángulo del cuadrado
		   0.45f, -0.338675f, 0.0f, // Inferior Izquierda (BL)
		   0.55f, -0.338675f, 0.0f, // Inferior Derecha (BR)
		   0.55f, -0.238675f, 0.0f, // Superior Derecha (TR)
		   // Segundo triángulo del cuadrado
			0.45f, -0.338675f, 0.0f, // Inferior Izquierda (BL)
			0.55f, -0.238675f, 0.0f, // Superior Derecha (TR)
			0.45f, -0.238675f, 0.0f, // Superior Izquierda (TL)

			// === 4. CUADRADO EN LA ESQUINA SUPERIOR (Centrado en V3) ===
			// Primer triángulo del cuadrado
			-0.05f,  0.527350f, 0.0f, // Inferior Izquierda (BL)
			 0.05f,  0.527350f, 0.0f, // Inferior Derecha (BR)
			 0.05f,  0.627350f, 0.0f, // Superior Derecha (TR)
			 // Segundo triángulo del cuadrado
			 -0.05f,  0.527350f, 0.0f, // Inferior Izquierda (BL)
			  0.05f,  0.627350f, 0.0f, // Superior Derecha (TR)
			 -0.05f,  0.627350f, 0.0f  // Superior Izquierda (TL)
	};

	// Crea una ventana del sistema operativo con resolución de 800x600 píxeles, título personalizado,
	// y sin usar pantalla completa ni compartir recursos (NULL, NULL).
	GLFWwindow* window = glfwCreateWindow(800, 600, "Aprender OpenGL", NULL, NULL);

	// Valida que el puntero devuelto no sea nulo. Si falló la creación, imprime error, cierra GLFW y sale.
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate(); // Libera los recursos asignados por GLFW.
		return -1; // Detiene el programa indicando fallo.
	}

	// Establece la ventana que acabamos de crear como el contexto OpenGL de renderizado actual para este hilo.
	glfwMakeContextCurrent(window);

	// GLAD carga dinámicamente todas las direcciones de memoria de las funciones nativas del driver de OpenGL.
	gladLoadGL();

	// Define las dimensiones físicas del Viewport de renderizado.
	// Indica a OpenGL que dibuje dentro de las coordenadas de píxeles que van de (0,0) a (800,600) de nuestra ventana.
	glViewport(0, 0, 800, 600);

	// Crea un objeto Shader vacío de tipo VERTEX_SHADER en la memoria de la GPU y nos devuelve su ID único.
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// Vincula el código fuente en texto (vertexShaderSource) con el objeto shader creado.
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	// Compila el código fuente del vertex shader directamente en el compilador integrado de la GPU.
	glCompileShader(vertexShader);

	// Crea un objeto Shader vacío de tipo FRAGMENT_SHADER en la memoria de la GPU y nos devuelve su ID único.
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// Vincula el código fuente en texto (fragmentShaderSource) con el objeto shader creado.
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	// Compila el fragment shader en la GPU para que pueda procesar la rasterización de píxeles.
	glCompileShader(fragmentShader);

	// Crea un objeto contenedor de programa de sombreado (Shader Program) en la GPU que unirá nuestros shaders individuales.
	GLuint shaderProgram = glCreateProgram();
	// Adjunta (asocia) el vertex shader compilado a nuestro programa final.
	glAttachShader(shaderProgram, vertexShader);
	// Adjunta (asocia) el fragment shader compilado a nuestro programa final.
	glAttachShader(shaderProgram, fragmentShader);
	// Enlaza (Links) el programa de sombreado para generar el ejecutable binario que correrá en los procesadores de la GPU.
	glLinkProgram(shaderProgram);

	// Una vez enlazado el programa, los shaders individuales ya no son necesarios en memoria suelta, por lo que se liberan.
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Declara las variables que almacenarán los ID de nuestros objetos de configuración (VAO) y búfer de datos (VBO).
	GLuint VAO, VBO;

	// Genera 1 objeto Vertex Array Object (VAO) y guarda su ID único en la variable 'VAO'.
	glGenVertexArrays(1, &VAO);
	// Genera 1 objeto Vertex Buffer Object (VBO) de memoria y guarda su ID único en la variable 'VBO'.
	glGenBuffers(1, &VBO);

	// Enlaza (Bindea) el VAO. A partir de aquí, el VAO grabará toda la configuración de punteros y buffers que hagamos.
	glBindVertexArray(VAO);

	// Vincula el VBO indicándole a OpenGL que funcionará como un almacén de atributos de vértices (GL_ARRAY_BUFFER).
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Sube los datos físicos del arreglo 'vertices' desde la RAM de la CPU a la memoria de alta velocidad de la GPU.
	// 'GL_STATIC_DRAW' optimiza el espacio asumiendo que los datos geométricos se subirán una vez y no cambiarán constantemente.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Define cómo debe interpretar OpenGL los datos del VBO para el shader de vértices:
	// 0: Índice del atributo (location = 0 en el vertex shader).
	// 3: Cantidad de componentes por vértice (coordenadas X, Y, Z).
	// GL_FLOAT: Tipo de datos que contiene cada componente.
	// GL_FALSE: Indica que no queremos que OpenGL normalice los valores (no escalarlos automáticamente entre 0 y 1).
	// 3 * sizeof(float): Stride (Paso) - espacio en bytes entre el inicio de un vértice y el inicio del siguiente.
	// (void*)0: Offset (Desplazamiento) - posición inicial en bytes dentro del búfer (inicia en el primer byte).
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	// Habilita el canal de atributos número 0 para que la GPU pueda leer los datos en el vertex shader.
	glEnableVertexAttribArray(0);

	// Desvincula (Unbindea) el VBO activo para evitar modificaciones accidentales en el estado global.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Desvincula el VAO activo para cerrar el manual de instrucciones y que no registre nada más por error.
	glBindVertexArray(0);

	// Establece el color de limpieza que usará el búfer de pantalla (un gris azulado oscuro).
	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	// Limpia la pantalla aplicando el color de fondo previamente establecido.
	glClear(GL_COLOR_BUFFER_BIT);
	// Intercambia el buffer trasero con el delantero para mostrarle el fondo limpio al usuario inmediatamente.
	glfwSwapBuffers(window);

	// Inicia el bucle principal de renderizado que mantiene vivo el software hasta que el usuario decida cerrarlo.
	while (!glfwWindowShouldClose(window))
	{
		// En cada frame, limpia la pantalla estableciendo de nuevo el color de fondo.
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		// Ejecuta físicamente la limpieza del búfer de color.
		glClear(GL_COLOR_BUFFER_BIT);
		// Le dice a la GPU que use el shaderProgram activo para realizar el dibujo de este frame.
		glUseProgram(shaderProgram);
		// Enlaza el VAO que contiene la configuración y punteros de nuestra geometría.
		glBindVertexArray(VAO);
		// Dibuja las primitivas en pantalla.
		// GL_TRIANGLES: Tipo de primitiva que interpretará la GPU.
		// 0: Índice del primer vértice que queremos dibujar.
		// 21: Número de vértices totales a renderizar (3 del triángulo + 18 de los tres cuadrados).
		glDrawArrays(GL_TRIANGLES, 0, 21);
		// Intercambia los buffers de imagen (Double Buffering) para evitar que el usuario note el proceso de dibujo en tiempo real.
		glfwSwapBuffers(window);

		// Procesa todos los eventos pendientes en la cola del sistema operativo (clics, teclas presionadas, mover la ventana).
		glfwPollEvents();
	}

	// Libera los recursos geométricos del VAO del hardware de la GPU al cerrar el programa.
	glDeleteVertexArrays(1, &VAO);
	// Libera la memoria del búfer de vértices VBO de la GPU.
	glDeleteBuffers(1, &VBO);
	// Destruye el programa ejecutable de Shaders de la memoria gráfica.
	glDeleteProgram(shaderProgram);

	// Destruye físicamente la ventana creada en el sistema operativo.
	glfwDestroyWindow(window);
	// Finaliza el sistema de GLFW y libera todos los recursos de biblioteca inicializados.
	glfwTerminate();
	return 0;
}