# Proyecto de Gráficos por Computadora – OpenGL

Aplicación interactiva de gráficos 3D desarrollada en **C++ y OpenGL 3.3**, que implementa geometría manual, importación de modelos, texturizado, iluminación y navegación mediante cámara interactiva.

La escena incluye una pirámide texturizada y un modelo 3D de un zorro, acompañados de una animación inicial de cámara mediante curvas de Bézier.

## Características

- Renderizado de escenas tridimensionales con OpenGL.
- Construcción manual de geometría.
- Importación de modelos GLB mediante Assimp.
- Aplicación de texturas difusas y mapas de normales.
- Iluminación mediante shaders GLSL.
- Transformaciones 3D: traslación, rotación y escala.
- Cámara interactiva con controles de teclado.
- Animación inicial mediante curvas de Bézier cúbicas.

## Tecnologías utilizadas

- **C++**
- **OpenGL 3.3**
- GLFW y GLAD
- GLM
- Assimp
- STB Image
- GLSL
- Visual Studio 2022
- vcpkg

## Controles

| Tecla | Acción |
|---|---|
| W | Avanzar |
| S | Retroceder |
| A | Desplazarse a la izquierda |
| D | Desplazarse a la derecha |
| ESC | Cerrar la aplicación |
| Mouse | Orientar la cámara (si está habilitado) |

Al iniciar, se reproduce una animación de cámara. Al finalizar, se habilita la navegación manual.

## Requisitos

- Windows 10/11.
- Visual Studio 2022.
- MSVC v143 y Windows SDK.
- Git y vcpkg.
- Arquitectura x64.
- Bibliotecas GLFW, GLAD, GLM, Assimp y STB Image.

## Instalación y ejecución

**1. Clonar el repositorio**

```bash
git clone https://github.com/SFandiv/proyectoejemplo1.git
cd proyectoejemplo1
```

**2. Configurar dependencias**

Instalar las bibliotecas necesarias mediante vcpkg y configurar su integración con Visual Studio utilizando la arquitectura `x64-windows`.

**3. Abrir el proyecto**

Abrir la solución en Visual Studio 2022 y seleccionar:

- Configuración: Debug o Release.
- Plataforma: x64.
- Toolset: v143.

**4. Compilar y ejecutar**

Compilar la solución y ejecutar el proyecto. Verificar que los modelos y las texturas estén disponibles en las rutas correspondientes.

## Arquitectura del proyecto

El código está organizado mediante los siguientes componentes:

- **Main.cpp:** inicialización de OpenGL, configuración de la escena y ciclo de renderizado.
- **Shader:** lectura, compilación y administración de shaders.
- **Mesh:** gestión de vértices, índices, normales y texturas.
- **Model:** importación y procesamiento de modelos 3D mediante Assimp.

## Conceptos implementados

- Pipeline de renderizado.
- VAO, VBO y EBO.
- Vertex Shader y Fragment Shader.
- Coordenadas UV, normales y tangentes.
- Texturas difusas y mapas de normales.
- Matrices Model, View y Projection.
- Curvas de Bézier cúbicas.
- Control de cámara y tiempo entre fotogramas.

## Autor

**Santiago Fandiño** 
6014431
Estudiante de Ingeniería Multimedia.

Proyecto académico enfocado en programación gráfica y representación tridimensional mediante OpenGL.
