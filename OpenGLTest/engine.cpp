/*
Code from https://learnopengl.com/
*/

#include <iostream>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <soloud.h>
#include <soloud_wav.h>

#include "Editor.h"
#include "mesh.h"
#include "shader.h"
#include "Texture.h"
#include "Utility.h"
#include "Sprite.h"
#include "EventManager.h"
#include "RessourceManager.h"
#include "Player.h"
#include "Camera.h"
#include "TextManager.h"
#include "Collider.h"
#include "EditorSprite.h"

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

    int width = 1280;
    int height = 720;

    window = glfwCreateWindow(width, height, "Teeeest!", NULL, NULL);
    if (window == NULL) 
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    Utility::screenX = width;
    Utility::screenY = height;

    // Init GLAD (get gl API)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // Setup OpenGL viewport
    glViewport(0, 0, Utility::screenX, Utility::screenY);

    // Init sound engine
    Utility::soloud = new SoLoud::Soloud();
    Utility::soloud->init();

    // Init text manager
    TextManager::Init();

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Eable blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Start event manager
    EventManager::SetupEvents();

    // Load shaders
    RessourceManager::LoadShader("Shaders\\SpriteVertexShader.glsl", "Shaders\\SpriteFragShader.glsl", "sprite");
    RessourceManager::LoadShader("Shaders\\SpriteVertexShader.glsl", "Shaders\\ColorFragShader.glsl", "spriteColor");
    RessourceManager::LoadShader("Shaders\\SpriteVertexShader.glsl", "Shaders\\LaserFragShader.glsl", "laser");

    // Start level editor
    Editor::CreateEditor();
    EditorSaveManager::LoadLevel("test.map", true);

    Camera::SetupCamera();

    /////// GAME LOOP
    while (!glfwWindowShouldClose(window))
    {
        // Get Time
        Utility::time = (float)glfwGetTime();

        // Clear image and depth buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Camera::UpdateCamera();

        EventManager::Call(&EventManager::OnMainLoop);

        Sprite::DrawAll();

        // Check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        Utility::lastTime = Utility::time;
    }

    EventManager::Call(&EventManager::OnExitApp);

    // Clear ressources
    RessourceManager::Clear();

    // Clean sound engine
    Utility::soloud->deinit();
    delete Utility::soloud;

    // Clean GLFW memory
    glfwTerminate();
    return 0;
}
