/*
Some code from https://learnopengl.com/
*/

#include <iostream>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_lofifilter.h>

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
#include "TerminalManager.h"
#include "LocalizationManager.h"
#include "SettingsManager.h"

using namespace std;
using namespace Utility;

extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// OpenGL error callback
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type, severity, message);
}

int main(int argc, void* argv[])
{
#if _DEBUG
    std::cerr << "The game is running in debug configuration!" << std::endl;
#endif

    SettingsManager::ReadSettings();
    SettingsManager::CreateGLFWWindow();
    SettingsManager::ApplySettings();

    // Set error callback
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    // Init sound engine
    Utility::soloud = new SoLoud::Soloud();
    Utility::soloud->init();

    // Init text manager
    TextManager::Init();

    // Eable blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Start event manager
    EventManager::SetupEvents();

    // Load localization
    LocalizationManager::LoadLanguage(LocalizationManager::Language::french);

    // Load shaders
    RessourceManager::LoadBaseShaders();

    // Open main menu
    MenuManager::OpenMenu(MenuManager::Menu::main);

    Camera::SetupCamera();

    // Create overlay sprite
    Sprite* overlaySprite = new Sprite(vec3(0, 0, overlayZ), vec3(screenX, screenY, overlayZ), overlayColor);

    TerminalManager::Init();

    // Set bg color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    Utility::lofiFilter = new SoLoud::LofiFilter();
    Utility::soloud->setGlobalFilter(0, lofiFilter);
    Utility::soloud->setFilterParameter(0, 0, SoLoud::LofiFilter::WET, 0);
    lofiFilter->setParams(5, 10);

    /////// GAME LOOP
    while (!glfwWindowShouldClose(window))
    {
        // Init test variable
        Utility::testCount = 0;

        // Get Time
        Utility::time = (float)glfwGetTime();

        // Bind frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, SettingsManager::FBO);

        // Clear color and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);

        /////////////////////
        // GAME LOOP START //
        /////////////////////

        Camera::UpdateCamera();
        EditorSaveManager::OnMainLoop();

        // Display the FPS
        if (!Editor::enabled && SettingsManager::settings["displayFPS"] == "1")
            TextManager::RenderText(std::to_string(GetFPS()) + " FPS",
                glm::vec3(screenX - Editor::margin, screenY - Editor::margin - Editor::textSize, Editor::UIBaseZPos),
                Editor::textSize, TextManager::left);

        // Call events
        EventManager::Call(&EventManager::nextFrameActions);
        EventManager::nextFrameActions = LinkedList<std::function<void()>>();
        EventManager::Call(&EventManager::OnMainLoop);
        EventManager::Call(&EventManager::OnAfterMainLoop);

        overlaySprite->color = overlayColor;
        overlaySprite->position.z = overlayZ;
        overlaySprite->Draw();

        // Draw transparent sprites
        Sprite::DrawAll();

        // Set soloud volume
        soloud->setGlobalVolume(globalVolume * globalVolumeOverride);

#if _DEBUG
        if (Utility::testCount != 0)
            std::cout << testCount << std::endl;
#endif

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
            glBindFramebuffer(GL_FRAMEBUFFER, SettingsManager::pingpongFBO[horizontal]);
            RessourceManager::shaders["screenBlur"].SetUniform("horizontal", horizontal);
            RessourceManager::shaders["screenBlur"].SetUniform("offsetSize", i > 1? firstOffsetSize : secondOffsetSize);
            BindTexture2D(first_iteration ? SettingsManager::colorTex[1] : SettingsManager::pingpongBuffer[!horizontal]);
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
        RessourceManager::shaders["screenShader"].SetUniform("corruptionTexture", 2);
        BindTexture2D(SettingsManager::colorTex[0], 0);
        BindTexture2D(SettingsManager::pingpongBuffer[0], 1);
        BindTexture2D(SettingsManager::colorTex[2], 2);

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

    EventManager::Call(&EventManager::OnExitApp);

    Editor::DestroyEditor();

    // Clear ressources
    RessourceManager::Clear();

    // Clean sound engine
    delete Utility::lofiFilter;
    Utility::soloud->deinit();
    delete Utility::soloud;

    LocalizationManager::Clear();

    // Clean GLFW memory
    glfwTerminate();

    std::cout << "The game closed sucessfully!" << std::endl;
    return 0;
}
