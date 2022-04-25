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

Texture* RessourceManager::GetTexture(std::string file)
{
    std::replace(file.begin(), file.end(), '/', '\\');

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
    std::string file = std::string(file_param);
    std::replace(file.begin(), file.end(), '/', '\\');

    if (sounds.find(file) == sounds.end())
    {
        SoLoud::Wav* newWave = new SoLoud::Wav();
        std::string fullPath = "Sounds\\" + file;
        newWave->load(fullPath.c_str());

        sounds[file] = newWave;

        std::cout << "Loaded sound " << fullPath << std::endl;
    }

    return sounds[file];
}

void RessourceManager::Clear()
{
    for (auto& shader : shaders)
        glDeleteProgram(shader.second.ID);

    for (auto& texture : textures)
    {
        glDeleteProgram(texture.second->ID);
        delete texture.second;
    }

    for (auto& sound : sounds)
    {
        delete sound.second;
    }
}

