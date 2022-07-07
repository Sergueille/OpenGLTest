#include "SettingsManager.h"

#include "EditorSaveManager.h"
#include "MenuManager.h"

std::map<std::string, std::string> SettingsManager::settings = std::map<std::string, std::string>();

unsigned int SettingsManager::FBO;
unsigned int SettingsManager::colorTex[3];
unsigned int SettingsManager::pingpongFBO[2];
unsigned int SettingsManager::pingpongBuffer[2];
bool SettingsManager::windowCreated = false;

void SettingsManager::ReadSettings()
{
	EditorSaveManager::ReadPropsFile("Settings\\options.set", &settings);
}

void SettingsManager::CreateGLFWWindow()
{
    if (windowCreated) // Delete previous window
    {
        glDeleteFramebuffers(1, &FBO);
        glDeleteFramebuffers(2, &pingpongFBO[0]);

        glDeleteTextures(3, &colorTex[0]);
        glDeleteBuffers(2, &pingpongBuffer[0]);

        glfwDestroyWindow(window);
    }

    bool fullscreen = SettingsManager::settings["fullscreen"] == "1";
    int smallWindowWidth = 1820;
    EditorSaveManager::IntProp(&SettingsManager::settings, "screenX", &smallWindowWidth);
    int smallWindowHeght = 720;
    EditorSaveManager::IntProp(&SettingsManager::settings, "screenY", &smallWindowHeght);
    bool displayFPS = SettingsManager::settings["displayFPS"] == "1";

    bool useMainMonitor = SettingsManager::settings["monitor"] == "main";
    int monitorID = useMainMonitor ? 0 : std::stoi(SettingsManager::settings["monitor"]);

    const char* windowName = "Teeeest!";

    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
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
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    // Init GLAD (get gl API)
    if (!windowCreated)
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return;
        }

    // Setup OpenGL viewport
    glViewport(0, 0, Utility::screenX, Utility::screenY);

    // Create frame buffer
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO); // Bind fbo

    // Create color texture (actual color; bright color; corruption amount)
    glGenTextures(3, colorTex);
    for (unsigned int i = 0; i < 3; i++)
    {
        BindTexture2D(colorTex[i]); // Bind colorTex

        if (i == 2) // cheapest data for corruption texture
            // glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, screenX, screenY, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenX, screenY, 0, GL_RED, GL_FLOAT, NULL);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenX, screenY, 0, GL_RGB, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 }; // MOVE THIS??
    glDrawBuffers(3, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Frame buffer creation failed :(" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind fbo

    // Create bloom blur buffers
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        BindTexture2D(pingpongBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenX / bloomResDivide, screenY / bloomResDivide, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
        );
    }

    windowCreated = true;
}

void SettingsManager::SaveSettings()
{
    EditorSaveManager::WritePropsFile("Settings\\options.set", [] {
        for (auto& pair : settings)
        {
            EditorSaveManager::WriteProp(pair.first, pair.second);
        }
    });
}

void SettingsManager::ApplySettings()
{
    Utility::globalVolume = stof(settings["globalVolume"]);
    Utility::musicVolume = stof(settings["musicVolume"]);
    Utility::gameSoundsVolume = stof(settings["gameSoundsVolume"]);
    MenuManager::uiSoundsVolume = stof(settings["uiVolume"]);
}

float SettingsManager::GetFloatSetting(std::string key)
{
    return stof(settings[key]);
}

int SettingsManager::GetIntSetting(std::string key)
{
    return stoi(settings[key]);
}

void SettingsManager::SetFloatSetting(std::string key, float val)
{
    settings[key] = std::to_string(val);
}

void SettingsManager::SetIntSetting(std::string key, int val)
{
    settings[key] = std::to_string(val);
}
