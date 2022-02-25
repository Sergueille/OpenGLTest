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
#include "EventManager.h"
#include "RessourceManager.h"
#include "Player.h"

using namespace std;
using namespace Utility;

int main(int argc, void* argv[])
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window
    window = glfwCreateWindow(Utility::screenX, Utility::screenY, "Teeeest!", NULL, NULL);
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

    /////// GAME START

    // Load basic ressources
    RessourceManager::LoadShader("Shaders\\SpriteVertexShader.glsl", "Shaders\\SpriteFragShader.glsl", "sprite");
    RessourceManager::LoadShader("Shaders\\SpriteVertexShader.glsl", "Shaders\\ColorFragShader.glsl", "spriteColor");
    RessourceManager::LoadTexture("Images\\circle.png", "circle");

    Sprite sprites[] = {
        Sprite(NULL, glm::vec3(0, -3, 0), glm::vec2(5, 1), 0, glm::vec4(0)),
    };

    // Create palyer
    Player player = Player();
    
    /////// GAME LOOP
    while (!glfwWindowShouldClose(window))
    {
        // Get Time
        Utility::time = glfwGetTime();

        // Clear image
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        EventManager::Call(&EventManager::OnMainLoop);

        Utility::lastTime = Utility::time;

        // Check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    EventManager::Call(&EventManager::OnExitApp);
    RessourceManager::Clear();

    // Clean memory
    glfwTerminate();
    return 0;
}
