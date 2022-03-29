#include "RessourceManager.h"

std::map<std::string, Shader> RessourceManager::shaders = std::map<std::string, Shader>();
std::map<std::string, Texture> RessourceManager::textures = std::map<std::string, Texture>();

// TODO: optimisation : avoiding load twice?
Shader* RessourceManager::LoadShader(const char* vtex, const char* frag, std::string programName)
{
    shaders[programName] = Shader(vtex, frag);
    return &shaders[programName];
}

Texture* RessourceManager::LoadTexture(const char* file, std::string textureName)
{
    textures[textureName] = Texture(file);
    return &textures[textureName];
}

void RessourceManager::Clear()
{
    for (auto& shader : shaders)
        glDeleteProgram(shader.second.ID);

    for (auto& texture : textures)
        glDeleteProgram(texture.second.ID);
}
