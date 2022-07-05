#include "RessourceManager.h"

#include <iostream>

std::map<std::string, Shader> RessourceManager::shaders = std::map<std::string, Shader>();
std::map<std::string, Texture*> RessourceManager::textures = std::map<std::string, Texture*>();
std::map<std::string, SoLoud::Wav*> RessourceManager::sounds = std::map<std::string, SoLoud::Wav*>();

// TODO: optimisation : avoiding load twice?
Shader* RessourceManager::LoadShader(const char* vtex, const char* frag, std::string programName)
{
    shaders[programName] = Shader(vtex, frag);
    return &shaders[programName];
}

Texture* RessourceManager::GetTexture(std::string file_param)
{
    // Change backslash and case to make sure the file is not already loaded
    std::string file = std::string(file_param);
    std::replace(file.begin(), file.end(), '/', '\\');
    std::transform(file.begin(), file.end(), file.begin(),
        [](char c) { return std::tolower(c); });

    if (textures.find(file) == textures.end())
    {
        textures[file] = new Texture("Images\\" + file);
    }

    if (textures[file]->loadingFailed)
    {
        return GetTexture("Engine\\notFound.png");
    }

    return textures[file];
}

SoLoud::Wav* RessourceManager::GetSound(std::string file_param)
{
    // Change backslash and case to make sure the file is not already loaded
    std::string file = std::string(file_param);
    std::replace(file.begin(), file.end(), '/', '\\');
    std::transform(file.begin(), file.end(), file.begin(),
        [](char c) { return std::tolower(c); });

    if (sounds.find(file) == sounds.end())
    {
        SoLoud::Wav* newWave = new SoLoud::Wav();
        std::string fullPath = "Sounds\\" + file;
        auto res = newWave->load(fullPath.c_str());
        
        if (res == SoLoud::FILE_LOAD_FAILED || res == SoLoud::FILE_NOT_FOUND)
        {
            std::cerr << "Failed to load sound " << fullPath << std::endl;
        }
        else
        {
            std::cout << "Loaded sound " << fullPath << std::endl;
        }

        sounds[file] = newWave;
    }

    return sounds[file];
}

void RessourceManager::Clear()
{
    for (auto& shader : shaders)
        glDeleteProgram(shader.second.ID);

    for (auto& texture : textures)
    {
        glDeleteTextures(1, &texture.second->ID);
        delete texture.second;
    }

    for (auto& sound : sounds)
    {
        delete sound.second;
    }
}

void RessourceManager::LoadBaseShaders()
{
    RessourceManager::LoadShader("Shaders\\SpriteVertexShader.glsl", "Shaders\\SpriteFragShader.glsl", "sprite");
    RessourceManager::LoadShader("Shaders\\SpriteVertexShader.glsl", "Shaders\\LaserFragShader.glsl", "laser");
    RessourceManager::LoadShader("Shaders\\RenderVtex.glsl", "Shaders\\RenderFrag.glsl", "screenShader");
    RessourceManager::LoadShader("Shaders\\RenderVtex.glsl", "Shaders\\GaussianBlur.glsl", "screenBlur");
    RessourceManager::LoadShader("Shaders\\LightmapperVtex.glsl", "Shaders\\LightmapperFrag.glsl", "lightmapper");
    RessourceManager::LoadShader("Shaders\\SpriteVertexShader.glsl", "Shaders\\AcidFrag.glsl", "acid");
    RessourceManager::LoadShader("Shaders\\SpriteVertexShader.glsl", "Shaders\\RadioFrag.glsl", "radioactivity");
    RessourceManager::LoadShader("Shaders\\SpriteVertexShader.glsl", "Shaders\\TeleportFrag.glsl", "teleport");
}

