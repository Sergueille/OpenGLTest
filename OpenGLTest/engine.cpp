/*
Some code from https://learnopengl.com/
*/

#include <iostream>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <soloud.h>
#include <soloud_wav.h>

#include "MenuManager.h"
#include "Editor.h"
#include "mesh.h"
#include "shader.h"
#include "Texture.h"
#include "Utility.h"
#include "Sprite.h"
#include "TweenManager.h"
#include "EventManager.h"
#include "RessourceManager.h"
#include "Player.h"
#include "Camera.h"
#include "TextManager.h"
#include "Collider.h"
#include "EditorSprite.h"
#include "LightManager.h"
#include "ParticleSystem.h"

using namespace std;
using namespace Utility;

extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int main(int argc, void* argv[])
{
    // Read setting file
    std::map<std::string, std::string> settings = std::map<std::string, std::string>();
    EditorSaveManager::ReadPropsFile("Settings\\options.set", &settings);

    bool fullscreen = settings["fullscreen"] == "1";
    int smallWindowWidth = 1820;
    EditorSaveManager::IntProp(&settings, "screenX", &smallWindowWidth);
    int smallWindowHeght = 720;
    EditorSaveManager::IntProp(&settings, "screenY", &smallWindowHeght);
    bool displayFPS = settings["displayFPS"] == "1";

    bool useMainMonitor = settings["monitor"] == "main";
    int monitorID = useMainMonitor? 0 : std::stoi(settings["monitor"]);

    const char* windowName = "Teeeest!";
    const int bloomResDivide = 2;

    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window
    if (fullscreen)
    {
        GLFWmonitor* monitor = nullptr;
        if (useMainMonitor)
            monitor = glfwGetPrimaryMonitor();
        else
        {
            int monitorCount;
            GLFWmonitor** first = glfwGetMonitors(&monitorCount);

            if (monitorID >= monitorCount)
            {
                std::cerr << "Unkown monitor index, displaying on main monitor" << std::endl;
                monitor = glfwGetPrimaryMonitor();
            }

            monitor = *(first + monitorID);
        }

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        Utility::window = glfwCreateWindow(mode->width, mode->height, windowName, monitor, NULL);

        Utility::screenX = mode->width;
        Utility::screenY = mode->height;
    }
    else
    {
        Utility::window = glfwCreateWindow(smallWindowWidth, smallWindowHeght, windowName, NULL, NULL);

        Utility::screenX = smallWindowWidth;
        Utility::screenY = smallWindowHeght;
    }
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

    // Init sound engine
    Utility::soloud = new SoLoud::Soloud();
    Utility::soloud->init();

    // Init text manager
    TextManager::Init();

    // Eable blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create frame buffer
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); // Bind fbo

    // Create color texture
    unsigned int colorTex[2];
    glGenTextures(2, colorTex);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorTex[i]); // Bind colorTex
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenX, screenY, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind colorTex
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorTex[i], 0); // Bind color texture to buffer
    }

    // Create depth and stencil render buffer
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); // Bind rbo
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenX, screenY);
    glBindRenderbuffer(GL_RENDERBUFFER, 0); // Unbind rbo

    // Bind render buffer to frame buffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 }; // MOVE THIS??
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Frame buffer creation failed :(" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind fbo

    // Start event manager
    EventManager::SetupEvents();

    // Load shaders
    RessourceManager::LoadBaseShaders();

    // Open main menu
    MenuManager::OpenMenu(MenuManager::Menu::main);

    Camera::SetupCamera();

    // Create overlay sprite
    Sprite* overlaySprite = new Sprite(vec3(0, 0, overlayZ), vec3(screenX, screenY, overlayZ), overlayColor);

    // Create bloom blur buffers
    unsigned int pingpongFBO[2];
    unsigned int pingpongBuffer[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer); 
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenX / bloomResDivide, screenY / bloomResDivide, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
        );
    }


    /////// GAME LOOP
    while (!glfwWindowShouldClose(window))
    {
        // Init test variable
        Utility::testCount = 0;

        // Get Time
        Utility::time = (float)glfwGetTime();

        // Bind frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // Clear color and depth buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);

        /////////////////////
        // GAME LOOP START //
        /////////////////////

        Camera::UpdateCamera();

        // Display the FPS
        if (!Editor::enabled && displayFPS)
            TextManager::RenderText(std::to_string(GetFPS()) + " FPS",
                glm::vec3(screenX - Editor::margin, screenY - Editor::margin - Editor::textSize, Editor::UIBaseZPos),
                Editor::textSize, TextManager::left);

        // Call events
        EventManager::Call(&EventManager::nextFrameActions);
        EventManager::nextFrameActions = LinkedList<std::function<void()>>();
        EventManager::Call(&EventManager::OnMainLoop);

        overlaySprite->color = overlayColor;
        overlaySprite->position.z = overlayZ;
        overlaySprite->Draw();

        // Draw transparent sprites
        Sprite::DrawAll();

        // Update lightmap if needed
        if (LightManager::forceRefreshOnNextFrame)
        {
            LightManager::forceRefreshOnNextFrame = false;
            LightManager::mustReadNewFile = true;
        }

        if (Utility::testCount != 0)
            std::cout << testCount << std::endl;

        ///////////////////
        // GAME LOOP END //
        ///////////////////

        // Bind default buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Clear color and depth buffer
        // glClearColor(0, 0, 0, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Disable depth test
        glDisable(GL_DEPTH_TEST);

        // Blur the bright areas
        bool horizontal = true; 
        bool first_iteration = true;
        unsigned int amount = 4;
        RessourceManager::shaders["screenBlur"].Use();

        glViewport(0, 0, (int)(screenX / bloomResDivide), (int)(screenY / bloomResDivide));
        const float firstOffsetSize = 2;
        const float secondOffsetSize = 1;
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            RessourceManager::shaders["screenBlur"].SetUniform("horizontal", horizontal);
            RessourceManager::shaders["screenBlur"].SetUniform("offsetSize", i > 1? firstOffsetSize : secondOffsetSize);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorTex[1] : pingpongBuffer[!horizontal]);
            SpriteRenderer::GetMesh()->DrawMesh();
            horizontal = !horizontal;
            first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Draw render quad
        RessourceManager::shaders["screenShader"].Use();
        RessourceManager::shaders["screenShader"].SetUniform("bloomBlur", 1);
        RessourceManager::shaders["screenShader"].SetUniform("time", Utility::time);
        RessourceManager::shaders["screenShader"].SetUniform("corruptionAmount", corruptionAmount);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorTex[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[0]);

        // Set original viewport
        glViewport(0, 0, Utility::screenX, Utility::screenY);

        SpriteRenderer::GetMesh()->DrawMesh(); // Stealing the sprite quad

        // Check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Record FPS
        Utility::FPSvalues[FPSvaluePos] = static_cast<int>(1.f / (Utility::time - Utility::lastTime));
        FPSvaluePos++;
        FPSvaluePos %= FPS_NB_VALUES;

        Utility::lastTime = Utility::time;
    }

    Editor::DestroyEditor();

    EventManager::Call(&EventManager::OnExitApp);

    // Clear ressources
    RessourceManager::Clear();

    // Clean sound engine
    Utility::soloud->deinit();
    delete Utility::soloud;

    // Clean GLFW memory
    glfwTerminate();

    std::cout << "The game closed sucessfully!" << std::endl;
    return 0;
}
