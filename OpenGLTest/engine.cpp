/*
Code from https://learnopengl.com/
*/

#include <iostream>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "mesh.h"
#include "shader.h"
#include "Texture.h"
#include "Utility.h"
#include "Sprite.h"

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
    GLFWwindow* window = glfwCreateWindow(Utility::screenX, Utility::screenY, "Teeeest!", NULL, NULL);
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
    glViewport(0, 0, Utility::screenX, Utility::screenY);

    // Create triangle
    Texture tex = Texture("Images/potplant.png");
    Shader defaultShader = Shader("Shaders\\DefaultVertexShader.glsl", "Shaders\\DefaultFragShader.glsl");
    Sprite test = Sprite(&defaultShader, &tex);
    
    // Rendering loop
    while (!glfwWindowShouldClose(window))
    {
        // Clear image
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //Draw sprite
        test.Draw();

        // Check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean memory
    glfwTerminate();
    return 0;
}
