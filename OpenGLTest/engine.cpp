/*
Code from https://learnopengl.com/
*/

#include <vector>
#include <iostream>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "mesh.h"
#include "shader.h"

using namespace std;

int main(int argc, void* argv[])
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Teeeest!", NULL, NULL);
    if (window == NULL) 
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Init GLAD (get gl API)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // Setup OpenGL viewport
    glViewport(0, 0, 800, 600);

    float vertices[] = {
     0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    // Create triangle
    Shader defaultShader = *new Shader("Shaders\\DefaultVertexShader.glsl", "Shaders\\DefaultFragShader.glsl");
    MeshVertices meshData = *new MeshVertices(&vertices[0], (int)size(vertices), &indices[0], (int)size(indices));
    Mesh triangle = *new Mesh(&meshData, defaultShader.ID);

    // Rendering loop
    while (!glfwWindowShouldClose(window))
    {
        // Clear image
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float blueValue = (sin(glfwGetTime()) / 2.0f) + 0.5f;
        defaultShader.SetUniform("mainColor", 0, 0, blueValue, 1);

        // Draw triangle
        triangle.DrawMesh();

        // Check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean memory
    glfwTerminate();
    return 0;
}
